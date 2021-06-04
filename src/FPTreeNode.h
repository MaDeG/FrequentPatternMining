#ifndef FREQUENTPATTERNMINING_FPTREENODE_H
#define FREQUENTPATTERNMINING_FPTREENODE_H

#include <list>
#include <set>
#include <memory>

template <typename T> class HeaderTable;
template <typename T> class FPTreeManager;

template<typename T>
class FPTreeNode : public std::enable_shared_from_this<FPTreeNode<T>> {
	friend class FPTreeManager<T>;
public:
	FPTreeNode(const T& value, std::shared_ptr<FPTreeNode<T>> parent);
	FPTreeNode(FPTreeNode<T>&& node) = delete;
	~FPTreeNode() = default;
	std::shared_ptr<FPTreeNode<T>> getptr();
	const T& getValue() const;
	int getFrequency() const;
	const std::weak_ptr<FPTreeNode<T>> getNext() const;
	const std::weak_ptr<FPTreeNode<T>> getPrevious() const;
	const std::weak_ptr<FPTreeNode<T>> getParent() const;
	void incrementFrequency();
	void incrementFrequency(const int addend);
	void setNext(std::weak_ptr<FPTreeNode<T>> next);
	void setPrevious(std::weak_ptr<FPTreeNode<T>> previous);
	void addSequence(std::unique_ptr<std::list<T>> values, HeaderTable<T>& _headerTable);
	std::shared_ptr<FPTreeNode<T>> getChildren(const T& item) const;
	operator std::string() const;

private:
	static bool nodeComparator(const std::shared_ptr<FPTreeNode<T>>& a, const std::shared_ptr<FPTreeNode<T>>& b);
	T value;
	int frequency;
	std::weak_ptr<FPTreeNode<T>> parent;
	std::weak_ptr<FPTreeNode<T>> next;
	std::weak_ptr<FPTreeNode<T>> previous;
	std::set<std::shared_ptr<FPTreeNode<T>>, decltype(FPTreeNode<T>::nodeComparator)*> children;

	FPTreeNode(const FPTreeNode<T>& node);
	std::shared_ptr<FPTreeNode<T>> deepCopy(std::shared_ptr<FPTreeNode<T>> parent, HeaderTable<T>& newHeaderTable) const;
	std::shared_ptr<FPTreeNode<T>> getPrefixTree(std::shared_ptr<FPTreeNode<T>> parent, HeaderTable<T>& newHeaderTable, const T& item) const;
};

template class FPTreeNode<int>;
//template class FPTreeNode<long>;
//template class FPTreeNode<float>;
//template class FPTreeNode<double>;
//template class FPTreeNode<std::string>;

template <typename T>
std::ostream& operator << (std::ostream& out, const FPTreeNode<T>& node) {
	return out << (std::string) node;
}

#endif //FREQUENTPATTERNMINING_FPTREENODE_H
