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

namespace rtree
{
	class RTree: public Index
	{
		public:
			Node *root;
			Statistics stats;

			// Constructors and destructors
			RTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			RTree(Node *root);
			~RTree();

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
