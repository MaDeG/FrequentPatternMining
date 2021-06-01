#ifndef FREQUENTPATTERNMINING_FPTREEMANAGER_H
#define FREQUENTPATTERNMINING_FPTREEMANAGER_H

#include <memory>
#include "FileOrderedReader.h"
#include "HeaderTable.h"

template <typename T>
class FPTreeNode;

template <typename T>
class FrequentItemsets;

template <typename T>
class FPTreeManager {
	friend FrequentItemsets<T>;
public:
	FPTreeManager(FileOrderedReader& reader, const double supportFraction, const bool debug);
	FPTreeManager(const FPTreeManager<T>& manager);
	FPTreeManager(FPTreeManager<T>&& manager) = default;
	~FPTreeManager() = default;
	const std::shared_ptr<FPTreeNode<T>> getRoot() const;
	const HeaderTable<T>& getHeaderTable() const;
	const int getSupportCount() const;
	std::unique_ptr<FPTreeManager<T>> getPrefixTree(const T& item) const;
	void pruneInfrequent();
	std::shared_ptr<FPTreeNode<T>> removeItem(const T& item);
	operator std::string() const;

private:
	std::shared_ptr<FPTreeNode<T>> root;
	HeaderTable<T> headerTable;
	int supportCount;
	const bool debug;

	FPTreeManager(const bool debug);
	void generateFPTree(FileOrderedReader& reader, double supportFraction);
	void deleteItem(std::shared_ptr<FPTreeNode<T>> node);
	void mergeChildren(std::shared_ptr<FPTreeNode<T>> node, std::shared_ptr<FPTreeNode<T>> parent);
};

template class FPTreeManager<int>;
//template class FPTreeManager<long>;
//template class FPTreeManager<float>;
//template class FPTreeManager<double>;
//template class FPTreeManager<std::string>;

template <typename T>
std::ostream& operator << (std::ostream& out, const FPTreeManager<T>& node) {
	return out << (std::string) node;
}

#endif //FREQUENTPATTERNMINING_FPTREEMANAGER_H
