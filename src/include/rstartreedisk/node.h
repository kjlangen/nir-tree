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

    template <int min_branch_factor, int max_branch_factor>
    class RStarTreeDisk;

    template <int min_branch_factor, int max_branch_factor>
    tree_node_allocator *get_node_allocator(
            RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return &(treeRef->node_allocator_);
    }

    template <int min_branch_factor, int max_branch_factor>
    float get_p_value(
            RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return treeRef->p;
    }

    template <int min_branch_factor, int max_branch_factor>
    tree_node_handle get_root_handle(
            RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return treeRef->root;
    }

    template <int min_branch_factor, int max_branch_factor>
	class Node
	{
		private:

			RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef;

			void searchSub(const Point &requestedPoint,
                    std::vector<Point> &accumulator);
			void searchSub(const Rectangle &rectangle,
                    std::vector<Point> &accumulator);

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

					bool operator==(const
                            Node<min_branch_factor,max_branch_factor>::Branch &o) const;
			};
			typedef std::variant<Point, Branch> NodeEntry;

			tree_node_handle parent;
            tree_node_handle self_handle_;

            // Obnoxiously, this needs to have a +1 so we can overflow
            // by 1 entry and deal with it later.
            typename std::array<NodeEntry, max_branch_factor+1> entries;
            unsigned cur_offset_;
			unsigned level;

			// Constructors and destructors
            Node(RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef,
                    tree_node_handle self_handle,
                    tree_node_handle parent, unsigned level=0) :
                treeRef(treeRef),
                parent(parent),
                self_handle_(self_handle),
                level(level)
                {
                    cur_offset_ = 0;
                }

            void addEntryToNode( const NodeEntry &entry ) {
                entries.at( cur_offset_ ) = entry;
                cur_offset_++;
            }

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

			std::vector<Point> search(const Point &requestedPoint);
			std::vector<Point> search(const Rectangle
                    &requestedRectangle);

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

    template <int min_branch_factor, int max_branch_factor>
	Rectangle boxFromNodeEntry(const typename Node<min_branch_factor,
            max_branch_factor>::NodeEntry &entry) {
        using NodeType = Node<min_branch_factor,max_branch_factor>;
        using BranchType = typename NodeType::Branch;
        if (std::holds_alternative<BranchType>(entry))
        {
            return std::get<BranchType>(entry).boundingBox;
        }

        const Point &p = std::get<Point>(entry);
        return Rectangle(p, p);

    }

    template <class NE, class B, int N>
	double computeOverlapGrowth(unsigned index, const
            std::array<NE, N+1> &entries,
            unsigned els_to_consider,
            const Rectangle &givenBox) {
        // We cannot be a leaf
        assert(els_to_consider > 0 );
        bool is_branch = std::holds_alternative<B>(
                    entries[0] );
        assert( is_branch );
        
        // 1. Make a test rectangle we will use to not modify the original
        const Rectangle &origRectangle =
            std::get<B>(entries[index]).boundingBox;
        Rectangle newRectangle =
            std::get<B>(entries[index]).boundingBox;
        
        // 2. Add the point to the copied Rectangle
        newRectangle.expand(givenBox);

        // 3. Compute the overlap expansion area 
        double overlapDiff = 0;
        unsigned num_entries_els = els_to_consider;
        for (unsigned i = 0; i < num_entries_els; ++i) {
            const auto &entry = entries[i];

            if (i == index)
            {
                continue;
            }

            overlapDiff +=
                (newRectangle.computeIntersectionArea(std::get<B>(entry).boundingBox)
                - origRectangle.computeIntersectionArea(std::get<B>(entry).boundingBox));
        }

        return overlapDiff;

    }

#include "node.tcc"
}
