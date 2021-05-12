#include "FPTreeManager.h"

using namespace std;

template <typename T>
FPTreeManager<T>::FPTreeManager(FileOrderedReader &fileOrderedReader) : _root(0),
																																				_fileOrderedReader(fileOrderedReader) {
	// Marks the root node as such
	this->_root._frequency = -1;
	generateFPTree();
}

template <typename T>
void FPTreeManager<T>::generateFPTree() {
	unique_ptr<list<T>> items = this->_fileOrderedReader.getNextOrderedTransaction();
	while (items) {
		this->_root.addSequence(move(items), this->_headerTable);
		items = this->_fileOrderedReader.getNextOrderedTransaction();
	}
}

template <typename T>
const FPTreeNode<T>& FPTreeManager<T>::getRoot() const {
	return this->_root;
}

template <typename T>
const std::map<T, std::shared_ptr<FPTreeNode<T>>>& FPTreeManager<T>::getHeaderTable() {
	return this->_headerTable;
}