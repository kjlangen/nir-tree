#ifndef __NIRNODE__
#define __NIRNODE__
#include <cassert>
#include <vector>
#include <stack>
#include <map>
#include <unordered_map>
#include <list>
#include <queue>
#include <utility>
#include <cmath>
#include <cstring>
#include <iostream>
#include <chrono>
#include <utility>
#include <util/geometry.h>

namespace nirtree
{
	class Node
	{
		class ReinsertionEntry
		{
			public:
				IsotheticPolygon boundingBox;
				Point data;
				Node *child;
				unsigned level;
		};

		unsigned minBranchFactor;
		unsigned maxBranchFactor;

		public:
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
			void updateBoundingBox(Node *child, Rectangle updatedBoundingBox);
			void removeChild(Node *child);
			void removeData(Point givenPoint);
			Node *chooseLeaf(Point givenPoint);
			Node *chooseNode(ReinsertionEntry e);
			Node *findLeaf(Point givenPoint);
			std::pair<Node *, IsotheticPolygon> splitNode(Node *newChild, IsotheticPolygon newPolygon);
			std::pair<Node *, IsotheticPolygon> splitNode(Point newData);
			std::pair<Node *, IsotheticPolygon> adjustTree(Node *sibling, IsotheticPolygon siblingPolygon);
			// Node *condenseTree();
			// Node *insert(ReinsertionEntry e);

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			Node *insert(Point givenPoint);
			// Node *remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			void print(unsigned n=0);
			void printTree(unsigned n=0);
	};

	void testPlayground();
	// void testBoundingBox();
	// void testUpdateBoundingBox();
	// void testRemoveChild();
	// void testRemoveData();
	// void testChooseLeaf();
	// void testFindLeaf();
	// void testSplitNode();
	// void testAdjustTree();
	// void testCondenseTree();
	// void testSearch();
	// void testInsert();
	// void testRemove();
}

#endif
