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

public:

	/*** constructor and destructor ***/

	RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);

	~RPlusTree();

	/*** general functions ***/

	RPlusTreeNode* getRoot() const;

	int height() const;

	bool exists(Point requestedPoint);

	std::vector<Point> search(Rectangle requestedRectangle) override;

	unsigned checksum() override;

	void print();

	/*** helper functions ***/

	void tighten(RPlusTreeNode* n);

	void adjustTree(RPlusTreeNode* n, RPlusTreeNode* nn);

	RPlusTreeNode* chooseLeaf(RPlusTreeNode* node, Point& givenPoint);

	/*** insert functions ***/

	void insert(Point givenPoint) override;

	Cost sweepData(std::vector<Point>& points, Orientation orientation);

	Cost sweep(std::vector<RPlusTreeNode*>& nodeList, Orientation orientation);

	Partition splitNodeAlongLine(RPlusTreeNode* n, float splitLine, Orientation splitAxis);

	Partition splitNode(RPlusTreeNode* n);

	/*** remove functions ***/

	void remove(Point givenPoint) override;

	void reinsert(RPlusTreeNode* n, int level);

	void condenseTree(RPlusTreeNode* n);

	void removeSubtree(RPlusTreeNode* r);
};

#endif // __RPLUSTREE__
