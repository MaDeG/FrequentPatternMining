#include <memory>
#include <iostream>
#include <cassert>
#include "FPTreeNode.h"
#include "HeaderTable.h"
#include "Params.h"

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
const weak_ptr<FPTreeNode<T>> FPTreeNode<T>::getNext() const {
	return this->next;
}

template <typename T>
const weak_ptr<FPTreeNode<T>> FPTreeNode<T>::getPrevious() const {
	return this->previous;
}

template <typename T>
const weak_ptr<FPTreeNode<T>> FPTreeNode<T>::getParent() const {
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
void FPTreeNode<T>::setNext(weak_ptr<FPTreeNode<T>> next) {
	this->next = next;
}

template <typename T>
void FPTreeNode<T>::setPrevious(weak_ptr<FPTreeNode<T>> previous) {
	this->previous = previous;
}

template<typename T>
void FPTreeNode<T>::addSequence(unique_ptr<list<T>> values, HeaderTable<T>& headerTable) {
	if (values->empty()) {
		return;
	}
	const T value = values->front();
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
		DEBUG(cout << "Create new node: " << *newNode;)
		childrenIt = this->children.insert(childrenIt, move(newNode));
		headerTable.addNode(*childrenIt);
	}
	// Add new item and/or update count in the header table
	headerTable.increaseFrequency(value, 1);
	(*childrenIt)->incrementFrequency();
	(*childrenIt)->addSequence(move(values), headerTable);
}

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getChildren(const T& item) const {
	auto it = lower_bound(this->children.begin(),
											  this->children.end(),
											  item,
											  [](const shared_ptr<FPTreeNode<T>>& child, const T& item) { return child->getValue() < item; });
	return it != this->children.end() && (*it)->getValue() == item ? (*it) : nullptr;
}

template <typename T>
FPTreeNode<T>::operator string() const {
	if (this->frequency < 0) {
		return "NULL";
	}
	ostringstream outStream;
	outStream << "'" << this->value << "' *" << this->frequency << " (" << (void *) this << ")";
	return outStream.str();
}

template <typename T>
FPTreeNode<T>::FPTreeNode(const FPTreeNode<T>& node) : value(node.value),
                                                       frequency(node.frequency),
                                                       children(FPTreeNode<T>::nodeComparator) {
	// Children, Next and Previous shall be initialized by FPTreeNode::deepCopy or FPTreeNode::getPrefixTree to ensure that only one copy per node is made
}

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeNode<T>::deepCopy(shared_ptr<FPTreeNode<T>> parent, HeaderTable<T>& newHeaderTable) const {
	DEBUG(cout << "Performing deep copy on " << *this;)
	// Set value and frequency
	shared_ptr<FPTreeNode<T>> newNode(new FPTreeNode<T>(*this));
	// Set parent
	newNode->parent = parent;
	if (this->frequency >= 0) {
		// Only if this is not the NULL root then set reference to next and previous and update header table so that we make a head insertion
		shared_ptr<FPTreeNode<T>> previous = newHeaderTable.addNode(newNode);
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
		// Only if this is not the NULL root then set reference to next and previous and update header table so that we make a head insertion
		shared_ptr<FPTreeNode<T>> previous = newHeaderTable.addNode(newNode);
		newNode->next = previous;
		newNode->previous.reset();
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