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


template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::deleteSubtrees()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    if( isLeaf() ) {
        return;
    }

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    for( size_t i = 0; i < cur_offset_; i++ ) {
        NodeEntry &entry = entries.at(i);
        tree_node_handle child_handle = std::get<Branch>( entry ).child;
        auto child = allocator->get_tree_node<NodeType>(
                child_handle );
        child->deleteSubtrees();
        // FIXME: GC child

    }
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor,max_branch_factor>::isLeaf()
{
    if( cur_offset_ == 0 ) {
        return true;
    }

    return std::holds_alternative<Point>( entries.at( 0 ) );
}

template <int min_branch_factor, int max_branch_factor>
Rectangle Node<min_branch_factor,max_branch_factor>::boundingBox()
{
    Rectangle bb;
    assert( cur_offset_ > 0 );

    if( isLeaf() ) {
        Point &p = std::get<Point>( entries.at(0) );
        bb = Rectangle(p, p);
        for( size_t i = 1; i < cur_offset_; i++ ) {
            bb.expand( std::get<Point>( entries.at( i ) ) );
        }
        return bb;
    }
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    IsotheticPolygon poly;
    Branch &b = std::get<Branch>(entries.at(0));
    if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                b.boundingPoly ) ) {
        poly = std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly ).materialize_polygon();
    } else {
        tree_node_handle poly_handle = std::get<tree_node_handle>( b.boundingPoly );
        auto poly_pin =
            allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
        poly = poly_pin->materialize_polygon();
    }

    bb = poly.boundingBox;
    for( size_t i = 1; i < cur_offset_; i++ ) {
        Branch &b2 = std::get<Branch>(entries.at(i));
        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                    b2.boundingPoly ) ) {
            poly = std::get<InlineBoundedIsotheticPolygon>(
                    b2.boundingPoly ).materialize_polygon();
        } else {
            tree_node_handle poly_handle = std::get<tree_node_handle>(
                    b2.boundingPoly );
            auto poly_pin =
                allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                        poly_handle );
            poly = poly_pin->materialize_polygon();
        }

        bb.expand( poly.boundingBox );
    }
    return bb;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::updateBranch(
    tree_node_handle child_handle, 
    const InlineBoundedIsotheticPolygon &boundingPoly
) {
    // Locate the child
    size_t childIndex;
    for( childIndex = 0; std::get<Branch>( entries.at(childIndex) ).child != child_handle &&
            childIndex < cur_offset_; childIndex++ ) { }

    // Update the child
    std::get<Branch>( entries.at( childIndex ) ).boundingPoly = boundingPoly;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::removeEntry(
    const tree_node_handle &entry
) {
    assert( !isLeaf() );

    size_t found_index = 0;
    while( found_index < cur_offset_ ) {
        Branch &b = std::get<Branch>( entries.at( found_index ) );
        if( b.child == entry ) {
            break;
        }
        found_index++;
    }
    entries.at( found_index ) = entries.at( cur_offset_-1 );
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::removeEntry(
    const Node<min_branch_factor,max_branch_factor>::NodeEntry &entry
) {
    // Locate the child
    size_t childIndex;
    for( childIndex = 0; entries.at( childIndex ) != entry &&
            childIndex < cur_offset_; childIndex++ ) { }

    // Delete the child by deleting it and overwriting its branch
    // FIXME GC child
    // delete child;

    // Replace this index with whatever is in the last position
    if( childIndex != cur_offset_-1 ) {
        entries.at(childIndex) = entries.at( cur_offset_-1 );
    }
    // Truncate array size
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    if( isLeaf() ) {
        // We are a leaf so add our data points when they are the search point
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &p = std::get<Point>( entries.at(i) );
            if( requestedPoint == p ) {
                accumulator.push_back( p );
            }
        }
    }
    // Follow all branches, this is exhaustive
    for( size_t i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        auto child = allocator->get_tree_node<NodeType>(
                b.child );
        child->exhaustiveSearch( requestedPoint, accumulator );
    }
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> Node<min_branch_factor,max_branch_factor>::search(
    Point &requestedPoint
) {
    using NodeType = Node<min_branch_factor, max_branch_factor>;

    std::vector<Point> accumulator;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push( this->self_handle_ );
    tree_node_handle current_handle;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        current_handle = context.top();
        context.pop();

        auto current_node =
            allocator->get_tree_node<NodeType>(
                    current_handle );

        if( current_node->isLeaf() ) {
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                // We are a leaf so add our data points when they are the search point
                Point &p = std::get<Point>( current_node->entries.at(i) );
                if( requestedPoint == p ) {
                    accumulator.push_back( p );
                }
            }
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        } else {
            // Determine which branches we need to follow
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                IsotheticPolygon poly;
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    poly = std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly ).materialize_polygon();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto poly_pin =
                        allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                poly_handle );
                   
                    poly = poly_pin->materialize_polygon();
                }
                if( poly.containsPoint( requestedPoint) ) {
                    // Add to the nodes we will check
                    context.push( b.child );
                }
            }
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        }
    }

#ifdef STAT
    treeRef->stats.resetSearchTracker<false>();
