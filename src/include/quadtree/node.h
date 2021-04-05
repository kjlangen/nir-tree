#ifndef __QUADNODE__
#define __QUADNODE__

#include <cassert>
#include <vector>
#include <stack>
#include <unordered_map>
#include <limits>
#include <list>
#include <queue>
#include <utility>
#include <cmath>
#include <cstring>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <globals/globals.h>
#include <util/geometry.h>
#include <util/statistics.h>

namespace quadtree
{
	class QuadTree;

	class Node
	{
		private:
			QuadTree &treeRef;

		public:
			Node *parent;
			std::vector<Node *> branches;
			Point data;

			// Constructors and destructors
			Node(QuadTree &treeRef, Point &givenPoint, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			unsigned nextBranch(Point &givenPoint);
			std::vector<unsigned> nextBranch(Rectangle &givenRectangle);
			bool isLeaf();

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			void insert(Point givenPoint);
			void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate(Node *expectedParent);
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();
	};
}

#endif
