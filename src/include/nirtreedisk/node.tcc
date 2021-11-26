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
#define NN_CLASS_TYPES Node<min_branch_factor,max_branch_factor,strategy>
#define LEAF_NODE_CLASS_TYPES LeafNode<min_branch_factor, max_branch_factor, strategy>
#define BRANCH_NODE_CLASS_TYPES BranchNode<min_branch_factor, max_branch_factor, strategy>

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::deleteSubtrees() {
    return;
}

NODE_TEMPLATE_PARAMS
Rectangle LEAF_NODE_CLASS_TYPES::boundingBox()
{
    assert( this->cur_offset_ > 0 );

    Point &p = entries.at(0);
    Rectangle bb(p, Point::closest_larger_point( p ) );
    for( unsigned i = 1; i < this->cur_offset_; i++ ) {
        bb.expand( entries.at( i ) );
    }
    return bb;
}

// If Branch:
// Removes a child logical pointer from a this->parent node, freeing that
// child's memory and the memory of the associated polygon (if external
// to the node).
// If Point: removes the point from the node

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::removePoint(
    const Point &point
) {
    // Locate the child
    unsigned childIndex;
    for( childIndex = 0; entries.at( childIndex ) != point and
            childIndex < this->cur_offset_; childIndex++ ) { }
    assert( entries.at( childIndex ) == point );

    // Replace this index with whatever is in the last position
    entries.at(childIndex) = entries.at( this->cur_offset_-1 );

    // Truncate array size
    this->cur_offset_--;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::exhaustiveSearch(
    Point &requestedPoint,
    std::vector<Point> &accumulator
) {
    // We are a leaf so add our data points when they are the search point
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Point &p = entries.at(i);
        if( requestedPoint == p ) {
            accumulator.push_back( p );
        }
    }
}

// Macros so that we don't have recursive function calls 

#define point_search_leaf_node( current_node, requestedPoint, accumulator ) \
    for( unsigned i = 0; i < current_node->cur_offset_; i++ ) { \
        Point &p = current_node->entries.at(i); \
        if( requestedPoint == p ) { \
            accumulator.push_back( p ); \
        } \
    }

#define point_search_leaf_handle( handle, requestedPoint, accumulator ) \
    auto current_node = treeRef->get_leaf_node( handle ); \
    point_search_leaf_node( current_node, requestedPoint, accumulator );
    

