#ifndef __RSTARTREE__
#define __RSTARTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <index/index.h>
#include <util/geometry.h>
#include <rstartree/node.h>
#include <util/bmpPrinter.h>

namespace rstartree
{
	class RStarTree : public Index
	{
		public:
			static constexpr float p = 0.3; // For reinsertion entries. 0.3 by default

			Node *root;
			Statistics stats;
			const unsigned minBranchFactor;
			const unsigned maxBranchFactor;

			std::vector<bool> hasReinsertedOnLevel;

			// Constructors and destructors
			RStarTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			~RStarTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint) CONST_IF_NOT_STAT;
			std::vector<Point> search(Rectangle requestedRectangle) CONST_IF_NOT_STAT;
			void insert(Point givenPoint);
			void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			void print();
			bool validate();
			void stat();
			void visualize();
	};
}

#endif
