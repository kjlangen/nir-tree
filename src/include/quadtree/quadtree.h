#ifndef __QUADTREE__
#define __QUADTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <quadtree/node.h>
#include <index/index.h>
#include <util/bmpPrinter.h>
#include <util/statistics.h>

namespace quadtree
{
	class QuadTree: public Index
	{
		public:
			Node *root;
			Statistics stats;
			unsigned quadrants;

			// Constructors and destructors
			QuadTree();
			QuadTree(Node *root);
			~QuadTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint);
			std::vector<Point> search(Rectangle requestedRectangle);
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
