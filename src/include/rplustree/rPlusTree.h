#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <utility>
#include <util/geometry.h>
#include <index/index.h>
#include <rplustree/rPlusTreeNode.h>

typedef std::pair<RPlusTreeNode*, RPlusTreeNode*> Partition;
typedef std::pair<float, float> Cost;

class RPlusTree: public Index
{
	unsigned minBranchFactor;
	unsigned maxBranchFactor;

	enum Orientation {ALONG_X_AXIS, ALONG_Y_AXIS};

	RPlusTreeNode *root = new RPlusTreeNode();

	/* remove helper functions */

	void reinsert(RPlusTreeNode* n, int level);

	void condenseTree(RPlusTreeNode* n);

	void removeSubtree(RPlusTreeNode* r);

	/* general helper functions */

	void tighten(RPlusTreeNode* n);

	void adjustTree(RPlusTreeNode* n, RPlusTreeNode* nn);

	RPlusTreeNode* chooseLeaf(RPlusTreeNode* node, Point& givenPoint);

	/* insert helper functions */

	Cost sweepData(std::vector<Point>& points, Orientation orientation);

	Cost sweep(std::vector<RPlusTreeNode*>& nodeList, Orientation orientation);

	Partition splitNodeAlongLine(RPlusTreeNode* n, float splitLine, Orientation splitAxis);

	Partition splitNode(RPlusTreeNode* n);

public:
	int height() const;

	RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);

	RPlusTree(RPlusTreeNode *root);

	~RPlusTree();

	bool exists(Point requestedPoint);

	std::vector<Point> search(Rectangle requestedRectangle) override;

	void insert(Point givenPoint) override;

	void remove(Point givenPoint) override;

	unsigned checksum() override;

	void print();
};

#endif // __RPLUSTREE__
