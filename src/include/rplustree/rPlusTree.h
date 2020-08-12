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

namespace rplustree
{
	typedef std::pair<Node *, Node *> Partition;
	typedef std::pair<float, float> Cost;

	class Tree : public Index
	{
		unsigned minBranchFactor;
		unsigned maxBranchFactor;
		Node *root = nullptr;

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

		Node *getRoot() const;

		int height() const;

		int numDataElements() const;

		bool exists(Point requestedPoint) const;

		std::vector<Point> search(Point requestedPoint) const override;

		std::vector<Point> search(Rectangle requestedRectangle) const override;

		unsigned checksum() override;

		/*** helper functions ***/

		void adjustTree(Node *n, Node *nn);

		Node *chooseLeaf(Node *node, Point &givenPoint) const;

		Node *chooseLeaf(Node *node, Rectangle &givenRectangle) const;

		Node *findLeaf(Point requestedPoint) const;

		/*** insert functions ***/

		void insert(Point givenPoint) override;

		static Cost sweepData(std::vector<Point> &points, Orientation orientation);

		static Cost sweepNodes(std::vector<Node *> &nodeList, Orientation orientation);

		Partition partition(Node *n, float splitLine, Orientation splitAxis);

		Partition splitNode(Node *n);

		/*** remove functions ***/

		void reinsert(Node *n, int level, std::vector<Point> &dataClone);

		void condenseTree(Node *n, std::vector<Point> &dataClone);

		void remove(Point givenPoint) override;

		/*** correctness checks ***/

		void checkBoundingBoxes();

		/*** tree traversal ***/

		friend std::ostream &operator<<(std::ostream &os, const Tree &tree);
	};
}

#endif // __RPLUSTREE__
