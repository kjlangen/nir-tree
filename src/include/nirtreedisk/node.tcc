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

// Shorthand so I don't have to write this a billion times
#define NODE_TEMPLATE_PARAMS template <int min_branch_factor, int max_branch_factor, class strategy>
#define NODE_CLASS_TYPES Node<min_branch_factor, max_branch_factor, strategy>

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::deleteSubtrees()
{
    if( isLeaf() ) {
        return;
    }

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    for( size_t i = 0; i < cur_offset_; i++ ) {
        NodeEntry &entry = entries.at(i);
        tree_node_handle child_handle = std::get<Branch>( entry ).child;
        auto child = treeRef->get_node( child_handle );
        child->deleteSubtrees();
        allocator->free( child_handle, sizeof( NODE_CLASS_TYPES ) );
    }
}

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::free() {
    //tree_node_allocator *allocator = get_node_allocator( treeRef );
    /*
    if( !isLeaf() ) {
        for( size_t i = 0; i < cur_offset_; i++ ) {
           Branch &b = std::get<Branch>( entries.at(i) );
           if( std::holds_alternative<tree_node_handle>( b.boundingPoly ) ) {
               auto poly_handle = std::get<tree_node_handle>(
                       b.boundingPoly );
               auto poly_pin =
                   InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                           allocator, poly_handle ); 
               poly_pin->free_subpages( allocator );
               allocator->free( poly_handle, PAGE_DATA_SIZE );
           }
        }
    }
    */
    //allocator->free( self_handle_, sizeof( NODE_CLASS_TYPES ) );
}

NODE_TEMPLATE_PARAMS
bool NODE_CLASS_TYPES::isLeaf()
{
    if( cur_offset_ == 0 ) {
        return true;
    }

    return std::holds_alternative<Point>( entries.at( 0 ) );
}

NODE_TEMPLATE_PARAMS
Rectangle NODE_CLASS_TYPES::boundingBox()
{
    Rectangle bb;
    assert( cur_offset_ > 0 );

    if( isLeaf() ) {
        Point &p = std::get<Point>( entries.at(0) );
        bb = Rectangle(p, Point::closest_larger_point( p ) );
        for( size_t i = 1; i < cur_offset_; i++ ) {
            bb.expand( std::get<Point>( entries.at( i ) ) );
        }
        return bb;
    }
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    Branch &b = std::get<Branch>(entries.at(0));
    bb = b.get_summary_rectangle( allocator );

    for( size_t i = 1; i < cur_offset_; i++ ) {
        Branch &b2 = std::get<Branch>(entries.at(i));
        bb.expand( b2.get_summary_rectangle( allocator ) );
    }
    return bb;
}

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::updateBranch(
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

// Removes a child logical pointer from a parent node, freeing that
// child's memory and the memory of the associated polygon (if external
// to the node).
NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::removeEntry(
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
    assert( std::get<Branch>( entries.at( found_index ) ).child == 
            entry );
    Branch &b = std::get<Branch>( entries.at( found_index ) );
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    if( std::holds_alternative<tree_node_handle>( b.boundingPoly ) ) {
        tree_node_handle free_poly_handle = std::get<tree_node_handle>(
                b.boundingPoly );
        auto poly_pin =
            InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                    allocator, free_poly_handle );
        poly_pin->free_subpages( allocator );
        size_t alloc_size = compute_sizeof_inline_unbounded_polygon(
                poly_pin->get_max_rectangle_count_on_first_page() );
        allocator->free( free_poly_handle, alloc_size );
    }

    allocator->free( b.child, sizeof( NODE_CLASS_TYPES ) );
    b.child = tree_node_handle( nullptr );

    entries.at( found_index ) = entries.at( cur_offset_-1 );
    cur_offset_--;
}

// If Branch:
// Removes a child logical pointer from a parent node, freeing that
// child's memory and the memory of the associated polygon (if external
// to the node).
// If Point: removes the point from the node

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::removeEntry(
    const NODE_CLASS_TYPES::NodeEntry &entry
) {
    // Locate the child
    size_t childIndex;
    for( childIndex = 0; entries.at( childIndex ) != entry and
            childIndex < cur_offset_; childIndex++ ) { }
    assert( entries.at( childIndex ) == entry );

    // Delete the child and overwrite its branch point
    if( std::holds_alternative<Branch>( entries.at( childIndex ) ) ) {
        Branch &b = std::get<Branch>( entries.at( childIndex ) );
        tree_node_allocator *allocator = get_node_allocator( treeRef );

        if( std::holds_alternative<tree_node_handle>( b.boundingPoly ) ) {
            tree_node_handle free_poly_handle =
                std::get<tree_node_handle>( b.boundingPoly );
            auto poly_pin =
                InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                        allocator, free_poly_handle );
            poly_pin->free_subpages( allocator );
            size_t alloc_size = compute_sizeof_inline_unbounded_polygon(
                poly_pin->get_max_rectangle_count_on_first_page() );
            allocator->free( free_poly_handle, alloc_size );
        }
        allocator->free( b.child, sizeof( NODE_CLASS_TYPES ) );
        b.child = tree_node_handle( nullptr );
    }

    // Replace this index with whatever is in the last position
    entries.at(childIndex) = entries.at( cur_offset_-1 );

    // Truncate array size
    cur_offset_--;
}

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
{
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
        auto child = treeRef->get_node( b.child );
        child->exhaustiveSearch( requestedPoint, accumulator );
    }
}