#endif

    return accumulator;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> Node<min_branch_factor,max_branch_factor>::search(
    Rectangle &requestedRectangle
) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    std::vector<Point> accumulator;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push(this->self_handle_);
    tree_node_handle current_handle;
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        current_handle = context.top();
        context.pop();
        auto current_node =
            allocator->get_tree_node<NodeType>(
                    current_handle );

        if( current_node->isLeaf() ) {
            // We are a leaf so add our data points when they are within the search rectangle
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = std::get<Point>( current_node->entries.at(i) );
                if( requestedRectangle.containsPoint(p) ) {
                    accumulator.push_back( p );
                }
            }

#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        } else {
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                // Determine which branches we need to follow
                IsotheticPolygon poly;
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    poly = std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly ).materialize_polygon();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto poly_pin =
                        allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                poly_handle );
                    poly = poly_pin->materialize_polygon();
                }
                if( poly.intersectsRectangle( requestedRectangle ) ) {
                    // Add to the nodes we will check
                    context.push( b.child );
                }
            }
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        }
    }
#ifdef STAT
    treeRef->stats.resetSearchTracker<true>();
#endif

    return accumulator;
}

// Always called on root, this = root
// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
// choosing a particular leaf
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::chooseNode(Point givenPoint)
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // CL1 [Initialize]
    tree_node_handle cur_node_handle = self_handle_;

    unsigned enclosingPolyIndex = 0;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    assert( cur_node_handle != nullptr );

    for( ;; ) {
        assert( cur_node_handle != nullptr );
        auto cur_node = allocator->get_tree_node<NodeType>(
                cur_node_handle );

        // CL2 [Leaf check]
        if( cur_node->isLeaf() ) {
            return cur_node_handle;
        } else {
            // Compute the smallest expansion
            assert( cur_node->cur_offset_ > 0 );

            unsigned smallestExpansionBranchIndex = 0;

            tree_node_allocator *allocator = get_node_allocator( treeRef );
            IsotheticPolygon node_poly;

            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(std::get<Branch>(
                    cur_node->entries.at(0)).boundingPoly ) ) {
                node_poly =
                    std::get<InlineBoundedIsotheticPolygon>(std::get<Branch>(
                        cur_node->entries.at(0) ).boundingPoly
                            ).materialize_polygon();
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>(std::get<Branch>(
                    cur_node->entries.at(0)).boundingPoly );
                auto node_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>
                    ( poly_handle );
                node_poly = node_pin->materialize_polygon();
            }

            IsotheticPolygon::OptimalExpansion smallestExpansion =
                node_poly.computeExpansionArea(givenPoint);
            IsotheticPolygon::OptimalExpansion evalExpansion;

            for( size_t i = 1; i < cur_node->cur_offset_ &&
                    smallestExpansion.area != -1.0; i++ ) {
                Branch &b = std::get<Branch>( cur_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    node_poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly
                                    ).materialize_polygon();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto node_pin =
                        allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                poly_handle );
                    node_poly = node_pin->materialize_polygon();
                }

                evalExpansion = node_poly.computeExpansionArea(givenPoint);
                if( evalExpansion.area < smallestExpansion.area && evalExpansion.area != 0.0 ) {
                    smallestExpansionBranchIndex = i;
                    smallestExpansion = evalExpansion;
                }
            }


            if( smallestExpansion.area != -1.0 ) {
                Branch &b = std::get<Branch>( cur_node->entries.at(
                    smallestExpansionBranchIndex ) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                      b.boundingPoly ) ) {
                    node_poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly ).materialize_polygon();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto node_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    node_poly = node_pin->materialize_polygon();
                }

                IsotheticPolygon subsetPolygon =
                    node_poly.basicRectangles[smallestExpansion.index];

                subsetPolygon.expand(givenPoint);

                for( size_t i = 0; i < cur_node->cur_offset_; i++ ) {
                    if( i != smallestExpansionBranchIndex ) {
                        Branch &b = std::get<Branch>(
                                cur_node->entries.at(i) );
                        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly ) ) {
                            node_poly =
                                std::get<InlineBoundedIsotheticPolygon>(b.boundingPoly).materialize_polygon();
                        } else {
                            tree_node_handle poly_handle =
                                std::get<tree_node_handle>(
                                        b.boundingPoly );
                            auto node_pin =
                                allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                        poly_handle );

                            node_poly = node_pin->materialize_polygon();
                        }

                        subsetPolygon.increaseResolution(
                                givenPoint, node_poly );
                    }
                }

                if( cur_node->parent != nullptr ) {
                    auto parent =
                        allocator->get_tree_node<NodeType>(
                                cur_node->parent );
                    Branch &b = std::get<Branch>(
                            parent->entries.at(enclosingPolyIndex));
                    if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly ) ) {
                        node_poly =
                            std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly ).materialize_polygon();
                    } else {
                        tree_node_handle poly_handle =
                            std::get<tree_node_handle>( b.boundingPoly );
                        auto node_pin =
                            allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                    poly_handle );
                        node_poly = node_pin->materialize_polygon();
                    }

                    subsetPolygon.intersection( node_poly );
                }

                Branch &b2 = std::get<Branch>(
                        cur_node->entries.at(smallestExpansionBranchIndex) );

                // If existing poly is inline in the page
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b2.boundingPoly ) ) {

                    node_poly = std::get<InlineBoundedIsotheticPolygon>(
                                b2.boundingPoly).materialize_polygon();
                } else {
                    // Otherwise, read the special page with the polygon
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b2.boundingPoly );
                    // pull pin ptr out of scope so this doesn't get
                    // immediately unpinned
                    auto node_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    node_poly = node_pin->materialize_polygon();
                }

                node_poly.remove( smallestExpansion.index );
                node_poly.merge( subsetPolygon );


                // What is B here?
                auto b_child =
                    allocator->get_tree_node<NodeType>( b2.child );

                if( b_child->isLeaf() and b_child->cur_offset_ > 0 ) {
                    b_child->entries.at( b_child->cur_offset_ ) = givenPoint;
                    b_child->cur_offset_++;
                    node_poly.shrink( b_child->entries.begin(),
                            b_child->entries.begin() +
                            b_child->cur_offset_ );
                    b_child->cur_offset_--;
                    for( unsigned i = 0; i < b_child->cur_offset_; i++ ) {
                        assert( std::holds_alternative<Point>(
                                    b_child->entries.at(i) ) or
                                std::holds_alternative<Branch>(
                                    b_child->entries.at(i) ) );
                    }
                }
                node_poly.refine();

                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b2.boundingPoly ) ) {

                    if( node_poly.basicRectangles.size() <=
                            MAX_RECTANGLE_COUNT ) {
                        InlineBoundedIsotheticPolygon inline_poly = &(std::get<InlineBoundedIsotheticPolygon>(
                                    b2.boundingPoly));
                        inline_poly.push_polygon_to_disk( node_poly );
                    } else {
                        auto alloc_data =
                            allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                                    PAGE_DATA_SIZE );
                        alloc_data.first->push_polygon_to_disk(
                                node_poly );
                        // Point to the newly created polygon location
                        b2.boundingPoly = alloc_data.second;
                    }
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b2.boundingPoly );
                    auto node_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    node_pin->push_polygon_to_disk( node_poly );
                }

            }

            // Descend
            Branch &b = std::get<Branch>(
                    cur_node->entries.at(smallestExpansionBranchIndex) );
            cur_node_handle = b.child;
            enclosingPolyIndex = smallestExpansionBranchIndex;
            assert( cur_node_handle != nullptr );
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::findLeaf(Point givenPoint)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push(self_handle_);
    tree_node_handle current_node_handle;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        current_node_handle = context.top();
        context.pop();

        auto current_node =
            allocator->get_tree_node<NodeType>(
                    current_node_handle );

        if( current_node->isLeaf() ) {
            // FL2 [Search leaf node for record]
            // Check each entry to see if it matches E
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = std::get<Point>( current_node->entries.at(i) );
                if( p == givenPoint ) {
                    return current_node_handle;
                }
            }
        } else {
            // FL1 [Search subtrees]
            // Determine which branches we need to follow
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                IsotheticPolygon poly;
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    poly = 
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly ).materialize_polygon();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    poly = poly_pin->materialize_polygon();

                }
                if( poly.containsPoint(givenPoint) ) {
                    // Add the child to the nodes we will consider
                    context.push( b.child );
                }

            }
        }
    }

    return tree_node_handle( nullptr );
}

