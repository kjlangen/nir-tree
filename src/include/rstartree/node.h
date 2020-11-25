#ifndef __RSTARTREE_NODE__
#define __RSTARTREE_NODE__
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

namespace rstartree
{
	class RStarTreeNode
	{
		class ReinsertionEntry
		{
			public:
				Rectangle boundingBox;
				Point data;
				RStarTreeNode *child;
				unsigned level;
		};
		
		// sorting structs to help in splitNode
		// As a note since points are X/Y, this will need to be updated when we
		// 	get to multi dimensional data
		// This may be worth moving to the geometry class?
		struct sortByXFirst
		{
			inline bool operator() (const Point& pointA, const Point& pointB)
			{
				return (pointA.x < pointB.x) || ((pointA.x == pointB.x) && (pointA.y < pointB.y));
			}
		};

		struct sortByYFirst{
			inline bool operator() (const Point& pointA, const Point& pointB)
			{
				return (pointA.y < pointB.y) || ((pointA.y == pointB.y) && (pointA.x < pointB.x));
			}
		};

		unsigned minBranchFactor;
		unsigned maxBranchFactor;

		public:
			RStarTreeNode *parent;
			std::vector<Rectangle> boundingBoxes;
			std::vector<RStarTreeNode *> children;
			std::vector<Point> data;

			// Constructors and destructors
			RStarTreeNode();
			RStarTreeNode(unsigned minBranchFactor, unsigned maxBranchFactor, RStarTreeNode *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			void updateBoundingBox(RStarTreeNode *child, Rectangle updatedBoundingBox);
			void removeChild(RStarTreeNode *child);
			void removeData(Point givenPoint);
			RStarTreeNode *chooseLeaf(Point givenPoint);
			RStarTreeNode *chooseNode(ReinsertionEntry e);
			RStarTreeNode *findLeaf(Point givenPoint);
			RStarTreeNode *splitNode(RStarTreeNode *newChild);
			unsigned int computeTotalMarginSum();
			unsigned int splitAxis(Point newData);
			std::vector<std::vector<unsigned int>> chooseSplitIndex(unsigned int axis);
			RStarTreeNode *splitNode(Point newData);
			RStarTreeNode *adjustTree(RStarTreeNode *siblingLeaf);
			RStarTreeNode *condenseTree();
			RStarTreeNode *insert(ReinsertionEntry e);

			// Datastructure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			RStarTreeNode *insert(Point givenPoint);
			RStarTreeNode *remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();

			// operator overlaod for sorting
			bool operator < (const RStarTreeNode &otherNode) const;
	};
}

#endif