NODE_TEMPLATE_PARAMS
std::vector<Point> NODE_CLASS_TYPES::search(
    Point &requestedPoint
) {
    std::vector<Point> accumulator;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push( this->self_handle_ );
    tree_node_handle current_handle;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        current_handle = context.top();
        context.pop();

        auto current_node = treeRef->get_node( current_handle );

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
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    InlineBoundedIsotheticPolygon &loc_poly = 
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly );
                    if( loc_poly.containsPoint( requestedPoint ) ) {
                        context.push( b.child );
                    }
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto poly_pin =
                        InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                                allocator, poly_handle );
                    if( poly_pin->containsPoint( requestedPoint ) ) {
                        context.push( b.child );
                    }
                }
            }
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        }
    }

#ifdef STAT
    treeRef->stats.resetSearchTracker( false );
#endif

    return accumulator;
}

NODE_TEMPLATE_PARAMS
std::vector<Point> NODE_CLASS_TYPES::search(
    Rectangle &requestedRectangle
) {
    std::vector<Point> accumulator;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push(this->self_handle_);
    tree_node_handle current_handle;
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        current_handle = context.top();
        context.pop();
        auto current_node = treeRef->get_node( current_handle );

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
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    InlineBoundedIsotheticPolygon &loc_poly = 
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly );

                    if( loc_poly.intersectsRectangle( requestedRectangle ) ) {
                        context.push( b.child );
                    }
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto poly_pin =
                        InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                                allocator, poly_handle );
                    if( poly_pin->intersectsRectangle( requestedRectangle ) ) {
                        context.push( b.child );
                    }
                }
            }
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        }
    }
#ifdef STAT
    treeRef->stats.resetSearchTracker( true );
#endif

    return accumulator;

}

template <typename iter>
void shrink(
    IsotheticPolygon &polygon,
    iter begin,
    iter end,
    tree_node_allocator *allocator
){
    // Early exit
    if( polygon.basicRectangles.size() == 0 || begin == end ) {
        return;
    }

    std::vector<Rectangle> rectangleSetShrunk;
    for (const Rectangle &basicRectangle : polygon.basicRectangles) {
        bool addRectangle = false;
        Rectangle shrunkRectangle = Rectangle(Point::atInfinity, Point::atNegInfinity);
        for( auto cur_iter = begin; cur_iter != end; cur_iter++ ) {
            if( std::holds_alternative<Point>( *cur_iter ) ) {
                Point &pinPoint = std::get<Point>( *cur_iter );
                if( basicRectangle.containsPoint(pinPoint) ) {
                    shrunkRectangle.expand( pinPoint );
                    addRectangle = true;
                    assert( shrunkRectangle.containsPoint(pinPoint) );
                }
            } else {
                Branch &b = std::get<Branch>(
                        *cur_iter );
                Rectangle bounding_box;
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    bounding_box =
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly).get_summary_rectangle();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>(
                                b.boundingPoly );
                    auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    bounding_box = poly_pin->get_summary_rectangle();
                }
                if( basicRectangle.intersectsRectangle(
                            bounding_box ) ) {
                    shrunkRectangle.expand( bounding_box );
                    addRectangle = true;
                }
            }
        }

        if( addRectangle ) {
            rectangleSetShrunk.emplace_back( std::move(shrunkRectangle) );
        }
    }

    assert(rectangleSetShrunk.size() > 0);

    polygon.basicRectangles.swap(rectangleSetShrunk);
    polygon.recomputeBoundingBox();
}


