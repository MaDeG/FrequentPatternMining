#include "FrequentItemsets.h"

using namespace std;

template <typename T>
FrequentItemsets<T>::FrequentItemsets(const FPTreeManager<T> &manager) : _manager(manager) {
	this->computeFrequentItemsets();
}

template <typename T>
const std::list<T>& FrequentItemsets<T>::getFrequentItemsets() const {
	return this->_frequentItemsets;
}

template <typename T>
void FrequentItemsets<T>::computeFrequentItemsets() {
	const int supportCount = this->_manager.getSupportCount();

}