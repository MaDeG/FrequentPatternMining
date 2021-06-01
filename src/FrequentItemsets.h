#ifndef FREQUENTPATTERNMINING_FREQUENTITEMSETS_H
#define FREQUENTPATTERNMINING_FREQUENTITEMSETS_H

#include "FPTreeManager.h"

template <typename T>
class FrequentItemsets {
public:
	FrequentItemsets(FPTreeManager<T>& manager, const bool debug);
	const std::shared_ptr<std::list<std::list<T>>> getFrequentItemsets() const;
	
private:
	std::shared_ptr<std::list<std::list<T>>> frequentItemsets;
	const bool debug;

	std::unique_ptr<std::list<std::list<T>>> computeFrequentItemsets(std::unique_ptr<FPTreeManager<T>> manager);
	void recomputeSupport(const T& item, HeaderTable<T>& headerTable);
};

template class FrequentItemsets<int>;

#endif //FREQUENTPATTERNMINING_FREQUENTITEMSETS_H