template <int min_branch_factor, int max_branch_factor>
Partition Node<min_branch_factor,max_branch_factor>::partitionNode()
{
    Partition defaultPartition;
    unsigned costMetric = std::numeric_limits<unsigned>::max();
    double totalMass = 0.0;

    if( isLeaf() ) {
        // Setup variance values
        Point variance = Point::atOrigin;
        Point average = Point::atOrigin;
        Point sumOfSquares = Point::atOrigin;

        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &dataPoint = std::get<Point>( entries.at(i) );
            average += dataPoint;
            sumOfSquares += dataPoint * dataPoint;
            totalMass += 1.0;
        }

        // Compute final terms
        average /= totalMass;
        sumOfSquares /= totalMass;

        // Compute final variance
        variance = sumOfSquares - average * average;

        // Choose most variate dimension
        defaultPartition.dimension = 0;
        for( unsigned d = 0; d < dimensions; d++ ) {
            if( variance[d] > variance[defaultPartition.dimension] ) {
                defaultPartition.dimension = d;
            }
        }
        defaultPartition.location = average[defaultPartition.dimension];

        return defaultPartition;
    } else {
        Point centreOfMass = Point::atOrigin;
        std::vector<Rectangle> sortable;
        double location;
        tree_node_allocator *allocator = get_node_allocator( treeRef );

        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            IsotheticPolygon poly;

            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        b.boundingPoly ) ) {
                poly = std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly ).materialize_polygon();
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( b.boundingPoly );
                auto poly_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                poly = poly_pin->materialize_polygon();
            }
            sortable.push_back( poly.boundingBox );
        }

        for( Rectangle &boundingBox : sortable ) {
            centreOfMass += boundingBox.lowerLeft;
            centreOfMass += boundingBox.upperRight;
            totalMass += 2.0;
        }

        centreOfMass /= totalMass;

        for( unsigned d = 0; d < dimensions; d++ ) {
            location = centreOfMass[d];

            // Compute cost, # of splits if d is chosen
            unsigned currentInducedSplits = 0;
            for( Rectangle &s : sortable ) {
                if( s.lowerLeft[d] < location && location < s.upperRight[d] ) {
                    currentInducedSplits++;
                }
            }

            // Compare cost
            if( currentInducedSplits < costMetric ) {
                defaultPartition.dimension = d;
                defaultPartition.location = location;
                costMetric = currentInducedSplits;
            }
        }

        return defaultPartition;
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::fix_polygon(
    IsotheticPolygon &existing_polygon
) {

    // Hopefully we can reuse this polygon, its on a page somewhere
    existing_polygon.reset();

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    // Walk over all our children's polygons, merge 'em all in
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );

        IsotheticPolygon branch_poly;
        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                    b.boundingPoly ) ) {
            // It's inline in the pinned node, jsut get the address
            branch_poly = std::get<InlineBoundedIsotheticPolygon>(
                        b.boundingPoly ).materialize_polygon();
        } else {
            // It's on another page, figure out where, read that page,
            // set the address
            tree_node_handle poly_handle =
                std::get<tree_node_handle>( b.boundingPoly );
            auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
            branch_poly = poly_pin->materialize_polygon();
        }
        
        // Merge the existing polygon in with our child
        existing_polygon.merge( branch_poly );
    }
}

