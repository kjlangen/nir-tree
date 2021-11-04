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
        // FIXME: GC child

    }
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
        bb = Rectangle(p, p);
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
    entries.at( found_index ) = entries.at( cur_offset_-1 );
    cur_offset_--;
}

NODE_TEMPLATE_PARAMS
void NODE_CLASS_TYPES::removeEntry(
    const NODE_CLASS_TYPES::NodeEntry &entry
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
    treeRef->stats.resetSearchTracker<false>();
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
    treeRef->stats.resetSearchTracker<true>();
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

            IsotheticPolygon::OptimalExpansion smallestExpansion =
                node_poly.computeExpansionArea(givenPoint);
            IsotheticPolygon::OptimalExpansion evalExpansion;

            for( size_t i = 1; i < cur_node->cur_offset_ &&
                    smallestExpansion.area != -1.0; i++ ) {
                Branch &b = std::get<Branch>( cur_node->entries.at(i) );
                node_poly = b.materialize_polygon( allocator );
             
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

                IsotheticPolygon subsetPolygon(
                        node_poly.basicRectangles[smallestExpansion.index] );

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

                node_poly.remove( smallestExpansion.index );
                node_poly.merge( subsetPolygon );

                if( b_child->isLeaf() and b_child->cur_offset_ > 0 ) {
                    b_child->entries.at( b_child->cur_offset_ ) = givenPoint;
                    b_child->cur_offset_++;
                    // Is this legit?
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

                node_poly.refine();

                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {

                    if( node_poly.basicRectangles.size() <=
                            MAX_RECTANGLE_COUNT ) {
                        InlineBoundedIsotheticPolygon *inline_poly = &(std::get<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly));
                        inline_poly->push_polygon_to_disk( node_poly );
                    } else {
                        auto alloc_data =
                            allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                                    PAGE_DATA_SIZE );
                        new (&(*alloc_data.first))
                            InlineUnboundedIsotheticPolygon( allocator );
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
                    node_pin->push_polygon_to_disk( node_poly );
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

// Splitting a node will remove it from its parent node and its memory will be freed
NODE_TEMPLATE_PARAMS
SplitResult NODE_CLASS_TYPES::splitNode(
    Partition p
) {
    using NodeType = NODE_CLASS_TYPES;
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    pinned_node_ptr<InlineUnboundedIsotheticPolygon> left_poly_pin(
            allocator->buffer_pool_, nullptr, nullptr );

    IsotheticPolygon node_poly(boundingBox());
    tree_node_handle poly_handle( nullptr );

    if( parent != nullptr ) {
        auto parent_node = treeRef->get_node( parent );
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
            left_poly_pin =
                InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                        allocator, poly_handle );
            node_poly = left_poly_pin->materialize_polygon();
        }
    }

    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first;
    new (&(*left_node)) NodeType( treeRef, parent, left_handle );
    assert( left_node->self_handle_ == left_handle );

    alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first;
    new (&(*right_node)) NodeType( treeRef, parent, right_handle );
    assert( right_node->self_handle_ == right_handle );

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
            assert( containedLeft or containedRight );

            if( containedLeft && !containedRight ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else if( !containedLeft && containedRight ) {
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            } else if( left_node->cur_offset_ < right_node->cur_offset_
                    ) {
                assert( containedLeft and containedRight );
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else {
                assert( containedLeft and containedRight );
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

        shrink( left_polygon, left_node->entries.begin(),
                left_node->entries.begin() + left_node->cur_offset_, allocator );

        shrink( right_polygon, right_node->entries.begin(),
                right_node->entries.begin() + right_node->cur_offset_ , allocator );

        if( left_polygon.basicRectangles.size() <= MAX_RECTANGLE_COUNT ) {
            split.leftBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(
                    split.leftBranch.boundingPoly
            ).push_polygon_to_disk( left_polygon );
        } else {
            auto poly_alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        PAGE_DATA_SIZE );
            new (&(*poly_alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator );
            split.leftBranch.boundingPoly = poly_alloc_data.second;
            poly_alloc_data.first->push_polygon_to_disk( left_polygon );
        }

        if( right_polygon.basicRectangles.size() <= MAX_RECTANGLE_COUNT ) {
            split.rightBranch.boundingPoly=
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(
                    split.rightBranch.boundingPoly
            ).push_polygon_to_disk( right_polygon );
        } else {
            auto poly_alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        PAGE_DATA_SIZE );
            new (&(*poly_alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator );
            split.rightBranch.boundingPoly = poly_alloc_data.second;
            poly_alloc_data.first->push_polygon_to_disk( right_polygon );
        }

        assert( node_poly.boundingBox.containsRectangle( left_polygon.boundingBox
                    ) and node_poly.boundingBox.containsRectangle(
                        right_polygon.boundingBox ) );
    } else {

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
            // Entirely contained in the left polygon
            if( is_contained_left and not is_contained_right) {
                //std::cout << "Entry goes left." << std::endl;
                auto child = treeRef->get_node(  branch.child );
                child->parent = split.leftBranch.child;
                assert( split.leftBranch.child == left_handle );
                left_node->entries.at( left_node->cur_offset_++ ) =
                    branch;
            // Entirely contained in the right polygon
            } else if( is_contained_right and not is_contained_left ) {
                //std::cout << "Entry goes right." << std::endl;
                auto child = treeRef->get_node( branch.child );
                child->parent = split.rightBranch.child;
                assert( split.rightBranch.child == right_handle );
                right_node->entries.at( right_node->cur_offset_++ ) = branch;
            } else if( summary_rectangle.upperRight[p.dimension] ==
                    summary_rectangle.lowerLeft[p.dimension] and
                    summary_rectangle.lowerLeft[p.dimension] ==
                    p.location ) {
                //std::cout << "Entry is contested." << std::endl;
                // These go left or right situationally
                auto child = treeRef->get_node( branch.child );
                if( left_node->cur_offset_ <= right_node->cur_offset_ ) {
                    //std::cout << "but goes left." << std::endl;
                    child->parent = split.leftBranch.child;
                    assert( split.leftBranch.child == left_handle );
                    left_node->entries.at( left_node->cur_offset_++ ) =
                        branch;
                } else {
                    //std::cout << "but goes right." << std::endl;
                    child->parent = split.rightBranch.child;
                    assert( split.rightBranch.child == right_handle );
                    right_node->entries.at( right_node->cur_offset_++ ) =
                        branch;
                }
            // Partially spanned by both nodes, need to downsplit
            } else {
                //std::cout << "Entry is downsplit, goes both." <<
                //    std::endl;


                auto child = treeRef->get_node( branch.child );

                SplitResult downwardSplit = child->splitNode( p );

                // FIXME: GC branch.child
                // delete branch.child;

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


        //std::cout << "True Split: " << left_node->cur_offset_ << " and " <<
        //    right_node->cur_offset_ << std::endl;
        assert( left_node->cur_offset_ <= max_branch_factor and
                right_node->cur_offset_ <= max_branch_factor );

        if( (left_node->cur_offset_ > max_branch_factor and 
                right_node->cur_offset_ == max_branch_factor) or
            (left_node->cur_offset_ == max_branch_factor and
                right_node->cur_offset_ > max_branch_factor ) ) {
            // This is a case where moving stuff between the nodes will
            // not ameliorate our overfull node, because one is
            // overflowing and the other is full.

            std::cout << "Weird edge case for poly split!" << std::endl;
            assert( false );
        }

        // OK, we aren't in one of the disaster scenarios where we can't
        // remedy the overfull situation. We need to move an entry from
        // the left node to the right node or vice versa.
        // I'm not totally sure how to do this, because we have split a
        // bunch of entries above but we now we need to move the
        // partition line. We would need to undo the partition line and
        // redraw it.

        if( left_node->cur_offset_ > max_branch_factor ) {

            std::sort( left_node->entries.begin(),
                    left_node->entries.begin() + left_node->cur_offset_,
                    summary_rectangle_sorter( p.dimension,
                        summary_rectangle_sorter::sort_point::LOWER_LEFT,
                        allocator )
            );

            // Move last entry to the right
            right_node->entries.at( right_node->cur_offset_ ) =
                left_node->entries.at( left_node->cur_offset_-1 );
            left_node->cur_offset_--;

            Branch &b_to_adjust = std::get<Branch>( right_node->entries.at(
                        right_node->cur_offset_ ) );

            // Figure out the box on what we just moved
            Rectangle bounding_box = b_to_adjust.get_summary_rectangle(
                    allocator );
            right_polygon.expand( bounding_box.lowerLeft );
            assert( right_polygon.containsPoint( bounding_box.lowerLeft ) );
            assert( right_polygon.containsPoint( bounding_box.upperRight ) );
            // We could make this closer by reading the last left
            // polygon, but not sure how much it matters
            left_polygon.maxLimit( bounding_box.lowerLeft[p.dimension], p.dimension
                    );


            auto child_to_adjust = treeRef->get_node( b_to_adjust.child );
            child_to_adjust->parent = right_node->self_handle_;
            right_node->cur_offset_++;

        } else if( right_node->cur_offset_ > max_branch_factor ) {

            std::sort( right_node->entries.begin(),
                    right_node->entries.begin() + right_node->cur_offset_,
                    summary_rectangle_sorter( p.dimension,
                       summary_rectangle_sorter::sort_point::UPPER_RIGHT,
                       allocator )
            );

            // Move first entry to the left
            left_node->entries.at( left_node->cur_offset_ ) =
                right_node->entries.at( 0 );

            // Swap
            right_node->entries.at( 0 ) = right_node->entries.at(
                    right_node->cur_offset_-1 );

            // Chop
            right_node->cur_offset_--;

            Branch &b_to_adjust = std::get<Branch>( left_node->entries.at(
                        left_node->cur_offset_ ) );

            Rectangle bounding_box = b_to_adjust.get_summary_rectangle(
                    allocator );
            left_polygon.expand( bounding_box.upperRight );
            assert( left_polygon.containsPoint( bounding_box.upperRight ) );
            assert( left_polygon.containsPoint( bounding_box.lowerLeft ) );

            // We could make this closer by reading the right's polygon in
            // slot 1, but this might be another page access and its not
            // clear how much it will help things
            right_polygon.minLimit( bounding_box.upperRight[p.dimension], p.dimension
                    );

            auto child_to_adjust = treeRef->get_node( b_to_adjust.child );
            assert( child_to_adjust->parent == right_node->self_handle_
                    );
            child_to_adjust->parent = left_node->self_handle_;
            left_node->cur_offset_++;
        }

        assert( left_node->cur_offset_ <= max_branch_factor );
        assert( right_node->cur_offset_ <= max_branch_factor );
        
        // Simplify the polygons
        // FIXME: we cannot simplify polygons in this way, since it
        // basically just reconstructs the area required with the
        // minimal set of rectangles. But our rectangles previously were
        // explicitly designed to try and dodge other polygons. Shrink
        // should really be "take only the rectangles we need from
        // existing polygons to represent the space"
        /*
        std::cout << "Shrinking polygons." << std::endl;
        shrink( left_polygon, left_node->entries.begin(),
                left_node->entries.end(), allocator );

        shrink( right_polygon, right_node->entries.begin(),
                right_node->entries.end(), allocator );

        std::cout << "Checking BB on: " << left_handle << std::endl;
        assert( left_polygon.boundingBox  ==
                left_node->boundingBox() );

        std::cout << "Left Polygon matches boundingBox() " <<
            left_handle << std::endl;

        std::cout << "Checking BB on: " << right_handle << std::endl;
        assert( right_polygon.boundingBox ==
                right_node->boundingBox() );

        std::cout << "Right Polygon matches boundingBox() " <<
            right_handle << std::endl;
        */

        // Writeback our polygons
        if( left_polygon.basicRectangles.size() > MAX_RECTANGLE_COUNT ) {
            //FIXME: we actually realloc this, even if we don't have to.
            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        PAGE_DATA_SIZE );
            new (&(*alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator );
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
            //FIXME: we actually realloc this, even if we don't have to.
            auto alloc_data =
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        PAGE_DATA_SIZE );
            new (&(*alloc_data.first))
                InlineUnboundedIsotheticPolygon( allocator );
            split.rightBranch.boundingPoly = alloc_data.second;
            alloc_data.first->push_polygon_to_disk( right_polygon );
        } else {
            split.rightBranch.boundingPoly =
                InlineBoundedIsotheticPolygon();
            std::get<InlineBoundedIsotheticPolygon>(split.rightBranch.boundingPoly).push_polygon_to_disk(
                    right_polygon );
        }

        assert( node_poly.boundingBox.containsRectangle( left_polygon.boundingBox
                    ) and node_poly.boundingBox.containsRectangle(
                        right_polygon.boundingBox ) );

        cur_offset_ = 0;
    }
    return split;
}

// Splitting a node will remove it from its parent node and its memory will be freed
NODE_TEMPLATE_PARAMS
SplitResult NODE_CLASS_TYPES::splitNode()
{
    SplitResult returnSplit = splitNode(partitionNode());
    return returnSplit;
}

// This bottom-to-top sweep is only for splitting bounding boxes as necessary
NODE_TEMPLATE_PARAMS
SplitResult NODE_CLASS_TYPES::adjustTree()
{
    tree_node_handle current_handle = self_handle_;

    SplitResult propagationSplit = {
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
    };

    // Loop from the bottom to the very top
    while( current_handle != nullptr ) {
        auto current_node = treeRef->get_node( current_handle );

        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            {
                auto left_node = treeRef->get_node( propagationSplit.leftBranch.child );
                if( left_node->cur_offset_ > 0 ) {
                    current_node->entries.at(
                            current_node->cur_offset_++ ) =
                        propagationSplit.leftBranch;
                }
            }

            {
                auto right_node = treeRef->get_node( propagationSplit.rightBranch.child );

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

        // If we have a parent, they need to know about the recomputed
        // bounding boxes we have
        /*
        if( current_node->parent != nullptr and
                current_node->cur_offset_ <= max_branch_factor ) {
            std::cout << "Fixing up parent entries for my node." <<
                std::endl;

            std::cout << "Parent: " << current_node->parent <<
                std::endl;
            std::cout << "Me: " << current_node->self_handle_ <<
                std::endl;

            auto parent_node = allocator->get_tree_node<NodeType>(
                    current_node->parent );
            Branch my_branch = parent_node->locateBranch(
                    current_node->self_handle_ );

            pinned_node_ptr<InlineUnboundedIsotheticPolygon>
                poly_pin( allocator->buffer_pool_, nullptr, nullptr );

            // Get whatever polygon they currently have for us, and
            // shrink it.
            IsotheticPolygon branch_poly =
                my_branch.materialize_polygon( allocator );

            shrink( branch_poly, parent_node->entries.begin(),
                    parent_node->entries.end(), allocator );

            if( branch_poly.basicRectangles.size() <= MAX_RECTANGLE_COUNT ) {
                my_branch.boundingPoly =
                    InlineBoundedIsotheticPolygon();
                std::get<InlineBoundedIsotheticPolygon>(
                        my_branch.boundingPoly
                        ).push_polygon_to_disk( branch_poly );
            } else {
                auto alloc_data =
                    allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                            PAGE_DATA_SIZE );
                new (&(*alloc_data.first))
                    InlineUnboundedIsotheticPolygon( allocator );
                my_branch.boundingPoly = alloc_data.second;
                alloc_data.first->push_polygon_to_disk( branch_poly
                        );
            }
        }
        */

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
        // Cleanup before ascending
        if( current_node->parent != nullptr ) {
            auto parent_node = treeRef->get_node( current_node->parent );
            parent_node->removeEntry( current_handle );
        }

        // Ascend, propagating splits
        auto left_node = treeRef->get_node( propagationSplit.leftBranch.child );
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


        // FIXME: GC original root node
        //delete this;

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
        // FIXME GC ME 
        //delete this;
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
#ifdef STAT
    using NodeType = NODE_CLASS_TYPES;

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
                auto child = treeRef->get_node( b.child );
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
