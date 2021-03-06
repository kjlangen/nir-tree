#ifndef __NIRNODE__
#define __NIRNODE__

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
#include <omp.h>
#include <globals/globals.h>
#include <util/geometry.h>
#include <util/graph.h>
#include <util/debug.h>
#include <util/statistics.h>
#include <variant>

namespace nirtree
{
    class NIRTree;

	class Node
	{
		private:
			NIRTree &treeRef;

		public:
			struct Branch
			{
				Node *child;
				IsotheticPolygon boundingPoly;
			};

			struct SplitResult
			{
				Branch leftBranch;
				Branch rightBranch;
			};

			struct Partition
			{
				unsigned dimension;
				double location;
			};

			typedef std::variant<Point, Branch> PointOrOrphan;

			Node *parent;
			unsigned level;
			std::vector<Branch> branches;
			std::vector<Point> data;

			// Constructors and destructors
			Node(NIRTree &treeRef, Node *p, unsigned level);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			Branch locateBranch(Node *child);
			void resizeBoundingPoly();
			void updateBranch(Node *child, IsotheticPolygon &boundingPoly);
			void removeBranch(Node *child);
			void removeData(Point givenPoint);
			Node *chooseNode(Point givenPoint);
			Node *chooseNode(Branch orphanedBranch);
			Node *findLeaf(Point givenPoint);
			Partition partitionNode();
			SplitResult splitNode(Partition p);
			SplitResult splitNode();
			SplitResult adjustTree(std::vector<bool> &hasReinsertedOnLevel);
			SplitResult reInsert(std::vector<bool> &hasReinsertedOnLevel);
			SplitResult overflowTreatment(std::vector<bool> &hasReinsertedOnLevel);
			void condenseTree();

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(const Point &requestedPoint);
			std::vector<Point> search(const Rectangle &requestedRectangle);
			Node *insert(PointOrOrphan given, std::vector<bool> &hasReinsertedOnLevel);
			Node *remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate(Node *expectedParent, unsigned index);
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();
	};
}

#endif
