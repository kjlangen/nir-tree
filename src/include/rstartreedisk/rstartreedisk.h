#pragma once
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>
#include <index/index.h>
#include <util/geometry.h>
#include <rstartreedisk/node.h>
#include <util/bmpPrinter.h>
#include <storage/tree_node_allocator.h>

namespace rstartreedisk
{
    template <int min_branch_factor, int max_branch_factor>
	class RStarTreeDisk : public Index
	{
		public:
			static constexpr float p = 0.3; // For reinsertion entries. 0.3 by default

			tree_node_handle root;
			Statistics stats;
            tree_node_allocator node_allocator_;

			std::vector<bool> hasReinsertedOnLevel;

			// Constructors and destructors
            RStarTreeDisk(size_t memory_budget, std::string backing_file
                    ) : node_allocator_( memory_budget, backing_file )
            {
                // Initialize buffer pool
                node_allocator_.initialize();

                hasReinsertedOnLevel = {false};

                /* We need to figure out if there was already data, and read
                 * that into memory if we have it. */

                size_t existing_page_count =
                    node_allocator_.buffer_pool_.get_preexisting_page_count();

                // If this is a fresh tree, then make a fresh root
                if( existing_page_count == 0 ) { 
                    std::pair<pinned_node_ptr<Node<min_branch_factor,max_branch_factor>>, tree_node_handle> alloc =
                        node_allocator_.create_new_tree_node<Node<min_branch_factor,max_branch_factor>>();
                    root = alloc.second;
                    new (&(*(alloc.first))) Node<min_branch_factor,max_branch_factor>( this, root, tree_node_handle() /*nullptr*/, 0
                            );
                    return;
                }

                // Hunt for the root
                for( size_t i = 0; i < existing_page_count; i++ ) {
                    page *p = node_allocator_.buffer_pool_.get_page( i );
                    node_allocator_.buffer_pool_.pin_page( p );
                    char *page_bytes = p->data_;
                    for( size_t offset_multiplier = 0; offset_multiplier <
                            (PAGE_DATA_SIZE /
                             sizeof(Node<min_branch_factor,max_branch_factor>));
                             offset_multiplier++ ) { 
                        Node<min_branch_factor,max_branch_factor> *interpreted_ptr =
                            (Node<min_branch_factor,max_branch_factor> *) (page_bytes +
                                offset_multiplier * sizeof(
                                    Node<min_branch_factor,max_branch_factor> ));
                        if( interpreted_ptr->parent == tree_node_handle() ) {
                            // Found the root
                            root = tree_node_handle( i, offset_multiplier *
                                    sizeof( Node<min_branch_factor,max_branch_factor> ));
                            node_allocator_.buffer_pool_.unpin_page( p );
                            return;
                        } 
                    }
                    node_allocator_.buffer_pool_.unpin_page( p );
                }

                assert( false );
            }

			~RStarTreeDisk() {
                pinned_node_ptr<Node<min_branch_factor,max_branch_factor>> root_ptr =
                    node_allocator_.get_tree_node<Node<min_branch_factor,max_branch_factor>>( root );
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
			void print();
			bool validate();
			void stat();
			void visualize();
	};

#include "rstartreedisk.tcc"

}