// Splitting a node will remove it from its parent node and its memory will be freed
template <int min_branch_factor, int max_branch_factor>
SplitResult
Node<min_branch_factor,max_branch_factor>::splitNode(
    Partition p
) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    pinned_node_ptr<InlineUnboundedIsotheticPolygon> left_poly_pin(
            allocator->buffer_pool_, nullptr, nullptr );

    IsotheticPolygon node_poly(boundingBox());
    tree_node_handle poly_handle( nullptr );

    if( parent != nullptr ) {
        auto parent_node =
            allocator->get_tree_node<NodeType>( parent );
        Branch &parent_branch = parent_node->locateBranch(this->self_handle_);

        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                    parent_branch.boundingPoly ) ) {

            // Copy because we will unpin parent now
            node_poly = std::get<InlineBoundedIsotheticPolygon>(
                        parent_branch.boundingPoly
                        ).materialize_polygon();
        } else {
            poly_handle = std::get<tree_node_handle>(
                    parent_branch.boundingPoly );
            left_poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
            node_poly = left_poly_pin->materialize_polygon();
        }
    } 

    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first;
    new (&(*left_node)) NodeType( treeRef, parent, left_handle );

    alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first;
    new (&(*right_node)) NodeType( treeRef, parent, right_handle );

    // At this point, we are going to use node_poly for both sides of
    // the split. If node_poly is an InlineBoundedIsotheticPolygon, then
    // it is of fixed size, and can be created on the stack and copied
    // around as we want. If node_poly is an
    // InlineUnboundedIsotheticPolygon, then we can't just pass it
    // around on the stack and copy it as we want. We need to create a
    // second copy of the polygon on disk somewhere, get a
    // tree_node_handle to that location, and pass that around instead

    // Default values, will override
    SplitResult split = {
        { tree_node_handle(nullptr), left_handle }, 
        { tree_node_handle(nullptr), right_handle } };


    // Duplicate
    // FIXME: one of these copies is unnecessary but it makes the code
    // below far more intelligible
    IsotheticPolygon left_polygon = node_poly;
    IsotheticPolygon right_polygon = node_poly;


    // this is the left polygon
    left_polygon.maxLimit( p.location, p.dimension );
    left_polygon.refine();

    // this is the right polygon
    right_polygon.minLimit( p.location, p.dimension );
    right_polygon.refine();

    if( isLeaf() ) {

        bool containedLeft, containedRight;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &dataPoint = std::get<Point>( entries.at(i) );
            containedLeft = left_polygon.containsPoint( dataPoint );
            containedRight = right_polygon.containsPoint( dataPoint );

            if( containedLeft && !containedRight ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else if( !containedLeft && containedRight ) {
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            } else if( left_node->cur_offset_ < right_node->cur_offset_
                    ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else {
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            }
            for( unsigned i = 0; i < left_node->cur_offset_; i++ ) {
                assert( std::holds_alternative<Point>(
                            left_node->entries.at(i) ) or
                        std::holds_alternative<Branch>(
                            left_node->entries.at(i) ) );
            }
            for( unsigned i = 0; i < right_node->cur_offset_; i++ ) {
                assert( std::holds_alternative<Point>(
                            right_node->entries.at(i) ) or
                        std::holds_alternative<Branch>(
                            right_node->entries.at(i) ) );
            }


        }

        cur_offset_ = 0;

        left_polygon.shrink( left_node->entries.begin(),
                left_node->entries.begin() + left_node->cur_offset_ );
        right_polygon.shrink(
                right_node->entries.begin(), right_node->entries.begin() +
                right_node->cur_offset_ );
    } else {

        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );

            pinned_node_ptr<InlineUnboundedIsotheticPolygon>
                branch_poly_pin( allocator->buffer_pool_, nullptr,
                        nullptr );

            IsotheticPolygon branch_poly;
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly ) ) {
                branch_poly = std::get<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly ).materialize_polygon();
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( branch.boundingPoly );
                branch_poly_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                branch_poly = branch_poly_pin->materialize_polygon();
            }

            bool is_contained_left =
                branch_poly.boundingBox.upperRight[p.dimension]
                <= p.location;
            bool is_contained_right =
                branch_poly.boundingBox.lowerLeft[p.dimension]
                >= p.location;
            if( is_contained_left and (not is_contained_right or
                (left_node->cur_offset_ < right_node->cur_offset_ ) ) ){
                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );
                child->parent = split.leftBranch.child;
                assert( split.leftBranch.child == left_handle );
                left_node->entries.at( left_node->cur_offset_++ ) =
                    branch;
            } else if( is_contained_right and (not is_contained_left or
                    (left_node->cur_offset_ >= right_node->cur_offset_) ) ) {
                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );
                child->parent = split.rightBranch.child;
                assert( split.rightBranch.child == right_handle );
                right_node->entries.at( right_node->cur_offset_++ ) = branch;
            } else {

                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );

                SplitResult downwardSplit = child->splitNode( p );

                // FIXME: GC branch.child
                // delete branch.child;

                auto left_child =
                    allocator->get_tree_node<NodeType>(
                            downwardSplit.leftBranch.child );
                if( left_child->cur_offset_ > 0 ) {
                    left_child->parent = split.leftBranch.child;

                    left_node->entries.at( left_node->cur_offset_++ ) =
                        downwardSplit.leftBranch;
                }
                auto right_child =
                    allocator->get_tree_node<NodeType>(
                            downwardSplit.rightBranch.child );
                if( right_child->cur_offset_ > 0 ) {
                    right_child->parent = split.rightBranch.child;

                    right_node->entries.at( right_node->cur_offset_++ )
                        = downwardSplit.rightBranch;
                }
            }

        }

        // It is possible that after splitting on the geometric median,
        // we still end up with an overfull node. This can happen
        // everything gets assigned to the left node except for one
        // branch that needs a downward split. That downward split
        // results in a node added to the left and to the right,
        // resulting in an overfull left node.


        // FIXME: presumes sorted to preserve no intersection
        if( (left_node->cur_offset_ > max_branch_factor and 
                right_node->cur_offset_ == max_branch_factor) or
            (left_node->cur_offset_ == max_branch_factor and
                right_node->cur_offset_ > max_branch_factor ) ) {
            std::cout << "Weird edge case! Is this possible?" <<
            std::endl;
            assert( false );
        }
        if( left_node->cur_offset_ > max_branch_factor ) {

            std::sort( left_node->entries.begin(),
                    left_node->entries.begin() + cur_offset_,
                    [&allocator,p]( NodeEntry &n1, NodeEntry &n2 ) {
                        Branch &b1 = std::get<Branch>( n1 );
                        Branch &b2 = std::get<Branch>( n2 );
                        tree_node_handle b1_handle = b1.child;
                        tree_node_handle b2_handle = b2.child;
                        auto child1 = allocator->get_tree_node<NodeType>( b1_handle );
                        auto child2 = allocator->get_tree_node<NodeType>( b2_handle );
                        Rectangle bb1 = child1->boundingBox();
                        Rectangle bb2 = child2->boundingBox();
                        return bb1.upperRight[ p.dimension ] <= bb2.upperRight[ p.dimension ];
                    }
            );
            // Move last entry to the right
            std::cout << "Zhuzh" << std::endl;
            right_node->entries.at( right_node->cur_offset_ ) =
                left_node->entries.at( left_node->cur_offset_-1 );
            left_node->cur_offset_--;

            Branch &b_to_adjust = std::get<Branch>( right_node->entries.at(
                        right_node->cur_offset_ ) );

            auto child_to_adjust = allocator->get_tree_node<NodeType>(
                    b_to_adjust.child );
            child_to_adjust->parent = right_node->self_handle_;
            right_node->cur_offset_++;
        } else if( right_node->cur_offset_ > max_branch_factor ) {
            std::sort( right_node->entries.begin(),
                    right_node->entries.begin() + cur_offset_,
                    [&allocator,p]( NodeEntry &n1, NodeEntry &n2 ) {
                        Branch &b1 = std::get<Branch>( n1 );
                        Branch &b2 = std::get<Branch>( n2 );
                        tree_node_handle b1_handle = b1.child;
                        tree_node_handle b2_handle = b2.child;
                        auto child1 = allocator->get_tree_node<NodeType>( b1_handle );
                        auto child2 = allocator->get_tree_node<NodeType>( b2_handle );
                        Rectangle bb1 = child1->boundingBox();
                        Rectangle bb2 = child2->boundingBox();
                        return bb1.upperRight[ p.dimension ] <= bb2.upperRight[ p.dimension ];
                    }
            );

            std::cout << "Zhuzh" << std::endl;
            // Move first entry to the left
            left_node->entries.at( left_node->cur_offset_ ) =
                right_node->entries.at( right_node->cur_offset_-1 );
            right_node->cur_offset_--;

            Branch &b_to_adjust = std::get<Branch>( left_node->entries.at(
                        left_node->cur_offset_ ) );
            auto child_to_adjust = allocator->get_tree_node<NodeType>(
                    b_to_adjust.child );
            child_to_adjust->parent = left_node->self_handle_;
            left_node->cur_offset_++;
        }

        /*
        std::cout << "After zhuzhing, left has: " <<
            left_node->cur_offset_ << std::endl;
        std::cout << "After zhuzhing, right has: " <<
            right_node->cur_offset_ << std::endl;
        */

        assert( left_node->cur_offset_ <= max_branch_factor );
        assert( right_node->cur_offset_ <= max_branch_factor );

        left_node->fix_polygon( left_polygon );
        right_node->fix_polygon( right_polygon );

        // Writeback our polygons
        if( left_polygon.basicRectangles.size() > MAX_RECTANGLE_COUNT ) {
            //FIXME: we actually realloc this, even if we don't have to.
            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        PAGE_DATA_SIZE );
            split.leftBranch.boundingPoly = alloc_data.second;
            alloc_data.first->push_polygon_to_disk( left_polygon );
        } else {
            split.leftBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(split.leftBranch.boundingPoly).push_polygon_to_disk(
                    left_polygon );
        }
        if( right_polygon.basicRectangles.size() > MAX_RECTANGLE_COUNT ) {
            //FIXME: we actually realloc this, even if we don't have to.
            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        PAGE_DATA_SIZE );
            split.rightBranch.boundingPoly = alloc_data.second;
            alloc_data.first->push_polygon_to_disk( right_polygon );
        } else {
            split.rightBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(split.rightBranch.boundingPoly).push_polygon_to_disk(
                    right_polygon );
        }

        cur_offset_ = 0;
    }
    return split;
}

