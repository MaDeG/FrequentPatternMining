#ifndef FREQUENTPATTERNMINING_FPTREEMANAGER_H
#define FREQUENTPATTERNMINING_FPTREEMANAGER_H

#include "FileOrderedReader.h"
#include "FPTreeNode.h"

template <typename T>
class FPTreeManager {
public:
	FPTreeManager(FileOrderedReader& fileOrderedReader);
	const FPTreeNode<T>& getRoot() const;
	const std::map<T, std::shared_ptr<FPTreeNode<T>>>& getHeaderTable();

private:
	FileOrderedReader& _fileOrderedReader;
	FPTreeNode<T> _root;
	std::map<T, std::shared_ptr<FPTreeNode<T>>> _headerTable;

	void generateFPTree();
};

template class FPTreeManager<int>;
//template class FPTreeManager<long>;
//template class FPTreeManager<float>;
//template class FPTreeManager<double>;
//template class FPTreeManager<std::string>;

#endif //FREQUENTPATTERNMINING_FPTREEMANAGER_H
