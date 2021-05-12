#ifndef FREQUENTPATTERNMINING_FPTREENODE_H
#define FREQUENTPATTERNMINING_FPTREENODE_H

#include <list>
#include <memory>
#include <map>

template <typename T> class FPTreeManager;

template<typename T>
class FPTreeNode {
	friend class FPTreeManager<T>;
public:
	FPTreeNode(const T &value);
	const T& getValue() const;
	int getFrequency() const;
	const std::shared_ptr<FPTreeNode<T>> getNext() const;
	const std::list<std::shared_ptr<FPTreeNode<T>>>& getChildren() const;
	void incrementFrequency();
	void setNext(std::shared_ptr<FPTreeNode<T>> next);
	void addSequence(std::unique_ptr<std::list<T>> values, std::map<T, std::shared_ptr<FPTreeNode<T>>>& _headerTable);
	operator std::string() const;

private:
	T _value;
	int _frequency;
	std::list<std::shared_ptr<FPTreeNode<T>>> _children;
	std::shared_ptr<FPTreeNode<T>> _next;
};

template class FPTreeNode<int>;
//template class FPTreeNode<long>;
//template class FPTreeNode<float>;
//template class FPTreeNode<double>;
//template class FPTreeNode<std::string>;

template <typename T>
std::ostream& operator << (std::ostream &out, const FPTreeNode<T>& node) {
	return out << (std::string) node;
}

#endif //FREQUENTPATTERNMINING_FPTREENODE_H
