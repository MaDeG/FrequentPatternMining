#ifndef FREQUENTPATTERNMINING_HEADERTABLE_H
#define FREQUENTPATTERNMINING_HEADERTABLE_H

#include <map>
#include <memory>
#include "FPTreeNode.h"
#include "HeaderEntry.h"

template <typename T>
class HeaderTable {
public:
	HeaderTable(const bool debug);
	std::shared_ptr<FPTreeNode<T>> addNode(const std::shared_ptr<FPTreeNode<T>> node);
	const std::shared_ptr<FPTreeNode<T>> getNode(const T& item) const;
	std::shared_ptr<FPTreeNode<T>> removeNode(const T& item);
	void increaseFrequency(const T& item, const int addend);
	void pruneInfrequent(int minSupportCount);
	typename std::map<T, HeaderEntry<T>>::const_iterator cbegin();
	typename std::map<T, HeaderEntry<T>>::const_iterator cend();
	operator std::string() const;

private:
	std::map<T, HeaderEntry<T>> headerTable;
	const bool debug;
};

template class HeaderTable<int>;

template <typename T>
std::ostream& operator << (std::ostream &out, const HeaderTable<T>& headerTable) {
	return out << (std::string) headerTable;
}

#endif //FREQUENTPATTERNMINING_HEADERTABLE_H
