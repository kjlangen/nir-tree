#ifndef __REVISEDRSTAR__
#define __REVISEDRSTAR__

#include <cassert>
#include <vector>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <revisedrstartree/node.h>
#include <index/index.h>
#include <util/bmpPrinter.h>
#include <util/statistics.h>

namespace revisedrstartree
{
	class RevisedRStarTree: public Index
	{
		public:
			Node *root;
			Statistics stats;

			const unsigned minBranchFactor;
			const unsigned maxBranchFactor;
			const double s = 0.5;

			// Constructors and destructors
			RevisedRStarTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			~RevisedRStarTree();

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
