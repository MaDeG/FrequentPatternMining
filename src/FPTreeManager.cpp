#include <deque>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>
#include "FPTreeManager.h"
#include "Log.h"

using namespace std;

template <typename T>
FPTreeManager<T>::FPTreeManager(FileOrderedReader& reader, const double supportFraction, const bool debug) : FPTreeManager(debug) {
	this->generateFPTree(reader, supportFraction);
}

template <typename T>
FPTreeManager<T>::FPTreeManager(const FPTreeManager<T>& manager) : debug(manager.debug),
																																	 headerTable(manager.debug),
																																	 supportCount(manager.supportCount) {
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
	unique_ptr<FPTreeManager<T>> newManager(new FPTreeManager<T>(this->debug));
	newManager->supportCount = this->supportCount;
	newManager->root = this->root->getPrefixTree(nullptr, newManager->headerTable, item);
	return move(newManager);
}

template <typename T>
void FPTreeManager<T>::pruneInfrequent() {
	for (typename map<T, HeaderEntry<T>>::const_iterator it = this->headerTable.cbegin(); it != this->headerTable.cend(); it++) {
		if (it->second.getTotalFrequency() < this->supportCount) {
			DEBUG(cout << "Deleting element " << *(it->second.getNode());)
			//DEBUG(cout << "Deleting from: " << endl << (string) *this;)
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
	DEBUG(cout << "Total itemsets parsed: " << itemsetCount << ", support count: " << this->supportCount;)
}

template <typename T>
void FPTreeManager<T>::deleteItem(shared_ptr<FPTreeNode<T>> node) {
	// Assume that itemsets with duplicate items do not exists
	for (; node; node = node->getNext().lock()) {
		DEBUG(cout << "Removing item " << (string) *node << " from:" << endl << (string) *this;)
		DEBUG(cout << "Header table: " << endl << (string) this->headerTable;)
		assert(!node->parent.expired());
		// Update children's parent
		for (shared_ptr<FPTreeNode<T>> nephew : node->children) {
			DEBUG(cout << "Set parent of " << *nephew << " to " << *(node->parent.lock());)
			nephew->parent = node->parent;
		}
		shared_ptr<FPTreeNode<T>> parent = node->parent.lock();
		parent->children.erase(node);
		this->headerTable.removeNode(node);
		this->mergeChildren(node, parent);
		DEBUG(cout << "Result:" << endl << (string) *this;)
		DEBUG(cout << "Result header table: " << endl << (string) this->headerTable;)
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
				})
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
			/*// Take care of the next and previous pointers of uncle in order to detach it from the list and reinsert in as the new head
			if (!uncle->previous.expired()) {
				uncle->previous.lock()->next = uncle->next;
			}
			if (!uncle->next.expired()) {
				uncle->next.lock()->previous = uncle->previous;
			}
			shared_ptr<FPTreeNode<T>> previous = this->headerTable.addNode(uncle);*/
			assert(!uncle->previous.expired() || this->headerTable.getNode(uncle->value) == uncle);
			// Since a merge is needed, adopt all the children of child by uncle
			for (shared_ptr<FPTreeNode<T>> nephew : child->children) {
				nephew->parent = uncle;
			}
			this->mergeChildren(child, uncle);
		}
	}
}