#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <utility>
#include <util/geometry.h>
#include <rplustree/rPlusTreeNode.h>

typedef struct {
	RPlusTreeNode* leftNode = new RPlusTreeNode();
	Rectangle leftBoundingBox;
	RPlusTreeNode* rightNode = new RPlusTreeNode();
	Rectangle rightBoundingBox;
} Partition;

typedef std::pair<float, float> Cost;

class RPlusTree
{
	unsigned minBranchFactor;
	unsigned maxBranchFactor;

	enum Orientation {ALONG_X_AXIS, ALONG_Y_AXIS};

	RPlusTreeNode *root = new RPlusTreeNode();

	void removeSubtree(RPlusTreeNode *r);

	RPlusTreeNode *chooseLeaf(Rectangle givenRectangle);

	Cost sweep(std::vector<Rectangle>& rectangles, Orientation orientation);

	Partition splitNodeDownwards(RPlusTreeNode* node, Orientation orientation, float splitLine);

	void splitNode(RPlusTreeNode* node, Orientation orientation, float splitLine);

public:
	RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);

	RPlusTree(RPlusTreeNode *root);

	~RPlusTree();

	std::vector<Point> search(Point requestedPoint);

	std::vector<Point> search(Rectangle requestedRectangle);

	void insert(Point givenPoint);

	void insert(Rectangle givenRectangle);

	void remove(Point givenPoint);

	unsigned checksum();

	void print();
};

#endif // __RPLUSTREE__
