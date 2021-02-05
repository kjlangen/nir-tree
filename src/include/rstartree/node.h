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
#include <memory>
#include <util/geometry.h>
#include <util/statistics.h>
#include <variant>


namespace rstartree
{

    class RStarTree;

	class Node
	{

        public:
        class Branch
        {
            public:
                Branch( Rectangle boundingBox, Node *child ) : boundingBox( boundingBox ), child( child ){}
                Branch( const Branch &other ) : boundingBox( other.boundingBox ), child( other.child ) { }

                bool operator==(const Branch &o) const {
                    return child == o.child and boundingBox == o.boundingBox;
                }

                ~Branch() { child = nullptr; }

                Rectangle boundingBox;
                Node *child;
        };
        typedef std::variant<Point, Branch> NodeEntry;

        private:
		
        const RStarTree &treeRef;

        void searchSub(const Point &requestedPoint, std::vector<Point> &accumulator) const;
        void searchSub(const Rectangle &rectangle, std::vector<Point> &accumulator) const;

		public:
			Node *parent;
            std::vector<NodeEntry> entries;
			unsigned int level = 0;

			// Constructors and destructors
			Node( const RStarTree &treeRef, Node *p=nullptr );
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox() const;
			void updateBoundingBox(Node *child, Rectangle updatedBoundingBox);
			void removeChild(Node *child);
			void removeData(Point givenPoint);
			Node *chooseSubtree(NodeEntry nodeEntry);
			Node *findLeaf(Point givenPoint);
			double computeTotalMarginSum();
            void entrySort(unsigned startingDimension);
			unsigned int chooseSplitAxis();
			unsigned chooseSplitIndex(unsigned int axis);
			Node *splitNode();
			Node *adjustTree(Node *siblingLeaf, std::vector<bool> &hasReinsertedOnLevel);
			Node *reInsert(std::vector<bool> &hasReinsertedOnLevel);
			Node *overflowTreatment(std::vector<bool> &hasReinsertedOnLevel);
			Node *condenseTree(std::vector<bool> &hasReinsertedOnLevel);

			// Datastructure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(const Point &requestedPoint) const;
			std::vector<Point> search(const Rectangle &requestedRectangle) const;

			// These return the root of the tree.
			Node *insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel);
			Node *remove(Point givenPoint, std::vector<bool> hasReinsertedOnLevel);

			// Miscellaneous
			unsigned checksum() const;
			void print() const;
			void printTree() const;
			unsigned height() const;
			void stat() const;

			// operator overlaod for sorting
			bool operator < (const Node &otherNode) const;
	};

    Rectangle boxFromNodeEntry( const Node::NodeEntry &entry );
    double computeOverlapGrowth(unsigned int index, const std::vector<Node::NodeEntry> &entries, const Rectangle &rect);
}


#endif
