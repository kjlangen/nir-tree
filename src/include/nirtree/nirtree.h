#ifndef __NIRTREE__
#define __NIRTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <nirtree/node.h>
#include <nirtree/pencilPrinter.h>
#include <index/index.h>

namespace nirtree
{
	// TODO: For now we will work with a NIRTree that only stores points
	class NIRTree: public Index
	{
		public:
			Node *root;

			// Constructors and destructors
			NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			NIRTree(Node *root);
			~NIRTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint) override;
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
}

#endif
