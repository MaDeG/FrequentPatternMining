#include "HeaderEntry.h"

using namespace std;

template <typename T>
HeaderEntry<T>::HeaderEntry(std::shared_ptr<FPTreeNode<T>> node) : node(node),
																																	 totalFrequency(node->getFrequency())
{ }

template <typename T>
std::shared_ptr<FPTreeNode<T>> HeaderEntry<T>::getNode() const {
	return this->node;
}

template <typename T>
int HeaderEntry<T>::getTotalFrequency() const {
	return this->totalFrequency;
}