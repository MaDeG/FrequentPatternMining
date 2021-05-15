#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "FPTreeNode.h"

using namespace std;

template <typename T>
FPTreeNode<T>::FPTreeNode(const T &value, shared_ptr<FPTreeNode<T>> parent) : _frequency(0),
																																							_value(value),
																																							_parent(parent)
{ }

template <typename T>
std::shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getptr() {
	return this->shared_from_this();
}

template <typename T>
const T& FPTreeNode<T>::getValue() const {
	return this->_value;
}

template <typename T>
int FPTreeNode<T>::getFrequency() const {
	return this->_frequency;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getNext() const {
	return this->_next;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getParent() const {
	return this->_parent;
}

template <typename T>
const list<shared_ptr<FPTreeNode<T>>>& FPTreeNode<T>::getChildren() const {
	return this->_children;
}

template <typename T>
void FPTreeNode<T>::incrementFrequency() {
	this->_frequency++;
}

template<typename T>
void FPTreeNode<T>::setNext(shared_ptr<FPTreeNode<T>> next) {
	this->_next = next;
}

template<typename T>
void FPTreeNode<T>::addSequence(unique_ptr<list<T>> values, map<T, shared_ptr<FPTreeNode<T>>>& headerTable) {
	if (values->empty()) {
		return;
	}
	T &value = values->front();
	values->pop_front();
	// Binary search among the children
	typename list<shared_ptr<FPTreeNode<T>>>::iterator childrenIt = lower_bound(this->_children.begin(),
	                                                                            this->_children.end(),
	                                                                            value,
	                                                                            [](shared_ptr<FPTreeNode<T>> a, const int& b) { return a->getValue() < b; });
	if (childrenIt == this->_children.end() || (*childrenIt)->getValue() != value) {
		// Need to create a new node
		if (childrenIt != this->_children.begin() && childrenIt != this->_children.end()) {
			// Place the element just before its lower bound
			childrenIt--;
		}
		shared_ptr<FPTreeNode<T>> newNode = make_shared<FPTreeNode<T>>(value, this->getptr());
		BOOST_LOG_TRIVIAL(debug) << "Create new node: " << *newNode;
		childrenIt = this->_children.insert(childrenIt, newNode);
		// Update header table
		typename map<T, shared_ptr<FPTreeNode<T>>>::iterator headerIt = headerTable.lower_bound(value);
		// Checks whether we are performing an add or an update
		if (headerIt != headerTable.end() && !(headerTable.key_comp()(value, headerIt->first))) {
			newNode->setNext(headerIt->second);
			headerIt->second = newNode;
		} else {
			BOOST_LOG_TRIVIAL(debug) << "Insert new element in header table: " << value << " -> " << *newNode;
			headerTable.insert(headerIt, pair<T, shared_ptr<FPTreeNode<T>>>(value, newNode));
		}
	}
	(*childrenIt)->incrementFrequency();
	(*childrenIt)->addSequence(move(values), headerTable);
}

template <typename T>
FPTreeNode<T>::operator string() const {
	return this->_frequency >= 0 ? "'" + to_string(this->_value) + "' *" + to_string(this->_frequency) : "NULL";
}

template <typename T>
FPTreeNode<T>::FPTreeNode(const FPTreeNode<T>& node) : _value(node._value),
																											 _frequency(node._frequency) {
	// Children and Next shall be initialized by FPTreeNode::deepCopy to ensure that only one copy per node is made
}

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeNode<T>::deepCopy(shared_ptr<FPTreeNode<T>> parent, map<T, shared_ptr<FPTreeNode<T>>>& newHeaderTable) const {
	// Set value and frequency
	shared_ptr<FPTreeNode<T>> newNode(new FPTreeNode<T>(*this));
	// Set parent
	newNode->_parent = parent;
	if (this->_frequency >= 0) {
		// Only if this is not the NULL root then set reference to next and update header table
		typename map<T, shared_ptr<FPTreeNode<T>>>::iterator headerIt = newHeaderTable.lower_bound(newNode->getValue());
		if (headerIt != newHeaderTable.end() && !(newHeaderTable.key_comp()(newNode->getValue(), headerIt->first))) {
			BOOST_LOG_TRIVIAL(debug) << "Deep copy - Updated header table reference to item " << newNode->getValue() << " from " << *(headerIt->second) << " to " << *newNode;
			newNode->setNext(headerIt->second);
			headerIt->second = newNode;
		} else {
			BOOST_LOG_TRIVIAL(debug) << "Deep copy - Insert new element in header table: " << newNode->getValue() << " -> " << *newNode;
			newHeaderTable.insert(headerIt, pair<T, shared_ptr<FPTreeNode<T>>>(newNode->getValue(), newNode));
		}
	}
	// Create new children
	for (shared_ptr<FPTreeNode<T>> child : this->_children) {
		newNode->_children.push_back(child.get()->deepCopy(newNode, newHeaderTable));
	}
	return newNode;
}