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
#include <omp.h>
#include <globals/globals.h>
#include <util/geometry.h>
#include <util/graph.h>
#include <util/debug.h>
#include <util/statistics.h>
#include <variant>

namespace nirtreedisk
{

    template <int min_branch_factor, int max_branch_factor>
    class NIRTreeDisk;

    template <int min_branch_factor, int max_branch_factor>
    tree_node_allocator *get_node_allocator(
            NIRTreeDisk<min_branch_factor,max_branch_factor> *treeRef ) {
        return &(treeRef->node_allocator_);
    }

    struct Branch
    {
        Branch( InlineBoundedIsotheticPolygon boundingPoly,
                tree_node_handle child ) : boundingPoly( boundingPoly ),
        child( child ) {}

        Branch( tree_node_handle boundingPoly, tree_node_handle child )
            : boundingPoly( boundingPoly ), child( child ) {}


        Rectangle get_summary_rectangle( tree_node_allocator *allocator ) {
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        boundingPoly ) ) {
                return std::get<InlineBoundedIsotheticPolygon>(
                        boundingPoly ).get_summary_rectangle();
            }
            tree_node_handle poly_handle = std::get<tree_node_handle>( boundingPoly );
            auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
            return poly_pin->get_summary_rectangle();
        }

        IsotheticPolygon materialize_polygon( tree_node_allocator
                *allocator ) {
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        boundingPoly ) ) {
                return std::get<InlineBoundedIsotheticPolygon>(
                        boundingPoly ).materialize_polygon();
            }
            tree_node_handle poly_handle = std::get<tree_node_handle>(
                    boundingPoly );
            auto poly_pin =
                allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                        poly_handle );
            return poly_pin->materialize_polygon();
        }

        std::variant<InlineBoundedIsotheticPolygon,tree_node_handle> boundingPoly;
        tree_node_handle child;

        bool operator==( const Branch &o ) const = default;
        bool operator!=( const Branch &o ) const = default;
    };

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
                tree_node_handle child;
				unsigned level;
			};

			NIRTreeDisk<min_branch_factor,max_branch_factor> *treeRef;

		public:
            typedef std::variant<Point, Branch> NodeEntry;

            tree_node_handle parent;
            std::array<NodeEntry, max_branch_factor+1> entries;
            size_t cur_offset_;
            tree_node_handle self_handle_;

			// Constructors and destructors
			Node(NIRTreeDisk<min_branch_factor,max_branch_factor> *treeRef, tree_node_handle parent,
                    tree_node_handle self_handle ) :
                treeRef( treeRef ),
                parent( parent ),
                cur_offset_( 0 ),
                self_handle_( self_handle ) {}
			void deleteSubtrees();

			// Helper functions
			bool isLeaf();
			Rectangle boundingBox();
			Branch &locateBranch(tree_node_handle child) {
                for( size_t i = 0; i < cur_offset_; i++ ) {
                    Branch &b = std::get<Branch>( entries.at(i) );
                    if (b.child == child)
                    {
                        return b;
                    }
                }
                // If we are here, panic
                assert(false);
            };

			void updateBranch(tree_node_handle child,  const InlineBoundedIsotheticPolygon &boundingPoly);
            void removeEntry( const NodeEntry &entry );
            void removeEntry( const tree_node_handle &handle );

            void addEntryToNode( const NodeEntry &entry ) {
                entries.at( cur_offset_++ ) = entry;
            }

			tree_node_handle chooseNode(Point givenPoint);
			tree_node_handle findLeaf(Point givenPoint);
			Partition partitionNode();
			SplitResult splitNode(Partition p);
			SplitResult splitNode();
			SplitResult adjustTree();
			void condenseTree();

			// Data structure interface functions
			void exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator);
			std::vector<Point> search(Point &requestedPoint);
			std::vector<Point> search(Rectangle &requestedRectangle);
			tree_node_handle insert(Point givenPoint);
			tree_node_handle remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			bool validate(tree_node_handle expectedParent, unsigned index);
			bool bounding_box_validate();
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();
	};

#include "node.tcc"
}