// Always called on root, this = root
// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
// choosing a particular leaf
NODE_TEMPLATE_PARAMS
tree_node_handle NODE_CLASS_TYPES::chooseNode(Point givenPoint)
{
    // FIXME: try and avoid all these materialize calls

    // CL1 [Initialize]
    tree_node_handle cur_node_handle = self_handle_;

    assert( cur_node_handle != nullptr );

    for( ;; ) {
        assert( cur_node_handle != nullptr );
        auto cur_node = treeRef->get_node( cur_node_handle );

        // CL2 [Leaf check]
        if( cur_node->isLeaf() ) {
            return cur_node_handle;
        } else {
            // Compute the smallest expansion
            assert( cur_node->cur_offset_ > 0 );

            unsigned smallestExpansionBranchIndex = 0;

            tree_node_allocator *allocator = get_node_allocator( treeRef );
            IsotheticPolygon node_poly = std::get<Branch>(
                    cur_node->entries.at(0) ).materialize_polygon(
                    allocator );
            assert( node_poly.basicRectangles.size() > 0 );

            IsotheticPolygon::OptimalExpansion smallestExpansion =
                node_poly.computeExpansionArea(givenPoint);
            IsotheticPolygon::OptimalExpansion evalExpansion;

            for( size_t i = 1; i < cur_node->cur_offset_ &&
                    smallestExpansion.area != -1.0; i++ ) {
                Branch &b = std::get<Branch>( cur_node->entries.at(i) );
                node_poly = b.materialize_polygon( allocator );
                assert( node_poly.basicRectangles.size() > 0 );
             
                evalExpansion = node_poly.computeExpansionArea(givenPoint);
                if( evalExpansion.area < smallestExpansion.area &&
                                                                  evalExpansion.area
                                                                  != 0.0
                                                                  ) {
                    smallestExpansionBranchIndex = i;
                    smallestExpansion = evalExpansion;
                }
            }


            if( smallestExpansion.area != -1.0 ) {
                Branch &b = std::get<Branch>( cur_node->entries.at(
                    smallestExpansionBranchIndex ) );
                node_poly = b.materialize_polygon( allocator );
                assert( node_poly.basicRectangles.size() > 0 );

                IsotheticPolygon subsetPolygon(
                        node_poly.basicRectangles.at(smallestExpansion.index) );

                subsetPolygon.expand(givenPoint);

                for( size_t i = 0; i < cur_node->cur_offset_; i++ ) {
                    if( i != smallestExpansionBranchIndex ) {
                        Branch &b_i = std::get<Branch>(
                                cur_node->entries.at(i) );
                        subsetPolygon.increaseResolution( givenPoint,
                                b_i.materialize_polygon( allocator ) );
                    }
                }

                if( cur_node->parent != nullptr ) {
                    auto parent = treeRef->get_node( cur_node->parent );
                    Branch &b_parent = parent->locateBranch(
                            cur_node->self_handle_ );
                    subsetPolygon.intersection(
                            b_parent.materialize_polygon( allocator ) );
                }

                auto b_child = treeRef->get_node( b.child );

                assert( node_poly.basicRectangles.size() > 0 );
                node_poly.remove( smallestExpansion.index );
                node_poly.merge( subsetPolygon );

                if( b_child->isLeaf() and b_child->cur_offset_ > 0 ) {
                    b_child->entries.at( b_child->cur_offset_ ) = givenPoint;
                    b_child->cur_offset_++;
                    shrink( node_poly, b_child->entries.begin(),
                            b_child->entries.begin() +
                            b_child->cur_offset_, allocator );
                    b_child->cur_offset_--;
                    for( unsigned i = 0; i < b_child->cur_offset_; i++ ) {
                        assert( std::holds_alternative<Point>(
                                    b_child->entries.at(i) ) or
                                std::holds_alternative<Branch>(
                                    b_child->entries.at(i) ) );
                    }
                }

                auto child_before_refine = treeRef->get_node( b.child );
                node_poly.refine();

                assert( node_poly.basicRectangles.size() > 0 );

                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {

                    if( node_poly.basicRectangles.size() <=
                            MAX_RECTANGLE_COUNT ) {
                        InlineBoundedIsotheticPolygon *inline_poly = &(std::get<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly));
                        inline_poly->push_polygon_to_disk( node_poly );
                    } else {
                        unsigned overfull_rect_count = (unsigned) (2 *
                                node_poly.basicRectangles.size() );
                        overfull_rect_count =
                            std::min(overfull_rect_count,
                                (unsigned) InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page()
                                );



                        auto alloc_data =
                            allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                                    compute_sizeof_inline_unbounded_polygon(
                                        overfull_rect_count ) );
                        new (&(*alloc_data.first))
                            InlineUnboundedIsotheticPolygon( allocator,
                                    overfull_rect_count );
                        alloc_data.first->push_polygon_to_disk(
                                node_poly );
                        // Point to the newly created polygon location
                        b.boundingPoly = alloc_data.second;
                    }
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto node_pin =
                        InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                                allocator, poly_handle );
                    // If we can fit this into the existing space on the
                    // first page OR we can't fit it into the first page
                    // no matter how hard we try, then don't bother
                    // reallocating here.
                    if( node_pin->get_max_rectangle_count_on_first_page()
                        >= node_poly.basicRectangles.size() or 
                        node_poly.basicRectangles.size() > 
                        InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page() ) {
                        // If it fits, push the new rectangle data into
                        // the page
                        node_pin->push_polygon_to_disk( node_poly );
                    } else {
                        // Reallocate to make space on first page to
                        // avoid pointer chasing

                        // Free existing crap
                        node_pin->free_subpages( allocator );
                        allocator->free( poly_handle,
                            compute_sizeof_inline_unbounded_polygon(
                                node_pin->get_max_rectangle_count_on_first_page()
                                )
                        );

                        // Alloc new crap
                        unsigned overfull_rect_count = (unsigned) (2 *
                                node_poly.basicRectangles.size() );

                        overfull_rect_count =
                            std::min(overfull_rect_count,
                                    (unsigned) InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page()
                                    );
                        auto poly_alloc_data =
                            allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                                    compute_sizeof_inline_unbounded_polygon(
                                        overfull_rect_count ) );
                        new (&(*poly_alloc_data.first))
                            InlineUnboundedIsotheticPolygon( allocator,
                                    overfull_rect_count );
                        b.boundingPoly = poly_alloc_data.second;
                        poly_alloc_data.first->push_polygon_to_disk(
                                node_poly );

                    }
                }
            }

            // Descend
            Branch &b = std::get<Branch>(
                    cur_node->entries.at(smallestExpansionBranchIndex) );
            cur_node_handle = b.child;
            assert( cur_node_handle != nullptr );
        }
    }
}

