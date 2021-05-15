#ifndef FREQUENTPATTERNMINING_FREQUENTITEMSETS_H
#define FREQUENTPATTERNMINING_FREQUENTITEMSETS_H

#include "FPTreeManager.h"

template <typename T>
class FrequentItemsets {
public:
	FrequentItemsets(const FPTreeManager<T>& manager);
	const std::list<T>& getFrequentItemsets() const;
	
private:
	std::list<T> _frequentItemsets;
	const FPTreeManager<T>& _manager;

	void computeFrequentItemsets();
};

#endif //FREQUENTPATTERNMINING_FREQUENTITEMSETS_H
