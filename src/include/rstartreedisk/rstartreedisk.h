#pragma once
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <memory>
#include <index/index.h>
#include <util/geometry.h>
#include <rstartreedisk/node.h>
#include <util/bmpPrinter.h>
#include <storage/tree_node_allocator.h>

#include <unistd.h>
#include <fcntl.h>

namespace rstartreedisk
{
    template <int min_branch_factor, int max_branch_factor>
	class RStarTreeDisk : public Index
	{
		public:
			static constexpr float p = 0.3; // For reinsertion entries. 0.3 by default

			tree_node_handle root;
			Statistics stats;
            std::unique_ptr<tree_node_allocator> node_allocator_;
            std::string backing_file_;

			std::vector<bool> hasReinsertedOnLevel;

			// Constructors and destructors
            RStarTreeDisk(size_t memory_budget, std::string backing_file
                    ) : backing_file_( backing_file )
            {

                node_allocator_ = std::make_unique<tree_node_allocator>(
                        memory_budget, backing_file );
                // Initialize buffer pool
                node_allocator_->initialize();

                hasReinsertedOnLevel = {false};

                /* We need to figure out if there was already data, and read
                 * that into memory if we have it. */

                size_t existing_page_count =
                    node_allocator_->buffer_pool_.get_preexisting_page_count();

                // If this is a fresh tree, then make a fresh root
                if( existing_page_count == 0 ) { 
                    auto alloc =
                        node_allocator_->create_new_tree_node<LeafNode<min_branch_factor,max_branch_factor>>(
                                NodeHandleType( LEAF_NODE ) );
                    root = alloc.second;
                    new (&(*(alloc.first)))
                        LeafNode<min_branch_factor,max_branch_factor>(
                                this, root, tree_node_handle( nullptr ), 0
                            );
                    return;
                }

                std::string meta_file = backing_file_ + ".meta";
                int fd = open( meta_file.c_str(), O_RDONLY );
                assert( fd >= 0 );

                int rc = read( fd, (char *) &root, sizeof( root ) );
                assert( rc == sizeof( root ) );

            }

			~RStarTreeDisk() {
            };

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint);
			std::vector<Point> search(Rectangle requestedRectangle);
			void insert(Point givenPoint);
			void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			void print();
			bool validate();
			void stat();
			void visualize();

            inline pinned_node_ptr<LeafNode<min_branch_factor,max_branch_factor>> get_leaf_node( tree_node_handle node_handle ) {
                assert( node_handle.get_type() == LEAF_NODE );
                auto ptr =
                    node_allocator_->get_tree_node<LeafNode<min_branch_factor,max_branch_factor>>(
                            node_handle );
                ptr->treeRef = this;
                return ptr;
            }

            inline pinned_node_ptr<BranchNode<min_branch_factor,max_branch_factor>> get_branch_node( tree_node_handle node_handle ) {
                assert( node_handle.get_type() == BRANCH_NODE );
                auto ptr =
                    node_allocator_->get_tree_node<BranchNode<min_branch_factor,max_branch_factor>>(
                            node_handle );
                ptr->treeRef = this;
                return ptr;
            }


            void write_metadata() {
                // Step 1:
                // Writeback everything to disk
                node_allocator_->buffer_pool_.writeback_all_pages();

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

#include "rstartreedisk.tcc"
}
