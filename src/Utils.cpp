#include <vector>
#include <memory>
#include "FPTreeNode.h"

using namespace std;

template <typename T>
vector<shared_ptr<FPTreeNode<T>>> listToVector(shared_ptr<FPTreeNode<T>> node) {
	vector<shared_ptr<FPTreeNode<T>>> result;
	for (; node; node = node->getNext().lock()) {
		result.push_back(node);
	}
	return result;
}