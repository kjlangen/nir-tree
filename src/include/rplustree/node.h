#ifndef __RPLUSNODE__
#define __RPLUSNODE__

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
#include <util/geometry.h>
#include <util/graph.h>
#include <util/debug.h>
#include <util/statistics.h>

namespace rplustree
{

	// Forward ref
	class RPlusTree;

	class Node
	{
		private:
			struct ReinsertionEntry
			{
				Rectangle boundingBox;
				Point data;
				Node *child;
				unsigned level;
			};

			RPlusTree &treeRef;
			unsigned minBranchFactor;
			unsigned maxBranchFactor;

			static std::vector<unsigned> histogramSearch;
			static std::vector<unsigned> histogramLeaves;
			static std::vector<unsigned> histogramRangeSearch;
			static std::vector<unsigned> histogramRangeLeaves;

		public:
			struct Branch
			{
				Node *child;
				Rectangle boundingBox;
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

			Node *parent;
			std::vector<Branch> branches;
			std::vector<Point> data;

			// Constructors and destructors
			Node(RPlusTree &treeRef);
			Node(RPlusTree &treeRef, unsigned minBranch, unsigned maxBranch, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			void updateBranch(Node *child, Rectangle &boundingBox);
			void removeBranch(Node *child);
			void removeData(Point givenPoint);
			Node *chooseNode(Point givenPoint);
			Node *findLeaf(Point givenPoint);
			Partition partitionNode();
			SplitResult splitNode(Partition p);
			SplitResult splitNode();
			SplitResult adjustTree();
			void pushDown(Point givenPoint);
			void condenseTree();

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			Node *insert(Point givenPoint);
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
