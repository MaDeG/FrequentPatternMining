#ifndef FREQUENTPATTERNMINING_HEADERTABLE_H
#define FREQUENTPATTERNMINING_HEADERTABLE_H

#include <map>
#include <memory>
#include <vector>
#include "FPTreeNode.h"
#include "HeaderEntry.h"

template <typename T>
class HeaderTable {
public:
	HeaderTable();
	~HeaderTable();
	std::shared_ptr<FPTreeNode<T>> addNode(const std::shared_ptr<FPTreeNode<T>> node);
	std::shared_ptr<FPTreeNode<T>> getNode(const T& item) const;
	std::shared_ptr<FPTreeNode<T>> removeNode(const T& item);
	bool removeNode(const std::shared_ptr<FPTreeNode<T>> node);
	std::shared_ptr<FPTreeNode<T>> resetEntry(const T& item);
	int increaseFrequency(const T& item, const int addend);
	void pruneInfrequent(int minSupportCount);
	std::vector<T> getItems() const;
	typename std::map<T, HeaderEntry<T>>::const_iterator cbegin();
	typename std::map<T, HeaderEntry<T>>::const_iterator cend();
	bool empty() const;
	operator std::string() const;

private:
	std::map<T, HeaderEntry<T>> headerTable;
	omp_lock_t lock;
};

template class HeaderTable<int>;

template <typename T>
inline std::ostream& operator << (std::ostream &out, const HeaderTable<T>& headerTable) {
	return out << (std::string) headerTable;
}

#endif //FREQUENTPATTERNMINING_HEADERTABLE_H
