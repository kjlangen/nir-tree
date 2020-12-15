#ifndef __RSTARTREE__
#define __RSTARTREE__
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <rstartree/node.h>
#include <index/index.h>

namespace rstartree
{
	// For now we will work with an RTree that only stores points
	class RStarTree: public Index
	{
		public:
			Node *root;
			std::vector<bool> hasReinsertedOnLevel;

			// Constructors and destructors
			RStarTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			RStarTree(Node *root);
			~RStarTree();

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
}

#endif
