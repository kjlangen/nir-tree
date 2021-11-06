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
void LEAF_NODE_CLASS_TYPES::deleteSubtrees()
{
    return;

}

NODE_TEMPLATE_PARAMS
Rectangle LEAF_NODE_CLASS_TYPES::boundingBox()
{
    assert( this->cur_offset_ > 0 );

    Point &p = entries.at(0);
    Rectangle bb(p, Point::closest_larger_point( p ) );
    for( size_t i = 1; i < this->cur_offset_; i++ ) {
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
    size_t childIndex;
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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
        Point &p = entries.at(i);
        if( requestedPoint == p ) {
            accumulator.push_back( p );
        }
    }
}

// Macros so that we don't have recursive function calls 

#define point_search_leaf_node( current_node, requestedPoint, accumulator ) \
    for( size_t i = 0; i < current_node->cur_offset_; i++ ) { \
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
    for( size_t i = 0; i < current_node->cur_offset_; i++ ) { \
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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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

    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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
            this->parent, left_handle );
    assert( left_node->self_handle_ == left_handle );
    assert( left_handle.get_type() == LEAF_NODE );

    alloc_data = allocator->create_new_tree_node<LEAF_NODE_CLASS_TYPES>(
            NodeHandleType( LEAF_NODE ) );
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first; // take pin
    new (&(*right_node)) LEAF_NODE_CLASS_TYPES( this->treeRef, this->parent, right_handle );
    assert( right_node->self_handle_ == right_handle );
    assert( right_handle.get_type() == LEAF_NODE );

    SplitResult split = {
        { tree_node_handle(nullptr), left_handle }, 
        { tree_node_handle(nullptr), right_handle } };

    bool containedLeft, containedRight;
    for( size_t i = 0; i < this->cur_offset_; i++ ) {


        Point &dataPoint =  entries.at(i);
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
                        overfull_rect_count ), NodeHandleType(
                            BIG_POLYGON ) );
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
                        overfull_rect_count ), NodeHandleType(
                            BIG_POLYGON ) );
        new (&(*poly_alloc_data.first))
            InlineUnboundedIsotheticPolygon( allocator,
                    overfull_rect_count );
        split.rightBranch.boundingPoly = poly_alloc_data.second;
        poly_alloc_data.first->push_polygon_to_disk( right_polygon );

    }
    return split;
}

// Splitting a node will remove it from its this->parent node and its memory will be freed
NODE_TEMPLATE_PARAMS
SplitResult LEAF_NODE_CLASS_TYPES::splitNode()
{
    SplitResult returnSplit = splitNode(partitionNode(), false);
    return returnSplit;
}

template <class NT, class TR>
std::pair<SplitResult, tree_node_handle> adjust_tree_bottom_half( NT
        current_node,  TR *tree_ref_backup, int max_branch_factor ) {
    if( current_node->cur_offset_ <= (unsigned) max_branch_factor ) {
        SplitResult propagationSplit = {
            {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
            {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
        };

        return std::make_pair( propagationSplit,
                tree_node_handle(nullptr) );
    }

    // Otherwise, split node
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
}

// This bottom-to-top sweep is only for splitting bounding boxes as necessary
NODE_TEMPLATE_PARAMS
SplitResult LEAF_NODE_CLASS_TYPES::adjustTree()
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
            auto current_leaf_node = treeRef->get_leaf_node(
                    current_handle );
            split_res_and_new_handle = adjust_tree_bottom_half(
                    current_leaf_node, tree_ref_backup, max_branch_factor );
        } else {
            auto current_branch_node = treeRef->get_branch_node(
                    current_handle );
            split_res_and_new_handle = adjust_tree_bottom_half(
                    current_branch_node, tree_ref_backup,
                    max_branch_factor );
        }

        propagationSplit = split_res_and_new_handle.first;
        current_handle = split_res_and_new_handle.second;
    }
    return propagationSplit;
}

