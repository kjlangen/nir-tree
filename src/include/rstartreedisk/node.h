#pragma once

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
#include <storage/tree_node_allocator.h>

namespace rstartreedisk
{
	class RStarTreeDisk;

    // FIXME later
    //template <int min_branch_factor, int max_branch_factor>
	class Node
	{
		private:

			RStarTreeDisk *treeRef;

			void searchSub(const Point &requestedPoint, std::vector<Point> &accumulator) CONST_IF_NOT_STAT;
			void searchSub(const Rectangle &rectangle, std::vector<Point> &accumulator) CONST_IF_NOT_STAT;

		public:
			class Branch
			{
				public:
					Rectangle boundingBox;
					tree_node_handle child;

					Branch(Rectangle boundingBox, tree_node_handle
                            child_handle ) : boundingBox(boundingBox),
                            child(child_handle) {}
					Branch(const Branch &other) : boundingBox(other.boundingBox), child(other.child) {}

					bool operator==(const Branch &o) const;
			};
			typedef std::variant<Point, Branch> NodeEntry;

			tree_node_handle parent;
            tree_node_handle self_handle_;

            std::array<NodeEntry, 7> entries;
            std::array<NodeEntry, 7>::iterator entries_insertion_point;
            // FIXME later
            //std::array<int, max_branch_factor> entries;
			unsigned level;

			// Constructors and destructors
			Node(RStarTreeDisk *treeRef, tree_node_handle parent, unsigned level=0);
			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox() const;
			bool updateBoundingBox(tree_node_handle child, Rectangle updatedBoundingBox);
			void removeChild(tree_node_handle child);
			void removeData(const Point &givenPoint);
			tree_node_handle chooseSubtree(const NodeEntry &nodeEntry);
			tree_node_handle findLeaf(const Point &givenPoint);
			inline bool isLeafNode() const { return level == 0; }
			unsigned chooseSplitLeafAxis();
			unsigned chooseSplitNonLeafAxis();
			unsigned chooseSplitAxis();
			unsigned chooseSplitIndex(unsigned axis);
			tree_node_handle splitNode();
			tree_node_handle adjustTree(tree_node_handle siblingLeaf, std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle reInsert(std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle overflowTreatment(std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle condenseTree(std::vector<bool> &hasReinsertedOnLevel);

			// Datastructure interface functions
			void exhaustiveSearch(const Point &requestedPoint, std::vector<Point> &accumulator) const;

			std::vector<Point> search(const Point &requestedPoint) CONST_IF_NOT_STAT;
			std::vector<Point> search(const Rectangle &requestedRectangle) CONST_IF_NOT_STAT;

			// These return the root of the tree.
			tree_node_handle insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle remove(Point &givenPoint, std::vector<bool> hasReinsertedOnLevel);

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
	double computeOverlapGrowth(unsigned index, const
            std::array<Node::NodeEntry, 7> &entries, const Rectangle &rect);
}