NODE_TEMPLATE_PARAMS
tree_node_handle NODE_CLASS_TYPES::findLeaf(Point givenPoint)
{

    // Initialize our context stack

    std::stack<tree_node_handle> context;
    context.push(self_handle_);
    tree_node_handle current_node_handle;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        current_node_handle = context.top();
        context.pop();

        auto current_node = treeRef->get_node( current_node_handle );

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
                    InlineBoundedIsotheticPolygon &loc_poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly );
                    // Quick check
                    if( !loc_poly.get_summary_rectangle().containsPoint(
                                givenPoint ) ) {
                        continue;
                    }
                    // Full containment check required
                    poly = loc_poly.materialize_polygon();
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    auto poly_pin =
                        InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                                allocator, poly_handle );
                    // Quick check
                    if( !poly_pin->get_summary_rectangle().containsPoint(
                                givenPoint ) ) {
                        continue;
                    }
                    // Full containment check required
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

NODE_TEMPLATE_PARAMS
Partition NODE_CLASS_TYPES::partitionLeafNode()
{
    Partition defaultPartition;
    double totalMass = 0.0;
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
}

NODE_TEMPLATE_PARAMS
Partition NODE_CLASS_TYPES::partitionNode()
{

    if( isLeaf() ) {
        return partitionLeafNode();
    }

    // Branch split
    //
    // We need to ensure that this branch split does not result in nodes
    // that are still overfull. Since choosing a partitioning that
    // splits a polygon results in a node being added to the left and
    // right (and thus does not help in reducing entry counts in the
    // split-nodes), we want to avoid that. So, we walk along polygon
    // boundaries.
    //
    // However, we also want to ensure that the split partitions the
    // space rougly evenly so that we don't have to split again so soon,
    // which just increases the depth of the tree.
    return partitionBranchNode();

}


struct summary_rectangle_sorter {

    enum class sort_point {
        LOWER_LEFT,
        UPPER_RIGHT
    };

    summary_rectangle_sorter( unsigned dimension, sort_point sort_on, tree_node_allocator
            *allocator ) :
        dimension_( dimension ),
        allocator_( allocator ),
        sort_on_( sort_on ) {
    }

    template <class NE>
    bool operator()( NE &n1, NE &n2 ) {
        Branch &b1 = std::get<Branch>( n1 );
        Branch &b2 = std::get<Branch>( n2 );

        Rectangle bb1 = b1.get_summary_rectangle(
                allocator_ );
        Rectangle bb2 = b2.get_summary_rectangle(
                allocator_ );
        if( sort_on_ == sort_point::LOWER_LEFT ) {
            return bb1.lowerLeft[ dimension_ ] <= bb2.lowerLeft[ dimension_ ];
        }
        return bb1.upperRight[ dimension_ ] <= bb2.upperRight[ dimension_ ];
    }

    unsigned dimension_;
    tree_node_allocator *allocator_;
    sort_point sort_on_;
};

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::make_disjoint_from_children(
    IsotheticPolygon &polygon,
    tree_node_handle handle_to_skip
) {
    tree_node_allocator *allocator = get_node_allocator( treeRef );
    for( auto iter = entries.begin(); iter !=
            entries.begin() + cur_offset_; iter++ ) {
        Branch &b = std::get<Branch>( *iter );
        if( b.child == handle_to_skip ) {
            continue;
        }
        IsotheticPolygon child_poly = b.materialize_polygon( allocator );
        polygon.increaseResolution( Point::atInfinity, child_poly );
    }
}

