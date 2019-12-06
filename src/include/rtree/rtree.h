#ifndef __RTREE__
#define __RTREE__
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <rtree/node.h>

// For now we will work with an RTree that only stores points
class RTree
{
	public:
		Node *root;

		// Constructors and destructors
		RTree(unsigned minBranchFactor, unsigned maxBrnachFactor);
		RTree(Node *root);
		~RTree();

		// Datastructure interface
		std::vector<Point> search(Rectangle requestedRectangle);
		void insert(Point givenPoint);
		void remove(Point givenPoint);

		// Miscellaneous
		void print();
};

void testSimpleSearch();
void testSimpleInsert();
void expandRootTest();

#endif
