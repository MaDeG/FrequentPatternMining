#include <cassert>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "HeaderTable.h"
#include "Params.h"

using namespace std;

template <typename T>
HeaderTable<T>::HeaderTable() {
	omp_init_lock(&this->lock);
}

template <typename T>
HeaderTable<T>::~HeaderTable() {
	omp_destroy_lock(&this->lock);
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::addNode(const shared_ptr<FPTreeNode<T>> node) {
	shared_ptr<FPTreeNode<T>> previous;
	omp_set_lock(&this->lock);
	typename map<T, HeaderEntry<T>>::iterator lb = this->headerTable.lower_bound(node->getValue());
	// Checks whether we are performing an add or an update
	if (lb != this->headerTable.cend() && !(this->headerTable.key_comp()(node->getValue(), lb->first))) {
		lb->second.totalFrequency += node->getFrequency();
		previous = node;
		// If true the node has already been inserted and its insertion would create a cycle
		assert(lb->second.node != previous);
		swap(lb->second.node, previous);
		// Update next and previous fields in the nodes
		previous->setPrevious(node);
		node->setPrevious(weak_ptr<FPTreeNode<T>>());
		node->setNext(previous);
	} else {
		DEBUG(cout << "Inserted new element in header table: " << (string) *node;)
		this->headerTable.try_emplace(lb, node->getValue(), node);
		node->setNext(weak_ptr<FPTreeNode<T>>());
		node->setPrevious(weak_ptr<FPTreeNode<T>>());
	}
	omp_unset_lock(&this->lock);
	return previous;
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::getNode(const T& item) const {
	omp_set_lock(const_cast<omp_lock_t*> (&this->lock));
	typename map<T, HeaderEntry<T>>::const_iterator it = this->headerTable.find(item);
	shared_ptr<FPTreeNode<T>> node = it != this->headerTable.cend() ? it->second.node : nullptr;
	omp_unset_lock(const_cast<omp_lock_t*> (&this->lock));
	return move(node);
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::removeNode(const T& item) {
	omp_set_lock(&this->lock);
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(item);
	assert(it != this->headerTable.end());
	shared_ptr<FPTreeNode<T>> entry = it->second.node;
	this->headerTable.erase(it);
	omp_unset_lock(&this->lock);
	return move(entry);
}

template <typename T>
bool HeaderTable<T>::removeNode(shared_ptr<FPTreeNode<T>> node) {
	omp_set_lock(&this->lock);
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(node->getValue());
	if (it != this->headerTable.end()) {
		assert(it->second.node);
		if (node->getPrevious().expired()) {
			assert(node == it->second.node);
			it->second.node = node->getNext().lock();
		}
		it->second.totalFrequency -= node->getFrequency();
		// Do not delete header table entries here since somebody may be iterating over them
		assert(it->second.totalFrequency >= 0);
		assert(it->second.node || it->second.totalFrequency == 0);
	}
	if (!node->getPrevious().expired()) {
		node->getPrevious().lock()->setNext(node->getNext());
		// Modify previous of the next item only if the deleted one is not the head of the list
		if (!node->getNext().expired()) {
			node->getNext().lock()->setPrevious(node->getPrevious());
		}
	}
	// Needed to be able to maintain a valid list of nodes whose heads are returned by pruneInfrequent
	node->getParent().reset();
	omp_unset_lock(&this->lock);
	return true;
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::resetEntry(const T& item) {
	omp_set_lock(&this->lock);
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(item);
	if (it == this->headerTable.end()) {
		omp_unset_lock(&this->lock);
		DEBUG(cout << "The item " << item << " is not in the header table";)
		return nullptr;
	}
	shared_ptr<FPTreeNode<T>> entry = it->second.node;
	it->second.totalFrequency = 0;
	it->second.node.reset();
	omp_unset_lock(&this->lock);
	return move(entry);
}

template <typename T>
void HeaderTable<T>::increaseFrequency(const T& item, const int addend) {
	// Addend can be 0 if a parent of this item has been chosen as prefix previously
	assert(addend >= 0);
	omp_set_lock(&this->lock);
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(item);
	assert(it != this->headerTable.cend());
	assert(it->second.totalFrequency >= 0);
	it->second.totalFrequency += addend;
	omp_unset_lock(&this->lock);
}

template <typename T>
list<shared_ptr<FPTreeNode<T>>> HeaderTable<T>::pruneInfrequent(int minSupportCount) {
	// Making them weak_ptr it is then possible to delete only the nodes that are not expired, hence still referenced by a parent
	list<shared_ptr<FPTreeNode<T>>> deleted;
	DEBUG(cout << "Header table size before pruning: " << this->headerTable.size() << ", minimum support count: " << minSupportCount)
	omp_set_lock(&this->lock);
	erase_if(this->headerTable, [minSupportCount, &deleted](const pair<T, HeaderEntry<T>>& i) {
		if (i.second.totalFrequency < minSupportCount) {
			deleted.push_back(move(i.second.node));
			return true;
		}
		return false;
	});
	omp_unset_lock(&this->lock);
	DEBUG(cout << "Header table size after pruning: " << this->headerTable.size())
	return deleted; // RVO
}

template <typename T>
vector<T> HeaderTable<T>::getItems() const {
	vector<T> result;
	omp_set_lock(const_cast<omp_lock_t*> (&this->lock));
	for (const auto& [item, _] : this->headerTable) {
		result.push_back(item);
	}
	omp_unset_lock(const_cast<omp_lock_t*> (&this->lock));
	return result; // RVO
}

template <typename T>
bool HeaderTable<T>::empty() const {
	return this->headerTable.empty();
}

template <typename T>
HeaderTable<T>::operator string() const {
	ostringstream outStream;
	outStream << setw(10) << "Key" << " | " << setw(38) << "Value(First - Total frequency)" << " | " << setw(15) << "Chain" << endl;
	omp_set_lock(const_cast<omp_lock_t*> (&this->lock));
	for (const typename map<T, HeaderEntry<T>>::value_type& entry : this->headerTable) {
		outStream << setw(10) << entry.first << " | ";
		if (!entry.second.node) {
			outStream << setw(37) << "NULL" << " |" << endl;
			continue;
		}
		outStream << setw(30) << *(entry.second.node) << " - " << setw(4) << entry.second.totalFrequency << " | ";
		for (shared_ptr<FPTreeNode<T>> node = entry.second.node->getNext().lock(); node; node = node->getNext().lock()) {
			outStream << "-> " << *node;
		}
		outStream << endl;
	}
	omp_unset_lock(const_cast<omp_lock_t*> (&this->lock));
	return outStream.str();
}