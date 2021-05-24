#include <assert.h>
#include <boost/log/trivial.hpp>
#include "FrequentItemsets.h"

using namespace std;

template <typename T>
FrequentItemsets<T>::FrequentItemsets(FPTreeManager<T>& manager, const bool debug) : debug(debug) {
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
	manager->pruneInfrequent();
	// All the remaining items in the header table are frequent
	for (typename map<T, HeaderEntry<T>>::const_iterator it = manager->headerTable.cbegin(); it != manager->headerTable.cend(); it++) {
		list<T> newItemset({it->first});
		frequentItemsets->emplace_back(move(newItemset));
	}
	// Iterate over all the unique items that appeared in the itemset collection
	for (typename map<T, HeaderEntry<T>>::const_iterator it = manager->headerTable.cbegin(); it != manager->headerTable.cend(); it++) {
		const T& item = it->first;
		BOOST_LOG_TRIVIAL(debug) << "Prefix element: " << item;
		unique_ptr<FPTreeManager<T>> prefixManager = manager->getPrefixTree(item);
		shared_ptr<FPTreeNode<T>> firstItemNode = prefixManager->headerTable.removeNode(item);
		this->recomputeSupport(firstItemNode, prefixManager->headerTable);
		BOOST_LOG_TRIVIAL(debug) << "Prefix tree with support recomputed: " << endl << *prefixManager;
		unique_ptr<list<list<T>>> partialFrequentItemsets = move(this->computeFrequentItemsets(move(prefixManager)));
		// Prepend the current element to the results found
		for (list<T>& partialItemset : *partialFrequentItemsets) {
			partialItemset.push_front(item);
			if (this->debug) {
				string out;
				for (T& i : partialItemset) {
					out.append(to_string(i) + " ");
				}
				BOOST_LOG_TRIVIAL(debug) << out;
			}
		}
		// Move partial result to the final result
		frequentItemsets->splice(frequentItemsets->end(), *partialFrequentItemsets);
	}
	return move(frequentItemsets);
}

template <typename T>
void FrequentItemsets<T>::recomputeSupport(shared_ptr<FPTreeNode<T>> node, HeaderTable<T>& headerTable) {
	assert(node);
	do {
		int frequency = node->getFrequency();
		if (frequency == 0) {
			continue;
		}
		assert(frequency > 0);
		// Walk to the root and recompute frequencies
		for (shared_ptr<FPTreeNode<T>> i = node->getParent(); i->getFrequency() >= 0; i = i->getParent()) {
			i->incrementFrequency(frequency);
			// Increment the total frequency for these items
			headerTable.increaseFrequency(i->getValue(), frequency);
		}
	} while ((node = node->getNext()));
}