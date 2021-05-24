#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "FPTreeNode.h"
#include "HeaderTable.h"

using namespace std;

template <typename T>
FPTreeNode<T>::FPTreeNode(const T& value, shared_ptr<FPTreeNode<T>> parent) : frequency(0),
                                                                              value(value),
                                                                              parent(parent),
                                                                              children(FPTreeNode<T>::nodeComparator)
{ }

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getptr() {
	return this->shared_from_this();
}

template <typename T>
const T& FPTreeNode<T>::getValue() const {
	return this->value;
}

template <typename T>
int FPTreeNode<T>::getFrequency() const {
	return this->frequency;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getNext() const {
	return this->next;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getParent() const {
	return this->parent;
}

template <typename T>
void FPTreeNode<T>::incrementFrequency() {
	this->frequency++;
}

template <typename T>
void FPTreeNode<T>::incrementFrequency(const int addend) {
	assert(addend >= 0);
	this->frequency += addend;
}

template<typename T>
void FPTreeNode<T>::setNext(shared_ptr<FPTreeNode<T>> next) {
	this->next = next;
}

template<typename T>
void FPTreeNode<T>::addSequence(unique_ptr<list<T>> values, HeaderTable<T>& headerTable) {
	if (values->empty()) {
		return;
	}
	T &value = values->front();
	values->pop_front();
	// Binary search among the children
	typename set<shared_ptr<FPTreeNode<T>>>::iterator childrenIt = lower_bound(this->children.begin(),
																																						 this->children.end(),
																																						 value,
																																						 [](const shared_ptr<FPTreeNode<T>>& a, const int& b) { return a->getValue() < b; });
	if (childrenIt == this->children.cend() || (*childrenIt)->getValue() != value) {
		// The iterator needs to point to the item that follows value in order to have an optimized insertion via hint
		assert(childrenIt == this->children.cend() || (*childrenIt)->value > value);
		// Need to create a new node
		shared_ptr<FPTreeNode<T>> newNode = make_shared<FPTreeNode<T>>(value, this->getptr());
		BOOST_LOG_TRIVIAL(debug) << "Create new node: " << *newNode;
		childrenIt = this->children.insert(childrenIt, move(newNode));
	}
	// Add new item and/or update count in the header table
	headerTable.addNode(*childrenIt);
	(*childrenIt)->incrementFrequency();
	(*childrenIt)->addSequence(move(values), headerTable);
}

template <typename T>
FPTreeNode<T>::operator string() const {
	return this->frequency >= 0 ? "'" + to_string(this->value) + "' *" + to_string(this->frequency) : "NULL";
}

template <typename T>
FPTreeNode<T>::FPTreeNode(const FPTreeNode<T>& node) : value(node.value),
                                                       frequency(node.frequency),
                                                       children(FPTreeNode<T>::nodeComparator) {
	// Children and Next shall be initialized by FPTreeNode::deepCopy or FPTreeNode::getPrefixTree to ensure that only one copy per node is made
}

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeNode<T>::deepCopy(shared_ptr<FPTreeNode<T>> parent, HeaderTable<T>& newHeaderTable) const {
	// Set value and frequency
	shared_ptr<FPTreeNode<T>> newNode(new FPTreeNode<T>(*this));
	// Set parent
	newNode->parent = parent;
	if (this->frequency >= 0) {
		// Only if this is not the NULL root then set reference to next and update header table
		shared_ptr<FPTreeNode<T>> previous = newHeaderTable.addNode(newNode);
		newNode->next = previous;
	}
	// Create new children
	for (shared_ptr<FPTreeNode<T>> child : this->children) {
		newNode->children.insert(child.get()->deepCopy(newNode, newHeaderTable));
	}
	return newNode;
}

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getPrefixTree(shared_ptr<FPTreeNode<T>> parent, HeaderTable<T>& newHeaderTable, const T& item) const {
	// Set value and frequency
	shared_ptr<FPTreeNode<T>> newNode(new FPTreeNode<T>(*this));
	if (this->value != item && this->frequency >= 0) {
		// We want to set the frequency only of the prefix that we are going to create since the other ones are going to be recomputed
		newNode->frequency = 0;
	}
	// Set parent
	newNode->parent = parent;
	if (this->frequency >= 0) {
		// Only if this is not the NULL root then set reference to next and update header table
		shared_ptr<FPTreeNode<T>> previous = newHeaderTable.addNode(newNode);
		newNode->next = previous;
	}
	// Create new children
	for (shared_ptr<FPTreeNode<T>> child : this->children) {
		newNode->children.insert(child.get()->getPrefixTree(newNode, newHeaderTable, item));
	}
	return newNode;
}

template <typename T>
bool FPTreeNode<T>::nodeComparator(const shared_ptr<FPTreeNode<T>>& a, const shared_ptr<FPTreeNode<T>>& b) {
	return a->value < b->value;
}