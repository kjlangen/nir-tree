#ifndef __RTREE__
#define __RTREE__
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <rtree/node.h>
#include <index/index.h>

// For now we will work with an RTree that only stores points
class RTree: public Index
{
	public:
		Node *root;

		// Constructors and destructors
		RTree(unsigned minBranchFactor, unsigned maxBranchFactor);
		RTree(Node *root);
		~RTree();

		// Datastructure interface
		std::vector<Point> exhaustiveSearch(Point requestedPoint);
		std::vector<Point> search(Point requestedPoint) override;
		std::vector<Point> search(Rectangle requestedRectangle) override;
		void insert(Point givenPoint) override;
		void remove(Point givenPoint) override;

		// Miscellaneous
		unsigned checksum() override;
		void print();
};

void testSimpleSearch();
void testSimpleInsert();
void expandRootTest();

#endif