// Splitting a node will remove it from its parent node and its memory will be freed
template <int min_branch_factor, int max_branch_factor>
SplitResult
Node<min_branch_factor,max_branch_factor>::splitNode()
{
    SplitResult returnSplit = splitNode(partitionNode());
    return returnSplit;
}

// This bottom-to-top sweep is only for splitting bounding boxes as necessary
template <int min_branch_factor, int max_branch_factor>
SplitResult
Node<min_branch_factor,max_branch_factor>::adjustTree()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    tree_node_handle current_handle = self_handle_;

    SplitResult propagationSplit = {
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
    };


    tree_node_allocator *allocator = get_node_allocator( treeRef );
    while( current_handle != nullptr ) {
        auto current_node =
            allocator->get_tree_node<NodeType>(
                    current_handle );

        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            {
                auto left_node =
                    allocator->get_tree_node<NodeType>(
                            propagationSplit.leftBranch.child );
                if( left_node->cur_offset_ > 0 ) {
                    current_node->entries.at(
                            current_node->cur_offset_++ ) =
                        propagationSplit.leftBranch;
                }
            }

            {
                auto right_node =
                    allocator->get_tree_node<NodeType>(
                            propagationSplit.rightBranch.child );

                if( right_node->cur_offset_  > 0 ) {

                    current_node->entries.at( current_node->cur_offset_++ ) =
                        propagationSplit.rightBranch;
                }
            }

            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                assert( std::holds_alternative<Point>(
                            current_node->entries.at(i) ) or
                        std::holds_alternative<Branch>(
                            current_node->entries.at(i) ) );
            }

        }

        // Early exit if this node does not overflow
        if( current_node->cur_offset_ <= max_branch_factor ) {
            propagationSplit = {
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
            };
            break;
        }

        // Otherwise, split node
        propagationSplit = current_node->splitNode();


        // Cleanup before ascending
        if( current_node->parent != nullptr ) {
            auto parent_node =
                allocator->get_tree_node<NodeType>(
                        current_node->parent );
            parent_node->removeEntry( current_handle );
        }

        // Ascend, propagating splits
        auto left_node =
            allocator->get_tree_node<NodeType>(
                    propagationSplit.leftBranch.child );
        current_handle = left_node->parent;
    }

    return propagationSplit;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::insert( Point givenPoint ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // Find the appropriate position for the new point
    tree_node_handle current_handle = chooseNode( givenPoint );

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    auto current_node = allocator->get_tree_node<NodeType>(
            current_handle ); 

    // Adjust the tree
    if( current_node->isLeaf() ) {
        // Add just the data
        current_node->entries.at( current_node->cur_offset_++ ) =
            givenPoint;
    }


    SplitResult finalSplit = current_node->adjustTree();


    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        auto alloc_data =
            allocator->create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( treeRef,
                tree_node_handle(nullptr), alloc_data.second );
        auto new_root_handle = alloc_data.second;
        auto new_root_node = alloc_data.first;

        auto left_node =
            allocator->get_tree_node<NodeType>(
                    finalSplit.leftBranch.child );
        left_node->parent = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.leftBranch;

        auto right_node =
            allocator->get_tree_node<NodeType>(
                    finalSplit.rightBranch.child );

        right_node->parent = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.rightBranch;

        for( unsigned i = 0; i < new_root_node->cur_offset_; i++ ) {
            assert( std::holds_alternative<Point>(
                        new_root_node->entries.at(i) ) or
                    std::holds_alternative<Branch>(
                        new_root_node->entries.at(i) ) );
        }


        // FIXME: GC original root node
        //delete this;

        return new_root_handle;
    }

    return self_handle_;
}

