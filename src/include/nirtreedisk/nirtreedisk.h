#pragma once

// Copyright 2021 Kyle Langendoen

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <cassert>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <index/index.h>
#include <storage/tree_node_allocator.h>
#include <util/bmpPrinter.h>
#include <util/statistics.h>

#include <nirtreedisk/node.h>

namespace nirtreedisk
{
    template <int min_branch_factor, int max_branch_factor>
	class NIRTreeDisk: public Index
	{
		public:
            tree_node_handle root;
            tree_node_allocator node_allocator_;
			Statistics stats;

			// Constructors and destructors
			NIRTreeDisk( size_t memory_budget, std::string backing_file  ) :
                node_allocator_( memory_budget, backing_file ) {
                    node_allocator_.initialize();
                size_t existing_page_count =
                    node_allocator_.buffer_pool_.get_preexisting_page_count();

                //If this is a fresh tree, we need a root
                if( existing_page_count == 0 ) { 
                    auto alloc =
                        node_allocator_.create_new_tree_node<Node<min_branch_factor,max_branch_factor>>();
                    root = alloc.second;
                    new (&(*(alloc.first)))
                        Node<min_branch_factor,max_branch_factor>( this,
                                tree_node_handle(nullptr), root );
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

			~NIRTreeDisk() {
                using NodeType =
                    Node<min_branch_factor,max_branch_factor>;
                auto root_node = node_allocator_.get_tree_node<NodeType>( root );
                root_node->deleteSubtrees();
                // FIXME: Free root_node
            }

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
#include "nirtreedisk.tcc"
}
