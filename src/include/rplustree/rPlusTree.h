#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <queue>
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
	RPlusTreeNode *root = new RPlusTreeNode();

public:

	enum Orientation {ALONG_X_AXIS, ALONG_Y_AXIS};

	/*** constructor and destructor ***/

	RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);

	~RPlusTree();

	/*** general functions ***/

	RPlusTreeNode* getRoot() const;

	int height() const;

	int numDataElements() const;

	bool exists(Point requestedPoint);

	std::vector<Point> search(Point requestedPoint) override;

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

	Cost sweepNodes(std::vector<RPlusTreeNode*>& nodeList, Orientation orientation);

	void partition(RPlusTreeNode* n, float splitLine, Orientation splitAxis, RPlusTreeNode* left, RPlusTreeNode* right);

	Partition splitNode(RPlusTreeNode* n);

	Partition splitNodeAlongLine(RPlusTreeNode* n, float splitLine, Orientation splitAxis);

	/*** remove functions ***/

	void remove(Point givenPoint) override;

	void findDataPoints(RPlusTreeNode* n, std::vector<Point>& dataClone);

	/*** tree traversals ***/

	void bfs();

	/*** correctness checks ***/
	void checkBoundingBoxes();
};

#endif // __RPLUSTREE__
