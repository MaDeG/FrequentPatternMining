#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "HeaderTable.h"

using namespace std;

template <typename T>
HeaderTable<T>::HeaderTable(const bool debug) : debug(debug)
{ }

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::addNode(const shared_ptr<FPTreeNode<T>> node) {
	shared_ptr<FPTreeNode<T>> previous;
	typename map<T, HeaderEntry<T>>::iterator lb = this->headerTable.lower_bound(node->getValue());
	// Checks whether we are performing an add or an update
	if (lb != this->headerTable.cend() && !(this->headerTable.key_comp()(node->getValue(), lb->first))) {
		lb->second.totalFrequency += node->getFrequency();
		previous = node;
		swap(lb->second.node, previous);
		// Update next and previous fields in the nodes
		previous->setPrevious(node);
		node->setPrevious(weak_ptr<FPTreeNode<T>>());
		node->setNext(previous);
	} else {
		cout << "Inserted new element in header table: " << (string) *node << endl;
		this->headerTable.try_emplace(lb, node->getValue(), node);
		node->setNext(weak_ptr<FPTreeNode<T>>());
		node->setPrevious(weak_ptr<FPTreeNode<T>>());
	}
	return previous;
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::getNode(const T& item) const {
	typename map<T, HeaderEntry<T>>::const_iterator it = this->headerTable.find(item);
	return it != this->headerTable.cend() ? it->second.node : nullptr;
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::removeNode(const T& item) {
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(item);
	assert(it != this->headerTable.end());
	shared_ptr<FPTreeNode<T>> entry = it->second.node;
	this->headerTable.erase(it);
	return move(entry);
}

template <typename T>
void HeaderTable<T>::removeNode(shared_ptr<FPTreeNode<T>> node) {
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(node->getValue());
	assert(it != this->headerTable.end());
	assert(it->second.node);
	if (node->getPrevious().expired()) {
		assert(node == it->second.node);
		it->second.node = node->getNext().lock();
	}
	it->second.totalFrequency -= node->getFrequency();
	if (!node->getPrevious().expired()) {
		node->getPrevious().lock()->setNext(node->getNext());
	}
	if (!node->getNext().expired()) {
		node->getNext().lock()->setPrevious(node->getPrevious());
	}
}

template <typename T>
void HeaderTable<T>::increaseFrequency(const T& item, const int addend) {
	// Addend can be 0 if a parent of this item has been chosen as prefix previously
	assert(addend >= 0);
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(item);
	assert(it != this->headerTable.cend());
	assert(it->second.totalFrequency >= 0);
	it->second.totalFrequency += addend;
}

template <typename T>
void HeaderTable<T>::pruneInfrequent(int minSupportCount) {
	if (debug) {
		BOOST_LOG_TRIVIAL(debug) << "Header table size before pruning: " << this->headerTable.size() << ", minimum support count: " << minSupportCount;
	}
	erase_if(this->headerTable, [minSupportCount](const pair<T, HeaderEntry<T>>& i) {
		return i.second.totalFrequency < minSupportCount;
	});
	if (debug) {
		BOOST_LOG_TRIVIAL(debug) << "Header table size after pruning: " << this->headerTable.size();
	}
}

template <typename T>
typename map<T, HeaderEntry<T>>::const_iterator HeaderTable<T>::cbegin() {
	return this->headerTable.cbegin();
}

template <typename T>
typename map<T, HeaderEntry<T>>::const_iterator HeaderTable<T>::cend() {
	return this->headerTable.cend();
}

template <typename T>
bool HeaderTable<T>::empty() const {
	return this->headerTable.empty();
}

template <typename T>
HeaderTable<T>::operator string() const {
	ostringstream outStream;
	outStream << setw(10) << "Key" << " | " << setw(38) << "Value(First - Total frequency)" << " | " << setw(15) << "Chain" << endl;
	for (const typename map<T, HeaderEntry<T>>::value_type& entry : this->headerTable) {
		assert(entry.second.node);
		outStream << setw(10) << entry.first << " |" << setw(35) << *(entry.second.node) << " - " << entry.second.totalFrequency << " | ";
		for (shared_ptr<FPTreeNode<T>> node = entry.second.node->getNext().lock(); node; node = node->getNext().lock()) {
			outStream << "-> " << *node;
		}
		outStream << endl;
	}
	return outStream.str();
}