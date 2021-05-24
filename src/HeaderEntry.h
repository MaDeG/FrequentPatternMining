#ifndef FREQUENTPATTERNMINING_HEADERENTRY_H
#define FREQUENTPATTERNMINING_HEADERENTRY_H

#include <memory>
#include "FPTreeNode.h"

template <typename T>
class HeaderTable;

template <typename T>
class HeaderEntry {
	friend class HeaderTable<T>;
public:
	HeaderEntry(std::shared_ptr<FPTreeNode<T>> node);
	std::shared_ptr<FPTreeNode<T>> getNode() const;
	int getTotalFrequency() const;

private:
	std::shared_ptr<FPTreeNode<T>> node;
	int totalFrequency;
};

template struct HeaderEntry<int>;

#endif //FREQUENTPATTERNMINING_HEADERENTRY_H