// We create two new nodes and free the old one.
// The old one is freed in adjustTree using removeEntry
// If we downsplit, then we won't call adjustTree for that split so we
// need to delete the node ourselves.
NODE_TEMPLATE_PARAMS
SplitResult NODE_CLASS_TYPES::splitNode(
    Partition p,
    bool is_downsplit
) {
    using NodeType = NODE_CLASS_TYPES;
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first; // take pin
    new (&(*left_node)) NodeType( treeRef, parent, left_handle );
    assert( left_node->self_handle_ == left_handle );

    alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first; // take pin
    new (&(*right_node)) NodeType( treeRef, parent, right_handle );
    assert( right_node->self_handle_ == right_handle );

    SplitResult split = {
        { tree_node_handle(nullptr), left_handle }, 
        { tree_node_handle(nullptr), right_handle } };

    if( this->isLeaf() ) {
        bool containedLeft, containedRight;
        for( size_t i = 0; i < cur_offset_; i++ ) {

            Point &dataPoint = std::get<Point>( entries.at(i) );
            containedLeft = dataPoint[ p.dimension ] < p.location; // Not inclusive
            containedRight = dataPoint[ p.dimension ] >= p.location;
            assert( containedLeft or containedRight );

            if( containedLeft and not containedRight ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else if( not containedLeft and containedRight ) {
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            }
        }

        // All points have been routed.

        IsotheticPolygon left_polygon( left_node->boundingBox() );
        IsotheticPolygon right_polygon( right_node->boundingBox() );
        assert( left_polygon.disjoint( right_polygon ) );

        assert( left_polygon.basicRectangles.size() <=
                MAX_RECTANGLE_COUNT );
        assert( right_polygon.basicRectangles.size() <=
                MAX_RECTANGLE_COUNT );

        // If we have a parent, we need to make these disjoint from our
        // siblings. If we don't, then we are automatically disjoint
        // from our siblings since these arethe only two polys and they
        // are disjoint from each other now.
        if( parent ) {
            auto parent_node = treeRef->get_node( parent );
            if( not is_downsplit ) {
                parent_node->make_disjoint_from_children( left_polygon,
                        self_handle_ );
                assert( left_polygon.basicRectangles.size() > 0 );
                left_polygon.refine();
                assert( left_polygon.basicRectangles.size() > 0 );
                parent_node->make_disjoint_from_children( right_polygon,
                        self_handle_ );
                assert( right_polygon.basicRectangles.size() > 0 );
                right_polygon.refine();
                assert( right_polygon.basicRectangles.size() > 0 );
            } else {
                // Intersect with our existing poly to avoid intersect
                // with other children
                Branch &b = parent_node->locateBranch( self_handle_ );
                IsotheticPolygon parent_poly = b.materialize_polygon(
                        allocator );
                assert( parent_poly.basicRectangles.size() > 0 );
                assert( left_polygon.basicRectangles.size() > 0 );
                IsotheticPolygon poly_backup = left_polygon;
                left_polygon.intersection( parent_poly );
                if( left_polygon.basicRectangles.size() == 0 ) {
                    std::cout << "Weird situation: " << poly_backup << 
                        " is disjoint from parent: " << parent_poly << std::endl;
                }
                assert( left_polygon.basicRectangles.size() > 0 );
                left_polygon.refine();
                assert( left_polygon.basicRectangles.size() > 0 );
                assert( right_polygon.basicRectangles.size() > 0 );
                right_polygon.intersection( parent_poly );
                assert( right_polygon.basicRectangles.size() > 0 );
                right_polygon.refine();
                assert( right_polygon.basicRectangles.size() > 0 );
            }

        }
        cur_offset_ = 0;

        // Push left to disk
        if( left_polygon.basicRectangles.size() <= MAX_RECTANGLE_COUNT ) {
            split.leftBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(
                    split.leftBranch.boundingPoly
            ).push_polygon_to_disk( left_polygon );
        } else {
            unsigned overfull_rect_count = (unsigned) (2 *
                    left_polygon.basicRectangles.size() );
            overfull_rect_count =
                std::min(overfull_rect_count,
                    (unsigned) InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page()
                    );

            auto poly_alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            overfull_rect_count ) );
            new (&(*poly_alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator,
                        overfull_rect_count );
            split.leftBranch.boundingPoly = poly_alloc_data.second;
            poly_alloc_data.first->push_polygon_to_disk( left_polygon );
        }

        // Push right to disk
        if( right_polygon.basicRectangles.size() <= MAX_RECTANGLE_COUNT ) {
            split.rightBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(
                    split.rightBranch.boundingPoly
            ).push_polygon_to_disk( right_polygon );
        } else {
            unsigned overfull_rect_count = (unsigned) (2 *
                    right_polygon.basicRectangles.size() );
            overfull_rect_count =
                std::min(overfull_rect_count,
                    (unsigned) InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page()
                    );

            auto poly_alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            overfull_rect_count ) );
            new (&(*poly_alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator,
                        overfull_rect_count );
            split.rightBranch.boundingPoly = poly_alloc_data.second;
            poly_alloc_data.first->push_polygon_to_disk( right_polygon );

        }
        return split;
    } else {

        // So we are going to split this branch node.
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );

            pinned_node_ptr<InlineUnboundedIsotheticPolygon>
                branch_poly_pin( allocator->buffer_pool_, nullptr,
                        nullptr );

            Rectangle summary_rectangle =
                branch.get_summary_rectangle( allocator );

            bool is_contained_left =
                summary_rectangle.upperRight[p.dimension]
                <= p.location;
            bool is_contained_right =
                summary_rectangle.lowerLeft[p.dimension]
                >= p.location;
            assert( not(is_contained_left and is_contained_right) );

            // Entirely contained in the left polygon
            if( is_contained_left and not is_contained_right) {
                auto child = treeRef->get_node(  branch.child );
                child->parent = split.leftBranch.child;
                assert( split.leftBranch.child == left_handle );
                left_node->entries.at( left_node->cur_offset_++ ) =
                    branch;
            // Entirely contained in the right polygon
            } else if( is_contained_right and not is_contained_left ) {
                auto child = treeRef->get_node( branch.child );
                child->parent = split.rightBranch.child;
                assert( split.rightBranch.child == right_handle );
                right_node->entries.at( right_node->cur_offset_++ ) = branch;
            } else if( summary_rectangle.upperRight[p.dimension] ==
                    summary_rectangle.lowerLeft[p.dimension] and
                    summary_rectangle.lowerLeft[p.dimension] ==
                    p.location ) {
                // These go left or right situationally
                auto child = treeRef->get_node( branch.child );
                if( left_node->cur_offset_ <= right_node->cur_offset_ ) {
                    child->parent = split.leftBranch.child;
                    assert( split.leftBranch.child == left_handle );
                    left_node->entries.at( left_node->cur_offset_++ ) =
                        branch;
                } else {
                    child->parent = split.rightBranch.child;
                    assert( split.rightBranch.child == right_handle );
                    right_node->entries.at( right_node->cur_offset_++ ) =
                        branch;
                }
            // Partially spanned by both nodes, need to downsplit
            } else {
                auto child = treeRef->get_node( branch.child );

                SplitResult downwardSplit = child->splitNode( p, true );
                // This branch is dead, along with its polygon
                allocator->free( branch.child, sizeof( NODE_CLASS_TYPES )  );

                if( std::holds_alternative<tree_node_handle>(
                            branch.boundingPoly ) ) {
                    // We can free this
                    tree_node_handle free_poly_handle = std::get<tree_node_handle>(
                                branch.boundingPoly );
                    auto free_poly_pin = InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                            allocator, free_poly_handle );
                    free_poly_pin->free_subpages( allocator );
                    size_t alloc_size = compute_sizeof_inline_unbounded_polygon(
                        free_poly_pin->get_max_rectangle_count_on_first_page() );

                    allocator->free( free_poly_handle, alloc_size );
                }

                auto left_child = treeRef->get_node( downwardSplit.leftBranch.child );
                if( left_child->cur_offset_ > 0 ) {
                    left_child->parent = split.leftBranch.child;

                    left_node->entries.at( left_node->cur_offset_++ ) =
                        downwardSplit.leftBranch;
                }
                auto right_child = treeRef->get_node( downwardSplit.rightBranch.child );
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
        // We have a heuristic that generally solves this problem, but
        // provably does not work in all cases. Fix in the future, alert
        // us if it happens
        // TODO: FIXME

        assert( left_node->cur_offset_ <= max_branch_factor and
                right_node->cur_offset_ <= max_branch_factor );

        IsotheticPolygon left_polygon( left_node->boundingBox() );
        IsotheticPolygon right_polygon( right_node->boundingBox() );
        assert( left_polygon.disjoint( right_polygon ) );

        // When we downsplit two nodes we can make our polygon bigger
        // than our parents, provided we don't intersect with our
        // siblings. But they can do that too, and these expansions can
        // intersect.
        // So, if we downsplit, we actually need to intersect with our
        // parents.
        if( parent ) {
            auto parent_node = treeRef->get_node( parent );
            if( not is_downsplit ) {
                parent_node->make_disjoint_from_children( left_polygon,
                        self_handle_ );
                assert( left_polygon.basicRectangles.size() > 0 );
                left_polygon.refine();
                assert( left_polygon.basicRectangles.size() > 0 );
                parent_node->make_disjoint_from_children( right_polygon,
                        self_handle_ );
                assert( right_polygon.basicRectangles.size() > 0 );
                right_polygon.refine();
                assert( right_polygon.basicRectangles.size() > 0 );
            } else {
                // Intersect with our existing poly to avoid intersect
                // with other children
                Branch &b = parent_node->locateBranch( self_handle_ );
                IsotheticPolygon parent_poly = b.materialize_polygon(
                        allocator );
                left_polygon.intersection( parent_poly );
                assert( left_polygon.basicRectangles.size() > 0 );
                left_polygon.refine();
                assert( left_polygon.basicRectangles.size() > 0 );
                right_polygon.intersection( parent_poly );
                assert( right_polygon.basicRectangles.size() > 0 );
                right_polygon.refine();
                assert( right_polygon.basicRectangles.size() > 0 );
            }

        }

        // Writeback our polygons
        if( left_polygon.basicRectangles.size() > MAX_RECTANGLE_COUNT ) {
            unsigned overfull_rect_count = (unsigned) (2 *
                    left_polygon.basicRectangles.size() );

            overfull_rect_count =
                std::min(overfull_rect_count,
                    (unsigned) InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page()
                    );

            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            overfull_rect_count ) );
            new (&(*alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator,
                        overfull_rect_count  );
            split.leftBranch.boundingPoly = alloc_data.second;
            alloc_data.first->push_polygon_to_disk( left_polygon );
        } else {
            split.leftBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(split.leftBranch.boundingPoly).push_polygon_to_disk(
                    left_polygon );
            assert( std::get<InlineBoundedIsotheticPolygon>(
                        split.leftBranch.boundingPoly
                        ).get_summary_rectangle() ==
                    left_polygon.boundingBox );
        }
        if( right_polygon.basicRectangles.size() > MAX_RECTANGLE_COUNT ) {
            unsigned overfull_rect_count = (unsigned) (2 *
                    right_polygon.basicRectangles.size() );

            overfull_rect_count =
                std::min(overfull_rect_count,
                    (unsigned) InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page()
                    );

            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            overfull_rect_count ) );
            new (&(*alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator,
                        overfull_rect_count );
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
NODE_TEMPLATE_PARAMS
SplitResult NODE_CLASS_TYPES::splitNode()
{
    SplitResult returnSplit = splitNode(partitionNode(), false);
    return returnSplit;
}

// This bottom-to-top sweep is only for splitting bounding boxes as necessary
NODE_TEMPLATE_PARAMS
SplitResult NODE_CLASS_TYPES::adjustTree()
{
    // N.B., as we walk up the tree, we may perform a bunch of splits,
    // which is liable to destroy nodes that are downsplit. These
    // downsplit nodes' memory can then be re-used for other things,
    // like polygons. If we try to use treeRef (or other outside
    // pointers) from that node, it can be clobbered leading to amazing
    // segfaults. It is important that any variables we reference are
    // those we know are alive --- don't just rely on whatever the 
    // leaf node class member to have reasonable things after split is
    // called!

    tree_node_handle current_handle = self_handle_;

    SplitResult propagationSplit = {
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
    };

    NIRTreeDisk<min_branch_factor,max_branch_factor, strategy> *tree_ref_backup =
        treeRef;

    // Loop from the bottom to the very top
    while( current_handle != nullptr ) {
        auto current_node = tree_ref_backup->get_node( current_handle );

        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            {
                auto left_node = tree_ref_backup->get_node( propagationSplit.leftBranch.child );
                if( left_node->cur_offset_ > 0 ) {
                    current_node->entries.at(
                            current_node->cur_offset_++ ) =
                        propagationSplit.leftBranch;
                }
            }

            {
                auto right_node = tree_ref_backup->get_node( propagationSplit.rightBranch.child );

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

        // If this node does not require splitting, that's great, let's
        // keep adjusting bounding boxes all the way to the top
        if( current_node->cur_offset_ <= max_branch_factor ) {
            propagationSplit = {
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
            };

            break;
        }

        // Otherwise, split node
        propagationSplit = current_node->splitNode();

        auto left_node = tree_ref_backup->get_node( propagationSplit.leftBranch.child );

        // Cleanup before ascending
        if( current_node->parent != nullptr ) {
            // This will probably destroy current_node, so if we need
            // current node for anything, need to do it before the
            // removeEntry call.
            auto parent_node = tree_ref_backup->get_node( current_node->parent );
            parent_node->removeEntry( current_handle );
        }

        // Ascend, propagating splits
        current_handle = left_node->parent;
    }

    return propagationSplit;
}

NODE_TEMPLATE_PARAMS
tree_node_handle NODE_CLASS_TYPES::insert( Point givenPoint ) {
    using NodeType = NODE_CLASS_TYPES;

    // Find the appropriate position for the new point
    tree_node_handle current_handle = chooseNode( givenPoint );

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    auto current_node = treeRef->get_node( current_handle ); 

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

        auto left_node = treeRef->get_node( finalSplit.leftBranch.child );
        left_node->parent = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.leftBranch;

        auto right_node = treeRef->get_node( finalSplit.rightBranch.child );

        right_node->parent = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.rightBranch;

        for( unsigned i = 0; i < new_root_node->cur_offset_; i++ ) {
            assert( std::holds_alternative<Point>(
                        new_root_node->entries.at(i) ) or
                    std::holds_alternative<Branch>(
                        new_root_node->entries.at(i) ) );
        }


        allocator->free( self_handle_, sizeof( NODE_CLASS_TYPES ) );
        self_handle_ = tree_node_handle( nullptr );

        return new_root_handle;
    }

    return self_handle_;
}

// To be called on a leaf
NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::condenseTree()
{
    auto current_node_handle = self_handle_;
    auto previous_node_handle = tree_node_handle( nullptr );

    while( current_node_handle != nullptr ) {
        auto current_node = treeRef->get_node( current_node_handle );
        current_node_handle = current_node->parent;
        if( previous_node_handle != nullptr ) {
            current_node = treeRef->get_node( current_node_handle );
            auto previous_node = treeRef->get_node( previous_node_handle );
            if( previous_node->cur_offset_ == 0 ) {
                current_node->removeEntry( previous_node_handle );
            }
        }

        previous_node_handle = current_node_handle;
    }
}

// Always called on root, this = root
NODE_TEMPLATE_PARAMS
tree_node_handle NODE_CLASS_TYPES::remove( Point givenPoint ) {

    // D1 [Find node containing record]
    tree_node_handle leaf_handle = findLeaf( givenPoint );
    // Record not in the tree
    if( leaf_handle == nullptr ) {
        return self_handle_;
    }

    // D2 [Delete record]
    auto leaf_node = treeRef->get_node( leaf_handle );
    leaf_node->removeEntry( givenPoint );

    // D3 [Propagate changes]
    leaf_node->condenseTree();

    // D4 [Shorten tree]
    if( cur_offset_ == 1 and std::holds_alternative<Branch>( entries.at(0)
                ) ) {
        tree_node_handle new_root_handle = std::get<Branch>(
                entries.at(0) ).child;
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        allocator->free( self_handle_, sizeof( NODE_CLASS_TYPES ) );
        auto new_root = treeRef->get_node( new_root_handle );
        new_root->parent = tree_node_handle( nullptr );
        return new_root_handle;
    }

    return self_handle_;
}

NODE_TEMPLATE_PARAMS
unsigned NODE_CLASS_TYPES::checksum() {
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
        for( size_t i = 0; i < cur_offset_; i++ ) {
            // Recurse
            Branch &branch = std::get<Branch>( entries.at(i) );
            auto child = treeRef->get_node( branch.child );
            sum += child->checksum();
        }
    }

    return sum;
}

