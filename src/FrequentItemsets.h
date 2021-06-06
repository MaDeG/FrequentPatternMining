#ifndef FREQUENTPATTERNMINING_FREQUENTITEMSETS_H
#define FREQUENTPATTERNMINING_FREQUENTITEMSETS_H

#include <omp.h>
#include "FPTreeManager.h"
#include "Params.h"

template <typename T>
class FrequentItemsets {
public:
	FrequentItemsets(FPTreeManager<T>& manager);
	const std::list<std::list<T>>& getFrequentItemsets() const;
	
private:
	std::list<std::list<T>> frequentItemsets;

	std::list<std::list<T>> computeFrequentItemsets(std::unique_ptr<FPTreeManager<T>> manager, int nThreads = Params::nThreads);
	void recomputeSupport(const T& item, HeaderTable<T>& headerTable);
};

template class FrequentItemsets<int>;

#endif //FREQUENTPATTERNMINING_FREQUENTITEMSETS_H
