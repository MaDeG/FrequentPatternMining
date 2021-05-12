#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "FPTreeNode.h"

using namespace std;

template <typename T>
FPTreeNode<T>::FPTreeNode(const T &value) : _frequency(0), _value(value) {}

template <typename T>
const T& FPTreeNode<T>::getValue() const {
	return this->_value;
}

template <typename T>
int FPTreeNode<T>::getFrequency() const {
	return this->_frequency;
}

template <typename T>
const std::shared_ptr<FPTreeNode<T>> FPTreeNode<T>::getNext() const {
	return this->_next;
}

template <typename T>
const std::list<std::shared_ptr<FPTreeNode<T>>>& FPTreeNode<T>::getChildren() const {
	return this->_children;
}

template <typename T>
void FPTreeNode<T>::incrementFrequency() {
	this->_frequency++;
}

template<typename T>
void FPTreeNode<T>::setNext(std::shared_ptr<FPTreeNode<T>> next) {
	this->_next = next;
}

template<typename T>
void FPTreeNode<T>::addSequence(unique_ptr<list<T>> values, std::map<T, shared_ptr<FPTreeNode<T>>>& headerTable) {
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
		shared_ptr<FPTreeNode<T>> newNode = make_shared<FPTreeNode<T>>(value);
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
	return "'" + to_string(this->_value) + "' *" + to_string(this->_frequency);
}