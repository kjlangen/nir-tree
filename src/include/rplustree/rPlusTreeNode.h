#ifndef __RPLUSTREE_NODE__
#define __RPLUSTREE_NODE__

#include <vector>
#include <util/geometry.h>

class RPlusTreeNode
{
public:
	Rectangle boundingBox;
	std::vector<RPlusTreeNode*> children;
	std::vector<Point> data;
	RPlusTreeNode* parent = nullptr;

	bool isRoot() const;
	bool isLeaf() const;
	unsigned int numChildren() const;
	unsigned int numDataEntries() const;
};

#endif // __RPLUSTREE_NODE__