#include <boost/log/trivial.hpp>
#include <deque>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "FPTreeManager.h"

using namespace std;

template <typename T>
FPTreeManager<T>::FPTreeManager(FileOrderedReader& reader, const double supportFraction, const bool debug) : FPTreeManager(debug) {
	this->generateFPTree(reader, supportFraction);
}

template <typename T>
FPTreeManager<T>::FPTreeManager(const FPTreeManager<T>& manager) : debug(manager.debug),
																																	 headerTable(manager.debug),
																																	 supportCount(manager.supportCount) {
	//BOOST_LOG_TRIVIAL(debug) << "Deep copy source FPTree: " << endl << manager;
	this->root = manager.root->deepCopy(nullptr, this->headerTable);
	//BOOST_LOG_TRIVIAL(debug) << "Deep copy destination FPTree: " << endl << *this;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> FPTreeManager<T>::getRoot() const {
	return this->root;
}

template <typename T>
const HeaderTable<T>& FPTreeManager<T>::getHeaderTable() const {
	return this->headerTable;
}

template <typename T>
const int FPTreeManager<T>::getSupportCount() const {
	return this->supportCount;
}

template <typename T>
unique_ptr<FPTreeManager<T>> FPTreeManager<T>::getPrefixTree(const T& item) const {
	unique_ptr<FPTreeManager<T>> newManager(new FPTreeManager<T>(this->debug));
	newManager->supportCount = this->supportCount;
	newManager->root = this->root->getPrefixTree(nullptr, newManager->headerTable, item);
	return move(newManager);
}

template <typename T>
void FPTreeManager<T>::pruneInfrequent() {
	for (typename map<T, HeaderEntry<T>>::const_iterator it = this->headerTable.cbegin(); it != this->headerTable.cend(); it++) {
		if (it->second.getTotalFrequency() < this->supportCount) {
			this->deleteItem(it->second.getNode());
		}
	}
	this->headerTable.pruneInfrequent(this->supportCount);
}

template <typename T>
shared_ptr<FPTreeNode<T>> FPTreeManager<T>::removeItem(const T& item) {
	shared_ptr<FPTreeNode<T>> first = this->headerTable.removeNode(item);
	assert(first);
	this->deleteItem(first);
	return move(first);
}

template <typename T>
FPTreeManager<T>::operator string() const {
	ostringstream outStream;
	deque<const FPTreeNode<int>*> nodes;
	deque<int> levels;
	nodes.push_front(root.get());
	levels.push_front(0);
	while(!nodes.empty()) {
		assert(nodes.size() == levels.size());
		const FPTreeNode<int>* node = nodes.front();
		const int level = levels.front();
		nodes.pop_front();
		levels.pop_front();
		for (int i = 0; i < level - 1; i++) {
			outStream << setw(10) << "| ";
		}
		if (node->getFrequency() >= 0) {
			outStream << setw(10) << "|-";
		}
		outStream << setfill('-') << setw(10) << *node << setfill(' ') << endl;
		for (shared_ptr<FPTreeNode<int>> i : node->children) {
			nodes.push_front(i.get());
			levels.push_front(level + 1);
		}
	}
	return outStream.str();
}

template <typename T>
FPTreeManager<T>::FPTreeManager(const bool debug) : root(make_shared<FPTreeNode<T>>(-1, nullptr)),
																										debug(debug),
																										headerTable(debug) {
	// Marks the root node as such
	this->root->frequency = -1;
}

template <typename T>
void FPTreeManager<T>::generateFPTree(FileOrderedReader& reader, double supportFraction) {
	int itemsetCount = 0;
	unique_ptr<list<T>> items;
	while ((items = reader.getNextOrderedTransaction())) {
		this->root->addSequence(move(items), this->headerTable);
		itemsetCount++;
	}
	// Knowing the number of the input itemsets I can determine the required count to be frequent given the required supportFraction percentage
	this->supportCount = itemsetCount * supportFraction;
	BOOST_LOG_TRIVIAL(debug) << "Total itemsets parsed: " << itemsetCount << ", support count: " << this->supportCount;
}

template <typename T>
void FPTreeManager<T>::deleteItem(shared_ptr<FPTreeNode<T>> node) {
	// Assume that two itemsets with duplicate items do not exists
	for (; node; node = node->getNext()) {
		assert(!node->parent.expired());
		// Update children's parent
		for (shared_ptr<FPTreeNode<T>> nephew : node->children) {
			nephew->parent = node->parent;
		}
		shared_ptr<FPTreeNode<T>> parent = node->parent.lock();
		parent->children.erase(node);
		// Adopt all the nephews, this works with the assumption that itemsets do NOT have repeated items
		parent->children.merge(node->children);
	}
}