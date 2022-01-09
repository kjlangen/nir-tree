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

    // Sets unsigned offset pointing to start of leaf entries and unsigned count
    #define decode_entry_count_and_offset_packed_node( data ) \
        [[maybe_unused]] unsigned offset = 0; \
        [[maybe_unused]] unsigned count = * (unsigned *) (data+offset); \
        offset += sizeof( unsigned );

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

            void update_repacked_parent(tree_node_handle parent, tree_node_handle old_child, tree_node_handle new_child ) {
                auto packed_parent = node_allocator_->get_tree_node<packed_node>( parent );
                char *buffer = packed_parent->buffer_;
                decode_entry_count_and_offset_packed_node( buffer );
                for ( unsigned i = 0; i < count; i++ ) {
                    tree_node_handle *child = (tree_node_handle *) (buffer + offset);
                    if (*child == old_child) {
                        *child = new_child;
                        return;
                    }
                    offset += sizeof( tree_node_handle );

                    bool is_compressed = child->get_associated_poly_is_compressed();
                    if ( is_compressed ) {

                        int new_offset;
                        IsotheticPolygon decoded_poly = decompress_polygon( (buffer + offset), &new_offset );
                        offset += new_offset;
                        continue;
                    }

                    unsigned rect_count = * (unsigned *) (buffer + offset);
                    offset += sizeof( unsigned );

                    if( rect_count == std::numeric_limits<unsigned>::max() ) {
                        offset += sizeof( tree_node_handle );
                    } else {
                        for( unsigned r = 0; r < rect_count; r++ ) {
                            offset += sizeof(Rectangle);
                        }

                    }
                }
            }

            inline Point get_a_contained_point( tree_node_handle
                    node_handle ) {
                tree_node_allocator *allocator = node_allocator_.get();
                while( node_handle.get_type() != LEAF_NODE and
                        node_handle.get_type() != REPACKED_LEAF_NODE ) {
                    if( node_handle.get_type() == BRANCH_NODE ) {
                        auto branch_node =
                            allocator->get_tree_node<BranchNode<min_branch_factor,max_branch_factor,strategy>>(
                                    node_handle );
                        Branch &b = branch_node->entries.at(0);
                        node_handle = b.child;
                    } else {
                        auto packed_branch_node =
                            allocator->get_tree_node<packed_node>(
                                    node_handle );
                        char *buffer = packed_branch_node->buffer_;
                        {
                            decode_entry_count_and_offset_packed_node( buffer );
                            node_handle = * (tree_node_handle *)
                                (buffer+offset);
                        }
                    }
                }
                if( node_handle.get_type() == LEAF_NODE ) {
                    auto leaf_node =
                        allocator->get_tree_node<LeafNode<min_branch_factor,max_branch_factor,strategy>>(
                            node_handle );
                    return leaf_node->entries.at(0);
                }
                // Decode and find offset
                auto leaf_node =
                    allocator->get_tree_node<packed_node>(
                            node_handle );
                char *buffer = leaf_node->buffer_;
                decode_entry_count_and_offset_packed_node( buffer );
                // First entry
                return * (Point *) (buffer+offset);
            }

            inline tree_node_handle get_parent_handle_for_repacked_node( tree_node_handle node_handle ) {
                Point p = get_a_contained_point( node_handle );
                tree_node_handle parent_handle = parent_handle_point_search( root, p, this, node_handle );
                return parent_handle;
            }

            inline pinned_node_ptr<LeafNode<min_branch_factor,max_branch_factor,strategy>>
            get_leaf_node(
                tree_node_handle node_handle,
                bool unpack_perm = true  /* If true, the tree will be modified */
            ) {

                assert( node_handle.get_type() == LEAF_NODE || node_handle.get_type() == REPACKED_LEAF_NODE );

                if ( node_handle.get_type() == REPACKED_LEAF_NODE ) {
                    tree_node_handle parent_handle = get_parent_handle_for_repacked_node( node_handle );

                    tree_node_handle unpacked_node_handle =
                        unpack<min_branch_factor,max_branch_factor,strategy>(node_handle,
                                node_allocator_.get(), this,
                                parent_handle
                         );
                    auto node = get_leaf_node( unpacked_node_handle );

                    // Update the parent!
                    if ( unpack_perm && node->parent ) {
                        // This does NOT unpack the parent
                        assert( node->parent == parent_handle );
                        if (node->parent.get_type() == REPACKED_BRANCH_NODE) {
                            update_repacked_parent( node->parent, node_handle, unpacked_node_handle );
                        } else {
                            auto parent = get_branch_node( node->parent );
                            Branch &b = parent->locateBranch( node_handle );
                            b.child = unpacked_node_handle;
                        }
                    }

                    if ( unpack_perm && root == node_handle) {
                        root = unpacked_node_handle;
                    }

                    node_handle = unpacked_node_handle;
                }

                assert( node_handle.get_type() == LEAF_NODE );
                auto ptr =
                    node_allocator_->get_tree_node<LeafNode<min_branch_factor,max_branch_factor,strategy>>( node_handle );
                ptr->treeRef = this;
                return ptr;
            }

            inline pinned_node_ptr<BranchNode<min_branch_factor,max_branch_factor,strategy>>
            get_branch_node(
                tree_node_handle node_handle,
                bool unpack_perm = true  /* If true, the tree will be modified */
            ) {

                assert( node_handle.get_type() == BRANCH_NODE || node_handle.get_type() == REPACKED_BRANCH_NODE );

                if( node_handle.get_type() == REPACKED_BRANCH_NODE ) {
                    tree_node_handle parent_handle = get_parent_handle_for_repacked_node( node_handle );
                    tree_node_handle unpacked_node_handle =
                        unpack<min_branch_factor,max_branch_factor,strategy>(node_handle,
                                node_allocator_.get(), this,
                                parent_handle );
                    auto node = get_branch_node( unpacked_node_handle );

                     // Update the parent!
                    if ( unpack_perm && node->parent ) {
                        assert( node->parent == parent_handle );
                        if (node->parent.get_type() == REPACKED_BRANCH_NODE) {
                            update_repacked_parent( node->parent, node_handle, unpacked_node_handle );
                        } else {
                            auto parent = get_branch_node( node->parent );
                            Branch &b = parent->locateBranch( node_handle );
                            b.child = unpacked_node_handle;
                        }
                    }

                    // Update the children!
                    if ( unpack_perm ) {
                        for ( unsigned i = 0; i < node->cur_offset_; i++ ) {
                            Branch &b = node->entries.at(i);

                            if (b.child.get_type() == REPACKED_LEAF_NODE || b.child.get_type() == REPACKED_BRANCH_NODE) {
                                continue;
                            }

                            // Branch or leaf?
                            if ( b.child.get_type() == LEAF_NODE) {
                                auto child = get_leaf_node( b.child );
                                child->parent = unpacked_node_handle;

                            } else {
                                // Branch
                                if ( b.child.get_type() != BRANCH_NODE ) {
                                    std::cerr << " LOOK " << (int)b.child.get_type() << std::endl;
                                }
                                auto child = get_branch_node( b.child );
                                child->parent = unpacked_node_handle;
                            }
                        }

                        if ( node_handle == root ) {
                            root = unpacked_node_handle;
                        }
                    }

                    node_handle = unpacked_node_handle;
                }

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
