#ifndef __RSTARTREE_NODE__
#define __RSTARTREE_NODE__
#include <cassert>
#include <vector>
#include <stack>
#include <map>
#include <list>
#include <utility>
#include <cmath>
#include <numeric>
#include <iostream>
#include <limits>
#include <util/geometry.h>
#include <util/statistics.h>

namespace rstartree
{
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
		
		unsigned minBranchFactor;		// this cannot be 1 or else splitNode will fail
		unsigned maxBranchFactor;
		static constexpr float p = 0.3; 			// for reinsertion entries - set to 0.3 on default

		public:

            // sorting structs to help in splitNode
            // TODO: As a note since points are X/Y, this will need to be updated when we
            // 		get to multi dimensional data
            struct sortByXFirst
            {
                inline bool operator() (const Point& pointA, const Point& pointB)
                {
                    // FIXME: multi-dimensional
                    return (pointA[0] < pointB[0]) || ((pointA[0] == pointB[0]) && (pointA[1] < pointB[1]));
                }
            };

            struct sortByYFirst{
                inline bool operator() (const Point& pointA, const Point& pointB)
                {
                    // FIXME: multi-dimensional
                    return (pointA[1] < pointB[1]) || ((pointA[1] == pointB[1]) && (pointA[0] < pointB[0]));
                }
            };

            struct sortByXRectangleFirst
            {
                inline bool operator() (const Rectangle& rectangleA, const Rectangle& rectangleB)
                {
                    return (rectangleA.lowerLeft[0] < rectangleB.lowerLeft[0])
                        || ((rectangleA.lowerLeft[0] == rectangleB.lowerLeft[0]) && (rectangleA.upperRight[1] < rectangleB.upperRight[1]));
                }
            };

            struct sortByYRectangleFirst{
                inline bool operator() (const Rectangle& rectangleA, const Rectangle& rectangleB)
                {
                    return (rectangleA.lowerLeft[1] < rectangleB.lowerLeft[1])
                        || ((rectangleA.lowerLeft[1] == rectangleB.lowerLeft[1]) && (rectangleA.upperRight[0] < rectangleB.upperRight[0]));
                }
            };


			Node *parent;

            // I have one bounding box for each of my children if I am not a leaf node.
			std::vector<Rectangle> childBoundingBoxes;
			std::vector<Node *> children;

            // I only have data points if I am a leaf node.
			std::vector<Point> data;
			unsigned int level = 0;

			// Constructors and destructors
			Node();
			Node(unsigned minBranchFactor, unsigned maxBranchFactor, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			void updateBoundingBox(Node *child, Rectangle updatedBoundingBox);
			void removeChild(Node *child);
			void removeData(Point givenPoint);
			unsigned computeOverlapGrowth(unsigned int index, std::vector<Rectangle> boundingBoxes, Rectangle givenRectangle);
			unsigned computeOverlapGrowth(unsigned int index, std::vector<Rectangle> boundingBoxes, Point givenPoint);
			Node *chooseSubtree(Point givenPoint);
			Node *chooseNode(ReinsertionEntry e);
			Node *findLeaf(Point givenPoint);
			double computeTotalMarginSumOfBoundingBoxes();
			unsigned int chooseSplitAxis(Node *newChild);
			std::vector<std::vector<unsigned int>> chooseSplitIndexByRectangle(unsigned int axis);
			Node *splitNode(Node *newChild);
			double computeTotalMarginSum();
			unsigned int chooseSplitAxis();
			unsigned chooseSplitIndex(unsigned int axis);
			Node *splitNode();
			Node *adjustTree(Node *siblingLeaf, std::vector<bool> hasReinsertedOnLevel);
			Node *reInsert(std::vector<bool> &hasReinsertedOnLevel);
			Node *reInsert(ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel);

			Node *overflowTreatment(std::vector<bool> &hasReinsertedOnLevel);

			Node *condenseTree(std::vector<bool> hasReinsertedOnLevel);
			Node *insert(ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel);

			// Datastructure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);

			// These return the root of the tree.
			Node *insert(Point givenPoint, std::vector<bool> &hasReinsertedOnLevel);
			Node *remove(Point givenPoint, std::vector<bool> hasReinsertedOnLevel);

			// Miscellaneous
			unsigned checksum();
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();

			// operator overlaod for sorting
			bool operator < (const Node &otherNode) const;

			Node *overflowTreatment(Node *nodeToInsert, std::vector<bool> &hasReinsertedOnLevel); // TODO: duplicated code; investigate ways to merge this with above

            // Static methods 
			static Node *overflowTreatment(Node *node, ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel);
			
	};
}


#endif
