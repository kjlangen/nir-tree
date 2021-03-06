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
#include <index/index.h>
#include <util/bmpPrinter.h>
#include <util/statistics.h>

namespace nirtree
{
	class NIRTree: public Index
	{
		public:
			static constexpr float p = 0.3; // For reinsertion entries. 0.3 by default

			Node *root;
			Statistics stats;
			const unsigned minBranchFactor;
			const unsigned maxBranchFactor;

			std::vector<bool> hasReinsertedOnLevel;
			std::vector<Node *> garbage;

			// Constructors and destructors
			NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			~NIRTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint) CONST_IF_NOT_STAT;
			std::vector<Point> search(Rectangle requestedRectangle) CONST_IF_NOT_STAT;
			void insert(Point givenPoint);
			void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate();
			void stat();
			void print();
			void visualize();
	};
}

#endif
