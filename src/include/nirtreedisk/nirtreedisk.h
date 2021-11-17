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
#include <memory>
#include <util/geometry.h>
#include <index/index.h>
#include <storage/tree_node_allocator.h>
#include <util/bmpPrinter.h>
#include <util/statistics.h>
#include <nirtreedisk/node.h>

#include <unistd.h>
#include <fcntl.h>

namespace nirtreedisk
{

    template <int min_branch_factor, int max_branch_factor,
             class strategy = LineMinimizeDownsplits>
	class NIRTreeDisk: public Index
	{
		public:
            tree_node_handle root;
            std::unique_ptr<tree_node_allocator> node_allocator_;
			std::vector<bool> hasReinsertedOnLevel;

			Statistics stats;

			// Constructors and destructors
			NIRTreeDisk( size_t memory_budget, std::string backing_file  ) :
                node_allocator_( std::make_unique<tree_node_allocator>(
                            memory_budget, backing_file ) )
            {
                node_allocator_->initialize();

                size_t existing_page_count =
                    node_allocator_->buffer_pool_.get_preexisting_page_count();

                hasReinsertedOnLevel = {false};
                // If this is a fresh tree, we need a root
                if( existing_page_count == 0 ) { 
                    auto alloc =
                        node_allocator_->create_new_tree_node<LeafNode<min_branch_factor,max_branch_factor,strategy>>(
                                NodeHandleType(LEAF_NODE) );
                    root = alloc.second;
                    new (&(*(alloc.first)))
                        LeafNode<min_branch_factor,max_branch_factor,strategy>( this,
                                tree_node_handle(nullptr), root, 0 );

                    return;
                }

                std::string meta_file = backing_file + ".meta";
                int fd = open( meta_file.c_str(), O_RDONLY );
                assert( fd >= 0 );

                int rc = read( fd, (char *) &root, sizeof( root ) );
                assert( rc == sizeof( root ) );
            }

			~NIRTreeDisk() {
                //auto root_node = node_allocator_.get_tree_node<NodeType>( root );
                //root_node->deleteSubtrees();
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

            inline pinned_node_ptr<LeafNode<min_branch_factor,max_branch_factor,strategy>>
                get_leaf_node( tree_node_handle node_handle ) {
                    assert( node_handle.get_type() == LEAF_NODE );
                auto ptr =
                    node_allocator_->get_tree_node<LeafNode<min_branch_factor,max_branch_factor,strategy>>( node_handle );
                ptr->treeRef = this;
                return ptr;
            }

            inline pinned_node_ptr<BranchNode<min_branch_factor,max_branch_factor,strategy>>
                get_branch_node( tree_node_handle node_handle ) {
                    assert( node_handle.get_type() == BRANCH_NODE );
                auto ptr =
                    node_allocator_->get_tree_node<BranchNode<min_branch_factor,max_branch_factor,strategy>>( node_handle );
                ptr->treeRef = this;
                return ptr;
            }


            void write_metadata() override {
                // Step 1:
                // Writeback everything to disk
                node_allocator_->buffer_pool_.writeback_all_pages();

                // Step 2:
                // Write metadata file
                std::string meta_fname =
                    node_allocator_->get_backing_file_name() + ".meta";
                int fd = open( meta_fname.c_str(), O_WRONLY |
                        O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR );
                assert( fd >= 0 );
                // yes, yes, i should loop this whatever
                int rc = write( fd, (char *) &root, sizeof(root) );
                assert( rc == sizeof(root) );
                close( fd );
            }
	};
#include "nirtreedisk.tcc"
}
