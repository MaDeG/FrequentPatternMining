#include <cassert>
#include <iostream>
#include "FrequentItemsets.h"
#include "Params.h"
#include "Utils.cpp"

using namespace std;

template <typename T>
FrequentItemsets<T>::FrequentItemsets(FPTreeManager<T>& manager) {
	manager.pruneInfrequent();
	// Create threads here in order to keep under control their quantity
	#pragma omp parallel shared(manager, cout) default(none)
	#pragma omp single
	this->frequentItemsets = this->computeFrequentItemsets(make_unique<FPTreeManager<T>>(manager));
}

template <typename T>
const list<list<T>>& FrequentItemsets<T>::getFrequentItemsets() const {
	return this->frequentItemsets;
}

template <typename T>
list<list<T>> FrequentItemsets<T>::computeFrequentItemsets(unique_ptr<FPTreeManager<T>> manager) {
	DEBUG(cout << "Received FPTree manager: " << endl << (string) *manager)
	DEBUG(cout << "Received Header Table: " << endl << (string) manager->headerTable)
	list<list<T>> frequentItemsets;
	const int supportCount = manager->getSupportCount();
	// Iterate over all the unique items that appeared in the itemset collection
	vector<T> items = manager->headerTable.getItems();
	if (items.empty()) {
		// Prevents a segfault in the OpenMP handling of empty items
		return frequentItemsets;
	}
	// Define custom reduction to move partial result to the final result, splice move data
	#pragma omp declare reduction (merge : list<list<T>> : omp_out.splice(omp_out.end(), omp_in))
	#pragma omp taskloop shared(items, manager, cout) default(none) reduction(merge: frequentItemsets) //grainsize(1)
	//#pragma omp parallel for schedule(dynamic) shared(items, manager, cout) default(none) num_threads(nThreads) reduction(merge: frequentItemsets)
	for (typename vector<T>::iterator it = items.begin(); it != items.end(); it++) {
		const T& item = *it;
		// All the remaining items in the header table are frequent
		frequentItemsets.emplace_back((initializer_list<T>) {item});
		DEBUG(cout << "Prefix element: " << item);
		unique_ptr<FPTreeManager<T>> prefixManager = manager->getPrefixTree(item);
		DEBUG(cout << "Raw Prefix tree: " << endl << *prefixManager);
		// After recomputing support we will not need the chosen prefix's nodes anymore
		this->recomputeSupport(item, prefixManager->headerTable);
		DEBUG(cout << "Recomputed support:" << endl << *prefixManager);
		prefixManager->removeItem(item);
		DEBUG(cout << "Removed prefix item " << item << endl << *prefixManager);
		prefixManager->pruneInfrequent();
		if (!prefixManager->headerTable.empty()) {
			DEBUG(cout << "Prefix tree pruned with support recomputed: " << endl << *prefixManager);
			list<list<T>> partialFrequentItemsets = this->computeFrequentItemsets(move(prefixManager));
			// Prepend the current element to the results found
			for (list<T>& partialItemset : partialFrequentItemsets) {
				partialItemset.push_front(item);
			}
			// Move partial result to the final result
			frequentItemsets.splice(frequentItemsets.end(), partialFrequentItemsets);
		} else {
			DEBUG(cout << "Empty FPTree found for prefix " << item << ", skipping");
		}
	}
	return frequentItemsets;
}

template <typename T>
void FrequentItemsets<T>::recomputeSupport(const T& item, HeaderTable<T>& headerTable) {
	shared_ptr<FPTreeNode<T>> node = headerTable.getNode(item);
	//vector<shared_ptr<FPTreeNode<T>>> nodes = listToVector(node);
	assert(node);
	//#pragma omp parallel shared(nodes, headerTable) default(none)
	//#pragma omp single
	do {
	//#pragma omp parallel for schedule(dynamic) shared(headerTable, nodes) default(none) if(Params::parallelRecomputeSupport)
	//#pragma omp taskloop shared(headerTable, nodes) default(none) grainsize(1)
	//for (typename vector<shared_ptr<FPTreeNode<T>>>::iterator it = nodes.begin(); it != nodes.end(); it++)
		//#pragma omp task firstprivate(node) shared(headerTable) default(none)
		{
			//shared_ptr<FPTreeNode<T>> node = *it;
			int frequency = node->getFrequency();
			if (frequency == 0) {
				continue;
			}
			assert(frequency > 0);
			// Walk to the root and recompute frequencies
			for (shared_ptr<FPTreeNode<T>> i = node->getParent().lock(); i->getFrequency() >= 0; i = i->getParent().lock()) {
				i->incrementFrequency(frequency);
				// Increment the total frequency for these items
				headerTable.increaseFrequency(i->getValue(), frequency);
			}
		}
	} while ((node = node->getNext().lock()));
}