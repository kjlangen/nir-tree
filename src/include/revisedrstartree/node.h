#ifndef __REVISEDNODE__
#define __REVISEDNODE__

#include <cassert>
#include <vector>
#include <stack>
#include <limits>
#include <queue>
#include <utility>
#include <cmath>
#include <cstring>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <functional>
#include <globals/globals.h>
#include <util/geometry.h>
#include <util/debug.h>
#include <util/statistics.h>

namespace revisedrstartree
{
	class RevisedRStarTree;

	class Node
	{
		private:
			RevisedRStarTree &treeRef;
			Point originalCentre;

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

			Node *parent;
			std::vector<Branch> branches;
			std::vector<Point> data;

			// Constructors and destructors
			Node(RevisedRStarTree &treeRef, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			bool isLeaf();
			Rectangle boundingBox();
			void removeBranch(Node *child);
			void removeData(Point givenPoint);
			void chooseNodeHelper(unsigned limitIndex, Point &givenPoint, unsigned &chosenIndex, bool &success, std::vector<bool> &candidates, std::vector<double> &deltas, unsigned startIndex, bool useMarginDelta);
			Node *chooseNode(Point givenPoint);
			Node *findLeaf(Point givenPoint);
			double evaluateSplit(unsigned splitIndex, std::function<double(Rectangle &, Rectangle &)> evaluator);
			unsigned chooseSplitAxis();
			double splitWeight(unsigned splitIndex, double ys, double y1, double u, double sigma, Rectangle &bb);
			unsigned chooseSplitIndex(unsigned splitAxis);
			SplitResult splitNode();
			SplitResult adjustTree();
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
