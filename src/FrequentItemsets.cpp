#include <assert.h>
#include <sstream>
#include <boost/log/trivial.hpp>
#include "FrequentItemsets.h"

using namespace std;

template <typename T>
FrequentItemsets<T>::FrequentItemsets(FPTreeManager<T>& manager, const bool debug) : debug(debug) {
	manager.pruneInfrequent();
	this->frequentItemsets = move(this->computeFrequentItemsets(make_unique<FPTreeManager<T>>(manager)));
}

template <typename T>
const shared_ptr<list<list<T>>> FrequentItemsets<T>::getFrequentItemsets() const {
	return this->frequentItemsets;
}

template <typename T>
unique_ptr<list<list<T>>> FrequentItemsets<T>::computeFrequentItemsets(unique_ptr<FPTreeManager<T>> manager) {
	BOOST_LOG_TRIVIAL(debug) << "Received FPTree manager: " << endl << (string) *manager;
	unique_ptr<list<list<T>>> frequentItemsets = make_unique<list<list<T>>>();
	const int supportCount = manager->getSupportCount();
	// Iterate over all the unique items that appeared in the itemset collection
	for (typename map<T, HeaderEntry<T>>::const_iterator it = manager->headerTable.cbegin(); it != manager->headerTable.cend(); it++) {
		const T& item = it->first;
		// All the remaining items in the header table are frequent
		frequentItemsets->emplace_back((initializer_list<T>) {item});
		BOOST_LOG_TRIVIAL(debug) << "Prefix element: " << item;
		unique_ptr<FPTreeManager<T>> prefixManager = manager->getPrefixTree(item);
		BOOST_LOG_TRIVIAL(debug) << "Raw Prefix tree: " << endl << *prefixManager;
		// After recomputing support we will not need the chosen prefix's nodes anymore
		this->recomputeSupport(item, prefixManager->headerTable);
		BOOST_LOG_TRIVIAL(debug) << "Recomputed support:" << endl << *prefixManager;
		prefixManager->removeItem(item);
		BOOST_LOG_TRIVIAL(debug) << "Removed prefix item " << item << endl << *prefixManager;
		prefixManager->pruneInfrequent();
		if (prefixManager->headerTable.empty()) {
			BOOST_LOG_TRIVIAL(debug) << "Empty FPTree found for prefix " << item << ", skipping";
			continue;
		}
		BOOST_LOG_TRIVIAL(debug) << "Prefix tree pruned with support recomputed: " << endl << *prefixManager;
		unique_ptr<list<list<T>>> partialFrequentItemsets = move(this->computeFrequentItemsets(move(prefixManager)));
		// Prepend the current element to the results found
		for (list<T>& partialItemset : *partialFrequentItemsets) {
			partialItemset.push_front(item);
		}
		// Move partial result to the final result
		frequentItemsets->splice(frequentItemsets->end(), *partialFrequentItemsets);
	}
	return move(frequentItemsets);
}

template <typename T>
void FrequentItemsets<T>::recomputeSupport(const T& item, HeaderTable<T>& headerTable) {
	shared_ptr<FPTreeNode<T>> node = headerTable.getNode(item);
	assert(node);
	do {
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
	} while ((node = node->getNext().lock()));
}