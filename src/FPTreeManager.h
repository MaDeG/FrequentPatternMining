#ifndef FREQUENTPATTERNMINING_FPTREEMANAGER_H
#define FREQUENTPATTERNMINING_FPTREEMANAGER_H

#include "FileOrderedReader.h"
#include "FPTreeNode.h"

template <typename T>
class FPTreeManager {
public:
	FPTreeManager(FileOrderedReader& reader, double supportFraction);
	FPTreeManager(const FPTreeManager<T>& manager);
	FPTreeManager(FPTreeManager<T>&& manager) = default;
	~FPTreeManager() = default;
	const std::shared_ptr<FPTreeNode<T>> getRoot() const;
	const std::map<T, std::shared_ptr<FPTreeNode<T>>>& getHeaderTable() const;
	const int getSupportCount() const;

private:
	std::shared_ptr<FPTreeNode<T>> _root;
	std::map<T, std::shared_ptr<FPTreeNode<T>>> _headerTable;
	int _supportCount;

	void generateFPTree(FileOrderedReader& reader, double supportFraction);
};

template class FPTreeManager<int>;
//template class FPTreeManager<long>;
//template class FPTreeManager<float>;
//template class FPTreeManager<double>;
//template class FPTreeManager<std::string>;

#endif //FREQUENTPATTERNMINING_FPTREEMANAGER_H