NODE_TEMPLATE_PARAMS
std::vector<Point> NODE_CLASS_TYPES::bounding_box_validate()
{
    if( !isLeaf() ) {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        std::vector<Point> all_child_points;
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Branch &b_i = std::get<Branch>( entries.at(i) );
            auto child_ptr = treeRef->get_node(  b_i.child );
            std::vector<Point> child_points =
                child_ptr->bounding_box_validate();
            for( Point &p : child_points ) {
                all_child_points.push_back( p );
            }
        }
        if( parent != nullptr ) {
            auto parent_node = treeRef->get_node( parent );
            Branch &parent_branch = parent_node->locateBranch( self_handle_ );
            IsotheticPolygon parent_poly =
                parent_branch.materialize_polygon( allocator );
            Rectangle bounding_box =
                parent_branch.get_summary_rectangle( allocator );
            for( Point &p : all_child_points ) {
                if( !parent_poly.containsPoint( p ) ) {
                    std::cout << "Parent poly (" << parent << "does not contain: " << p
                        << std::endl;
                    std::cout << "Poly was: " << parent_poly <<
                        std::endl;
                    std::cout << "My node is: " << self_handle_ <<
                        std::endl;
                    assert( false );
                }
                if( !bounding_box.containsPoint( p ) ) {
                    std::cout << "Parent poly contains " << p <<
                        " but the box does not!" << std::endl;
                    assert( false );
                }
            }
        }
        return all_child_points;
    }
    std::vector<Point> my_points;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        my_points.push_back( std::get<Point>( entries.at(i) ) );
    }
    return my_points;
}

