#pragma once

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
#include <storage/tree_node_allocator.h>

namespace rtreedisk
{
    template <int min_branch_factor, int max_branch_factor>
    class RTreeDisk;

    template <int min_branch_factor, int max_branch_factor>
    tree_node_allocator *get_node_allocator(
        RTreeDisk<min_branch_factor, max_branch_factor> *treeRef)
    {
        return &(treeRef->node_allocator_);
    }

    template <int min_branch_factor, int max_branch_factor>
    float get_p_value(
        RTreeDisk<min_branch_factor, max_branch_factor> *treeRef)
    {
        return treeRef->p;
    }

    template <int min_branch_factor, int max_branch_factor>
    tree_node_handle get_root_handle(
        RTreeDisk<min_branch_factor, max_branch_factor> *treeRef)
    {
        return treeRef->root;
    }

    template <int min_branch_factor, int max_branch_factor>
    class Node
    {
        class ReinsertionEntry
        {
        public:
            Rectangle boundingBox;
            Point data;
            tree_node_handle child;
            unsigned level;
        };

        RTreeDisk<min_branch_factor, max_branch_factor> *treeRef;

    public:
        tree_node_handle parent;
        tree_node_handle self_handle_;

        unsigned cur_offset_ = 0;      // For children + boxes
        unsigned cur_offset_data_ = 0; // For data (leaf)
        inline bool isLeafNode() const { return cur_offset_ == 0; }
        typename std::array<Rectangle, max_branch_factor + 1> boundingBoxes;
        typename std::array<tree_node_handle, max_branch_factor + 1> children;
        typename std::array<Point, max_branch_factor + 1> data;

        // Constructors and destructors
        Node(RTreeDisk<min_branch_factor, max_branch_factor> *treeRef, tree_node_handle self_handle);
        Node(RTreeDisk<min_branch_factor, max_branch_factor> *treeRef, tree_node_handle self_handle, tree_node_handle parent);
        void deleteSubtrees();

        // Helper functions
        Rectangle boundingBox();
        void updateBoundingBox(tree_node_handle child, Rectangle updatedBoundingBox);
        void removeChild(tree_node_handle child);
        void removeData(Point givenPoint);
        void removeChild(unsigned idx);
        void removeData(unsigned idx);
        tree_node_handle chooseLeaf(Point givenPoint);
        tree_node_handle chooseNode(ReinsertionEntry e);
        tree_node_handle findLeaf(Point givenPoint);
        void moveData(unsigned fromIndex, std::vector<Point> &toData);
        void moveChild(unsigned fromIndex, std::vector<Rectangle> &toRectangles, std::vector<tree_node_handle> &toChildren);
        tree_node_handle splitNode(tree_node_handle newChildHandle);
        tree_node_handle splitNode(Point newData);
        tree_node_handle adjustTree(tree_node_handle siblingLeaf);
        tree_node_handle condenseTree();
        tree_node_handle insert(ReinsertionEntry e);

        // Datastructure interface functions
        void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
        std::vector<Point> search(Point &requestedPoint);
        std::vector<Point> search(Rectangle &requestedRectangle);
        tree_node_handle insert(Point givenPoint);
        tree_node_handle remove(Point givenPoint);

        // Miscellaneous
        unsigned checksum();
        bool validate(tree_node_handle expectedParent, unsigned index);
        void print(unsigned n = 0);
        void printTree(unsigned n = 0);
        unsigned height();
        void stat();
    };

#include "node.tcc"
}
