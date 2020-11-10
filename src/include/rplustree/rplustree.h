#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <rplustree/node.h>
#include <index/index.h>
#include <util/pencilPrinter.h>
#include <util/bmpPrinter.h>

namespace rplustree
{
	class RPlusTree: public Index
	{
		public:
			Node *root;

			// Constructors and destructors
			RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			RPlusTree(Node *root);
			~RPlusTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint);
			std::vector<Point> search(Rectangle requestedRectangle);
			void insert(Point givenPoint);
			void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			void stat();
			void print();
			void pencilPrint();
	};
}

#endif
