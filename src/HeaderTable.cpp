#include <assert.h>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include "HeaderTable.h"

using namespace std;

template <typename T>
HeaderTable<T>::HeaderTable(const bool debug) : debug(debug) {}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::addNode(const shared_ptr<FPTreeNode<T>> node) {
	shared_ptr<FPTreeNode<T>> previous;
	typename map<T, HeaderEntry<T>>::iterator lb = this->headerTable.lower_bound(node->getValue());
	// Checks whether we are performing an add or an update
	if (lb != this->headerTable.cend() && !(this->headerTable.key_comp()(node->getValue(), lb->first))) {
		lb->second.totalFrequency += node->getFrequency();
		previous = node;
		swap(lb->second.node, previous);
	} else {
		BOOST_LOG_TRIVIAL(debug) << "Inserted new element in header table: " << *node;
		this->headerTable.try_emplace(lb, node->getValue(), node);
	}
	return previous;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> HeaderTable<T>::getNode(const T& item) const {
	typename map<T, HeaderEntry<T>>::const_iterator it = this->headerTable.find(item);
	return it != this->headerTable.cend() ? it->second.node : nullptr;
}

template <typename T>
shared_ptr<FPTreeNode<T>> HeaderTable<T>::removeNode(const T& item) {
	typename map<T, HeaderEntry<T>>::iterator it = this->headerTable.find(item);
	assert(it != this->headerTable.end());
	shared_ptr<FPTreeNode<T>> entry = it->second.node;
	this->headerTable.erase(it);
	return entry;
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
HeaderTable<T>::operator string() const {
	string out = "Key\t\t\t|\t\t\tValue(First - Total frequency)\n";
	for (const typename map<T, HeaderEntry<T>>::value_type& entry : this->headerTable) {
		out += to_string(entry.first) + "\t\t\t|\t\t\t" + (string) *(entry.second.node) + " - " + to_string(entry.second.totalFrequency) + "\n";
	}
	return out;
}