// To be called on a leaf
template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::condenseTree()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    auto current_node_handle = self_handle_;
    auto previous_node_handle = tree_node_handle( nullptr );

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( current_node_handle != nullptr ) {
        auto current_node =
            allocator->get_tree_node<NodeType>(
                    current_node_handle );
        current_node_handle = current_node->parent;
        if( previous_node_handle != nullptr ) {
            current_node =
            allocator->get_tree_node<NodeType>(
                    current_node_handle );
            auto previous_node =
                allocator->get_tree_node<NodeType>(
                        previous_node_handle );
            if( previous_node->cur_offset_ == 0 ) {
                current_node->removeEntry( previous_node_handle );
            }
        }

        previous_node_handle = current_node_handle;
    }
}

// Always called on root, this = root
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::remove( Point givenPoint ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // D1 [Find node containing record]
    tree_node_handle leaf_handle = findLeaf( givenPoint );
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    // Record not in the tree
    if( leaf_handle == nullptr ) {
        return self_handle_;
    }

    // D2 [Delete record]
    auto leaf_node = allocator->get_tree_node<NodeType>( leaf_handle );
    leaf_node->removeEntry( givenPoint );

    // D3 [Propagate changes]
    leaf_node->condenseTree();

    // D4 [Shorten tree]
    if( cur_offset_ == 1 and std::holds_alternative<Branch>( entries.at(0)
                ) ) {
        tree_node_handle new_root_handle = std::get<Branch>(
                entries.at(0) ).child;
        // FIXME GC ME 
        //delete this;
        auto new_root = allocator->get_tree_node<NodeType>( new_root_handle );
        new_root->parent = tree_node_handle( nullptr );
        return new_root_handle;
    }

    return self_handle_;
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::checksum() {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    unsigned sum = 0;

    if( isLeaf() ) {
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &dataPoint = std::get<Point>( entries.at(i) );
            for( unsigned d = 0; d < dimensions; d++ ) {
                sum += (unsigned) dataPoint[d];
            }
        }
    }
    else {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( size_t i = 0; i < cur_offset_; i++ ) {
            // Recurse
            Branch &branch = std::get<Branch>( entries.at(i) );
            auto child =
                allocator->get_tree_node<NodeType>(
                        branch.child );
            sum += child->checksum();
        }
    }

    return sum;
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor,max_branch_factor>::bounding_box_validate()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    if( !isLeaf() ) {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            Rectangle boundingBox;
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        b.boundingPoly ) ) {
                boundingBox = std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
                        ).materialize_polygon().boundingBox;
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( b.boundingPoly );
                auto poly_ptr =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                boundingBox =
                    poly_ptr->materialize_polygon().boundingBox;
            }
            tree_node_handle child_handle = b.child;
            auto child = allocator->get_tree_node<NodeType>(
                    child_handle );
            assert( child->boundingBox() == boundingBox );
            child->bounding_box_validate();
        }
    }
    return true;
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor,max_branch_factor>::validate( tree_node_handle expectedParent, unsigned index) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;


    tree_node_allocator *allocator = get_node_allocator( treeRef );

    if( expectedParent != nullptr and (parent != expectedParent ||
                cur_offset_ > max_branch_factor )) {
        std::cout << "node = " << (void *)this << std::endl;
        std::cout << "parent = " << parent << " expectedParent = " << expectedParent << std::endl;
        std::cout << "maxBranchFactor = " << max_branch_factor << std::endl;
        std::cout << "entries.size() = " << cur_offset_ << std::endl;
        assert(parent == expectedParent);
    }

    if( expectedParent != nullptr) {
        if( isLeaf() ) {
            tree_node_allocator *allocator = get_node_allocator( treeRef );
            auto parent_node =
                allocator->get_tree_node<NodeType>( parent );
            Branch &branch = std::get<Branch>(
                    parent_node->entries.at(index) );

            IsotheticPolygon poly;
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly ) ) {
                poly = std::get<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly).materialize_polygon();
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( branch.boundingPoly );
                auto poly_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                poly = poly_pin->materialize_polygon();
            }
            for( size_t i = 0; i < cur_offset_; i++ ) {
                Point &dataPoint = std::get<Point>( entries.at(i) );
                if( !poly.containsPoint( dataPoint ) ) {
                    std::cout << "fails to contain " << dataPoint << std::endl;
                    assert( false );
                }
            }

            return true;
        }

        for( size_t i = 0; i < cur_offset_; i++ ) {
            for( size_t j = 0; j < cur_offset_; j++ ) {
                if( i != j ) {
                    Branch &b_i = std::get<Branch>( entries.at(i) );
                    Branch &b_j = std::get<Branch>( entries.at(j) );
                    IsotheticPolygon poly;
                    if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                                b_i.boundingPoly ) ) {
                        poly = std::get<InlineBoundedIsotheticPolygon>(
                                b_i.boundingPoly).materialize_polygon();
                    } else {
                        tree_node_handle poly_handle =
                            std::get<tree_node_handle>( b_i.boundingPoly );
                        auto poly_pin =
                            allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                    poly_handle );
                        poly = poly_pin->materialize_polygon();
                    }
                    IsotheticPolygon poly2;
                    if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                                b_j.boundingPoly ) ) {
                        poly2 = std::get<InlineBoundedIsotheticPolygon>(
                                b_j.boundingPoly).materialize_polygon();
                    } else {
                        tree_node_handle poly_handle =
                            std::get<tree_node_handle>( b_j.boundingPoly );
                        auto poly_pin2 =
                            allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                    poly_handle );
                        poly2 = poly_pin2->materialize_polygon();
                    }



                    if( !poly.disjoint(poly2) ) {
                        std::cout << "Branch " << i << " is not disjoint from sibling Branch " << j << std::endl;
                        /*
                        std::cout << "branches[" << i << "].boundingPoly= " << b_i_poly << std::endl;
                        std::cout << "branches[" << j << "].boundingPoly = " << b_j_poly << std::endl;
                        */
                        assert( false );
                    }
                }
            }
        }
    }

    bool valid = true;
    if( !isLeaf() ) {
        for( size_t i = 0; i < cur_offset_; i++ ) {

            Branch &b = std::get<Branch>( entries.at(i) );
            auto child =
                allocator->get_tree_node<NodeType>( b.child );
            valid = valid and child->validate( self_handle_, i );
        }
    }

    return valid;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::print(unsigned n)
{
    std::string indendtation(n * 4, ' ');
    std::cout << indendtation << "Node " << (void *)this << std::endl;
    std::cout << indendtation << "    Parent: " << parent << std::endl;
    if( isLeaf() ) {
        std::cout << indendtation << "    Data: ";
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &dataPoint = std::get<Point>( entries.at(i) );
            std::cout << dataPoint;
        }
    } else {
        std::cout << indendtation << "    Branches: " << std::endl;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );
            // FIXME: out of band poly
            auto poly = std::get<InlineBoundedIsotheticPolygon>(
                    branch.boundingPoly ).materialize_polygon();
            std::cout << indendtation << "		" << branch.child << std::endl;
            std::cout << indendtation << "		" << poly << std::endl;
        }
    }
    std::cout << std::endl;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::printTree(unsigned n)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // Print this node first
    print(n);

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    // Print any of our children with one more level of indentation
    std::string indendtation(n * 4, ' ');
    if( !isLeaf() ) {
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );
            auto child =
                allocator->get_tree_node<NodeType>(
                        branch.child );

            // Recurse
            child->printTree(n + 1);
        }
    }
    std::cout << std::endl;
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::height()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    unsigned ret = 0;
    tree_node_handle current_handle = self_handle_;
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    for( ;; ) {
        auto node = allocator->get_tree_node<NodeType>(
                current_handle );

        ret++;
        if( node->isLeaf() ) {
            return ret;
        }
        current_handle = std::get<Branch>( node->entries.at(0) ).child;
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::stat() {
#ifdef STAT
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    std::stack<tree_node_handle> context;

    // Initialize our context stack
    context.push( self_handle_ );
    tree_node_handle currentContext;
    unsigned long branchesSize;
    unsigned long dataSize;
    unsigned long polygonSize;
    unsigned long totalPolygonSize = 0;
    unsigned long totalLines = 0;
    size_t memoryFootprint = 0;
    unsigned long totalNodes = 1;
    unsigned long singularBranches = 0;
    unsigned long totalLeaves = 0;

    std::vector<unsigned long> histogramPolygon;
    histogramPolygon.resize(10000, 0);
    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize( max_branch_factor, 0 );

    double coverage = 0.0;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        currentContext = context.top();
        context.pop();

        auto current_node =
            allocator->get_tree_node<NodeType>( currentContext );


        branchesSize = current_node->cur_offset_;
        dataSize = current_node->cur_offset_;

        unsigned fanout = branchesSize == 0 ? dataSize : branchesSize;
        if( fanout >= histogramFanout.size() ) {
            histogramFanout.resize(2*fanout, 0);
        }
        histogramFanout[fanout]++;

        if( current_node->isLeaf() ) {
            totalLeaves++;
            memoryFootprint += sizeof(Node) + current_node->cur_offset_ * sizeof(Point);
        } else {

            // Compute the overlap and coverage of our children
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i)
                        );
                coverage += b.boundingPoly.area();
            }

            totalNodes += current_node->cur_offset_;
            memoryFootprint += sizeof(Node) + current_node->cur_offset_ * sizeof(Node::Branch);
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i)
                        );
                auto child =
                    allocator->get_tree_node<NodeType>(
                            b.child );
                if( child->cur_offset_ == 1 ) {
                    singularBranches++;
                }

                polygonSize = b.boundingPoly.basicRectangles.size();
                assert( polygonSize < histogramPolygon.size() );
                histogramPolygon[polygonSize]++;
                totalPolygonSize += polygonSize;

                for( Rectangle r : b.boundingPoly.basicRectangles) {
                    if( r.area() == 0.0 ) {
                        totalLines++;
                    }
                }

                context.push(b.child);
            }
        }
    }

    // Print out what we have found
    STATEXEC(std::cout << "### Statistics ###" << std::endl);
    STATMEM(memoryFootprint);
    STATHEIGHT(height());
    STATSIZE(totalNodes);
    STATSINGULAR(singularBranches);
    STATLEAF(totalLeaves);
    STATBRANCH(totalNodes - 1);
    STATCOVER(coverage);
    STATFANHIST();
    for (unsigned i = 0; i < histogramFanout.size(); ++i)
    {
        if (histogramFanout[i] > 0)
        {
            STATHIST(i, histogramFanout[i]);
        }
    }
    STATLINES(totalLines);
    STATTOTALPOLYSIZE(totalPolygonSize);
    STATPOLYHIST();
    for (unsigned i = 0; i < histogramPolygon.size(); ++i)
    {
        if (histogramPolygon[i] > 0)
        {
            STATHIST(i, histogramPolygon[i]);
        }
    }
    std::cout << treeRef->stats;

    STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
(void) 0;
#endif
}
