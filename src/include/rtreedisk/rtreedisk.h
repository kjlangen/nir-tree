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
#include <unistd.h>
#include <fcntl.h>

namespace rtreedisk
{
    template <int min_branch_factor, int max_branch_factor>
    class RTreeDisk : public Index
    {
    public:
        tree_node_handle root;
        Statistics stats;
        tree_node_allocator node_allocator_;
        std::string backing_file_;

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

        inline pinned_node_ptr<Node<min_branch_factor,max_branch_factor>> get_node( tree_node_handle node_handle ) {
            auto ptr =
                node_allocator_.get_tree_node<Node<min_branch_factor,max_branch_factor>>(
                        node_handle );
            ptr->treeRef = this;
            return ptr;
        }


        void write_metadata() override {
            // Step 1:
            // Writeback everything to disk
            node_allocator_.buffer_pool_.writeback_all_pages();

            // Step 2:
            // Write metadata file
            std::string meta_fname = backing_file_ + ".meta";
            int fd = open( meta_fname.c_str(), O_WRONLY |
                    O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
            assert( fd >= 0 );
            // yes, yes, i should loop this whatever
            int rc = write( fd, (char *) &root, sizeof(root) );
            assert( rc == sizeof(root) );
            close( fd );
        }

    };
#include "rtreedisk.tcc"

}