// This always get called on the root node. So if it got called on us,
// that's because we are the only node in the whole tree.
NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::insert( Point givenPoint ) {

    // This is a leaf, so we are the ONLY node.
    entries.at( this->cur_offset_++ ) = givenPoint;

    SplitResult finalSplit = adjustTree();

    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        tree_node_allocator *allocator = get_node_allocator( this->treeRef );
        auto alloc_data =
            allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                    NodeHandleType( BRANCH_NODE ));
        new (&(*alloc_data.first)) BRANCH_NODE_CLASS_TYPES( this->treeRef,
                tree_node_handle(nullptr), alloc_data.second );
        auto new_root_handle = alloc_data.second;
        assert( new_root_handle.get_type() == BRANCH_NODE );
        auto new_root_node = alloc_data.first;


        auto left_node = this->treeRef->get_leaf_node( finalSplit.leftBranch.child );
        left_node->parent = new_root_handle;
        new_root_node->addBranchToNode( finalSplit.leftBranch );

        auto right_node = this->treeRef->get_leaf_node( finalSplit.rightBranch.child );
        right_node->parent = new_root_handle;
        new_root_node->addBranchToNode( finalSplit.rightBranch );

        assert( this->self_handle_.get_type() == LEAF_NODE );
        allocator->free( this->self_handle_, sizeof( LEAF_NODE_CLASS_TYPES ) );
        this->self_handle_ = tree_node_handle( nullptr );

        assert( new_root_handle.get_type() == BRANCH_NODE );
        return new_root_handle;
    }

    return this->self_handle_;
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
            size_t loc_cur_offset = 0;
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

    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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

       for( size_t i = 0; i < this->cur_offset_; i++ ) {
            Point &dataPoint = entries.at(i);
            if( !poly.containsPoint( dataPoint ) ) {
                std::cout << poly << " fails to contain " << dataPoint << std::endl;
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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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
void LEAF_NODE_CLASS_TYPES::stat() {
    /*
    std::stack<tree_node_handle> context;

    // Initialize our context stack
    context.push( this->self_handle_ );
    tree_node_handle currentContext;
    size_t memoryFootprint = 0;
    unsigned long totalLeaves = 0;
    size_t deadSpace = 0;

    std::vector<unsigned long> histogramPolygon;
    histogramPolygon.resize(10000, 0);
    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize( max_branch_factor, 0 );

    unsigned fanout = this->cur_offset_;
    if( fanout >= histogramFanout.size() ) {
        histogramFanout.resize(2*fanout, 0);
    }
    histogramFanout[fanout]++;

    totalLeaves++;
    memoryFootprint +=
        sizeof(LeafNode<min_branch_factor,max_branch_factor,strategy>) + this->cur_offset_ * sizeof(Point);
    deadSpace += (sizeof(Point) *
            (max_branch_factor-this->cur_offset_) );
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
    std::cout << this->treeRef->stats;

    STATEXEC(std::cout << "### ### ### ###" << std::endl);
    */
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
uint16_t BRANCH_NODE_CLASS_TYPES::compute_packed_size() {
    uint16_t sz = 0;
    sz += sizeof( treeRef );
    sz += sizeof( self_handle_ );
    sz += sizeof( parent );
    sz += sizeof( cur_offset_ );
    for( size_t i = 0; i < cur_offset_; i++ ) {
        sz += entries.at(i).compute_packed_size();
    }
    return sz;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::repack( tree_node_allocator *allocator ) {
    static_assert( sizeof( void * ) == sizeof(uint64_t) );
    uint16_t alloc_size = compute_packed_size();
    auto alloc_data = allocator->create_new_tree_node<packed_node>(
            alloc_size, NodeHandleType(REPACKED_LEAF_NODE) );

    char *buffer = alloc_data.first->buffer_;
    buffer += write_data_to_buffer( buffer, &treeRef );
    buffer += write_data_to_buffer( buffer, &(alloc_data.second) );
    buffer += write_data_to_buffer( buffer, &parent );
    buffer += write_data_to_buffer( buffer, &cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        buffer += write_data_to_buffer( buffer, &(entries.at(i)) );
    }
    assert( buffer - alloc_data.first->buffer_ == alloc_size );
    return alloc_data.second;
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::repack( tree_node_allocator *allocator ) {
    static_assert( sizeof( void * ) == sizeof(uint64_t) );
    uint16_t alloc_size = compute_packed_size();
    auto alloc_data = allocator->create_new_tree_node<packed_node>(
            alloc_size, NodeHandleType(REPACKED_BRANCH_NODE) );

    char *buffer = alloc_data.first->buffer_;
    buffer += write_data_to_buffer( buffer, &treeRef );
    buffer += write_data_to_buffer( buffer, &(alloc_data.second) );
    buffer += write_data_to_buffer( buffer, &parent );
    buffer += write_data_to_buffer( buffer, &cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = entries.at(i);
        buffer += b.repack_into( buffer, allocator );
    }
    size_t true_size = (buffer - alloc_data.first->buffer_);
    assert( true_size == alloc_size );
    return alloc_data.second;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::deleteSubtrees()
{
    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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

    for( size_t i = 1; i < this->cur_offset_; i++ ) {
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
    size_t childIndex;
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
    size_t found_index = 0;
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
        size_t alloc_size = compute_sizeof_inline_unbounded_polygon(
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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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

#define point_search_packed_leaf_node( packed_leaf, requestedPoint, accumulator ) \
    char *data = packed_leaf->buffer_; \
    size_t offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    size_t &point_count = * (size_t *) (data + offset); \
    offset += sizeof(size_t); \
    for( size_t i = 0; i < point_count; i++ ) { \
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
    size_t offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    size_t &point_count = * (size_t *) (data + offset); \
    offset += sizeof(size_t); \
    for( size_t i = 0; i < point_count; i++ ) { \
        Point *p = (Point *) (data + offset); \
        if( requestedRectangle.containsPoint( *p ) ) { \
            accumulator.push_back( *p ); \
        } \
        offset += sizeof( Point ); \
    }

#define point_search_branch_handle( handle, requestedPoint, context ) \
    auto current_node = treeRef->get_branch_node( handle ); \
    for( size_t i = 0; i < current_node->cur_offset_; i++ ) { \
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

#define point_search_packed_branch_handle( handle , requestedPoint, context ) \
    auto packed_branch = allocator->get_tree_node<packed_node>( handle ); \
    char *buffer = packed_branch->buffer_; \
    size_t offset = sizeof(void *) + 2*sizeof(tree_node_handle); \
    size_t entry_count =  * (size_t *) (buffer+offset); \
    offset += sizeof( entry_count ); \
    for( size_t i = 0; i < entry_count; i++ ) { \
        tree_node_handle *child = (tree_node_handle *) (buffer + offset); \
        offset += sizeof( tree_node_handle ); \
        Rectangle *summary_rectangle = (Rectangle *) (buffer + offset); \
        offset += sizeof( Rectangle ); \
        unsigned rect_count = * (unsigned *) (buffer + offset); \
        offset += sizeof( unsigned ); \
        if( not summary_rectangle->containsPoint( requestedPoint ) ) { \
            if( rect_count == std::numeric_limits<unsigned>::max() ) { \
                offset += sizeof(tree_node_handle); \
            } else { \
                offset += rect_count * sizeof( Rectangle ); \
            }\
            continue; \
        } \
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
    for( size_t i = 0; i < current_node->cur_offset_; i++ ) { \
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
    size_t offset = sizeof(void *) + 2*sizeof(tree_node_handle); \
    size_t entry_count =  * (size_t *) (buffer+offset); \
    offset += sizeof( entry_count ); \
    for( size_t i = 0; i < entry_count; i++ ) { \
        tree_node_handle *child = (tree_node_handle *) (buffer + offset); \
        offset += sizeof( tree_node_handle ); \
        Rectangle *summary_rectangle = (Rectangle *) (buffer + offset); \
        offset += sizeof( Rectangle ); \
        unsigned rect_count = * (unsigned *) (buffer + offset); \
        offset += sizeof( unsigned ); \
        if( not summary_rectangle->intersectsRectangle( requestedRectangle ) ) { \
            if( rect_count == std::numeric_limits<unsigned>::max() ) { \
                offset += sizeof(tree_node_handle); \
            } else { \
                offset += rect_count * sizeof( Rectangle ); \
            }\
            continue; \
        } \
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
    tree_node_allocator *allocator = &(treeRef->node_allocator_);

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
            treeRef->stats.markNonLeafSearched();
#endif
        } else if( current_handle.get_type() == REPACKED_BRANCH_NODE ) {
            point_search_packed_branch_handle( current_handle,
                    requestedPoint, context );
#ifdef STAT

            treeRef->stats.markNonLeafSearched();
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
            for( size_t i = 0; i < branch_node->cur_offset_; i++ ) {
                auto child_handle = branch_node->entries.at(i).child;
                auto new_child_handle =
                    repack_subtree<min_branch_factor,max_branch_factor,strategy>( child_handle,
                        existing_allocator, new_allocator );
                branch_node->entries.at(i).child = new_child_handle;
            }
            auto new_handle = branch_node->repack( new_allocator );
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
    tree_node_allocator *allocator = &(treeRef->node_allocator_);
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
            treeRef->stats.markNonLeafSearched();
#endif
        } else if( current_handle.get_type() == REPACKED_BRANCH_NODE ) {
            rectangle_search_packed_branch_handle( current_handle,
                    requestedRectangle, context );
#ifdef STAT
            treeRef->stats.markNonLeafSearched();
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
tree_node_handle BRANCH_NODE_CLASS_TYPES::chooseNode(Point givenPoint)
{
    // FIXME: try and avoid all these materialize calls

    // CL1 [Initialize]
    tree_node_handle cur_node_handle = this->self_handle_;

    assert( cur_node_handle != nullptr );

    for( ;; ) {
        assert( cur_node_handle != nullptr );
        if( cur_node_handle.get_type() == LEAF_NODE ) {
            return cur_node_handle;
        } else {
            assert( cur_node_handle.get_type() == BRANCH_NODE );

            auto cur_node = treeRef->get_branch_node( cur_node_handle );
            // Compute the smallest expansion
            assert( cur_node->cur_offset_ > 0 );

            unsigned smallestExpansionBranchIndex = 0;

            tree_node_allocator *allocator = get_node_allocator( this->treeRef );
            IsotheticPolygon node_poly = cur_node->entries.at(0).materialize_polygon(
                    allocator );
            assert( node_poly.basicRectangles.size() > 0 );

            IsotheticPolygon::OptimalExpansion smallestExpansion =
                node_poly.computeExpansionArea(givenPoint);
            IsotheticPolygon::OptimalExpansion evalExpansion;

            for( size_t i = 1; i < cur_node->cur_offset_ &&
                    smallestExpansion.area != -1.0; i++ ) {
                Branch &b = cur_node->entries.at(i);
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
                Branch &b = cur_node->entries.at( smallestExpansionBranchIndex );
                node_poly = b.materialize_polygon( allocator );
                assert( node_poly.basicRectangles.size() > 0 );

                IsotheticPolygon subsetPolygon(
                        node_poly.basicRectangles.at(smallestExpansion.index) );

                subsetPolygon.expand(givenPoint);

                for( size_t i = 0; i < cur_node->cur_offset_; i++ ) {
                    if( i != smallestExpansionBranchIndex ) {
                        Branch &b_i = cur_node->entries.at(i);
                        subsetPolygon.increaseResolution( givenPoint,
                                b_i.materialize_polygon( allocator ) );
                    }
                }

                if( cur_node->parent != nullptr ) {
                    auto parent_node = treeRef->get_branch_node(
                            cur_node->parent );
                    Branch &b_parent = parent_node->locateBranch(
                            cur_node->self_handle_ );
                    subsetPolygon.intersection(
                            b_parent.materialize_polygon( allocator ) );
                }

                assert( node_poly.basicRectangles.size() > 0 );
                node_poly.remove( smallestExpansion.index );
                node_poly.merge( subsetPolygon );

                if( b.child.get_type() == LEAF_NODE ) {
                    auto child_as_leaf = treeRef->get_leaf_node( b.child );
                    if( child_as_leaf->cur_offset_ > 0 ) {
                        child_as_leaf->entries.at( child_as_leaf->cur_offset_ ) = givenPoint;
                        child_as_leaf->cur_offset_++;
                        // shrink_leaf
                        shrink( node_poly, child_as_leaf->entries.begin(),
                                child_as_leaf->entries.begin() +
                                child_as_leaf->cur_offset_, allocator );
                        child_as_leaf->cur_offset_--;
                    }
                }

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
                                        overfull_rect_count ),
                                    NodeHandleType( BIG_POLYGON ) );
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
                                        overfull_rect_count ),
                                    NodeHandleType( BIG_POLYGON ) );
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
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = current_node->entries.at(i);
                if( p == givenPoint ) {
                    return current_node_handle;
                }
            }
        } else {
            auto current_node = treeRef->get_branch_node( current_node_handle );
            // FL1 [Search subtrees]
            // Determine which branches we need to follow
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
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
    new (&(*left_node)) NodeType( this->treeRef, this->parent, left_handle );
    assert( left_node->self_handle_ == left_handle );

    alloc_data =
        allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                NodeHandleType( BRANCH_NODE ) );
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first; // take pin
    new (&(*right_node)) NodeType( this->treeRef, this->parent, right_handle );
    assert( right_node->self_handle_ == right_handle );

    SplitResult split = {
        { tree_node_handle(nullptr), left_handle }, 
        { tree_node_handle(nullptr), right_handle } };

    // So we are going to split this branch node.
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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
                size_t alloc_size = compute_sizeof_inline_unbounded_polygon(
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

    // When we downsplit two nodes we can make our polygon bigger
    // than our parents, provided we don't intersect with our
    // siblings. But they can do that too, and these expansions can
    // intersect.
    // So, if we downsplit, we actually need to intersect with our
    // parents.
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
tree_node_handle BRANCH_NODE_CLASS_TYPES::insert( Point givenPoint ) {

    assert( this->self_handle_.get_type() == BRANCH_NODE );
    // Find the appropriate position for the new point
    tree_node_handle current_handle = chooseNode( givenPoint );

    tree_node_allocator *allocator = get_node_allocator( this->treeRef );
    assert( current_handle.get_type() == LEAF_NODE );

    auto current_node = treeRef->get_leaf_node( current_handle );
    current_node->addPoint( givenPoint );

    SplitResult finalSplit = current_node->adjustTree();

    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        auto alloc_data =
            allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data.first)) BRANCH_NODE_CLASS_TYPES( this->treeRef,
                tree_node_handle(nullptr), alloc_data.second );
        auto new_root_handle = alloc_data.second;
        auto new_root_node = alloc_data.first;

        if( finalSplit.leftBranch.child.get_type() == LEAF_NODE ) {
            auto left_node = treeRef->get_leaf_node( finalSplit.leftBranch.child );
            left_node->parent = new_root_handle;
        } else {
            auto left_node = treeRef->get_branch_node( finalSplit.leftBranch.child );
            left_node->parent = new_root_handle;
        }
        new_root_node->addBranchToNode( finalSplit.leftBranch );

        if( finalSplit.rightBranch.child.get_type() == LEAF_NODE ) {
            auto right_node = treeRef->get_leaf_node( finalSplit.rightBranch.child );
            right_node->parent = new_root_handle;
        } else {
            auto right_node = treeRef->get_branch_node( finalSplit.rightBranch.child );
            right_node->parent = new_root_handle;
        }
        new_root_node->addBranchToNode( finalSplit.rightBranch );

        assert( this->self_handle_.get_type() == BRANCH_NODE );
        allocator->free( this->self_handle_, sizeof( BRANCH_NODE_CLASS_TYPES ) );
        this->self_handle_ = tree_node_handle( nullptr );

        return new_root_handle;
    }

    return this->self_handle_;
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

    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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
                std::cout << "Parent poly (" << this->parent << "does not contain: " << p
                    << std::endl;
                std::cout << "Poly was: " << parent_poly <<
                    std::endl;
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
        for( size_t i = 0; i < this->cur_offset_; i++ ) {
            for( size_t j = 0; j < this->cur_offset_; j++ ) {
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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {

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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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
    for( size_t i = 0; i < this->cur_offset_; i++ ) {
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

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::stat() {
    std::stack<tree_node_handle> context;

    // Initialize our context stack
    context.push( this->self_handle_ );
    tree_node_handle currentContext;
    unsigned long polygonSize;
    unsigned long totalPolygonSize = 0;
    unsigned long totalLines = 0;
    size_t memoryFootprint = 0;
    unsigned long totalNodes = 1;
    unsigned long totalLeaves = 0;
    size_t deadSpace = 0;

    std::vector<unsigned long> histogramPolygon;
    histogramPolygon.resize(10000, 0);
    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize( max_branch_factor, 0 );

    double coverage = 0.0;

    tree_node_allocator *allocator = get_node_allocator( this->treeRef );

    while( !context.empty() ) {
        currentContext = context.top();
        context.pop();


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

        } else {
            auto current_branch_node = treeRef->get_branch_node( currentContext
                    );

            unsigned fanout = current_branch_node->cur_offset_;
            if( fanout >= histogramFanout.size() ) {
                histogramFanout.resize(2*fanout, 0);
            }
            histogramFanout[fanout]++;

            // Compute the overlap and coverage of our children
            for( size_t i = 0; i < current_branch_node->cur_offset_; i++ ) {
                Branch &b = current_branch_node->entries.at(i);
                IsotheticPolygon polygon = b.materialize_polygon( allocator );
                coverage += polygon.area();
            }

            totalNodes += current_branch_node->cur_offset_;
            memoryFootprint +=
                sizeof(BranchNode<min_branch_factor,max_branch_factor,strategy>);// +
                // other out of line polys

            deadSpace += (sizeof(Branch) *
                    (max_branch_factor-current_branch_node->cur_offset_ ) );
            
            for( size_t i = 0; i < current_branch_node->cur_offset_; i++ ) {
                Branch &b = current_branch_node->entries.at(i);
                /*
                auto child = this->treeRef->get_node( b.child );
                if( child->get_entry_count() == 1 ) {
                    singularBranches++;
                }
                */
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
    STATMEM(memoryFootprint);
    STATHEIGHT(height());
    STATSIZE(totalNodes);
    STATEXEC(std::cout << "DeadSpace: " << deadSpace << std::endl);
    //STATSINGULAR(singularBranches);
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
    std::cout << this->treeRef->stats;

    STATEXEC(std::cout << "### ### ### ###" << std::endl);
}



#undef NODE_TEMPLATE_PARAMS
#undef LEAF_NODE_CLASS_TYPES 
#undef BRANCH_NODE_CLASS_TYPES
