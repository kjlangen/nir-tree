#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <utility>
#include <util/geometry.h>
#include <rplustree/rPlusTreeNode.h>

typedef std::pair<RPlusTreeNode*, RPlusTreeNode*> Partition;
typedef std::pair<float, float> Cost;

class RPlusTree
{
	unsigned minBranchFactor;
	unsigned maxBranchFactor;

	enum Orientation {ALONG_X_AXIS, ALONG_Y_AXIS};

	RPlusTreeNode *root = new RPlusTreeNode(false);

	void removeSubtree(RPlusTreeNode* r);

	void tighten(RPlusTreeNode* n);

	void adjustTree(RPlusTreeNode* n, RPlusTreeNode* nn);

	void chooseLeaves(RPlusTreeNode* node, Rectangle& givenRectangle, std::vector<RPlusTreeNode*>& leaves);

	Cost sweep(std::vector<RPlusTreeNode*>& nodeList, Orientation orientation);

	Partition splitNodeAlongLine(RPlusTreeNode* n, float splitLine, Orientation splitAxis);

	Partition splitNode(RPlusTreeNode* n);

public:
	RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);

	RPlusTree(RPlusTreeNode *root);

	~RPlusTree();

	bool exists(Rectangle requestedRectangle);

	std::vector<Point> search(Rectangle requestedRectangle);

	void insert(Rectangle givenRectangle);

	void remove(Point givenPoint);

	unsigned checksum();

	void print();
};

#endif // __RPLUSTREE__
