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
	};
}

#endif
