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
	class RStarTree: public Index
	{
		public:
			Node *root;
            const unsigned minBranchFactor;
            const unsigned maxBranchFactor;
            static constexpr float p = 0.3; // For reinsertion entries. 0.3 by default
            std::vector<bool> hasReinsertedOnLevel;

			// Constructors and destructors
			RStarTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			~RStarTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint);
			std::vector<Point> search(Rectangle requestedRectangle);
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