NODE_TEMPLATE_PARAMS
bool NODE_CLASS_TYPES::validate( tree_node_handle expectedParent, unsigned index) {

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
            auto parent_node = treeRef->get_node( parent );
            Branch &branch = parent_node->locateBranch( self_handle_ );

            IsotheticPolygon poly;
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly ) ) {
                poly = std::get<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly).materialize_polygon();
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( branch.boundingPoly );
                auto poly_pin =
                    InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                            allocator, poly_handle );
                poly = poly_pin->materialize_polygon();
           }
           for( size_t i = 0; i < cur_offset_; i++ ) {
                Point &dataPoint = std::get<Point>( entries.at(i) );
                if( !poly.containsPoint( dataPoint ) ) {
                    std::cout << poly << " fails to contain " << dataPoint << std::endl;
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
                        auto poly_pin = InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                                allocator, poly_handle );
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
                            InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                                    allocator, poly_handle );
                        poly2 = poly_pin2->materialize_polygon();
                    }


                    if( !poly.disjoint(poly2) ) {
                        std::cout << "Branch " << i << " is not disjoint from sibling Branch " << j << std::endl;
                        std::cout << "Branch " << i << " " << b_i.child <<
                            std:: endl;
                        std::cout << "Branch " << j << " " << b_j.child <<
                            std:: endl;

                        std::cout << "Poly1 is: " << poly << std::endl;
                        std::cout << "Poly2 is: " << poly2 << std::endl;
                        std::cout << "Parent is: " << self_handle_ <<
                            std::endl;

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
            auto child = treeRef->get_node( b.child );
            valid = valid and child->validate( self_handle_, i );
        }
    }

    return valid;
}

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::print(unsigned n)
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

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::printTree(unsigned n)
{
    // Print this node first
    print(n);

    // Print any of our children with one more level of indentation
    std::string indendtation(n * 4, ' ');
    if( !isLeaf() ) {
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );
            auto child = treeRef->get_node( branch.child );

            // Recurse
            child->printTree(n + 1);
        }
    }
    std::cout << std::endl;
}

