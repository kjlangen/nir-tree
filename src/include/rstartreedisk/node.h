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
#include <util/repacking.h>
#include <util/statistics.h>
#include <storage/tree_node_allocator.h>

namespace rstartreedisk
{
    template <int min_branch_factor, int max_branch_factor>
    class RStarTreeDisk;

    template <int min_branch_factor, int max_branch_factor>
    tree_node_allocator *get_node_allocator(
            RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return treeRef->node_allocator_.get();
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

    class Branch
    {
        public:

            Branch(Rectangle boundingBox, tree_node_handle
                    child_handle ) : boundingBox(boundingBox),
                    child(child_handle) {}

            Branch() = default;

            Branch(const Branch &other) : boundingBox(other.boundingBox), child(other.child) {}

            bool operator==(const Branch &o) const = default;

            Rectangle boundingBox;
            tree_node_handle child;
    };

    typedef std::variant<Branch,Point> NodeEntry;

    template <int min_branch_factor, int max_branch_factor>
	class LeafNode
	{
        private:
			void searchSub(const Point &requestedPoint,
                    std::vector<Point> &accumulator);
			void searchSub(const Rectangle &rectangle,
                    std::vector<Point> &accumulator);
        public:
			RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef;
			tree_node_handle parent;
            tree_node_handle self_handle_;
            unsigned cur_offset_;
			unsigned level;

            // Obnoxiously, this needs to have a +1 so we can overflow
            // by 1 entry and deal with it later.
            std::array<Point, max_branch_factor+1> entries;

			// Constructors and destructors
            LeafNode(RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef,
                    tree_node_handle self_handle,
                    tree_node_handle parent, unsigned level=0) :
                treeRef(treeRef),
                parent(parent),
                self_handle_(self_handle),
                level(level)
                {
                    cur_offset_ = 0;
                }

            void addPoint( const Point &p ) {
                entries.at( cur_offset_++ ) = p; 
            }

			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox() const;
			void removePoint(const Point &givenPoint);

			tree_node_handle chooseSubtree(const NodeEntry &nodeEntry);
			tree_node_handle findLeaf(const Point &givenPoint);
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

			// These return the root of the tree.
			tree_node_handle insert(Point nodeEntry, std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle remove(Point &givenPoint, std::vector<bool> hasReinsertedOnLevel);

			// Miscellaneous
			unsigned checksum() const;
			void print() const;
			void printTree() const;
			unsigned height() const;
			void stat() const;

            uint16_t compute_packed_size();
            tree_node_handle repack( tree_node_allocator *allocator );

			// Operators
			bool operator<(const LeafNode &otherNode) const;
	};

    template <int min_branch_factor, int max_branch_factor>
	class BranchNode
	{
		private:

			void searchSub(const Point &requestedPoint,
                    std::vector<Point> &accumulator);
			void searchSub(const Rectangle &rectangle,
                    std::vector<Point> &accumulator);

		public:
			RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef;
			tree_node_handle parent;
            tree_node_handle self_handle_;

            // Obnoxiously, this needs to have a +1 so we can overflow
            // by 1 entry and deal with it later.
            typename std::array<Branch, max_branch_factor+1> entries;
            unsigned cur_offset_;
			unsigned level;

			// Constructors and destructors
            BranchNode(RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef,
                    tree_node_handle self_handle,
                    tree_node_handle parent, unsigned level=0) :
                treeRef(treeRef),
                parent(parent),
                self_handle_(self_handle),
                level(level)
                {
                    cur_offset_ = 0;
                }

            void addBranchToNode( const Branch &b ) {
                entries.at( cur_offset_++ ) = b;
            }

			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox() const;
			bool updateBoundingBox(tree_node_handle child, Rectangle updatedBoundingBox);
			void removeChild(tree_node_handle child);
			tree_node_handle chooseSubtree(const NodeEntry &nodeEntry);
			tree_node_handle findLeaf(const Point &givenPoint);
			unsigned chooseSplitLeafAxis();
			unsigned chooseSplitNonLeafAxis();
			unsigned chooseSplitAxis();
			unsigned chooseSplitIndex(unsigned axis);
			tree_node_handle splitNode();
			tree_node_handle adjustTree(
                    tree_node_handle siblingLeaf,
                    std::vector<bool> &hasReinsertedOnLevel
            );
			tree_node_handle reInsert(std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle overflowTreatment(std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle condenseTree(std::vector<bool> &hasReinsertedOnLevel);

			// Datastructure interface functions
			void exhaustiveSearch(const Point &requestedPoint, std::vector<Point> &accumulator) const;

			// These return the root of the tree.
			tree_node_handle insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel);
			tree_node_handle remove(Point &givenPoint, std::vector<bool> hasReinsertedOnLevel);

			// Miscellaneous
			unsigned checksum() const;
			void print() const;
			void printTree() const;
			unsigned height() const;
			void stat() const;

            uint16_t compute_packed_size();
            tree_node_handle repack( tree_node_allocator *allocator );

			// Operators
			bool operator<(const BranchNode &otherNode) const;
	};

    template <class NE, class B, int N>
	double computeOverlapGrowth(unsigned index, const
            std::array<B, N+1> &entries,
            unsigned els_to_consider,
            const Rectangle &givenBox) {
        // We cannot be a leaf
        assert(els_to_consider > 0 );
        
        // 1. Make a test rectangle we will use to not modify the original
        const Rectangle &origRectangle =
            entries[index].boundingBox;
        Rectangle newRectangle =
            entries[index].boundingBox;
        
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
                (newRectangle.computeIntersectionArea(entry.boundingBox)
                - origRectangle.computeIntersectionArea(entry.boundingBox));
        }

        return overlapDiff;

    }

#include "node.tcc"
}
