#pragma once
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <index/index.h>
#include <util/geometry.h>
#include <rstartreedisk/node.h>
#include <util/bmpPrinter.h>
#include <storage/tree_node_allocator.h>

namespace rstartreedisk
{
	class RStarTreeDisk : public Index
	{
		public:
			static constexpr float p = 0.3; // For reinsertion entries. 0.3 by default

			tree_node_handle root;
			Statistics stats;
            tree_node_allocator node_allocator_;
			const unsigned minBranchFactor;
			const unsigned maxBranchFactor;

			std::vector<bool> hasReinsertedOnLevel;

			// Constructors and destructors
			RStarTreeDisk( size_t memory_budget, unsigned minBranchFactor, unsigned maxBranchFactor);
			~RStarTreeDisk();

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