NODE_TEMPLATE_PARAMS
unsigned NODE_CLASS_TYPES::height()
{
    unsigned ret = 0;
    tree_node_handle current_handle = self_handle_;

    for( ;; ) {
        auto node = treeRef->get_node( current_handle );

        ret++;
        if( node->isLeaf() ) {
            return ret;
        }
        current_handle = std::get<Branch>( node->entries.at(0) ).child;
    }
}

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::stat() {
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
    size_t deadSpace = 0;

    std::vector<unsigned long> histogramPolygon;
    histogramPolygon.resize(10000, 0);
    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize( max_branch_factor, 0 );

    double coverage = 0.0;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        currentContext = context.top();
        context.pop();

        auto current_node = treeRef->get_node( currentContext );


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
            deadSpace += current_node->cur_offset_ * (sizeof(Branch) - sizeof(Point));
            deadSpace += (sizeof(Branch) *
                    (max_branch_factor-current_node->cur_offset_) );

        } else {


            // Compute the overlap and coverage of our children
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i)
                        );
                IsotheticPolygon polygon = b.materialize_polygon( allocator );
                coverage += polygon.area();
            }

            totalNodes += current_node->cur_offset_;
            memoryFootprint += sizeof(Node) + current_node->cur_offset_ * sizeof(Branch);
            deadSpace += (sizeof(Branch) *
                    (max_branch_factor-current_node->cur_offset_) );
            
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i)
                        );
                auto child = treeRef->get_node( b.child );
                if( child->cur_offset_ == 1 ) {
                    singularBranches++;
                }
                IsotheticPolygon polygon = b.materialize_polygon(
                        allocator );

                polygonSize = polygon.basicRectangles.size();
                assert( polygonSize < histogramPolygon.size() );
                histogramPolygon[polygonSize]++;
                totalPolygonSize += polygonSize;

                for( Rectangle r : polygon.basicRectangles) {
                    if( r.area() == 0.0 ) {
                        totalLines++;
                    }
                }

                if( polygonSize < MAX_RECTANGLE_COUNT ) {
                    deadSpace += (MAX_RECTANGLE_COUNT-polygonSize) *
                        sizeof(Rectangle);

                } else if( polygonSize > MAX_RECTANGLE_COUNT ) {
                    deadSpace += sizeof(Branch) -
                        sizeof(tree_node_handle);
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
    STATEXEC(std::cout << "DeadSpace: " << deadSpace << std::endl);
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
}

#undef NODE_TEMPLATE_PARAMS
#undef NODE_CLASS_TYPES
