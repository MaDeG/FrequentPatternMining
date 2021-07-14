#include <deque>
#include <iostream>
#include <iomanip>
#include <cassert>
#include "FPTreeManager.h"
#include "Params.h"
#include "Utils.cpp"

using namespace std;

template <typename T>
FPTreeManager<T>::FPTreeManager(FileOrderedReader& reader, const double supportFraction) : FPTreeManager() {
	this->generateFPTree(reader, supportFraction);
}

template <typename T>
FPTreeManager<T>::FPTreeManager(const FPTreeManager<T>& manager) : supportCount(manager.supportCount) {
	DEBUG(cout << "Deep copy of FPTreeManager requested");
	this->root = manager.root->deepCopy(nullptr, this->headerTable);
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
	unique_ptr<FPTreeManager<T>> newManager(new FPTreeManager<T>());
	newManager->supportCount = this->supportCount;
	newManager->root = this->root->getPrefixTree(nullptr, newManager->headerTable, item);
	return move(newManager);
}

template <typename T>
void FPTreeManager<T>::pruneInfrequent() {
	for (typename map<T, HeaderEntry<T>>::const_iterator it = this->headerTable.cbegin(); it != this->headerTable.cend(); it++) {
		if (it->second.getTotalFrequency() < this->supportCount) {
			DEBUG(cout << "Deleting element " << *(it->second.getNode());)
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
			outStream << setw(25) << "| ";
		}
		if (node->getFrequency() >= 0) {
			outStream << setw(25) << "|-";
		}
		outStream << setfill('-') << setw(25) << *node << setfill(' ') << endl;
		for (shared_ptr<FPTreeNode<int>> i : node->children) {
			nodes.push_front(i.get());
			levels.push_front(level + 1);
		}
	}
	return outStream.str();
}

template <typename T>
FPTreeManager<T>::FPTreeManager() : root(make_shared<FPTreeNode<T>>(-1, nullptr)) {
	// Marks the root node as such
	this->root->frequency = -1;
}

template <typename T>
void FPTreeManager<T>::generateFPTree(FileOrderedReader& reader, double supportFraction) {
	int itemsetCount = 0;
	list<T> items;
	//HeaderTable<T>& headerTableAlias = this->headerTable;
	#pragma omp parallel shared(reader, itemsetCount, items) default(none)
	#pragma omp single
	do {
		#pragma omp task shared(reader, itemsetCount) firstprivate(items) default(none)
		{
			items = reader.getNextOrderedTransaction();
			if (!items.empty()) {
				this->root->addSequence(items, this->headerTable);
				#pragma omp atomic
				itemsetCount++;
			}
		}
	} while(!reader.isEOF());
	#pragma omp taskwait
	// Knowing the number of the input itemsets I can determine the required count to be frequent given the required supportFraction percentage
	this->supportCount = itemsetCount * supportFraction;
	DEBUG(cout << "Total itemsets parsed: " << itemsetCount << ", support count: " << this->supportCount)
}

template <typename T>
void FPTreeManager<T>::deleteItem(shared_ptr<FPTreeNode<T>> node) {
	if (Params::parallelDelete) {
		this->deleteItemParallel(node);
	} else {
		this->deleteItemSequential(node);
	}
}

template <typename T>
void FPTreeManager<T>::deleteItemParallel(shared_ptr<FPTreeNode<T>> node) {
	// Assume that itemsets with duplicate items do not exists, hence every path from the root to a leaf contains unique items
	assert(node->previous.expired());
	this->headerTable.resetEntry(node->value);
	vector<shared_ptr<FPTreeNode<T>>> nodes = listToVector(node);
	#pragma omp parallel for schedule(dynamic) shared(nodes, cout) default(none) if(Params::parallelDelete) //if(nodes.size() > 100) //num_threads(Params::nThreads)
	//#pragma omp parallel shared(nodes, cout) default(none)
	//#pragma omp single
	//#pragma omp taskloop shared(nodes, cout) default(none) if(Params::parallelDelete) //grainsize(1) //if(nodes.size() > 100) //num_tasks(nodes.size() / Params::nThreads > 200 ? Params::nThreads : 1)
	for (typename vector<shared_ptr<FPTreeNode<T>>>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		shared_ptr<FPTreeNode<T>>& node = *it;
		DEBUG(cout << "Removing item " << *node << " from:" << endl << (string) *this)
		DEBUG(cout << "Header table for removing item: " << *node << endl << (string) this->headerTable)
		assert(!node->parent.expired());
		// Update children's parent
		for (const shared_ptr<FPTreeNode<T>>& nephew : node->children) {
			DEBUG(cout << "Set parent of " << *nephew << " to " << *(node->parent.lock()))
			nephew->parent = node->parent;
		}
		shared_ptr<FPTreeNode<T>> parent = node->parent.lock();
		parent->children.erase(node);
		this->mergeChildren(node, parent);
		DEBUG(cout << "Result:" << endl << (string) *this)
		DEBUG(cout << "Result header table: " << endl << (string) this->headerTable)
		assert(!node->parent.expired());
	}
}

template <typename T>
void FPTreeManager<T>::deleteItemSequential(shared_ptr<FPTreeNode<T>> node) {
	// Assume that itemsets with duplicate items do not exists, hence every path from the root to a leaf contains unique items
	assert(node->previous.expired());
	this->headerTable.resetEntry(node->value);
	for (; node; node = node->getNext().lock()) {
		DEBUG(cout << "Removing item " << *node << " from:" << endl << (string) *this)
		DEBUG(cout << "Header table for removing item: " << *node << endl << (string) this->headerTable)
		assert(!node->parent.expired());
		// Update children's parent
		for (const shared_ptr<FPTreeNode<T>>& nephew : node->children) {
			DEBUG(cout << "Set parent of " << *nephew << " to " << *(node->parent.lock()))
			nephew->parent = node->parent;
		}
		shared_ptr<FPTreeNode<T>> parent = node->parent.lock();
		parent->children.erase(node);
		this->mergeChildren(node, parent);
		DEBUG(cout << "Result:" << endl << (string) *this)
		DEBUG(cout << "Result header table: " << endl << (string) this->headerTable)
		assert(!node->parent.expired());
	}
}

template <typename T>
void FPTreeManager<T>::mergeChildren(shared_ptr<FPTreeNode<T>> node, shared_ptr<FPTreeNode<T>> parent) {
	assert(node && parent && !node->parent.expired());
	// Adopt all the nephews, this works with the assumption that itemsets do NOT have repeated items
	parent->children.merge(node->children);
	DEBUG(for (shared_ptr<FPTreeNode<T>> child : parent->children) {
					assert(!child->parent.expired() && child->parent.lock() == parent);
				}
				cout << "The children of node " << *parent << " have valid pointers to their father and vice-versa"
	)
	if (!node->children.empty()) {
		// Parent's and node's children has some common items, then the common item's children must be merged
		for (shared_ptr <FPTreeNode<T>> child : node->children) {
			shared_ptr<FPTreeNode<T>> uncle = parent->getChildren(child->value);
			assert(child->value == uncle->value);
			// Transfer frequency from child to uncle
			uncle->incrementFrequency(child->frequency);
			child->frequency = 0;
			// Remove child from the Header Table and disconnect it from its next and previous
			this->headerTable.removeNode(child);
			assert(!uncle->previous.expired() || this->headerTable.getNode(uncle->value) == uncle);
			// Since a merge is needed, adopt all the children of child by uncle
			for (shared_ptr<FPTreeNode<T>> nephew : child->children) {
				nephew->parent = uncle;
			}
			this->mergeChildren(child, uncle);
		}
	}
}