#define rectangle_search_leaf_handle( handle, requestedRectangle, accumulator ) \
    auto current_node = treeRef->get_leaf_node( handle ); \
    for( unsigned i = 0; i < current_node->cur_offset_; i++ ) { \
        if( requestedRectangle.containsPoint( current_node->entries.at(i) ) ) { \
            accumulator.push_back( current_node->entries.at(i) ); \
        } \
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
            Point &pinPoint = *cur_iter;
            if( basicRectangle.containsPoint(pinPoint) ) {
                shrunkRectangle.expand( pinPoint );
                addRectangle = true;
                assert( shrunkRectangle.containsPoint(pinPoint) );
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
tree_node_handle LEAF_NODE_CLASS_TYPES::chooseNode(Point givenPoint)
{
    return this->self_handle_;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::findLeaf(Point givenPoint)
{
    // FL2 [Search leaf node for record]
    // Check each entry to see if it matches E
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Point &p = std::get<Point>(entries.at(i) );
        if( p == givenPoint ) {
            return this->self_handle_;
        }
    }
    return tree_node_handle( nullptr );
}

NODE_TEMPLATE_PARAMS
Partition LEAF_NODE_CLASS_TYPES::partitionLeafNode()
{
    Partition defaultPartition;
    double totalMass = 0.0;
    Point variance = Point::atOrigin;
    Point average = Point::atOrigin;
    Point sumOfSquares = Point::atOrigin;

    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Point &dataPoint = entries.at(i);
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
Partition LEAF_NODE_CLASS_TYPES::partitionNode()
{
    return partitionLeafNode();
}

// We create two new nodes and free the old one.
// The old one is freed in adjustTree using removeEntry
// If we downsplit, then we won't call adjustTree for that split so we
// need to delete the node ourselves.
NODE_TEMPLATE_PARAMS
SplitResult LEAF_NODE_CLASS_TYPES::splitNode(
    Partition p,
    bool is_downsplit
) {
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );

    auto alloc_data =
        allocator->create_new_tree_node<LEAF_NODE_CLASS_TYPES>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first; // take pin
    new (&(*left_node)) LEAF_NODE_CLASS_TYPES( this->treeRef,
            this->parent, left_handle, level_ );
    assert( left_node->self_handle_ == left_handle );
    assert( left_handle.get_type() == LEAF_NODE );

    alloc_data = allocator->create_new_tree_node<LEAF_NODE_CLASS_TYPES>(
            NodeHandleType( LEAF_NODE ) );
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first; // take pin
    new (&(*right_node)) LEAF_NODE_CLASS_TYPES( this->treeRef,
            this->parent, right_handle, level_ );
    assert( right_node->self_handle_ == right_handle );
    assert( right_handle.get_type() == LEAF_NODE );

    SplitResult split = {
        { tree_node_handle(nullptr), left_handle }, 
        { tree_node_handle(nullptr), right_handle } };

    bool containedLeft, containedRight;
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Point &dataPoint =  entries.at(i);
        containedLeft = dataPoint[ p.dimension ] < p.location; // Not inclusive
        containedRight = dataPoint[ p.dimension ] >= p.location;
        assert( containedLeft or containedRight );

        if( containedLeft and not containedRight ) {
            left_node->addPoint( dataPoint );
        } else if( not containedLeft and containedRight ) {
            right_node->addPoint( dataPoint );
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
    if( this->parent ) {
        auto parent_node = this->treeRef->get_branch_node( this->parent );
        if( not is_downsplit ) {
            parent_node->make_disjoint_from_children( left_polygon,
                    this->self_handle_ );
            assert( left_polygon.basicRectangles.size() > 0 );
            left_polygon.refine();
            assert( left_polygon.basicRectangles.size() > 0 );
            parent_node->make_disjoint_from_children( right_polygon,
                    this->self_handle_ );
            assert( right_polygon.basicRectangles.size() > 0 );
            right_polygon.refine();
            assert( right_polygon.basicRectangles.size() > 0 );
        } else {
            // Intersect with our existing poly to avoid intersect
            // with other children
            Branch &b = parent_node->locateBranch( this->self_handle_ );
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
    this->cur_offset_ = 0;

    update_branch_polygon( split.leftBranch, left_polygon, treeRef, true );
    update_branch_polygon( split.rightBranch, right_polygon, treeRef, true );

    return split;
}

// Splitting a node will remove it from its this->parent node and its memory will be freed
NODE_TEMPLATE_PARAMS
SplitResult LEAF_NODE_CLASS_TYPES::splitNode()
{
    SplitResult returnSplit = splitNode(partitionNode(), false);
    return returnSplit;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::reInsert(
    std::vector<bool> &hasReinsertedOnLevel
) {

    // Taken from R*
    hasReinsertedOnLevel.at( level_ ) = true;

    Point globalCenterPoint = boundingBox().centrePoint();

    std::sort( entries.begin(), entries.begin() + cur_offset_,
        [&globalCenterPoint](Point &a, Point &b)
        {
            Rectangle rectA( a, Point::closest_larger_point( a ) );
            Rectangle rectB( b, Point::closest_larger_point( b ) );
            return rectA.centrePoint().distance( globalCenterPoint ) > rectB.centrePoint().distance( globalCenterPoint );
        });


    unsigned numNodesToReinsert =  0.3 * cur_offset_;
    unsigned remainder = cur_offset_ - numNodesToReinsert;

    std::vector<Point> entriesToReinsert;
    entriesToReinsert.reserve( numNodesToReinsert );

    std::copy(
        entries.begin() + remainder,
        entries.begin() + cur_offset_,
        std::back_inserter(entriesToReinsert)
    );

    cur_offset_ = remainder; 

    // These points will endup in exactly the same spot unless we
    // touch up the bounding boxes on the way up the tree to the root.
    // Each point path is unique, which means we would end up in the
    // same spot --- but it doesn't mean it is a good spot!

    // FIXME: Yolo for now, under the assumption that our siblings are
    // always disjoint so it should be fine.
    // Need to really think about whether this is correct
    // We want to adjust the paths on the way up to precisely reflect
    // what region we are in.

    fix_up_path_polys( self_handle_, treeRef );
    tree_node_handle root_handle = treeRef->root;

    for( const Point &entry : entriesToReinsert ) {
        if( root_handle.get_type() == LEAF_NODE ) {

            auto root_node = treeRef->get_leaf_node( root_handle );
            root_handle = root_node->insert( entry, hasReinsertedOnLevel );
        } else {
            auto root_node = treeRef->get_branch_node( root_handle );
            std::variant<Branch,Point> ent = entry;
            root_handle = root_node->insert( ent, hasReinsertedOnLevel );
        }
    }
}

template <int min_branch_factor, int max_branch_factor, class strategy>
IsotheticPolygon get_polygon_path_constraints(
    tree_node_handle start_handle,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    tree_node_handle parent_handle;
    if( start_handle.get_type() == LEAF_NODE ) {
        auto leaf_node = treeRef->get_leaf_node( start_handle );
        parent_handle = leaf_node->parent;
    } else {
        auto branch_node = treeRef->get_branch_node( start_handle );
        parent_handle = branch_node->parent;
    }
    if( not parent_handle ) {
        return IsotheticPolygon( Rectangle(Point::atNegInfinity,
                    Point::atInfinity) );
    }
    auto parent_node = treeRef->get_branch_node( parent_handle );
    Branch &b = parent_node->locateBranch( start_handle );
    IsotheticPolygon constraint_poly = b.materialize_polygon( allocator
            );
    tree_node_handle current_handle = parent_handle;

    while( current_handle ) {
        auto current_node = treeRef->get_branch_node( current_handle );
        tree_node_handle parent_handle = current_node->parent;
        if( not parent_handle ) {
            return constraint_poly;
        }
        auto parent_node = treeRef->get_branch_node( parent_handle );
        Branch &parent_branch = parent_node->locateBranch(
                current_handle );
        IsotheticPolygon parent_poly = parent_branch.materialize_polygon(
                    allocator );

        constraint_poly.intersection( parent_poly );
        constraint_poly.recomputeBoundingBox();
        current_handle = parent_handle;
    }
    return constraint_poly;
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void update_branch_polygon(
    Branch &branch_to_update,
    IsotheticPolygon &polygon_to_write,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef,
    bool force_create = false
) {
    if( polygon_to_write.basicRectangles.size() <= MAX_RECTANGLE_COUNT ) {
        // Could leak if we had an out of band rectangle before
        branch_to_update.boundingPoly = InlineBoundedIsotheticPolygon();
        std::get<InlineBoundedIsotheticPolygon>( branch_to_update.boundingPoly
                ).push_polygon_to_disk( polygon_to_write );
    } else {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        if( std::holds_alternative<tree_node_handle>(
                    branch_to_update.boundingPoly ) and not force_create ) {
            auto poly_pin =
                allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                        std::get<tree_node_handle>(
                            branch_to_update.boundingPoly ) );
            if( poly_pin->get_max_rectangle_count_on_first_page() ==
                    InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page() ) {
                poly_pin->push_polygon_to_disk( polygon_to_write );
                return;
            }
        }

        unsigned rect_count = force_create ?
            polygon_to_write.basicRectangles.size() :
            InlineUnboundedIsotheticPolygon::maximum_possible_rectangles_on_first_page();

        auto alloc_data =
            allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                    compute_sizeof_inline_unbounded_polygon(
                        rect_count ), NodeHandleType( BIG_POLYGON ) );
        new (&(*alloc_data.first)) InlineUnboundedIsotheticPolygon(
                allocator,
                rect_count );
        alloc_data.first->push_polygon_to_disk( polygon_to_write );
        branch_to_update.boundingPoly = alloc_data.second;
    }
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::reInsert(
    std::vector<bool> &hasReinsertedOnLevel
) {

    // Taken from R*
    hasReinsertedOnLevel.at( level_ ) = true;

    Point globalCenterPoint = boundingBox().centrePoint();

    std::sort( entries.begin(), entries.begin() + cur_offset_,
        [&globalCenterPoint, this](Branch &a, Branch &b)
        {
            Rectangle rectA = a.materialize_polygon(
                    treeRef->node_allocator_.get() ).boundingBox;
            Rectangle rectB = b.materialize_polygon(
                    treeRef->node_allocator_.get() ).boundingBox;

            return rectA.centrePoint().distance(
                    globalCenterPoint ) > rectB.centrePoint().distance( globalCenterPoint );
        });


    unsigned numNodesToReinsert =  0.3 * cur_offset_;
    unsigned remainder = cur_offset_ - numNodesToReinsert;

    std::vector<Branch> entriesToReinsert;
    entriesToReinsert.reserve( numNodesToReinsert );

    std::copy(
        entries.begin() + remainder,
        entries.begin() + cur_offset_,
        std::back_inserter(entriesToReinsert)
    );

    // We need to perfectly qualify what this branch holds so that other
    // people can fragment around it.
    // This needs to be here because otherwise the mask will be changed
    // by adjusting our bb.
    IsotheticPolygon true_region_mask = get_polygon_path_constraints(
            self_handle_, treeRef );

    cur_offset_ = remainder; 

    auto tree_ref_backup = treeRef;
    tree_node_allocator *allocator = get_node_allocator(
            tree_ref_backup );

    fix_up_path_polys( self_handle_, treeRef );
    for( Branch &entry : entriesToReinsert ) {
        // FIXME: consolidate with stuff below.
        IsotheticPolygon branch_poly = entry.materialize_polygon(
                allocator );
        branch_poly.intersection( true_region_mask );

        cut_out_branch_region_in_path( self_handle_, branch_poly, treeRef );
    }

    tree_node_handle root_handle = tree_ref_backup->root;

    for( Branch &entry : entriesToReinsert ) {

        IsotheticPolygon branch_poly = entry.materialize_polygon(
                allocator );

        branch_poly.intersection( true_region_mask );
        update_branch_polygon( entry, branch_poly, tree_ref_backup );
        auto root_node = tree_ref_backup->get_branch_node( root_handle );

        std::variant<Branch,Point> ent = entry;
        root_handle = root_node->insert( ent, hasReinsertedOnLevel );
    }
}

template <class NT, class TR>
std::pair<SplitResult, tree_node_handle> adjust_tree_bottom_half(
    NT current_node,
    TR *tree_ref_backup,
    int max_branch_factor,
    std::vector<bool> &hasReinsertedOnLevel
) {
    SplitResult propagationSplit = {
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
    };

    if( current_node->cur_offset_ <= (unsigned) max_branch_factor ) {
        return std::make_pair( propagationSplit,
                tree_node_handle(nullptr) );
    }

    // Otherwise, split node
    if( hasReinsertedOnLevel.at( current_node->level_ ) or true ) {
        tree_node_handle parent = current_node->parent;
        auto propagationSplit = current_node->splitNode();

        // Cleanup before ascending
        if( parent != nullptr ) {
            // This will probably destroy current_node, so if we need
            // current node for anything, need to do it before the
            // removeEntry call.

            auto parent_node = tree_ref_backup->get_branch_node( parent );
            parent_node->removeBranch( current_node->self_handle_ );
        }

        // Ascend, propagating splits
        return std::make_pair( propagationSplit, parent );
    } else {
        // Nothing is real after you make this call
        // The reinsert might have come back around again and split this
        // node, or other nodes, or everyting
        // Signal to the caller that we shoudl stop
        current_node->reInsert( hasReinsertedOnLevel );
        return std::make_pair( propagationSplit, 
                tree_node_handle(nullptr) );
    }
}

// This bottom-to-top sweep is only for splitting bounding boxes as necessary
NODE_TEMPLATE_PARAMS
SplitResult LEAF_NODE_CLASS_TYPES::adjustTree(
    std::vector<bool> &hasReinsertedOnLevel
) {
    // N.B., as we walk up the tree, we may perform a bunch of splits,
    // which is liable to destroy nodes that are downsplit. These
    // downsplit nodes' memory can then be re-used for other things,
    // like polygons. If we try to use treeRef (or other outside
    // pointers) from that node, it can be clobbered leading to amazing
    // segfaults. It is important that any variables we reference are
    // those we know are alive --- don't just rely on whatever the 
    // leaf node class member to have reasonable things after split is
    // called!

    tree_node_handle current_handle = this->self_handle_;

    SplitResult propagationSplit = {
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
    };

    NIRTreeDisk<min_branch_factor,max_branch_factor, strategy> *tree_ref_backup =
        this->treeRef;

    // Loop from the bottom to the very top
    while( current_handle != nullptr ) {

        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            // We are at least one level up, so have to be a branch

            auto current_branch_node = tree_ref_backup->get_branch_node(
                    current_handle );
            {
                if( propagationSplit.leftBranch.child.get_type() ==
                        LEAF_NODE ) {
                    auto left_node =
                        tree_ref_backup->get_leaf_node( propagationSplit.leftBranch.child );
                    if( left_node->cur_offset_ > 0 ) {
                        current_branch_node->addBranchToNode(
                            propagationSplit.leftBranch );
                    }
                } else {
                    auto left_node =
                        tree_ref_backup->get_branch_node( propagationSplit.leftBranch.child );
                    if( left_node->cur_offset_ > 0 ) {
                        current_branch_node->addBranchToNode(
                            propagationSplit.leftBranch );
                    }
                }
            }
            {

                if( propagationSplit.rightBranch.child.get_type() ==
                        LEAF_NODE ) {
                    auto right_node = tree_ref_backup->get_leaf_node( propagationSplit.rightBranch.child );
                    if( right_node->cur_offset_ > 0 ) {

                        current_branch_node->addBranchToNode( propagationSplit.rightBranch );
                    }
                } else {
                    auto right_node = tree_ref_backup->get_branch_node( propagationSplit.rightBranch.child );
                    if( right_node->cur_offset_ > 0 ) {

                        current_branch_node->addBranchToNode( propagationSplit.rightBranch );
                    }

                }
            }

        }

        std::pair<SplitResult, tree_node_handle>
            split_res_and_new_handle;
        if( current_handle.get_type() == LEAF_NODE ) {
            auto current_leaf_node = tree_ref_backup->get_leaf_node(
                    current_handle );
            split_res_and_new_handle = adjust_tree_bottom_half(
                    current_leaf_node, tree_ref_backup,
                    max_branch_factor, hasReinsertedOnLevel );
        } else {
            auto current_branch_node = tree_ref_backup->get_branch_node(
                    current_handle );
            split_res_and_new_handle = adjust_tree_bottom_half(
                    current_branch_node, tree_ref_backup,
                    max_branch_factor, hasReinsertedOnLevel );
        }

        propagationSplit = split_res_and_new_handle.first;
        current_handle = split_res_and_new_handle.second;
    }
    return propagationSplit;
}

template <int min_branch_factor, int max_branch_factor, class strategy>
void fix_up_path_polys(
    tree_node_handle start_handle,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {
    tree_node_handle current_handle = start_handle;
    while( current_handle != nullptr ) {
        // Regenerate this node's bounding polygon

        tree_node_handle parent_handle;
        IsotheticPolygon our_poly;
        if( current_handle.get_type() == LEAF_NODE ) {
            auto leaf_node = treeRef->get_leaf_node( current_handle );
            our_poly = IsotheticPolygon( leaf_node->boundingBox() );
            parent_handle = leaf_node->parent;
        } else {
            auto branch_node = treeRef->get_branch_node( current_handle );
            our_poly = IsotheticPolygon( branch_node->boundingBox() );
            parent_handle = branch_node->parent;
        }
        if( parent_handle ) {
            // Q: Is it possible that this is bad?
            // Suppose we just transferred a branch from another region
            // to this spot. Then we might need to expand our polygon,
            // which intersects with other person's polygon who is
            // slightly more cavalier about what regions they think they
            // own. But we now *own* this space. So we need to make sure
            // they don't take it. How can we do that? Fragment their
            // rectangle on the way down.
            auto parent_node = treeRef->get_branch_node( parent_handle );
   
            // Make this polygon disjoint from its siblings
            parent_node->make_disjoint_from_children( our_poly, current_handle );

            // Now we need to store this poly
            Branch &parent_branch = parent_node->locateBranch( current_handle );
            update_branch_polygon( parent_branch, our_poly, treeRef );
        }
        current_handle = parent_handle;
    }
    // Hit the root, done!
}



template <int min_branch_factor, int max_branch_factor, class strategy>
void cut_out_branch_region_in_path(
    tree_node_handle start_handle,
    IsotheticPolygon &region_to_cut_out,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {
    tree_node_handle current_handle = start_handle;
    while( current_handle != nullptr ) {
        // Regenerate this node's bounding polygon

        tree_node_handle parent_handle;
        IsotheticPolygon our_poly;
        if( current_handle.get_type() == LEAF_NODE ) {
            auto leaf_node = treeRef->get_leaf_node( current_handle );
            our_poly = IsotheticPolygon( leaf_node->boundingBox() );
            parent_handle = leaf_node->parent;
        } else {
            auto branch_node = treeRef->get_branch_node( current_handle );
            our_poly = IsotheticPolygon( branch_node->boundingBox() );
            parent_handle = branch_node->parent;
        }
        if( parent_handle ) {
            auto parent_node = treeRef->get_branch_node( parent_handle );

            Branch &my_branch  = parent_node->locateBranch( current_handle );
            IsotheticPolygon our_poly = my_branch.materialize_polygon(
                    treeRef->node_allocator_.get() );
            our_poly.increaseResolution( Point::atInfinity, region_to_cut_out );
            our_poly.refine();
            our_poly.recomputeBoundingBox();
   
            Branch &parent_branch = parent_node->locateBranch( current_handle );
            update_branch_polygon( parent_branch, our_poly, treeRef );
        }
        current_handle = parent_handle;
    }
    // Hit the root, done!
}

template <int min_branch_factor, int max_branch_factor, class strategy>
SplitResult adjustTreeSub(
    std::vector<bool> &hasReinsertedOnLevel,
    tree_node_handle start_handle,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {
    // N.B., as we walk up the tree, we may perform a bunch of splits,
    // which is liable to destroy nodes that are downsplit. These
    // downsplit nodes' memory can then be re-used for other things,
    // like polygons. If we try to use variables stored in that
    // node, it can be clobbered leading to amazing
    // segfaults. It is important that any variables we reference are
    // those we know are alive.

    tree_node_handle current_handle = start_handle;

    SplitResult propagationSplit = {
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
        {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
    };

    // Loop from the bottom to the very top
    while( current_handle != nullptr ) {

        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            // We are at least one level up, so have to be a branch

            auto current_branch_node = treeRef->get_branch_node(
                    current_handle );

            {
                if( propagationSplit.leftBranch.child.get_type() ==
                        LEAF_NODE ) {
                    auto left_node =
                        treeRef->get_leaf_node( propagationSplit.leftBranch.child );
                    if( left_node->cur_offset_ > 0 ) {
                        current_branch_node->addBranchToNode(
                            propagationSplit.leftBranch );
                    }
                } else {
                    auto left_node =
                        treeRef->get_branch_node( propagationSplit.leftBranch.child );
                    if( left_node->cur_offset_ > 0 ) {
                        current_branch_node->addBranchToNode(
                            propagationSplit.leftBranch );
                    }
                }
            }
            {

                if( propagationSplit.rightBranch.child.get_type() ==
                        LEAF_NODE ) {
                    auto right_node = treeRef->get_leaf_node( propagationSplit.rightBranch.child );
                    if( right_node->cur_offset_ > 0 ) {

                        current_branch_node->addBranchToNode( propagationSplit.rightBranch );
                    }
                } else {
                    auto right_node = treeRef->get_branch_node( propagationSplit.rightBranch.child );
                    if( right_node->cur_offset_ > 0 ) {

                        current_branch_node->addBranchToNode( propagationSplit.rightBranch );
                    }

                }
            }
        }


        std::pair<SplitResult, tree_node_handle>
            split_res_and_new_handle;
        if( current_handle.get_type() == LEAF_NODE ) {
            auto current_leaf_node = treeRef->get_leaf_node(
                    current_handle );
            split_res_and_new_handle = adjust_tree_bottom_half(
                    current_leaf_node, treeRef,
                    max_branch_factor, hasReinsertedOnLevel );
        } else {
            auto current_branch_node = treeRef->get_branch_node(
                    current_handle );
            split_res_and_new_handle = adjust_tree_bottom_half(
                    current_branch_node, treeRef,
                    max_branch_factor, hasReinsertedOnLevel );
        }

        propagationSplit = split_res_and_new_handle.first;
        current_handle = split_res_and_new_handle.second;
    }
    return propagationSplit;
}

// This always get called on the root node. So if it got called on us,
// that's because we are the only node in the whole tree.
NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::insert(
    Point givenPoint,
    std::vector<bool> &hasReinsertedOnLevel
) {

    // This is a leaf, so we are the ONLY node.
    addPoint( givenPoint );

    auto tree_ref_backup = treeRef;

    SplitResult finalSplit = adjustTreeSub( hasReinsertedOnLevel,
            self_handle_, treeRef );

    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        tree_node_allocator *allocator = get_node_allocator( this->treeRef );
        auto alloc_data =
            allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                    NodeHandleType( BRANCH_NODE ));
        new (&(*alloc_data.first)) BRANCH_NODE_CLASS_TYPES( this->treeRef,
                tree_node_handle(nullptr), alloc_data.second, level_+1 );
        auto new_root_handle = alloc_data.second;
        assert( new_root_handle.get_type() == BRANCH_NODE );
        auto new_root_node = alloc_data.first;


        auto left_node = this->treeRef->get_leaf_node( finalSplit.leftBranch.child );
        left_node->parent = new_root_handle;
        new_root_node->addBranchToNode( finalSplit.leftBranch );

        auto right_node = this->treeRef->get_leaf_node( finalSplit.rightBranch.child );
        right_node->parent = new_root_handle;
        new_root_node->addBranchToNode( finalSplit.rightBranch );

        assert( new_root_handle.get_type() == BRANCH_NODE );
        treeRef->root = new_root_handle;

        assert( this->self_handle_.get_type() == LEAF_NODE );
        allocator->free( this->self_handle_, sizeof( LEAF_NODE_CLASS_TYPES ) );


        // Fix the reinserted length
        hasReinsertedOnLevel.push_back(false);

        return new_root_handle;
    }


    return tree_ref_backup->root;
}

// To be called on a leaf
NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::condenseTree()
{
    auto current_node_handle = this->self_handle_;
    auto previous_node_handle = tree_node_handle( nullptr );

    while( current_node_handle != nullptr ) {
        if( current_node_handle.get_type() == LEAF_NODE ) {
            auto current_node = treeRef->get_leaf_node( current_node_handle );
            current_node_handle = current_node->parent;
        } else {
            auto current_node = treeRef->get_branch_node( current_node_handle );
            current_node_handle = current_node->parent;
        }
        if( previous_node_handle != nullptr ) {
            auto current_parent_node = this->treeRef->get_branch_node( current_node_handle );
            unsigned loc_cur_offset = 0;
            if( previous_node_handle.get_type() == LEAF_NODE ) {
                auto previous_node = treeRef->get_leaf_node(
                        previous_node_handle );
                loc_cur_offset = previous_node->cur_offset_;
            } else {
                auto previous_node = treeRef->get_branch_node(
                        previous_node_handle );
                loc_cur_offset = previous_node->cur_offset_;
            }
            if( loc_cur_offset == 0 ) {
                current_parent_node->removeBranch( previous_node_handle );
            }
        }
        previous_node_handle = current_node_handle;
    }
}

// Always called on root, this = root
NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::remove( Point givenPoint ) {

    removePoint( givenPoint );
    return this->self_handle_;
}

NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::checksum() {
    unsigned sum = 0;

    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Point &dataPoint = entries.at(i);
        for( unsigned d = 0; d < dimensions; d++ ) {
            sum += (unsigned) dataPoint[d];
        }
    }
    return sum;
}

NODE_TEMPLATE_PARAMS
std::vector<Point> LEAF_NODE_CLASS_TYPES::bounding_box_validate()
{
    std::vector<Point> my_points;
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        my_points.push_back( entries.at(i) );
    }
    return my_points;
}

NODE_TEMPLATE_PARAMS
bool LEAF_NODE_CLASS_TYPES::validate( tree_node_handle expectedParent, unsigned index) {

    if( expectedParent != nullptr and (this->parent != expectedParent ||
                this->cur_offset_ > max_branch_factor )) {
        std::cout << "node = " << (void *)this << std::endl;
        std::cout << "parent = " << this->parent << " expectedParent = " << expectedParent << std::endl;
        std::cout << "maxBranchFactor = " << max_branch_factor << std::endl;
        std::cout << "entries.size() = " << this->cur_offset_ << std::endl;
        assert(this->parent == expectedParent);
    }

    if( expectedParent != nullptr ) {
        tree_node_allocator *allocator = get_node_allocator( this->treeRef );

        auto parent_node = treeRef->get_branch_node( parent );
        Branch &branch = parent_node->locateBranch( this->self_handle_ );

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

       for( unsigned i = 0; i < this->cur_offset_; i++ ) {
            Point &dataPoint = entries.at(i);
            if( !poly.containsPoint( dataPoint ) ) {
                std::cout << poly << " fails to contain " << dataPoint << std::endl;
                std::cout << "Node: " << self_handle_ << std::endl;
                std::cout << "Parent: " << parent << std::endl;
                assert( false );
            }
        }

    }
    return true;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::print(unsigned n)
{
    std::string indentation(n * 4, ' ');
    std::cout << indentation << "Node " << (void *)this << std::endl;
    std::cout << indentation << "    Parent: " << this->parent << std::endl;
    std::cout << indentation << "    Data: ";
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        std::cout << entries.at(i);
    }
    std::cout << std::endl;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::printTree(unsigned n)
{
    // Print this node first
    print(n);
    std::cout << std::endl;
}

NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::height()
{
    return 1;
}

NODE_TEMPLATE_PARAMS
uint16_t LEAF_NODE_CLASS_TYPES::compute_packed_size() {
    uint16_t sz = 0;
    sz += sizeof( treeRef );
    sz += sizeof( self_handle_ );
    sz += sizeof( parent );
    sz += sizeof( cur_offset_ );
    sz += cur_offset_ * sizeof(Point);
    return sz;
}

NODE_TEMPLATE_PARAMS
uint16_t BRANCH_NODE_CLASS_TYPES::compute_packed_size(
    tree_node_allocator *existing_allocator,
    tree_node_allocator *new_allocator,
    unsigned &maximum_repacked_rect_size
) {
    uint16_t sz = 0;
    sz += sizeof( treeRef );
    sz += sizeof( self_handle_ );
    sz += sizeof( parent );
    sz += sizeof( cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        sz += entries.at(i).compute_packed_size( existing_allocator,
                new_allocator, maximum_repacked_rect_size );
    }
    return sz;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::repack( tree_node_allocator *allocator ) {
    std::cout << "Repacking: " << self_handle_ << std::endl;
    static_assert( sizeof( void * ) == sizeof(uint64_t) );
    uint16_t alloc_size = compute_packed_size();
    auto alloc_data = allocator->create_new_tree_node<packed_node>(
            alloc_size, NodeHandleType(REPACKED_LEAF_NODE) );
    std::cout << "New allocator handed us handle: " << alloc_data.second
        << std::endl;

    char *buffer = alloc_data.first->buffer_;
    buffer += write_data_to_buffer( buffer, &treeRef );
    buffer += write_data_to_buffer( buffer, &(alloc_data.second) );
    buffer += write_data_to_buffer( buffer, &parent );
    buffer += write_data_to_buffer( buffer, &cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        buffer += write_data_to_buffer( buffer, &(entries.at(i)) );
    }
    if( buffer - alloc_data.first->buffer_ != alloc_size ) {
        std::cout << "Memory corruption, abort." << std::endl;
        abort();
    }

    assert( buffer - alloc_data.first->buffer_ == alloc_size );
    std::cout << "Done. New handle is now: " << alloc_data.second << std::endl;
    return alloc_data.second;
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::repack( tree_node_allocator
        *existing_allocator, tree_node_allocator *new_allocator ) {
    std::cout << "Repacking: " << self_handle_ << std::endl;
    static_assert( sizeof( void * ) == sizeof(uint64_t) );
    unsigned maximum_repacked_rect_size;
    uint16_t alloc_size = 0;
    for( maximum_repacked_rect_size = 25;
            maximum_repacked_rect_size >= 5; maximum_repacked_rect_size
            /= 2 ) {
        alloc_size = compute_packed_size(
            existing_allocator,
            new_allocator,
            maximum_repacked_rect_size
        );
        if( alloc_size <= PAGE_DATA_SIZE ) {
            break;
        }
    }
    assert( alloc_size <= PAGE_DATA_SIZE );
    assert( maximum_repacked_rect_size >= 5 );
    auto alloc_data = new_allocator->create_new_tree_node<packed_node>(
            alloc_size, NodeHandleType(REPACKED_BRANCH_NODE) );
    std::cout << "New allocator handed us handle: " << alloc_data.second
        << std::endl;

    std::cout << "Going to write to buffer." << std::endl;
    char *buffer = alloc_data.first->buffer_;
    std::cout << "Writing " << (void *) treeRef << " to " << (void *)
        buffer;
    buffer += write_data_to_buffer( buffer, &treeRef );
    buffer += write_data_to_buffer( buffer, &(alloc_data.second) );
    buffer += write_data_to_buffer( buffer, &parent );
    buffer += write_data_to_buffer( buffer, &cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = entries.at(i);
        buffer += b.repack_into( buffer, existing_allocator,
                new_allocator, maximum_repacked_rect_size );
    }
    unsigned true_size = (buffer - alloc_data.first->buffer_);
    if( alloc_size != true_size ) {
        std::cout << "Memory corruption, abort." << std::endl;
        abort();
    }
    assert( true_size == alloc_size );
    std::cout << "Done. New handle is now: " << alloc_data.second << std::endl;
    return alloc_data.second;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::deleteSubtrees()
{
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Branch &b = entries.at(i);
        tree_node_handle child_handle = b.child;
        
        if( child_handle.get_type() == LEAF_NODE ) {
            allocator->free(
                child_handle,
                sizeof( LEAF_NODE_CLASS_TYPES )
            );
        } else if( child_handle.get_type() == BRANCH_NODE ) {
            auto child = this->treeRef->get_branch_node( child_handle );
            child->deleteSubtrees();
            allocator->free(
                child_handle,
                sizeof( BRANCH_NODE_CLASS_TYPES )
            );
        }
    }
}

NODE_TEMPLATE_PARAMS
Rectangle BRANCH_NODE_CLASS_TYPES::boundingBox()
{
    assert( this->cur_offset_ > 0 );

    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    Branch &b = entries.at(0);
    Rectangle bb = b.get_summary_rectangle( allocator );

    for( unsigned i = 1; i < this->cur_offset_; i++ ) {
        Branch &b2 = entries.at(i);
        bb.expand( b2.get_summary_rectangle( allocator ) );
    }
    return bb;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::updateBranch(
    tree_node_handle child_handle, 
    const InlineBoundedIsotheticPolygon &boundingPoly
) {
    // Locate the child
    unsigned childIndex;
    for( childIndex = 0; entries.at(childIndex).child != child_handle &&
            childIndex < this->cur_offset_; childIndex++ ) { }

    // Update the child
    entries.at( childIndex ).boundingPoly = boundingPoly;
}

// Removes a child logical pointer from a this->parent node, freeing that
// child's memory and the memory of the associated polygon (if external
// to the node).
NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::removeBranch(
    const tree_node_handle &entry
) {
    unsigned found_index = 0;
    while( found_index < this->cur_offset_ ) {
        Branch &b = entries.at( found_index );
        if( b.child == entry ) {
            break;
        }
        found_index++;
    }
    assert( entries.at( found_index ).child ==  entry );
    Branch &b = entries.at( found_index );
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );

    if( std::holds_alternative<tree_node_handle>( b.boundingPoly ) ) {
        tree_node_handle free_poly_handle = std::get<tree_node_handle>(
                b.boundingPoly );
        auto poly_pin =
            InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                    allocator, free_poly_handle );
        poly_pin->free_subpages( allocator );
        uint16_t alloc_size = compute_sizeof_inline_unbounded_polygon(
                poly_pin->get_max_rectangle_count_on_first_page() );
        allocator->free( free_poly_handle, alloc_size );
    }

    if( b.child.get_type() == LEAF_NODE ) {
        allocator->free( b.child, sizeof( LEAF_NODE_CLASS_TYPES ) );
    } else {
        allocator->free( b.child, sizeof( BRANCH_NODE_CLASS_TYPES ) );
    }
    b.child = tree_node_handle( nullptr );

    entries.at( found_index ) = entries.at( this->cur_offset_-1 );
    this->cur_offset_--;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
{
    // Follow all branches, this is exhaustive
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Branch &b = entries.at(i);
        if( b.child.get_type() == LEAF_NODE ) {
            auto child = this->treeRef->get_leaf_node( b.child );
            child->exhaustiveSearch( requestedPoint, accumulator );
        } else {
            auto child = this->treeRef->get_branch_node( b.child );
            child->exhaustiveSearch( requestedPoint, accumulator );

        }
    }
}

// Macros for sub-routines in search to guarantee that this code is
// inlined into the main methods without a function call or the need for
// recursive search calls. 

// Sets unsigned offset pointing to start of leaf entries and unsigned count
#define decode_entry_count_and_offset_packed_node( data ) \
    unsigned offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    unsigned count = * (unsigned *) (data+offset); \
    offset += sizeof( unsigned );

#define point_search_packed_leaf_node( packed_leaf, requestedPoint, accumulator ) \
    char *data = packed_leaf->buffer_; \
    decode_entry_count_and_offset_packed_node( data ); \
    for( unsigned i = 0; i < count; i++ ) { \
        Point *p = (Point *) (data + offset); \
        if( *p == requestedPoint ) { \
            accumulator.push_back( requestedPoint ); \
        } \
        offset += sizeof( Point ); \
    }

#define point_search_packed_leaf_handle( handle, requestedPoint, accumulator ) \
    assert( handle.get_type() == REPACKED_LEAF_NODE ); \
    auto packed_leaf = allocator->get_tree_node<packed_node>( handle ); \
    point_search_packed_leaf_node( packed_leaf, requestedPoint, accumulator );

#define rectangle_search_packed_leaf_handle( handle, requestedRectangle, accumulator ) \
    auto packed_leaf = allocator->get_tree_node<packed_node>( handle ); \
    char *data = packed_leaf->buffer_; \
    decode_entry_count_and_offset_packed_node( data ) \
    for( unsigned i = 0; i < count; i++ ) { \
        Point *p = (Point *) (data + offset); \
        if( requestedRectangle.containsPoint( *p ) ) { \
            accumulator.push_back( *p ); \
        } \
        offset += sizeof( Point ); \
    }

#define point_search_branch_handle( handle, requestedPoint, context ) \
    auto current_node = treeRef->get_branch_node( handle ); \
    for( unsigned i = 0; i < current_node->cur_offset_; i++ ) { \
        Branch &b = current_node->entries.at(i); \
        if( std::holds_alternative<InlineBoundedIsotheticPolygon>( \
                    b.boundingPoly ) ) { \
            InlineBoundedIsotheticPolygon &loc_poly =  \
                std::get<InlineBoundedIsotheticPolygon>( \
                        b.boundingPoly ); \
            if( loc_poly.containsPoint( requestedPoint ) ) { \
                context.push( b.child ); \
            } \
        } else { \
            tree_node_handle poly_handle = \
                std::get<tree_node_handle>( b.boundingPoly ); \
            auto poly_pin = \
                InlineUnboundedIsotheticPolygon::read_polygon_from_disk( \
                        allocator, poly_handle ); \
            if( poly_pin->containsPoint( requestedPoint ) ) { \
                context.push( b.child ); \
            } \
        } \
    }

#define point_search_packed_branch_handle( handle, requestedPoint, context ) \
    auto packed_branch = allocator->get_tree_node<packed_node>( handle ); \
    char *buffer = packed_branch->buffer_; \
    decode_entry_count_and_offset_packed_node( buffer ); \
    for( unsigned i = 0; i < count; i++ ) { \
        tree_node_handle *child = (tree_node_handle *) (buffer + offset); \
        offset += sizeof( tree_node_handle ); \
        unsigned rect_count = * (unsigned *) (buffer + offset); \
        offset += sizeof( unsigned ); \
        if( rect_count == std::numeric_limits<unsigned>::max() ) { \
            tree_node_handle *poly_handle = (tree_node_handle *) (buffer + offset ); \
            offset += sizeof( tree_node_handle ); \
            auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>( *poly_handle ); \
            if( poly_pin->containsPoint( requestedPoint ) ) { \
                context.push( *child ); \
            } \
        } else { \
            for( unsigned r = 0; r < rect_count; r++ ) { \
                Rectangle *rect = (Rectangle *) (buffer + offset); \
                offset += sizeof(Rectangle); \
                if( rect->containsPoint( requestedPoint ) ) { \
                    context.push( *child ); \
                    offset += (rect_count-r-1) * sizeof(Rectangle); \
                    break; \
                } \
            } \
        } \
    }

#define rectangle_search_branch_handle( handle, requestedRectangle, context ) \
    auto current_node = treeRef->get_branch_node( handle ); \
    for( unsigned i = 0; i < current_node->cur_offset_; i++ ) { \
        Branch &b = current_node->entries.at(i); \
        if( std::holds_alternative<InlineBoundedIsotheticPolygon>( \
                    b.boundingPoly ) ) { \
            InlineBoundedIsotheticPolygon &loc_poly =  \
                std::get<InlineBoundedIsotheticPolygon>( \
                        b.boundingPoly ); \
            if( loc_poly.intersectsRectangle( requestedRectangle) ) { \
                context.push( b.child ); \
            } \
        } else { \
            tree_node_handle poly_handle = \
                std::get<tree_node_handle>( b.boundingPoly ); \
            auto poly_pin = \
                InlineUnboundedIsotheticPolygon::read_polygon_from_disk( \
                        allocator, poly_handle ); \
            if( poly_pin->intersectsRectangle( requestedRectangle ) ) { \
                context.push( b.child ); \
            } \
        } \
    }

#define rectangle_search_packed_branch_handle( handle, requestedRectangle, context ) \
    auto packed_branch = allocator->get_tree_node<packed_node>( handle ); \
    char *buffer = packed_branch->buffer_; \
    decode_entry_count_and_offset_packed_node( buffer ); \
    for( unsigned i = 0; i < count; i++ ) { \
        tree_node_handle *child = (tree_node_handle *) (buffer + offset); \
        offset += sizeof( tree_node_handle ); \
        unsigned rect_count = * (unsigned *) (buffer + offset); \
        offset += sizeof( unsigned ); \
        if( rect_count == std::numeric_limits<unsigned>::max() ) { \
            tree_node_handle *poly_handle = (tree_node_handle *) (buffer + offset ); \
            offset += sizeof( tree_node_handle ); \
            auto poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>( *poly_handle ); \
            if( poly_pin->intersectsRectangle(requestedRectangle) ) { \
                context.push( *child ); \
            } \
        } else { \
            for( unsigned r = 0; r < rect_count; r++ ) { \
                Rectangle *rect = (Rectangle *) (buffer + offset); \
                offset += sizeof(Rectangle); \
                if( rect->intersectsRectangle( requestedRectangle ) ) { \
                    context.push( *child ); \
                    offset += (rect_count-r-1) * sizeof(Rectangle); \
                    break; \
                } \
            } \
        } \
    }


template <int min_branch_factor, int max_branch_factor, class
    strategy>
std::vector<Point> point_search(
    tree_node_handle start_point,
    Point &requestedPoint,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {
    std::vector<Point> accumulator;
    std::stack<tree_node_handle> context;
    context.push( start_point );
    tree_node_allocator *allocator = treeRef->node_allocator_.get();

    while( not context.empty() ) {
        tree_node_handle current_handle = context.top();
        context.pop();
        if( current_handle.get_type() == LEAF_NODE ) {
            point_search_leaf_handle( current_handle, requestedPoint, accumulator );
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        } else if( current_handle.get_type() == REPACKED_LEAF_NODE ) {
            point_search_packed_leaf_handle( current_handle,
                    requestedPoint, accumulator );
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        } else if( current_handle.get_type() == BRANCH_NODE ) {
            point_search_branch_handle( current_handle,
                    requestedPoint, context );
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        } else if( current_handle.get_type() == REPACKED_BRANCH_NODE ) {
            point_search_packed_branch_handle( current_handle,
                    requestedPoint, context );
#ifdef STAT

            treeRef->stats.markNonLeafNodeSearched();
#endif
        } else {
            assert( false );
        }
    }
#ifdef STAT
    treeRef->stats.resetSearchTracker( false );
#endif
    return accumulator;
}

// Repack this subtree's data into the most compact representation
// we can muster. This greatly increases query performance, but means
// that we will need to convert it back if we ever need to shard a
// rectangle during insertion. 
//
// N.B.: This code frees the old subtree and polygons as well.
// You should not rely on any of the data in the old tree after you call
// ths function!
NODE_TEMPLATE_PARAMS
tree_node_handle repack_subtree(
    tree_node_handle handle,
    tree_node_allocator *existing_allocator,
    tree_node_allocator *new_allocator
) {
    std::vector<tree_node_handle> repacked_handles;
    switch( handle.get_type() ) {
        case LEAF_NODE: {
            auto leaf_node =
                existing_allocator->get_tree_node<LEAF_NODE_CLASS_TYPES>( handle );
            auto new_handle = leaf_node->repack( new_allocator );
            existing_allocator->free( handle, sizeof(
                        LEAF_NODE_CLASS_TYPES ) );
            return new_handle;
        }
        case BRANCH_NODE: {
            auto branch_node =
                existing_allocator->get_tree_node<BRANCH_NODE_CLASS_TYPES>( handle );
            // Repack all my children, adjust my handles
            for( unsigned i = 0; i < branch_node->cur_offset_; i++ ) {
                auto child_handle = branch_node->entries.at(i).child;
                auto new_child_handle =
                    repack_subtree<min_branch_factor,max_branch_factor,strategy>( child_handle,
                        existing_allocator, new_allocator );
                branch_node->entries.at(i).child = new_child_handle;
            }
            auto new_handle = branch_node->repack( existing_allocator, new_allocator );

            // Children nodes want to know who their parent is, but
            // we don't know that until we repack the branch node above.
            // So, we re-walk the new children here and set up all their
            // parents.
            std::cout << "Adjusting children parents" << std::endl;
            for( unsigned i = 0; i < branch_node->cur_offset_; i++ ) {
                std::cout << "Adjusting child: " <<
                    branch_node->entries.at(i).child << std::endl;
                auto new_child =
                    new_allocator->get_tree_node<packed_node>(
                            branch_node->entries.at(i).child );
                std::cout << "Reading " << (void *) new_child->buffer_
                    << std::endl;
                std::cout << "Got treeRef: " << (void *)
                    (new_child->buffer_) << std::endl;
                * (tree_node_handle *) (new_child->buffer_ + sizeof(void*) +
                    sizeof(tree_node_handle)) = new_handle;
            }
            std::cout << "Done adjusting children." << std::endl;

            existing_allocator->free( handle, sizeof(
                    BRANCH_NODE_CLASS_TYPES ) );
            return new_handle;
          }
        default:
            assert( false );
            return tree_node_handle( nullptr );
    }
}

NODE_TEMPLATE_PARAMS
std::vector<Point> rectangle_search(
    tree_node_handle start_point,
    Rectangle &requestedRectangle,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {    
    std::vector<Point> accumulator;

    std::stack<tree_node_handle> context;
    tree_node_allocator *allocator = treeRef->node_allocator_.get();
    context.push( start_point );

    while( not context.empty() ) {
        tree_node_handle current_handle = context.top();
        context.pop();

        if( current_handle.get_type() == LEAF_NODE ) {
            rectangle_search_leaf_handle( current_handle, requestedRectangle, accumulator );
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        } else if( current_handle.get_type() == REPACKED_LEAF_NODE ) {
            rectangle_search_packed_leaf_handle( current_handle,
                    requestedRectangle, accumulator );
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        } else if( current_handle.get_type() == BRANCH_NODE ) {
            rectangle_search_branch_handle( current_handle,
                    requestedRectangle, context );
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        } else if( current_handle.get_type() == REPACKED_BRANCH_NODE ) {
            rectangle_search_packed_branch_handle( current_handle,
                    requestedRectangle, context );
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        } else {
            assert(false);
        }
    }
#ifdef STAT
    treeRef->stats.resetSearchTracker( true);
#endif
    return accumulator;
}

// Always called on root, this = root
// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
// choosing a particular leaf
NODE_TEMPLATE_PARAMS
tree_node_handle
BRANCH_NODE_CLASS_TYPES::chooseNode(
    std::variant<Branch,Point> &nodeEntry,
    uint8_t stopping_level
) {
    // FIXME: try and avoid all these materialize calls

    // CL1 [Initialize]
    tree_node_handle cur_node_handle = this->self_handle_;

    assert( cur_node_handle != nullptr );

    for( ;; ) {
        assert( cur_node_handle != nullptr );
        if( cur_node_handle.get_type() == LEAF_NODE ) {
            assert( std::holds_alternative<Point>( nodeEntry ) );
            return cur_node_handle;
        } else {
            assert( cur_node_handle.get_type() == BRANCH_NODE );

            auto cur_node = treeRef->get_branch_node( cur_node_handle );
            // Compute the smallest expansion
            assert( cur_node->cur_offset_ > 0 );
            if( cur_node->level_ == stopping_level ) {
                return cur_node_handle;
            }

            tree_node_allocator *allocator = get_node_allocator( this->treeRef );
            inline_poly node_poly = cur_node->entries.at(0).get_inline_polygon(
                    allocator );
            bool node_poly_unbounded = std::holds_alternative<unbounded_poly_pin>(node_poly);
            if (node_poly_unbounded) {
                assert( std::get<unbounded_poly_pin>(node_poly)->get_total_rectangle_count() > 0 );
            } else {
                assert( std::get<InlineBoundedIsotheticPolygon>(node_poly).get_rectangle_count() > 0 );
            }

            // This is the minimum amount of additional area we need in
            // one of the branches to encode our expansion
            double minimal_area_expansion =
                std::numeric_limits<double>::max();
            double minimal_poly_area = std::numeric_limits<double>::max();

            // This is the branch that gives us that minimum area
            // expansion
            unsigned smallestExpansionBranchIndex = 0;

            // This is the list of optimal expansiosn we need to perform
            // to get the bounding box/bounding polygon
            std::vector<IsotheticPolygon::OptimalExpansion> expansions;

            if( std::holds_alternative<Branch>( nodeEntry ) ) {
                inline_poly branch_poly =
                    std::get<Branch>(nodeEntry).get_inline_polygon( allocator );
                auto expansion_computation_results = computeExpansionArea( node_poly, branch_poly );
                minimal_area_expansion = expansion_computation_results.first;

                bool branch_poly_unbounded = std::holds_alternative<unbounded_poly_pin>(branch_poly);
                if (branch_poly_unbounded) {
                    minimal_poly_area = std::get<unbounded_poly_pin>(branch_poly)->area();
                } else {
                    minimal_poly_area = std::get<InlineBoundedIsotheticPolygon>(branch_poly).area();
                }

                expansions = expansion_computation_results.second;
                /*
                if( minimal_area_expansion == -1.0 ) {
                    abort();
                }
                */
            } else {
                IsotheticPolygon::OptimalExpansion exp;
                if (node_poly_unbounded) {
                    auto unb_node_poly = std::get<unbounded_poly_pin>(node_poly);
                    auto inline_exp = computeExpansionArea<InlineUnboundedIsotheticPolygon, InlineUnboundedIsotheticPolygon::Iterator>(
                        *unb_node_poly , unb_node_poly->begin(), unb_node_poly->end(), std::get<Point>( nodeEntry ) );
                    exp = inline_exp;
                } else {
                    auto b_node_poly = std::get<InlineBoundedIsotheticPolygon>(node_poly);
                    exp = computeExpansionArea<InlineBoundedIsotheticPolygon, InlineBoundedIsotheticPolygon::iterator>(
                        b_node_poly , b_node_poly.begin(), b_node_poly.end(), std::get<Point>( nodeEntry ) );
                }
                minimal_area_expansion = exp.area;
                expansions.push_back( exp );
            }

            for( unsigned i = 1; i < cur_node->cur_offset_; i++ ) {
                Branch &b = cur_node->entries.at(i);
                node_poly = b.get_inline_polygon( allocator );
                bool node_poly_unbounded = std::holds_alternative<unbounded_poly_pin>(node_poly);
                if (node_poly_unbounded) {
                    assert( std::get<unbounded_poly_pin>(node_poly)->get_total_rectangle_count() > 0 );
                } else {
                    assert( std::get<InlineBoundedIsotheticPolygon>(node_poly).get_rectangle_count() > 0 );
                }

                if( std::holds_alternative<Branch>( nodeEntry ) ) {
                    inline_poly branch_poly =
                        std::get<Branch>(nodeEntry).get_inline_polygon(
                                allocator );
                    // Walk every rectangle in the branch's polygon
                    // Find rectangle in our polygon that needs to be
                    // expanded the least to fit the branch's rectangle
                    // inside it.
                    // N.B., this does not split the rectangle apart if
                    // the expanded rectangle could be part of two
                    // distinct polygons. So as a result of doing this
                    // the polygon's constituent rectangles may now
                    // overlap.
                    auto expansion_computation_results = computeExpansionArea( node_poly, branch_poly );
                    double poly_area;

                    if (node_poly_unbounded) {
                        poly_area = std::get<unbounded_poly_pin>(node_poly)->area();
                    } else {
                        poly_area = std::get<InlineBoundedIsotheticPolygon>(node_poly).area();
                    }

                    if( expansion_computation_results.first <
                            minimal_area_expansion or 
                     (expansion_computation_results.first ==
                      minimal_area_expansion and poly_area <
                      minimal_poly_area)
                     ) {
                        minimal_area_expansion =
                            expansion_computation_results.first;
                        minimal_poly_area = poly_area;
                        expansions = expansion_computation_results.second;
                        smallestExpansionBranchIndex = i;
                    }
                    /*
                    if( minimal_area_expansion == -1.0 ) {
                        abort();
                    }
                    */
                } else {
                    IsotheticPolygon::OptimalExpansion exp;
                    if (node_poly_unbounded) {
                        auto unb_node_poly = std::get<unbounded_poly_pin>(node_poly);
                        auto inline_exp = computeExpansionArea<InlineUnboundedIsotheticPolygon, InlineUnboundedIsotheticPolygon::Iterator>(
                            *unb_node_poly , unb_node_poly->begin(), unb_node_poly->end(), std::get<Point>( nodeEntry ) );
                        exp = inline_exp;
                    } else {
                        auto b_node_poly = std::get<InlineBoundedIsotheticPolygon>(node_poly);
                        auto inline_exp = computeExpansionArea<InlineBoundedIsotheticPolygon, InlineBoundedIsotheticPolygon::iterator>(
                            b_node_poly , b_node_poly.begin(), b_node_poly.end(), std::get<Point>( nodeEntry ) );
                        exp = inline_exp;
                    }

                    if( exp.area < minimal_area_expansion ) {
                        minimal_area_expansion = exp.area;
                        expansions.clear();
                        expansions.push_back( exp );
                        smallestExpansionBranchIndex = i;
                    }
                }
            }

            if( minimal_area_expansion != -1.0 ) {
#ifndef NDEBUG 
                for( unsigned i = 0; i < cur_node->cur_offset_; i++ ) {
                    for( unsigned j = 0; j < cur_node->cur_offset_; j++
                            ) {
                        if( i == j ) {
                            continue;
                        }
                        Branch &b_i = cur_node->entries.at(i);
                        Branch &b_j = cur_node->entries.at(j);
                        IsotheticPolygon poly_i =
                            b_i.materialize_polygon( allocator );
                        IsotheticPolygon poly_j =
                            b_j.materialize_polygon( allocator );
                        assert( poly_i.disjoint( poly_j ) );
                    }
                }
#endif

                Branch &chosen_branch =
                    cur_node->entries.at(smallestExpansionBranchIndex);
                node_poly = chosen_branch.get_inline_polygon( allocator );

                IsotheticPolygon mat_node_poly = chosen_branch.materialize_polygon( allocator );


                if( std::holds_alternative<Branch>( nodeEntry ) ) {
                    // Fragment them on the way down.
                    Branch &inserting_branch = std::get<Branch>( nodeEntry );
                    IsotheticPolygon insertion_poly = inserting_branch.materialize_polygon( allocator );
                    for( unsigned i = 0; i < cur_node->cur_offset_; i++ ) {
                        if( i == smallestExpansionBranchIndex ) { 
                            continue;
                        }
                        Branch &other_branch = cur_node->entries.at(i);
                        IsotheticPolygon other_poly =
                            other_branch.materialize_polygon( allocator );

                        other_poly.increaseResolution( Point::atInfinity, insertion_poly );
                        other_poly.refine();
                        other_poly.recomputeBoundingBox();

                        update_branch_polygon( other_branch, other_poly, treeRef );

                    }
                }
                
                // We need to expand on way down so we know who is
                // responsible for the new point/branch
                // Everyone else needs to fragment around my nodeEntry,
                // then we expand and fragment around them.
                if( std::holds_alternative<Branch>( nodeEntry ) ) {
                    Branch &inserting_branch = std::get<Branch>( nodeEntry );
                    IsotheticPolygon insertion_poly = inserting_branch.materialize_polygon( allocator );
                    assert( insertion_poly.basicRectangles.size() ==
                            expansions.size() );
                    for( unsigned i = 0; i <
                            insertion_poly.basicRectangles.size(); i++ ) {
                        auto &expansion = expansions.at(i);
                        auto &insertion_rect= insertion_poly.basicRectangles.at(i);
                        Rectangle &existing_rect =
                            mat_node_poly.basicRectangles.at(expansion.index);
                        // Expand the existing rectangle. This rectangle
                        // might now overlap with other rectangles in
                        // the polygon. But if we make it not overlap,
                        // then we alter the indices of the expansion
                        // rectangles, which kind of sucks, So, leave it
                        // for now.
                        existing_rect.expand( insertion_rect );
                        assert( existing_rect.containsRectangle(
                                    insertion_rect ) );
                    }
                    mat_node_poly.recomputeBoundingBox();
                } else {
                    assert( expansions.size() == 1 );
                    Point &p = std::get<Point>( nodeEntry );
                    Rectangle &existing_rect =
                        mat_node_poly.basicRectangles.at(expansions.at(0).index);
                    existing_rect.expand( p );
                    mat_node_poly.recomputeBoundingBox();
                    assert( mat_node_poly.containsPoint( p ) );
                }

                // Dodge all the other branches
                for( unsigned i = 0; i < cur_node->cur_offset_;
                        i++ ) {
                    if( i == smallestExpansionBranchIndex ) {
                        continue;
                    }
                    Branch &other_branch = cur_node->entries.at(i);
                    mat_node_poly.increaseResolution( Point::atInfinity,
                            other_branch.materialize_polygon( allocator ) );
                }

                /*
                if( std::holds_alternative<Branch>( nodeEntry ) ) {
                    Branch &inserting_branch = std::get<Branch>( nodeEntry );
                    IsotheticPolygon insertion_poly = inserting_branch.materialize_polygon( allocator );
                    for( unsigned i = 0; i < insertion_poly.basicRectangles.size(); i++ ) {
                        bool contained = false;
                        for( unsigned j = 0; j < node_poly.basicRectangles.size(); j++ ) {
                            if(
                                    node_poly.basicRectangles.at(j).containsRectangle(
                                        insertion_poly.basicRectangles.at(i)
                                        ) ) {
                                contained = true;
                                break;
                            }
                        }
                        assert( contained );
                    }

                }
                */

                mat_node_poly.refine();
                mat_node_poly.recomputeBoundingBox();

                update_branch_polygon( chosen_branch, mat_node_poly, treeRef );
            }

            // Descend
            Branch &b = cur_node->entries.at(smallestExpansionBranchIndex);
            cur_node_handle = b.child;
            assert( cur_node_handle != nullptr );
        }
    }
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::findLeaf(Point givenPoint)
{

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push( this->self_handle_ );
    tree_node_handle current_node_handle;

    tree_node_allocator *allocator = get_node_allocator( this->treeRef );

    while( !context.empty() ) {
        current_node_handle = context.top();
        context.pop();

        if( current_node_handle.get_type() == LEAF_NODE ) {
            auto current_node = treeRef->get_leaf_node(
                    current_node_handle );
            // FL2 [Search leaf node for record]
            // Check each entry to see if it matches E
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = current_node->entries.at(i);
                if( p == givenPoint ) {
                    return current_node_handle;
                }
            }
        } else {
            auto current_node = treeRef->get_branch_node( current_node_handle );
            // FL1 [Search subtrees]
            // Determine which branches we need to follow
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = current_node->entries.at(i);
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
Partition BRANCH_NODE_CLASS_TYPES::partitionNode()
{
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
void BRANCH_NODE_CLASS_TYPES::make_disjoint_from_children(
    IsotheticPolygon &polygon,
    tree_node_handle handle_to_skip
) {
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    for( auto iter = entries.begin(); iter !=
            entries.begin() + this->cur_offset_; iter++ ) {
        Branch &b = *iter;
        if( b.child == handle_to_skip ) {
            continue;
        }
        IsotheticPolygon child_poly = b.materialize_polygon( allocator );
        polygon.increaseResolution( Point::atInfinity, child_poly );
    }
    polygon.recomputeBoundingBox();
}

// We create two new nodes and free the old one.
// The old one is freed in adjustTree using removeEntry
// If we downsplit, then we won't call adjustTree for that split so we
// need to delete the node ourselves.
NODE_TEMPLATE_PARAMS
SplitResult BRANCH_NODE_CLASS_TYPES::splitNode(
    Partition p,
    bool is_downsplit
) {
    assert( this->self_handle_.get_type() == BRANCH_NODE );
    using NodeType = BRANCH_NODE_CLASS_TYPES;
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );

    auto alloc_data =
        allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                NodeHandleType( BRANCH_NODE ) );
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first; // take pin
    new (&(*left_node)) NodeType( this->treeRef, this->parent,
            left_handle, level_ );
    assert( left_node->self_handle_ == left_handle );

    alloc_data =
        allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                NodeHandleType( BRANCH_NODE ) );
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first; // take pin
    new (&(*right_node)) NodeType( this->treeRef, this->parent,
            right_handle, level_ );
    assert( right_node->self_handle_ == right_handle );

    SplitResult split = {
        { tree_node_handle(nullptr), left_handle }, 
        { tree_node_handle(nullptr), right_handle } };

    // So we are going to split this branch node.
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Branch &branch = entries.at(i);

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


        IsotheticPolygon child_poly = branch.materialize_polygon(
                allocator );
        // Entirely contained in the left polygon
        if( is_contained_left and not is_contained_right) {
            if( branch.child.get_type() == LEAF_NODE ) {
                auto child = this->treeRef->get_leaf_node( branch.child );
                child->parent = split.leftBranch.child;
            } else {
                auto child = this->treeRef->get_branch_node( branch.child );
                child->parent = split.leftBranch.child;
            }

            assert( split.leftBranch.child == left_handle );
            left_node->addBranchToNode( branch );
        // Entirely contained in the right polygon
        } else if( is_contained_right and not is_contained_left ) {
            if( branch.child.get_type() == LEAF_NODE ) {
                auto child = this->treeRef->get_leaf_node( branch.child );
                child->parent = split.rightBranch.child;
            } else {
                auto child = this->treeRef->get_branch_node( branch.child );
                child->parent = split.rightBranch.child;
            }
            assert( split.rightBranch.child == right_handle );
            right_node->addBranchToNode( branch );
        } else if( summary_rectangle.upperRight[p.dimension] ==
                summary_rectangle.lowerLeft[p.dimension] and
                summary_rectangle.lowerLeft[p.dimension] ==
                p.location ) {
            // These go left or right situationally
            if( left_node->cur_offset_ <= right_node->cur_offset_ ) {
                if( branch.child.get_type() == LEAF_NODE ) {
                    auto child = this->treeRef->get_leaf_node( branch.child );
                    child->parent = split.leftBranch.child;
                } else {
                    auto child = this->treeRef->get_branch_node( branch.child );
                    child->parent = split.leftBranch.child;
                }
                assert( split.leftBranch.child == left_handle );
                left_node->addBranchToNode( branch );
            } else {
                if( branch.child.get_type() == LEAF_NODE ) {
                    auto child = this->treeRef->get_leaf_node( branch.child );
                    child->parent = split.rightBranch.child;
                } else {
                    auto child = this->treeRef->get_branch_node( branch.child );
                    child->parent = split.rightBranch.child;
                }
                assert( split.rightBranch.child == right_handle );
                right_node->addBranchToNode( branch );
            }
        // Partially spanned by both nodes, need to downsplit
        } else {
            IsotheticPolygon branch_poly = branch.materialize_polygon(
                    allocator );
            SplitResult downwardSplit;

            if( branch.child.get_type() == LEAF_NODE ) {
                auto child = treeRef->get_leaf_node( branch.child );
                downwardSplit = child->splitNode( p, true );
                allocator->free( branch.child, sizeof(
                            LEAF_NODE_CLASS_TYPES )  );
            } else {
                auto child = treeRef->get_branch_node( branch.child );
                downwardSplit = child->splitNode( p, true );
                allocator->free( branch.child, sizeof(
                            BRANCH_NODE_CLASS_TYPES )  );
            }

            if( std::holds_alternative<tree_node_handle>(
                        branch.boundingPoly ) ) {
                // We can free this
                tree_node_handle free_poly_handle = std::get<tree_node_handle>(
                            branch.boundingPoly );
                auto free_poly_pin = InlineUnboundedIsotheticPolygon::read_polygon_from_disk(
                        allocator, free_poly_handle );
                free_poly_pin->free_subpages( allocator );
                uint16_t alloc_size = compute_sizeof_inline_unbounded_polygon(
                    free_poly_pin->get_max_rectangle_count_on_first_page() );

                allocator->free( free_poly_handle, alloc_size );
            }

            if( downwardSplit.leftBranch.child.get_type() == LEAF_NODE ) {
                auto left_child =
                    treeRef->get_leaf_node(downwardSplit.leftBranch.child);
                if( left_child->cur_offset_ > 0 ) {
                    left_child->parent = split.leftBranch.child;
                    left_node->addBranchToNode( downwardSplit.leftBranch );
                }
            } else {
                auto left_child =
                    treeRef->get_branch_node(downwardSplit.leftBranch.child);
                if( left_child->cur_offset_ > 0 ) {
                    left_child->parent = split.leftBranch.child;
                    left_node->addBranchToNode( downwardSplit.leftBranch );
                }
            }

            if( downwardSplit.rightBranch.child.get_type() == LEAF_NODE ) {
                auto right_child =
                    treeRef->get_leaf_node(downwardSplit.rightBranch.child);
                if( right_child->cur_offset_ > 0 ) {
                    right_child->parent = split.rightBranch.child;
                    right_node->addBranchToNode( downwardSplit.rightBranch );
                }

            } else {
                auto right_child =
                    treeRef->get_branch_node(downwardSplit.rightBranch.child);
                if( right_child->cur_offset_ > 0 ) {
                    right_child->parent = split.rightBranch.child;
                    right_node->addBranchToNode( downwardSplit.rightBranch );
                }
            }
        } //downsplit 
    } //split

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

    // When downsplitting our node, one part of this node goes
    // to the "left parent", and one part of the node goes to 
    // the "right parent". These node parts could revise their
    // bounding boxes to reflect only what is left in their nodes,
    // because part of their contents have been removed. These
    // revisions may include "simplifying" the bounding box, in
    // which case the space the polygon consumes becomes larger,
    // but it contains fewer rectangles.
    // E.g., space used be to be an "L" shape, but is now just
    // a rectangle.
    //
    // However, our siblings may also be revising their bounding
    // boxes, and they will do so based on what our bounding
    // box looks like. They need to ensure that the boxes do not
    // intersect. They could theoretically look at the
    // new parent node, figure out what all of their sibling's
    // boxes are, and then make themselves disjoint, but that's
    // a lot more work than just intersectingw ith our existing
    // polygon, which is already guaranteed to be disjoint from
    // our siblings. So, we do the latter.
    if( this->parent ) {
        auto parent_node = treeRef->get_branch_node( parent );
        if( not is_downsplit ) {
            parent_node->make_disjoint_from_children( left_polygon,
                    this->self_handle_ );
            assert( left_polygon.basicRectangles.size() > 0 );
            left_polygon.refine();
            assert( left_polygon.basicRectangles.size() > 0 );
            parent_node->make_disjoint_from_children( right_polygon,
                    this->self_handle_ );
            assert( right_polygon.basicRectangles.size() > 0 );
            right_polygon.refine();
            assert( right_polygon.basicRectangles.size() > 0 );
        } else {
            // Intersect with our existing poly to avoid intersect
            // with other children
            Branch &b = parent_node->locateBranch( this->self_handle_ );
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
                        overfull_rect_count ), NodeHandleType(
                            BIG_POLYGON ) );
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
                        overfull_rect_count ), NodeHandleType(
                            BIG_POLYGON ) );
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
    this->cur_offset_ = 0;

    return split;
}

// Splitting a node will remove it from its this->parent node and its memory will be freed
NODE_TEMPLATE_PARAMS
SplitResult BRANCH_NODE_CLASS_TYPES::splitNode()
{
    SplitResult returnSplit = splitNode(partitionNode(), false);
    return returnSplit;
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::insert(
    std::variant<Branch,Point> &nodeEntry,
    std::vector<bool> &hasReinsertedOnLevel
) {

    bool givenIsPoint = std::holds_alternative<Point>( nodeEntry );
    uint8_t stopping_level = 0;
    if( not givenIsPoint ) {
        Branch &b = std::get<Branch>( nodeEntry );
        auto child_handle = b.child;
        if( child_handle.get_type() == LEAF_NODE ) {
            stopping_level = 1;
        } else {
            auto child_node = treeRef->get_branch_node( child_handle );
            stopping_level = child_node->level_ + 1;
        }
    }

    assert( this->self_handle_.get_type() == BRANCH_NODE );

    // Find the appropriate position for the entry
    // Should stop at appropriate depth level
    tree_node_handle current_handle = chooseNode( nodeEntry, stopping_level );

    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    SplitResult finalSplit;
    auto tree_ref_backup = treeRef;
    if( std::holds_alternative<Point>( nodeEntry ) ) {
        assert( current_handle.get_type() == LEAF_NODE );
        auto current_node = treeRef->get_leaf_node( current_handle );

        current_node->addPoint( std::get<Point>( nodeEntry ) );

        finalSplit = adjustTreeSub( hasReinsertedOnLevel,
                current_handle, treeRef  );
    } else {
        assert( current_handle.get_type() == BRANCH_NODE );
        auto current_node = treeRef->get_branch_node( current_handle );
        Branch &sub_branch = std::get<Branch>(nodeEntry);

        IsotheticPolygon insertion_polygon =
            sub_branch.materialize_polygon( allocator );

        // Before I add this node in, I need to fragment everyone else
        // around it
        for( unsigned int i = 0; i < current_node->cur_offset_; i++ ) {
            Branch &b = current_node->entries.at(i);
            IsotheticPolygon branch_polygon = b.materialize_polygon(
                    allocator );
            branch_polygon.increaseResolution( Point::atInfinity, insertion_polygon );
            branch_polygon.recomputeBoundingBox();
            update_branch_polygon( b, branch_polygon, treeRef );
        }

        current_node->addBranchToNode( sub_branch );
        if( sub_branch.child.get_type() == LEAF_NODE ) {
            auto child_node = treeRef->get_leaf_node( sub_branch.child );
            assert( child_node->level_ == current_node->level_-1 );
            child_node->parent = current_handle;
        } else {
            auto child_node = treeRef->get_branch_node( sub_branch.child );
            assert( child_node->level_ == current_node->level_-1 );
            child_node->parent = current_handle;
        }

        finalSplit = adjustTreeSub( hasReinsertedOnLevel,
                current_handle, treeRef );
    }


    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        auto alloc_data =
            allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data.first)) BRANCH_NODE_CLASS_TYPES(
                tree_ref_backup, tree_node_handle(nullptr), alloc_data.second, level_+1 );
        auto new_root_handle = alloc_data.second;
        auto new_root_node = alloc_data.first;

        if( finalSplit.leftBranch.child.get_type() == LEAF_NODE ) {
            auto left_node = tree_ref_backup->get_leaf_node( finalSplit.leftBranch.child );
            left_node->parent = new_root_handle;
        } else {
            auto left_node = tree_ref_backup->get_branch_node( finalSplit.leftBranch.child );
            left_node->parent = new_root_handle;
        }
        new_root_node->addBranchToNode( finalSplit.leftBranch );

        if( finalSplit.rightBranch.child.get_type() == LEAF_NODE ) {
            auto right_node = tree_ref_backup->get_leaf_node( finalSplit.rightBranch.child );
            right_node->parent = new_root_handle;
        } else {
            auto right_node = tree_ref_backup->get_branch_node( finalSplit.rightBranch.child );
            right_node->parent = new_root_handle;
        }
        new_root_node->addBranchToNode( finalSplit.rightBranch );

        tree_ref_backup->root = new_root_handle;

        assert( this->self_handle_.get_type() == BRANCH_NODE );
        allocator->free( this->self_handle_, sizeof( BRANCH_NODE_CLASS_TYPES ) );
        this->self_handle_ = tree_node_handle( nullptr );

        // Fix the reinserted length
        hasReinsertedOnLevel.push_back(false);

        return new_root_handle;
    }

    return tree_ref_backup->root;
}

// Always called on root, this = root
NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::remove( Point givenPoint ) {

    // D1 [Find node containing record]
    tree_node_handle leaf_handle = findLeaf( givenPoint );
    // Record not in the tree
    if( leaf_handle == nullptr ) {
        return this->self_handle_;
    }

    // D2 [Delete record]
    auto leaf_node = treeRef->get_leaf_node( leaf_handle );
    leaf_node->removePoint( givenPoint );

    // D3 [Propagate changes]
    leaf_node->condenseTree();

    // D4 [Shorten tree]
    assert( this->self_handle_.get_type() == BRANCH_NODE );
    if( this->cur_offset_ == 1 ) {
        tree_node_handle new_root_handle = entries.at(0).child;
        tree_node_allocator *allocator = get_node_allocator( this->treeRef );
        allocator->free( this->self_handle_, sizeof( BRANCH_NODE_CLASS_TYPES ) );
        if( new_root_handle.get_type() == LEAF_NODE ) {
            auto new_root = treeRef->get_leaf_node( new_root_handle );
            new_root->parent = tree_node_handle( nullptr );
        } else {
            auto new_root = this->treeRef->get_branch_node( new_root_handle );
            new_root->parent = tree_node_handle( nullptr );
        }

        return new_root_handle;
    }

    return this->self_handle_;
}

NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::checksum() {
    unsigned sum = 0;

    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        // Recurse
        Branch &branch = entries.at(i);
        if( branch.child.get_type() == LEAF_NODE ) {
            auto child = this->treeRef->get_leaf_node( branch.child );
            sum += child->checksum();
        } else {
            auto child = this->treeRef->get_branch_node( branch.child );
            sum += child->checksum();
        }
    }

    return sum;
}

NODE_TEMPLATE_PARAMS
std::vector<Point> BRANCH_NODE_CLASS_TYPES::bounding_box_validate()
{
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    std::vector<Point> all_child_points;
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Branch &b_i = entries.at(i);
        if( b_i.child.get_type() == LEAF_NODE ) {
            auto child_ptr = this->treeRef->get_leaf_node( b_i.child );
            std::vector<Point> child_points =
                child_ptr->bounding_box_validate();
            for( Point &p : child_points ) {
                all_child_points.push_back( p );
            }
        } else {
            auto child_ptr = this->treeRef->get_branch_node( b_i.child );
            std::vector<Point> child_points =
                child_ptr->bounding_box_validate();
            for( Point &p : child_points ) {
                all_child_points.push_back( p );
            }
        }
    }
    if( this->parent != nullptr ) {
        auto parent_node = treeRef->get_branch_node( parent );
        Branch &parent_branch = parent_node->locateBranch( this->self_handle_ );
        IsotheticPolygon parent_poly =
            parent_branch.materialize_polygon( allocator );
        Rectangle bounding_box =
            parent_branch.get_summary_rectangle( allocator );
        for( Point &p : all_child_points ) {
            if( !parent_poly.containsPoint( p ) ) {
                std::cout << "Parent poly " << this->parent << "does not contain: " << p
                    << std::endl;
                std::cout << "Poly was: " << parent_poly <<
                    std::endl;
                std::cout << "BB was: " << bounding_box << std::endl;
                std::cout << "My node is: " << this->self_handle_ <<
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

NODE_TEMPLATE_PARAMS
bool BRANCH_NODE_CLASS_TYPES::validate( tree_node_handle expectedParent, unsigned index) {

    tree_node_allocator *allocator = get_node_allocator( this->treeRef );

    if( expectedParent != nullptr and (this->parent != expectedParent ||
                this->cur_offset_ > max_branch_factor )) {
        std::cout << "node = " << (void *)this << std::endl;
        std::cout << "parent = " << this->parent << " expectedParent = " << expectedParent << std::endl;
        std::cout << "maxBranchFactor = " << max_branch_factor << std::endl;
        std::cout << "entries.size() = " << this->cur_offset_ << std::endl;
        assert(this->parent == expectedParent);
    }

    if( expectedParent != nullptr) {
        for( unsigned i = 0; i < this->cur_offset_; i++ ) {
            for( unsigned j = 0; j < this->cur_offset_; j++ ) {
                if( i != j ) {
                    Branch &b_i = entries.at(i);
                    Branch &b_j = entries.at(j);
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
                        std::cout << "Parent is: " << this->self_handle_ <<
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
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {

        Branch &b = entries.at(i);
        if( b.child.get_type() == LEAF_NODE ) {
            auto child = this->treeRef->get_leaf_node( b.child );
            valid = valid and child->validate( this->self_handle_, i );
        } else {
            auto child = this->treeRef->get_branch_node( b.child );
            valid = valid and child->validate( this->self_handle_, i );
        }
    }

    return valid;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::print(unsigned n)
{
    std::string indentation(n * 4, ' ');
    std::cout << indentation << "Node " << (void *)this << std::endl;
    std::cout << indentation << "    Parent: " << this->parent << std::endl;
    std::cout << indentation << "    Branches: " << std::endl;
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Branch &branch = entries.at(i);
        // FIXME: out of band poly
        auto poly = std::get<InlineBoundedIsotheticPolygon>(
                branch.boundingPoly ).materialize_polygon();
        std::cout << indentation << "		" << branch.child << std::endl;
        std::cout << indentation << "		" << poly << std::endl;
    }
    std::cout << std::endl;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::printTree(unsigned n)
{
    // Print this node first
    print(n);

    // Print any of our children with one more level of indentation
    std::string indendtation(n * 4, ' ');
    for( unsigned i = 0; i < this->cur_offset_; i++ ) {
        Branch &branch = entries.at(i);
        if( branch.child.get_type() == LEAF_NODE ) {
            auto child = this->treeRef->get_leaf_node( branch.child );

            // Recurse
            child->printTree(n + 1);
        } else {
            auto child = this->treeRef->get_branch_node( branch.child );

            // Recurse
            child->printTree(n + 1);

        }
    }
    std::cout << std::endl;
}

NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::height()
{
    unsigned ret = 0;
    tree_node_handle current_handle = this->self_handle_;

    for( ;; ) {
        ret++;
        if( current_handle.get_type() == LEAF_NODE ) {
            return ret;
        }

        auto node = treeRef->get_branch_node( current_handle );
        current_handle = node->entries.at(0).child;
    }
}


template <int min_branch_factor, int max_branch_factor, class strategy>
void stat_node(
    tree_node_handle start_handle,
    NIRTreeDisk<min_branch_factor,max_branch_factor,strategy> *treeRef
) {
    std::stack<tree_node_handle> context;

    // Initialize our context stack
    context.push( start_handle );
    unsigned long polygonSize;
    unsigned long totalPolygonSize = 0;
    unsigned long totalLines = 0;
    size_t memoryFootprint = 0;
    size_t memoryPolygons = 0;
    unsigned long totalNodes = 0;
    unsigned long totalLeaves = 0;
    size_t deadSpace = 0;

    std::vector<unsigned long> histogramPolygon;
    histogramPolygon.resize(10000, 0);
    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize( max_branch_factor, 0 );

    double coverage = 0.0;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    while( !context.empty() ) {
        auto currentContext = context.top();
        context.pop();

        totalNodes++;

        if( currentContext.get_type() == LEAF_NODE ) {

            auto current_node = treeRef->get_leaf_node( currentContext );

            unsigned fanout = current_node->cur_offset_;
            if( fanout >= histogramFanout.size() ) {
                histogramFanout.resize(2*fanout, 0);
            }
            histogramFanout[fanout]++;

            totalLeaves++;
            memoryFootprint +=
                sizeof(LeafNode<min_branch_factor,max_branch_factor,strategy>);
        } else if( currentContext.get_type() == REPACKED_LEAF_NODE ) {
            auto current_node = allocator->get_tree_node<packed_node>(
                    currentContext );
            
            char *data = current_node->buffer_;
            decode_entry_count_and_offset_packed_node( data );
            totalLeaves++;
            unsigned fanout = (unsigned) count;
            if( fanout >= histogramFanout.size() ) {
                histogramFanout.resize(2*fanout, 0);
            }
            histogramFanout[fanout]++;
            memoryFootprint += sizeof(void*) +
                sizeof(tree_node_handle)*2 + sizeof(unsigned) +
                count * sizeof(Point);
        } else if( currentContext.get_type() == REPACKED_BRANCH_NODE ) {
            auto current_node = allocator->get_tree_node<packed_node>(
                    currentContext );
            
            char *buffer = current_node->buffer_;
            decode_entry_count_and_offset_packed_node( buffer );
            unsigned fanout = (unsigned) count;
            if( fanout >= histogramFanout.size() ) {
                histogramFanout.resize(2*fanout, 0);
            }
            histogramFanout[fanout]++;
            memoryFootprint += sizeof(void*)+ sizeof(tree_node_handle)*2
                + sizeof(unsigned);
            for( unsigned i = 0; i < count; i++ ) {
                tree_node_handle *child = (tree_node_handle *) (buffer + offset);
                offset += sizeof( tree_node_handle );
                unsigned rect_count = * (unsigned *) (buffer + offset);
                offset += sizeof( unsigned );
                memoryFootprint += sizeof(tree_node_handle) + sizeof(unsigned);
                context.push( *child );
                if( rect_count == std::numeric_limits<unsigned>::max() ) {
                    auto handle = * (tree_node_handle *) (buffer +
                            offset);
                    offset += sizeof( tree_node_handle );
                    memoryFootprint += sizeof( tree_node_handle );
                    auto poly_pin =
                        allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                handle );
                    memoryFootprint +=
                        compute_sizeof_inline_unbounded_polygon(
                                poly_pin->get_total_rectangle_count() );
                    memoryPolygons +=
                        (compute_sizeof_inline_unbounded_polygon (
                                poly_pin->get_total_rectangle_count() )
                         - sizeof(Rectangle));
                } else {
                    offset += rect_count * sizeof( Rectangle );
                    memoryFootprint += rect_count * sizeof(Rectangle);
                    memoryPolygons += (rect_count * sizeof(Rectangle) -
                            sizeof(Rectangle));
                }
            }
        } else if( currentContext.get_type() == BRANCH_NODE ) {
            auto current_branch_node = treeRef->get_branch_node( currentContext
                    );

            unsigned fanout = current_branch_node->cur_offset_;
            if( fanout >= histogramFanout.size() ) {
                histogramFanout.resize(2*fanout, 0);
            }
            histogramFanout[fanout]++;

            // Compute the overlap and coverage of our children
            for( unsigned i = 0; i < current_branch_node->cur_offset_; i++ ) {
                Branch &b = current_branch_node->entries.at(i);
                IsotheticPolygon polygon = b.materialize_polygon( allocator );
                coverage += polygon.area();
            }

            memoryFootprint +=
                sizeof(BranchNode<min_branch_factor,max_branch_factor,strategy>);// +
                // other out of line polys

            deadSpace += (sizeof(Branch) *
                    (max_branch_factor-current_branch_node->cur_offset_ ) );
            
            for( unsigned i = 0; i < current_branch_node->cur_offset_; i++ ) {
                Branch &b = current_branch_node->entries.at(i);
                IsotheticPolygon polygon = b.materialize_polygon(
                        allocator );

                polygonSize = polygon.basicRectangles.size();
                assert( polygonSize < histogramPolygon.size() );
                histogramPolygon[polygonSize]++;
                totalPolygonSize += polygonSize;

                // FIXME: these stats are all wrong now.
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
    // Memory footprint is wrong!
    STATMEM(memoryFootprint);
    STATMEM(memoryPolygons);
    //STATHEIGHT(height());
    STATSIZE(totalNodes);
    //STATEXEC(std::cout << "DeadSpace: " << deadSpace << std::endl);
    //STATSINGULAR(singularBranches);
    STATLEAF(totalLeaves);
    STATBRANCH(totalNodes - totalLeaves);
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
#undef LEAF_NODE_CLASS_TYPES
#undef BRANCH_NODE_CLASS_TYPES
#undef point_search_leaf_handle
#undef point_search_leaf_node
#undef point_search_packed_leaf_node
#undef point_search_packed_branch_handle
#undef point_search_branch_handle
#undef rectangle_search_leaf_handle
#undef rectangle_search_leaf_node
#undef rectangle_search_packed_leaf_node
#undef rectangle_search_packed_branch_handle
#undef rectangle_search_branch_handle
#undef decode_entry_count_and_offset_packed_node
