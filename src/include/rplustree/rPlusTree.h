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

namespace RPlusTree
{
	typedef std::pair<RPlusTreeNode *, RPlusTreeNode *> Partition;
	typedef std::pair<float, float> Cost;

	class Tree : public Index
	{
		unsigned minBranchFactor;
		unsigned maxBranchFactor;
		RPlusTreeNode *root = nullptr;

	public:

		enum Orientation
		{
			ALONG_X_AXIS, ALONG_Y_AXIS
		};

		/*** constructor and destructor ***/

		Tree(unsigned minBranchFactor, unsigned maxBranchFactor);

		~Tree();

		/*** general functions ***/

		bool isEmpty() const;

		RPlusTreeNode *getRoot() const;

		int height() const;

		int numDataElements() const;

		bool exists(Point requestedPoint) const;

		std::vector<Point> search(Point requestedPoint) const override;

		std::vector<Point> search(Rectangle requestedRectangle) const override;

		unsigned checksum() override;

		/*** helper functions ***/

		void adjustTree(RPlusTreeNode *n, RPlusTreeNode *nn);

		RPlusTreeNode *chooseLeaf(RPlusTreeNode *node, Point &givenPoint) const;

		RPlusTreeNode *chooseLeaf(RPlusTreeNode *node, Rectangle &givenRectangle) const;

		RPlusTreeNode *findLeaf(Point requestedPoint) const;

		/*** insert functions ***/

		void insert(Point givenPoint) override;

		static Cost sweepData(std::vector<Point> &points, Orientation orientation);

		static Cost sweepNodes(std::vector<RPlusTreeNode *> &nodeList, Orientation orientation);

		Partition partition(RPlusTreeNode *n, float splitLine, Orientation splitAxis);

		Partition splitNode(RPlusTreeNode *n);

		/*** remove functions ***/

		void reinsert(RPlusTreeNode *n, int level, std::vector<Point> &dataClone);

		void condenseTree(RPlusTreeNode *n, std::vector<Point> &dataClone);

		void remove(Point givenPoint) override;

		/*** correctness checks ***/

		void checkBoundingBoxes();

		/*** tree traversal ***/

		friend std::ostream &operator<<(std::ostream &os, const Tree &tree);
	};
}

#endif // __RPLUSTREE__
