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
#include <variant>
#include <globals/globals.h>
#include <util/geometry.h>
#include <util/statistics.h>

namespace rstartree
{
	class RStarTree;

	class Node
	{
		private:
			static std::vector<unsigned> histogramSearch;
			static std::vector<unsigned> histogramLeaves;
			static std::vector<unsigned> histogramRangeSearch;
			static std::vector<unsigned> histogramRangeLeaves;
			static unsigned leavesSearched;
			static unsigned nodesSearched;

			const RStarTree &treeRef;

			void searchSub(const Point &requestedPoint, std::vector<Point> &accumulator) const;
			void searchSub(const Rectangle &rectangle, std::vector<Point> &accumulator) const;

		public:
			class Branch
			{
				public:
					Rectangle boundingBox;
					Node *child;

					Branch(Rectangle boundingBox, Node *child) : boundingBox(boundingBox), child(child) {}
					Branch(const Branch &other) : boundingBox(other.boundingBox), child(other.child) {}
					~Branch();

					bool operator==(const Branch &o) const;
			};
			typedef std::variant<Point, Branch> NodeEntry;

			Node *parent;
			std::vector<NodeEntry> entries;
			unsigned level = 0;

			// Constructors and destructors
			Node(const RStarTree &treeRef, Node *p=nullptr);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox() const;
			void updateBoundingBox(Node *child, Rectangle updatedBoundingBox);
			void removeChild(Node *child);
			void removeData(const Point &givenPoint);
			Node *chooseSubtree(const NodeEntry &nodeEntry);
			Node *findLeaf(const Point &givenPoint);
			double computeTotalMarginSum();
			void entrySort(unsigned startingDimension);
			unsigned chooseSplitAxis();
			unsigned chooseSplitIndex(unsigned axis);
			Node *splitNode();
			Node *adjustTree(Node *siblingLeaf, std::vector<bool> &hasReinsertedOnLevel);
			Node *reInsert(std::vector<bool> &hasReinsertedOnLevel);
			Node *overflowTreatment(std::vector<bool> &hasReinsertedOnLevel);
			Node *condenseTree(std::vector<bool> &hasReinsertedOnLevel);

			// Datastructure interface functions
			void exhaustiveSearch(const Point &requestedPoint, std::vector<Point> &accumulator) const;
			std::vector<Point> search(const Point &requestedPoint) const;
			std::vector<Point> search(const Rectangle &requestedRectangle) const;

			// These return the root of the tree.
			Node *insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel);
			Node *remove(Point &givenPoint, std::vector<bool> hasReinsertedOnLevel);

			// Miscellaneous
			unsigned checksum() const;
			void print() const;
			void printTree() const;
			unsigned height() const;
			void stat() const;

			// Operators
			bool operator<(const Node &otherNode) const;
	};

	Rectangle boxFromNodeEntry(const Node::NodeEntry &entry);
	double computeOverlapGrowth(unsigned index, const std::vector<Node::NodeEntry> &entries, const Rectangle &rect);
}

#endif
