#pragma once

#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <rtreedisk/node.h>
#include <index/index.h>
#include <util/bmpPrinter.h>
#include <storage/tree_node_allocator.h>

namespace rtreedisk
{
    template <int min_branch_factor, int max_branch_factor>
    class RTreeDisk : public Index
    {
    public:
        tree_node_handle root;
        Statistics stats;
        tree_node_allocator node_allocator_;

        // Constructors and destructors
        RTreeDisk(size_t memory_budget, std::string backing_file);
        //RTreeDisk(tree_node_handle root);
        ~RTreeDisk()
        {
            pinned_node_ptr<Node<min_branch_factor, max_branch_factor>> root_ptr =
                node_allocator_.get_tree_node<Node<min_branch_factor, max_branch_factor>>(root);
            root_ptr->deleteSubtrees();
        };

        // Datastructure interface
        std::vector<Point> exhaustiveSearch(Point requestedPoint);
        std::vector<Point> search(Point requestedPoint);
        std::vector<Point> search(Rectangle requestedRectangle);
        void insert(Point givenPoint);
        void remove(Point givenPoint);

        // Miscellaneous
        unsigned checksum();
        bool validate();
        void stat();
        void print();
        void visualize();
    };
#include "rtreedisk.tcc"

}