#ifndef __NIRNODE__
#define __NIRNODE__

#include <cassert>
#include <vector>
#include <stack>
#include <unordered_map>
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
#include <nirtree/pencilPrinter.h>

namespace nirtree
{
	class Node
	{
		private:
			typedef std::pair<unsigned, unsigned> edge;

			struct ReinsertionEntry
			{
				IsotheticPolygon boundingBox;
				Point data;
				Node *child;
				unsigned level;
			};

			unsigned minBranchFactor;
			unsigned maxBranchFactor;

		public:
			struct SplitResult
			{
				Node *left;
				IsotheticPolygon leftPolygon;
				Node *right;
				IsotheticPolygon rightPolygon;
			};

			Node *parent;
			std::vector<IsotheticPolygon> boundingBoxes;
			std::vector<Node *> children;
			std::vector<Point> data;

			// Constructors and destructors
			Node();
			Node(unsigned minBranchFactor, unsigned maxBranchFactor, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			void updateChild(Node *child, IsotheticPolygon &boundingBox);
			void removeChild(Node *child);
			void removeData(Point givenPoint);
			Node *chooseLeaf(Point givenPoint);
			Node *chooseNode(ReinsertionEntry e);
			Node *findLeaf(Point givenPoint);
			std::vector<Rectangle> decomposeNode(IsotheticPolygon &boundingPolygon);
			SplitResult splitNodeSpecialCase(Point newData);
			SplitResult splitNode(Point newData);
			SplitResult splitNode(SplitResult &lowerSplit);
			SplitResult adjustTree(SplitResult &leafSplit);
			bool condenseLeaf(Point givenPoint);
			bool condenseNode(IsotheticPolygon &givenPolygon);
			Node *condenseTree(Point givenPoint);
			Node *insert(ReinsertionEntry e);

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			Node *insert(Point givenPoint);
			Node *remove(Point givenPoint);
			Node *remove2(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate(Node *expectedParent, unsigned index);
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			bool tighten();
			unsigned height();
			void stat();
	};

	void testPlayground();
	void testRemoveChild();
	void testRemoveData();
	void testChooseLeaf();
	void testFindLeaf();
	void testSplitNodeLeaf();
	void testSplitNodeRoutingSimple();
	void testSplitNodeRoutingComplex();
	void testCondenseTree();
	void testSearch();
	void testInsert();
	void testRemove();
}

#endif
