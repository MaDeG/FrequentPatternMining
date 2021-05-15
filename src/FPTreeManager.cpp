#include "FPTreeManager.h"

using namespace std;

template <typename T>
FPTreeManager<T>::FPTreeManager(FileOrderedReader& reader, double supportFraction) : _root(make_shared<FPTreeNode<T>>(-1, shared_ptr<FPTreeNode<T>>())) {
	// Marks the root node as such
	this->_root->_frequency = -1;
	this->generateFPTree(reader, supportFraction);
}

template <typename T>
FPTreeManager<T>::FPTreeManager(const FPTreeManager<T>& manager) {
	this->_root = manager._root->deepCopy(shared_ptr<FPTreeNode<T>>(), this->_headerTable);
}

template <typename T>
void FPTreeManager<T>::generateFPTree(FileOrderedReader& reader, double supportFraction) {
	int itemsetCount = 0;
	unique_ptr<list<T>> items;
	while ((items = reader.getNextOrderedTransaction())) {
		this->_root->addSequence(move(items), this->_headerTable);
		itemsetCount++;
	}
	// Knowing the number of the input itemsets I can determine the required count to be frequent given the required supportFraction percentage
	this->_supportCount = itemsetCount * supportFraction;
}

template <typename T>
const shared_ptr<FPTreeNode<T>> FPTreeManager<T>::getRoot() const {
	return this->_root;
}

template <typename T>
const map<T, shared_ptr<FPTreeNode<T>>>& FPTreeManager<T>::getHeaderTable() const {
	return this->_headerTable;
}

template <typename T>
const int FPTreeManager<T>::getSupportCount() const {
	return this->_supportCount;
}