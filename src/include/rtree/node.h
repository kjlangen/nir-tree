#ifndef __NODE__
#define __NODE__
#include <cassert>
#include <vector>
#include <stack>
#include <map>
#include <list>
#include <utility>
#include <cmath>
#include <iostream>
#include <util/geometry.h>
#include <util/statistics.h>

namespace rtree
{

    class RTree;

	class Node
	{
		class ReinsertionEntry
		{
			public:
				Rectangle boundingBox;
				Point data;
				Node *child;
				unsigned level;
		};

		RTree &treeRef;
		unsigned minBranchFactor;
		unsigned maxBranchFactor;

		public:
			Node *parent;
			std::vector<Rectangle> boundingBoxes;
			std::vector<Node *> children;
			std::vector<Point> data;

			// Constructors and destructors
			Node(RTree &treeRef);
			Node(RTree &treeRef, unsigned minBranchFactor, unsigned maxBranchFactor, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			void updateBoundingBox(Node *child, Rectangle updatedBoundingBox);
			void removeChild(Node *child);
			void removeData(Point givenPoint);
			Node *chooseLeaf(Point givenPoint);
			Node *chooseNode(ReinsertionEntry e);
			Node *findLeaf(Point givenPoint);
			void moveData(unsigned fromIndex, std::vector<Point> &toData);
			void moveChild(unsigned fromIndex, std::vector<Rectangle> &toRectangles, std::vector<Node *> &toChildren);
			Node *splitNode(Node *newChild);
			Node *splitNode(Point newData);
			Node *adjustTree(Node *siblingLeaf);
			Node *condenseTree();
			Node *insert(ReinsertionEntry e);

			// Datastructure interface functions
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
