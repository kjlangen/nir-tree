#ifndef __RPLUSNODE__
#define __RPLUSNODE__

#include <cassert>
#include <vector>
#include <stack>
#include <unordered_map>
#include <limits>
#include <list>
#include <queue>
#include <utility>
#include <cmath>
#include <cstring>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <util/geometry.h>
#include <util/graph.h>
#include <util/debug.h>
#include <util/statistics.h>

namespace rplustreedisk
{

	// Forward ref
    template <int min_branch_factor, int max_branch_factor>
	class RPlusTreeDisk;

    template <int min_branch_factor, int max_branch_factor>
    tree_node_allocator *get_node_allocator(
            RPlusTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return &(treeRef->node_allocator_);
    }

    template <int min_branch_factor, int max_branch_factor>
    tree_node_handle get_root_handle(
            RPlusTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return treeRef->root_;
    }

    struct Branch
    {

        Branch( tree_node_handle child, const Rectangle &boundingBox ) :
            boundingBox( boundingBox ), 
            child( child )
        {}

        Branch &operator=( const Branch &other ) = default;
        bool operator==( const Branch &other ) const = default;

        Rectangle boundingBox;
        tree_node_handle child;

    };
    typedef std::variant<Point, Branch> NodeEntry;

    struct SplitResult
    {
        Branch leftBranch;
        Branch rightBranch;
    };

    struct Partition
    {
        unsigned dimension;
        double location;
    };


    template <int min_branch_factor, int max_branch_factor>
	class Node
	{
		private:
			struct ReinsertionEntry
			{
				Rectangle boundingBox;
				Point data;
				Node *child;
				unsigned level;
			};

		public:
			RPlusTreeDisk<min_branch_factor,max_branch_factor> *treeRef;

            tree_node_handle self_handle_;
			tree_node_handle parent_;
            unsigned cur_offset_ = 0;
            typename std::array<NodeEntry, max_branch_factor+1> entries;

			// Constructors and destructors
			Node(RPlusTreeDisk<min_branch_factor,max_branch_factor> *treeRef, tree_node_handle self_handle,
                    tree_node_handle parent_handle) :
                treeRef( treeRef ),
                self_handle_( self_handle ),
                parent_( parent_handle ),
                cur_offset_( 0 ) {}

			void deleteSubtrees();

			// Helper functions
			Rectangle boundingBox();
			void updateBranch(tree_node_handle child, Rectangle &boundingBox);
			void removeBranch(tree_node_handle child);
			void removePoint(Point &givenPoint);
			tree_node_handle chooseNode(Point givenPoint);
			tree_node_handle findLeaf(Point givenPoint);
			Partition partitionNode();
			SplitResult splitNode(Partition p);
			SplitResult splitNode();
			SplitResult adjustTree();
			void pushDown(Point givenPoint);
			void condenseTree();

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			tree_node_handle insert(Point givenPoint);
			tree_node_handle remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate( tree_node_handle expectedParent, unsigned index );
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();

            bool isLeaf() {
                return cur_offset_ == 0 or std::holds_alternative<Point>( entries.at(0) );
            }
	};
#include "node.tcc"
}

#endif
