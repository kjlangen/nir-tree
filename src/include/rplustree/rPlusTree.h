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

	class RPlusTree: public Index
	{
		unsigned minBranchFactor;
		unsigned maxBranchFactor;
		Node *root = nullptr;

		public:
			enum Orientation
			{
				ALONG_X_AXIS, ALONG_Y_AXIS
			};

			// Constructors and Destructors
			RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			~RPlusTree();

			// Helper functions
			bool isEmpty();
			Node *getRoot();
			unsigned height();
			unsigned numDataElements();
			bool exists(Point requestedPoint);
			void adjustTree(Node *n, Node *nn);
			Node *chooseLeaf(Node *node, Point &givenPoint);
			Node *chooseLeaf(Node *node, Rectangle &givenRectangle);
			Node *findLeaf(Point requestedPoint);

			// Insert helper functions
			static Cost sweepData(std::vector<Point> &points, Orientation orientation);
			static Cost sweepNodes(std::vector<Node *> &nodeList, Orientation orientation);
			Partition partition(Node *n, float splitLine, Orientation splitAxis);
			Partition splitNode(Node *n);

			// Delete helper functions
			void reinsert(Node *n, unsigned level, std::vector<Point> &dataClone);
			void condenseTree(Node *n, std::vector<Point> &dataClone);

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint) override;
			std::vector<Point> search(Point requestedPoint) override;
			std::vector<Point> search(Rectangle requestedRectangle) override;
			void insert(Point givenPoint) override;
			void remove(Point givenPoint) override;

			// Miscellaneous
			void checkBoundingBoxes();
			unsigned checksum() override;
			friend std::ostream &operator<<(std::ostream &os, RPlusTree &tree);
	};
}

#endif
