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
    struct BranchPartitionStrategy {};

    struct LineMinimizeDownsplits : BranchPartitionStrategy {};

    struct LineMinimizeDistanceFromMean : BranchPartitionStrategy {};

    struct ExperimentalStrategy : BranchPartitionStrategy {};

    template <int min_branch_factor, int max_branch_factor,
             class strategy>
    class NIRTreeDisk;

    template <int min_branch_factor, int max_branch_factor,
             class strategy>
    requires (std::derived_from<strategy,BranchPartitionStrategy>)
    tree_node_allocator *get_node_allocator(
            NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef ) {
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

    template <int min_branch_factor, int max_branch_factor,
             class strategy>
    requires (std::derived_from<strategy,BranchPartitionStrategy>)
    class Node {
		private:
			struct ReinsertionEntry
			{
				Rectangle boundingBox;
				Point data;
                tree_node_handle child;
				unsigned level;
			};

		public:
			NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef;
            typedef std::variant<Point, Branch> NodeEntry;

            tree_node_handle parent;
            std::array<NodeEntry, max_branch_factor+1> entries;
            size_t cur_offset_;
            tree_node_handle self_handle_;

			// Constructors and destructors
			Node(NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef, tree_node_handle parent,
                    tree_node_handle self_handle ) :
                treeRef( treeRef ),
                parent( parent ),
                cur_offset_( 0 ),
                self_handle_( self_handle ) {
            }
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
			Partition partitionLeafNode();

            template <class S = strategy>
            Partition partitionBranchNode( typename
                    std::enable_if<std::is_same<S,LineMinimizeDownsplits>::value,S>::type
                    * = 0 ) {
                Partition defaultPartition;

                tree_node_allocator *allocator = get_node_allocator( treeRef );
                std::vector<Rectangle> all_branch_polys;
                for( unsigned i = 0; i < cur_offset_; i++ ) {
                    Branch &b_i = std::get<Branch>( entries.at(i) );
                    all_branch_polys.push_back( b_i.get_summary_rectangle(
                                allocator ) );
                }

                double best_candidate = 0.0;
                double min_cost = std::numeric_limits<double>::max();
                unsigned best_dimension = 0;
                // D * ( M LOG M + M ) ~> O( D M LOG M )
                for( unsigned d = 0; d < dimensions; d++ ) {
                    std::sort( all_branch_polys.begin(), all_branch_polys.end(),
                            [d](Rectangle &poly1, Rectangle &poly2
                                ) {
                                return poly1.upperRight[d] <
                                poly2.upperRight[d];
                            }
                    );
                    for( unsigned i = 0; i < all_branch_polys.size(); i++ ) {
                        double cost = 0;
                        // starts at 1 cause first goes left
                        // Technically we should also walk the bottom bounds to
                        // be sure, even in the non F, C case.
                        unsigned left_count = 1;
                        unsigned right_count = 0; 
                        double partition_candidate =
                            all_branch_polys.at(i).upperRight[d];
                        double running_total = 0.0;
                        // Existing metric wanted to avoid recursive splits
                        // Let's try and do the same
                        for( unsigned j = 0; j < all_branch_polys.size(); j++ ) {
                            Rectangle &poly_ref = all_branch_polys.at(j);
                            running_total += poly_ref.lowerLeft[d] +
                                poly_ref.upperRight[d];

                            if( i != j ) {
                                bool greater_than_left = poly_ref.lowerLeft[d] <
                                    partition_candidate;
                                bool less_than_right = partition_candidate <
                                    poly_ref.upperRight[d];
                                bool requires_split = greater_than_left and
                                    less_than_right;

                                bool should_go_left = poly_ref.upperRight[d] <=
                                    partition_candidate;
                                bool should_go_right = poly_ref.lowerLeft[d] >=
                                    partition_candidate;

                                if( requires_split ) {
                                    left_count++;
                                    right_count++;
                                    cost++;
                                } else if( should_go_left ) {
                                    // the zero area polys end up here too
                                    left_count++;
                                } else if( should_go_right ) {
                                    right_count++;
                                }

                            }
                        }
                        if( cost < min_cost and left_count <= max_branch_factor
                                and right_count <= max_branch_factor and
                                left_count > 0 and right_count > 0 ) {
                            best_candidate = partition_candidate;
                            best_dimension = d;
                            min_cost = cost;
                        }
                    }
                }
                // Degenerate case
                assert( min_cost < std::numeric_limits<double>::max() );

                defaultPartition.dimension = best_dimension;
                defaultPartition.location = best_candidate;
                    
                return defaultPartition;
            }

            template <class S = strategy>
            Partition partitionBranchNode(typename
                    std::enable_if<std::is_same<S,LineMinimizeDistanceFromMean>::value,
                    S>::type * = 0) {
                Partition defaultPartition;

                tree_node_allocator *allocator = get_node_allocator( treeRef );
                std::vector<Rectangle> all_branch_polys;
                for( unsigned i = 0; i < cur_offset_; i++ ) {
                    Branch &b_i = std::get<Branch>( entries.at(i) );
                    all_branch_polys.push_back( b_i.get_summary_rectangle(
                                allocator ) );
                }

                double best_candidate = 0.0;
                double min_cost = std::numeric_limits<double>::max();
                unsigned best_dimension = 0;
                // D * ( M LOG M + M ) ~> O( D M LOG M )
                for( unsigned d = 0; d < dimensions; d++ ) {
                    std::sort( all_branch_polys.begin(), all_branch_polys.end(),
                            [d](Rectangle &poly1, Rectangle &poly2
                                ) {
                                if( poly1.upperRight[d] ==
                                        poly2.upperRight[d]  ) {
                                    for( unsigned i = 0; i < dimensions;
                                            i++ ) {
                                        if( poly1.upperRight[i] ==
                                                poly2.upperRight[i] ) {
                                            continue;
                                        }
                                        return poly1.upperRight[i] <
                                            poly2.upperRight[i];
                                    }
                                }
                                return poly1.upperRight[d] <
                                poly2.upperRight[d];
                            }
                    );
                    for( unsigned i = 0; i < all_branch_polys.size(); i++ ) {
                        double cost = 0;
                        // starts at 1 cause {i} goes left
                        // Technically we should also walk the bottom bounds to
                        // be sure, even in the non F, C case.
                        unsigned left_count = 0;
                        unsigned right_count = 0; 
                        double partition_candidate =
                            all_branch_polys.at(i).upperRight[d];
                        double running_total = 0.0;
                        // Existing metric wanted to avoid recursive splits
                        // Let's try and do the same
                        for( unsigned j = 0; j < all_branch_polys.size(); j++ ) {
                            Rectangle &poly_ref = all_branch_polys.at(j);
                            running_total += poly_ref.lowerLeft[d] +
                                poly_ref.upperRight[d];

                            bool greater_than_left = poly_ref.lowerLeft[d] <
                                partition_candidate;
                            bool less_than_right = partition_candidate <
                                poly_ref.upperRight[d];
                            bool requires_split = greater_than_left and
                                less_than_right;

                            bool should_go_left = poly_ref.upperRight[d] <=
                                partition_candidate;
                            bool should_go_right = poly_ref.lowerLeft[d] >=
                                partition_candidate;
                            bool is_zero_area =
                                poly_ref.lowerLeft[d] ==
                                poly_ref.upperRight[d];

                            if( requires_split ) {
                                left_count++;
                                right_count++;
                                cost++;
                            } else if( is_zero_area and
                                    poly_ref.upperRight[d] ==
                                    partition_candidate ) {
                                // Partition on a zero-area thing, can
                                // pick either side as convenient
                                if( left_count <= right_count ) {
                                    left_count++;
                                } else {
                                    right_count++;
                                }
                            } else if( should_go_left ) {
                                left_count++;
                            } else if( should_go_right ) {
                                right_count++;
                            } else {
                                assert( false );
                            }
                        }

                        // Cost function 2
                        // If the split will not overflow our children
                        if( left_count <= max_branch_factor
                            and right_count <= max_branch_factor and
                            left_count > 0 and right_count > 0 )
                        {
                            double mean_d_pt = running_total /
                                (2*all_branch_polys.size());
                            // Distance
                            cost = (mean_d_pt-partition_candidate);
                            cost = cost * cost;
                            if( cost < min_cost ) {
                                best_candidate = partition_candidate;
                                best_dimension = d;
                                min_cost = cost;
                            }
                        }

                    }
                }
                // Degenerate case
                assert( min_cost < std::numeric_limits<double>::max() );

                defaultPartition.dimension = best_dimension;
                defaultPartition.location = best_candidate;

                // Sort per the dimension we need
                std::sort( entries.begin(), entries.begin() +
                        cur_offset_,
                        [this,allocator,best_dimension](NodeEntry &entry1, NodeEntry &entry2 ) {
                            Branch &b1 = std::get<Branch>( entry1 );
                            Branch &b2 = std::get<Branch>( entry2 );
                            Rectangle poly1 = b1.get_summary_rectangle(
                                    allocator );
                            Rectangle poly2 = b2.get_summary_rectangle(
                                    allocator );
                            if( poly1.upperRight[best_dimension] ==
                                    poly2.upperRight[best_dimension]  ) {
                                for( unsigned i = 0; i < dimensions; i++ ) {
                                    if( poly1.upperRight[i] ==
                                            poly2.upperRight[i] ) {
                                        continue;
                                    }
                                    return poly1.upperRight[i] <
                                        poly2.upperRight[i];
                                }
                            }
                            return poly1.upperRight[best_dimension] <
                            poly2.upperRight[best_dimension];
                        }
                );

                    
                return defaultPartition;
            }

            std::pair<bool, Partition> try_cut_geo_mean(
                    std::vector<Rectangle> &all_branch_polys
            ) {
                Partition defaultPartition;
                Point mean_point = Point::atOrigin;

                double mass = 0.0;
                for( auto &branch_bounding_box : all_branch_polys ) {
                    mean_point += branch_bounding_box.lowerLeft;
                    mean_point += branch_bounding_box.upperRight;
                    mass += 2.0;
                }

                mean_point /= mass;

                unsigned best_cost =
                    std::numeric_limits<unsigned>::max();

                // Need to determine left and right count

                for( unsigned d = 0; d < dimensions; d++ ) {
                    // Is this a valid split?
                    double location = mean_point[d];
                    unsigned cost = 0;
                    unsigned left_count = 0;
                    unsigned right_count = 0;
                    for( auto &branch_bounding_box : all_branch_polys ) {
                        bool greater_than_left = branch_bounding_box.lowerLeft[d] <
                            location;
                        bool less_than_right = location <
                            branch_bounding_box.upperRight[d];
                        bool requires_split = greater_than_left and
                            less_than_right;

                        bool should_go_left = branch_bounding_box.upperRight[d]
                            <= location;
                        bool should_go_right = branch_bounding_box.lowerLeft[d]
                            >= location;
                        assert( not (should_go_left and should_go_right)
                                );


                        if( requires_split ) {
                            left_count++;
                            right_count++;
                            cost++;
                        } else if( should_go_left ) {
                            left_count++;
                        } else if( should_go_right ) {
                            right_count++;
                        } else {
                            assert( false );
                        }
                    }

                    if( left_count > 0 and right_count > 0 and
                            left_count <= max_branch_factor and
                            right_count <= max_branch_factor ) {
                        if( cost < best_cost ) {
                            best_cost = cost;
                            defaultPartition.location = mean_point[d];
                            defaultPartition.dimension = d;
                        }
                    }
                }

                if( best_cost < std::numeric_limits<unsigned>::max() ) {
                    return std::make_pair( true, defaultPartition );
                }
                return std::make_pair( false, defaultPartition );
            }

            template <class S = strategy>
            Partition partitionBranchNode(typename
                    std::enable_if<std::is_same<S,ExperimentalStrategy>::value,
                    S>::type * = 0) {
                Partition defaultPartition;

                tree_node_allocator *allocator = get_node_allocator( treeRef );
                std::vector<Rectangle> all_branch_polys;
                for( unsigned i = 0; i < cur_offset_; i++ ) {
                    Branch &b_i = std::get<Branch>( entries.at(i) );
                    all_branch_polys.push_back( b_i.get_summary_rectangle(
                                allocator ) );
                }

                auto geo_cut = try_cut_geo_mean( all_branch_polys );
                if( geo_cut.first ) {
                    return geo_cut.second;
                }
                // Can we cut along the geometric mean in any dimension
                // without overflowing our children?

                // If that didn't work, we gotta try something else.
                for( unsigned d = 0; d < dimensions; d++ ) {
                    std::sort( all_branch_polys.begin(), all_branch_polys.end(),
                            [d](Rectangle &poly1, Rectangle &poly2
                                ) {
                                if( poly1.upperRight[d] ==
                                        poly2.upperRight[d]  ) {
                                    for( unsigned i = 0; i < dimensions;
                                            i++ ) {
                                        if( poly1.upperRight[i] ==
                                                poly2.upperRight[i] ) {
                                            continue;
                                        }
                                        return poly1.upperRight[i] <
                                            poly2.upperRight[i];
                                    }
                                }
                                return poly1.upperRight[d] <
                                poly2.upperRight[d];
                            }
                    );


                }

                double best_candidate = 0.0;
                double min_cost = std::numeric_limits<double>::max();
                unsigned best_dimension = 0;
                // D * ( M LOG M + M ) ~> O( D M LOG M )
                for( unsigned d = 0; d < dimensions; d++ ) {
                    std::sort( all_branch_polys.begin(), all_branch_polys.end(),
                            [d](Rectangle &poly1, Rectangle &poly2
                                ) {
                                if( poly1.upperRight[d] ==
                                        poly2.upperRight[d]  ) {
                                    for( unsigned i = 0; i < dimensions;
                                            i++ ) {
                                        if( poly1.upperRight[i] ==
                                                poly2.upperRight[i] ) {
                                            continue;
                                        }
                                        return poly1.upperRight[i] <
                                            poly2.upperRight[i];
                                    }
                                }
                                return poly1.upperRight[d] <
                                poly2.upperRight[d];
                            }
                    );
                    for( unsigned i = 0; i < all_branch_polys.size(); i++ ) {
                        double cost = 0;
                        // starts at 1 cause {i} goes left
                        // Technically we should also walk the bottom bounds to
                        // be sure, even in the non F, C case.
                        unsigned left_count = 0;
                        unsigned right_count = 0; 
                        double partition_candidate =
                            all_branch_polys.at(i).upperRight[d];
                        double running_total = 0.0;
                        // Existing metric wanted to avoid recursive splits
                        // Let's try and do the same
                        for( unsigned j = 0; j < all_branch_polys.size(); j++ ) {
                            Rectangle &poly_ref = all_branch_polys.at(j);
                            running_total += poly_ref.lowerLeft[d] +
                                poly_ref.upperRight[d];

                            bool greater_than_left = poly_ref.lowerLeft[d] <
                                partition_candidate;
                            bool less_than_right = partition_candidate <
                                poly_ref.upperRight[d];
                            bool requires_split = greater_than_left and
                                less_than_right;

                            bool should_go_left = poly_ref.upperRight[d]
                                <= partition_candidate;
                            bool should_go_right = poly_ref.lowerLeft[d]
                                >= partition_candidate;
                            assert( not( should_go_left and
                                        should_go_right ) );
                            bool is_zero_area =
                                poly_ref.lowerLeft[d] ==
                                poly_ref.upperRight[d];

                            if( requires_split ) {
                                //std::cout << "SIMUL: entry requires split." << std::endl;
                                left_count++;
                                right_count++;
                                cost++;
                            } else if( is_zero_area and
                                    poly_ref.upperRight[d] ==
                                    partition_candidate ) {
                                assert( false );
                                // Partition on a zero-area thing, can
                                // pick either side as convenient
                                if( left_count <= right_count ) {
                                    //std::cout << "SIMUL: entry contest, goes left." << std::endl;
                                    left_count++;
                                } else {
                                    //std::cout << "SIMUL: entry contest, goes right." << std::endl;
                                    right_count++;
                                }
                            } else if( should_go_left ) {
                                //std::cout << "SIMUL: entry goes left." << std::endl;
                                left_count++;
                            } else if( should_go_right ) {
                                //std::cout << "SIMUL: entry goes right." << std::endl;
                                right_count++;
                            } else {
                                assert( false );
                            }
                        }

                        // Cost function 2
                        // If the split will not overflow our children
                        if( left_count <= max_branch_factor
                            and right_count <= max_branch_factor and
                            left_count > 0 and right_count > 0 )
                        {
                            double mean_d_pt = running_total /
                                (2*all_branch_polys.size());
                            // Distance
                            cost = (mean_d_pt-partition_candidate);
                            cost = cost * cost;
                            if( cost < min_cost ) {
                                best_candidate = partition_candidate;
                                best_dimension = d;
                                min_cost = cost;
                                //std::cout << "Best Candidate LC: " <<
                                //    left_count << " and RC: " <<
                                //    right_count << std::endl;
                            }
                        }

                    }
                }
                // Degenerate case
                assert( min_cost < std::numeric_limits<double>::max() );

                defaultPartition.dimension = best_dimension;
                defaultPartition.location = best_candidate;

                // Sort per the dimension we need
                std::sort( entries.begin(), entries.begin() +
                        cur_offset_,
                        [this,allocator,best_dimension](NodeEntry &entry1, NodeEntry &entry2 ) {
                            Branch &b1 = std::get<Branch>( entry1 );
                            Branch &b2 = std::get<Branch>( entry2 );
                            Rectangle poly1 = b1.get_summary_rectangle(
                                    allocator );
                            Rectangle poly2 = b2.get_summary_rectangle(
                                    allocator );
                            if( poly1.upperRight[best_dimension] ==
                                    poly2.upperRight[best_dimension]  ) {
                                for( unsigned i = 0; i < dimensions; i++ ) {
                                    if( poly1.upperRight[i] ==
                                            poly2.upperRight[i] ) {
                                        continue;
                                    }
                                    return poly1.upperRight[i] <
                                        poly2.upperRight[i];
                                }
                            }
                            return poly1.upperRight[best_dimension] <
                            poly2.upperRight[best_dimension];
                        }
                );

                    
                return defaultPartition;
            }


            void make_disjoint_from_children( IsotheticPolygon &polygon,
                    tree_node_handle handle_to_skip );
			SplitResult splitNode(Partition p, bool is_downsplit);
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
            std::vector<Point> bounding_box_validate();
			void print(unsigned n=0);
			void printTree(unsigned n=0);
			unsigned height();
			void stat();
	};

#include "node.tcc"
}
