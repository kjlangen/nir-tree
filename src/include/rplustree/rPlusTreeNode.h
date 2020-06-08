#ifndef __RPLUSTREE_NODE__
#define __RPLUSTREE_NODE__

#include <vector>
#include <util/geometry.h>

class RPlusTreeNode
{
public:
	Rectangle boundingBox;
	std::vector<RPlusTreeNode*> children;
	RPlusTreeNode* parent = nullptr;
	bool isDataNode = false;

	explicit RPlusTreeNode(bool isDataNode);

	bool isRoot() const;
	bool isLeaf() const;
	unsigned int numChildren() const;
};

#endif // __RPLUSTREE_NODE__
