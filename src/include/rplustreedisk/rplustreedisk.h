#ifndef __RPLUSTREE__
#define __RPLUSTREE__

#include <cassert>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <iostream>
#include <utility>
#include <util/geometry.h>
#include <rplustreedisk/node.h>
#include <index/index.h>
#include <util/statistics.h>
#include <unistd.h>
#include <fcntl.h>

namespace rplustreedisk
{
    template <int min_branch_factor, int max_branch_factor>
	class RPlusTreeDisk: public Index
	{
		public:
			tree_node_handle root_;
            tree_node_allocator node_allocator_;
            std::string backing_file_;
#ifdef STAT
			Statistics stats;
#endif

			// Constructors and destructors
			RPlusTreeDisk( size_t memory_budget, const std::string &backing_file )
                : node_allocator_( memory_budget, backing_file ),
                backing_file_( backing_file )
            {
                node_allocator_.initialize();

                size_t existing_page_count =
                    node_allocator_.buffer_pool_.get_preexisting_page_count();

                // If existing page count is zero, then create a new
                // root node.
                if( existing_page_count == 0 ) {
                    auto alloc_data =
                        node_allocator_.create_new_tree_node<Node<min_branch_factor,max_branch_factor>>();
                    root_ = alloc_data.second;
                    new (&(*(alloc_data.first)))
                        Node<min_branch_factor,max_branch_factor>( this,
                                root_, tree_node_handle( nullptr ) );
                    return;
                }

                // Find existing root node.
                std::string meta_file = backing_file_ + ".meta";
                int fd = open( meta_file.c_str(), O_RDONLY );
                assert( fd >= 0 );

                int rc = read( fd, (char *) &root_, sizeof( root_ ) );
                assert( rc == sizeof( root_ ) );

            }

			~RPlusTreeDisk();

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

	};
#include "rplustreedisk.tcc"
}

#endif
