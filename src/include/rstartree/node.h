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
#include <variant>

namespace rstartree
{
	class Node
	{

        public:
        class Branch
        {
            public:
                Branch( Rectangle boundingBox, Node *child ) : boundingBox( boundingBox ), child( child ){}

                Rectangle boundingBox;
                Node *child;
        };
        typedef std::variant<Point, Branch> NodeEntry;

        private:

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

        void searchSub(Point &requestedPoint, std::vector<Point> &accumulator);
        void searchSub(Rectangle &rectangle, std::vector<Point> &accumulator);

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



			Node *parent;
            std::vector<NodeEntry> entries;
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

			// Datastructure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);

			// These return the root of the tree.
			Node *insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel);
			Node *remove(Point givenPoint, std::vector<bool> hasReinsertedOnLevel);

			// Miscellaneous
			unsigned checksum() const;
			void print(unsigned n=0) const;
			void printTree(unsigned n=0) const;
			unsigned height() const;
			void stat() const;

			// operator overlaod for sorting
			bool operator < (const Node &otherNode) const;

			Node *overflowTreatment(Node *nodeToInsert, std::vector<bool> &hasReinsertedOnLevel); // TODO: duplicated code; investigate ways to merge this with above

            // Static methods 
			static Node *overflowTreatment(Node *node, ReinsertionEntry e, std::vector<bool> hasReinsertedOnLevel);

	};

    Rectangle boxFromNodeEntry( const Node::NodeEntry &entry );
    unsigned computeOverlapGrowth(unsigned int index, const std::vector<Node::NodeEntry> &entries, const Rectangle &rect );

    struct sortByXRectangleFirst
    {
        inline bool operator() (const Node::NodeEntry &a, const Node::NodeEntry &b)
        {
            Rectangle rectangleA = boxFromNodeEntry( a );
            Rectangle rectangleB = boxFromNodeEntry( b );
            return (rectangleA.lowerLeft[0] < rectangleB.lowerLeft[0])
                || ((rectangleA.lowerLeft[0] == rectangleB.lowerLeft[0]) && (rectangleA.upperRight[1] < rectangleB.upperRight[1]));
        }
    };

    struct sortByYRectangleFirst{
        inline bool operator() (const Node::NodeEntry &a, const Node::NodeEntry &b)
        {
            Rectangle rectangleA = boxFromNodeEntry( a );
            Rectangle rectangleB = boxFromNodeEntry( b );
            return (rectangleA.lowerLeft[1] < rectangleB.lowerLeft[1])
                || ((rectangleA.lowerLeft[1] == rectangleB.lowerLeft[1]) && (rectangleA.upperRight[0] < rectangleB.upperRight[0]));
        }
    };


}


#endif
