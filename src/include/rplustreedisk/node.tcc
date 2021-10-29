#include <rplustree/node.h>
#include <rplustree/rplustree.h>

#define NODE_TEMPLATE_TYPES template <int min_branch_factor, int max_branch_factor>
#define NODE_CLASS_TYPES Node<min_branch_factor,max_branch_factor>

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::deleteSubtrees() {

    // N.B., does not actually delete anything
    // FIXME
    if( not isLeaf() ) {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>(entries.at(i));
            auto child_node = treeRef->get_node( b.child );
            child_node->deleteSubtrees();
        }
    }
}

NODE_TEMPLATE_TYPES
Rectangle NODE_CLASS_TYPES::boundingBox() {
    if( isLeaf() ) {
        Point &p = std::get<Point>( entries.at(0) );
        Rectangle bb = Rectangle( p, Point::closest_larger_point( p ) );
        for( unsigned i = 1; i < cur_offset_; i++ ) {
            bb.expand( std::get<Point>( entries.at(i) ) );
        }
        return bb;
    }

    Rectangle bb = std::get<Branch>( entries.at(0) ).boundingBox;
    for( unsigned i = 1; i < cur_offset_; i++ ) {
        bb.expand( std::get<Branch>( entries.at(i) ).boundingBox );
    }
    return bb;
}

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::updateBranch(
    tree_node_handle child_handle,
    Rectangle &bounding_box
) {
    // Locate the child
    for( unsigned child_index = 0; child_index < cur_offset_;
            child_index++ ) {
        Branch &b = std::get<Branch>( entries.at( child_index ) );
        if( b.child != child_handle ) {
            b.boundingBox = bounding_box;
            return;
        }
    }
}

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::removeBranch( tree_node_handle child_handle ) {
    assert( not isLeaf() );

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );
        if( b.child == child_handle ) {
            entries.at(i) == entries.at(cur_offset_-1);
            cur_offset_--;
            break;
        }
    }
}

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::removePoint( Point &givenPoint ) {
    assert( isLeaf() );

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Point &p = std::get<Point>( entries.at(i) );
        if( p == givenPoint) {
            entries.at(i) == entries.at(cur_offset_-1);
            cur_offset_--;
            break;
        }
    }
}


NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::exhaustiveSearch(
    Point &requestedPoint,
    std::vector<Point> &accumulator
) {
    if( isLeaf() ) {
        // Scan
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Point &p = std::get<Point>( entries.at(i) );
            if( p == requestedPoint ) {
                accumulator.push_back( requestedPoint );
                break;
            }
        }
        return;
    }
    // Determine which branches we need to follow
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        // Recurse
        Branch &b = std::get<Branch>( entries.at(i) );
        auto child_node = treeRef->get_node( b.child );
        child_node->exhaustiveSearch( requestedPoint, accumulator );
    }
}

NODE_TEMPLATE_TYPES
std::vector<Point> NODE_CLASS_TYPES::search(
    Point &requestedPoint
) {
    std::vector<Point> matchingPoints;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push( self_handle_ );

    while( not context.empty() ) {

        tree_node_handle current_handle = context.top();
        context.pop();

        auto current_node = treeRef->get_node( current_handle );

        if( current_node->isLeaf() ) {
            // We are a leaf so add our data points when they are the search point
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = std::get<Point>( current_node->entries.at(i) );
                if( requestedPoint == p ) {
                    matchingPoints.push_back( p );
                }
            }

#ifdef STAT
            treeRef.stats.markLeafSearched();
#endif
        } else {
            // Determine which branches we need to follow
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( b.boundingBox.containsPoint( requestedPoint ) ) {
                    // Add to the nodes we will check
                    context.push( b.child );
                }
            }

#ifdef STAT
            treeRef.stats.markNonLeafNodeSearched();
#endif
        }
    }

#ifdef STAT
    treeRef.stats.resetSearchTracker<false>();
#endif

    return matchingPoints;
}

NODE_TEMPLATE_TYPES
std::vector<Point> NODE_CLASS_TYPES::search(
    Rectangle &requestedRectangle
) {
    std::vector<Point> matchingPoints;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push( self_handle_ );

    while( not context.empty() ) {
        tree_node_handle current_handle = context.top();
        context.pop();
        auto current_node = treeRef->get_node( current_handle );

        if( current_node->isLeaf() ) {
            // We are a leaf so add our data points when they are within the search rectangle
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = std::get<Point>( current_node->entries.at(i) ); 
                if( requestedRectangle.containsPoint( p ) ) {
                    matchingPoints.push_back( p );
                }
            }

#ifdef STAT
            treeRef.stats.markLeafSearched();
#endif
        } else {

            // Determine which branches we need to follow
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( b.boundingBox.intersectsRectangle( requestedRectangle ) ) {
                    // Add to the nodes we will check
                    context.push( b.child );
                }
            }
#ifdef STAT
            treeRef.stats.markNonLeafNodeSearched();
#endif
        }
    }

#ifdef STAT
    treeRef.stats.resetSearchTracker<true>();
#endif

    return matchingPoints;
}

// Always called on root, this = root
// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
// choosing a particular leaf
NODE_TEMPLATE_TYPES
tree_node_handle NODE_CLASS_TYPES::chooseNode(Point givenPoint)
{
    // CL1 [Initialize]
    tree_node_handle current_handle = self_handle_;

    for (;;) {
        // CL2 [Leaf check]
        auto current_node = treeRef->get_node( current_handle );
        if( current_node->isLeaf() ) {
            return current_handle;
        }


        // Compute the smallest expansion
        unsigned smallestExpansionIndex = 0;
        Branch &b_0 = std::get<Branch>( current_node->entries.at(0) );
        double smallestExpansionArea = b_0.boundingBox.computeExpansionArea( givenPoint );
        for( unsigned i = 1; i < current_node->cur_offset_ and smallestExpansionArea != -1.0; i++ ) {

            Branch &b_i = std::get<Branch>( current_node->entries.at(i) ) ;
            double expansionArea = b_i.boundingBox.computeExpansionArea( givenPoint );
            if( expansionArea < smallestExpansionArea ) {
                smallestExpansionIndex = i;
                smallestExpansionArea = expansionArea;
            }

        }

        if( smallestExpansionArea != -1.0 ) {
            Branch &b = std::get<Branch>( current_node->entries.at(
                        smallestExpansionIndex ) );
            b.boundingBox.expand( givenPoint );
        }

        // Descend
        Branch &b = std::get<Branch>( current_node->entries.at( smallestExpansionIndex ) );
        current_handle = b.child;
    }
}

NODE_TEMPLATE_TYPES
tree_node_handle NODE_CLASS_TYPES::findLeaf(
    Point givenPoint
) {

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push(self_handle_);

    while( not context.empty() ) {

        tree_node_handle current_handle = context.top();
        context.pop();

        auto current_node = treeRef->get_node( current_handle );

        if( current_node->isLeaf() ) {

            // FL2 [Search leaf node for record]
            // Check each entry to see if it matches E
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Point &p = std::get<Point>( entries.at(i) );
                if( p == givenPoint ) {
                    return current_handle;
                }
            }
            // Fall through, not contained in this node.
        } else {

            // FL1 [Search subtrees]
            // Determine which branches we need to follow
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( entries.at(i) );
                if( b.boundingBox.containsPoint( givenPoint ) ) {
                    // Add the child to the nodes we will consider
                    context.push( b.child );
                }
            }
        }
    }

    return tree_node_handle( nullptr );
}

NODE_TEMPLATE_TYPES
Partition NODE_CLASS_TYPES::partitionNode()
{
    Partition defaultPartition;
    unsigned costMetric = std::numeric_limits<unsigned>::max();
    double location;

    if( isLeaf() ) {

        for( unsigned d = 0; d < dimensions; d++ ) {
            // Sort along dimension d
            std::sort( entries.begin(), entries.begin() + cur_offset_,
                [d](NodeEntry &a, NodeEntry &b) {
                    auto &a_point = std::get<Point>( a );
                    auto &b_point = std::get<Point>( b );
                    return a_point[d] < b_point[d];
                 } );

            // Pick at least half the data
            location = std::get<Point>(entries.at(cur_offset_ / 2 - 1))[d];

            // Compute cost, # of duplicates of this location
            unsigned duplicateCount = 0;
            for( unsigned i = 0; i < cur_offset_; i++ ) {
                Point &data_point = std::get<Point>( entries.at(i) );
                if( location == data_point[ d ] ) {
                    duplicateCount++;
                }
            }

            // Compare cost
            if( duplicateCount < costMetric ) {
                defaultPartition.dimension = d;
                defaultPartition.location = location;
                costMetric = duplicateCount;
            }
        }

        return defaultPartition;
    }

    std::vector<Rectangle> sortableBoundingBoxes;

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );
        sortableBoundingBoxes.push_back( b.boundingBox );
    }

    for( unsigned d = 0; d < dimensions; d++ ) {
        // Sort along d
        std::sort( sortableBoundingBoxes.begin(), sortableBoundingBoxes.end(),
                [d](Rectangle a, Rectangle b){ return a.upperRight[d] < b.upperRight[d]; } );

        // Pick at least half of the rectangles
        location = sortableBoundingBoxes[sortableBoundingBoxes.size() / 2 - 1].upperRight[d];

        // Compute cost, # of splits if d is chosen
        unsigned currentInducedSplits = 0;
        for( unsigned i = 0; i < sortableBoundingBoxes.size(); i++ ) {
            if( sortableBoundingBoxes[i].lowerLeft[d] < location and
                    location < sortableBoundingBoxes[i].upperRight[d] ) {
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

// Splitting a node will remove it from its parent node and its memory will be freed
NODE_TEMPLATE_TYPES
SplitResult NODE_CLASS_TYPES::splitNode( Partition p ) {
    using NodeType = NODE_CLASS_TYPES;

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    new (&(*(alloc_data.first))) NodeType( treeRef, alloc_data.second,
            parent_ );
    auto left_handle = alloc_data.second;
    auto left_node = alloc_data.first;

    alloc_data = allocator->create_new_tree_node<NodeType>();
    new (&(*(alloc_data.first))) NodeType( treeRef, alloc_data.second,
            parent_ );
    auto right_handle = alloc_data.second;
    auto right_node = alloc_data.first;
    std::cout << "Going to split: " << self_handle_ << " into nodes: "
        << left_handle << " and " << right_handle << std::endl;

    if( isLeaf() ) {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Point &data_point = std::get<Point>( entries.at(i) );
            if( data_point[p.dimension] <= p.location and
                    left_node->cur_offset_ < max_branch_factor ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    data_point;
            } else {
                right_node->entries.at( right_node->cur_offset_++ ) =
                    data_point;
            }
        }
    } else {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            if( b.boundingBox.upperRight[p.dimension] <= p.location ) {
                auto child_node = treeRef->get_node( b.child );
                child_node->parent_ = left_handle;
                left_node->entries.at( left_node->cur_offset_++ ) = b;
            } else if( b.boundingBox.lowerLeft[p.dimension] >= p.location ) {
                auto child_node = treeRef->get_node( b.child );
                child_node->parent_ = right_handle;
                right_node->entries.at( right_node->cur_offset_++ ) = b;
            } else {
                auto child_node = treeRef->get_node( b.child );
                SplitResult downwardSplit = child_node->splitNode( p );

                if( downwardSplit.leftBranch.boundingBox != Rectangle::atInfinity ) {
                    auto left_child_node =
                        treeRef->get_node( downwardSplit.leftBranch.child );
                    left_child_node->parent_ = left_handle;
                    left_node->entries.at( left_node->cur_offset_++ ) = downwardSplit.leftBranch;
                }

                if( downwardSplit.rightBranch.boundingBox != Rectangle::atInfinity ) {
                    auto right_child_node =
                        treeRef->get_node( downwardSplit.rightBranch.child );
                    right_child_node->parent_ = right_handle;
                    right_node->entries.at( right_node->cur_offset_++ ) = downwardSplit.rightBranch;
                }
            }
        }
    }

    return {{left_handle, left_node->boundingBox()}, {right_handle,
        right_node->boundingBox()}};
}

// Splitting a node will remove it from its parent node and its memory will be freed
NODE_TEMPLATE_TYPES
SplitResult NODE_CLASS_TYPES::splitNode()
{
    SplitResult returnSplit = splitNode( partitionNode() );

    return returnSplit;
}

// This bottom-to-top sweep is only for splitting bounding boxes as necessary
NODE_TEMPLATE_TYPES
SplitResult NODE_CLASS_TYPES::adjustTree()
{
    tree_node_handle current_handle = self_handle_;
    SplitResult propagationSplit = { 
        {tree_node_handle(nullptr), Rectangle()}, 
        {tree_node_handle(nullptr), Rectangle()}
    };

    while( current_handle != nullptr ) {

        auto current_node = treeRef->get_node( current_handle );
        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and
                propagationSplit.rightBranch.child != nullptr ) {
            auto left_node = treeRef->get_node( propagationSplit.leftBranch.child
                    );
            if( left_node->cur_offset_ > 0 ) {
                current_node->entries.at( current_node->cur_offset_ ) =
                    propagationSplit.leftBranch;
                current_node->cur_offset_++;
            }

            auto right_node = treeRef->get_node(
                    propagationSplit.rightBranch.child );
            if( right_node->cur_offset_ > 0 ) {
                current_node->entries.at( current_node->cur_offset_ ) =
                    propagationSplit.rightBranch;
                current_node->cur_offset_++;
            }
        }

        // Early exit if this node does not overflow
        if( current_node->cur_offset_ <= max_branch_factor ) {
            propagationSplit = { 
                {tree_node_handle( nullptr ), Rectangle()},
                {tree_node_handle( nullptr ), Rectangle()}
            };
            break;
        }

        std::cout << "Told to split!" << std::endl;
        // Otherwise, split node
        propagationSplit = current_node->splitNode();

        // Cleanup before ascending
        if( current_node->parent_ != nullptr ) {
            std::cout << "Removed " << current_handle << " from parent "
                << current_node->parent_ << std::endl;
            auto parent_node = treeRef->get_node( current_node->parent_ );
            parent_node->removeBranch( current_handle );
        }

        // Ascend, propagating splits
        auto left_split_node = treeRef->get_node(
                propagationSplit.leftBranch.child );
        current_handle = left_split_node->parent_;
        std::cout << "Recursing up: " << current_handle << std::endl;
    }

    return propagationSplit;
}

// Always called on root, this = root
NODE_TEMPLATE_TYPES
tree_node_handle NODE_CLASS_TYPES::insert( Point givenPoint ) {
    using NodeType = NODE_CLASS_TYPES;

    // Find the appropriate position for the new point
    tree_node_handle adjustContext = chooseNode(givenPoint);
    auto adjust_node = treeRef->get_node( adjustContext );
    adjust_node->entries.at( adjust_node->cur_offset_++ ) = givenPoint;

    auto finalSplit = adjust_node->adjustTree();

    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        std::cout << "Need to grow the tree." << std::endl;
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        auto alloc_data = allocator->create_new_tree_node<NodeType>();
        new (&(*(alloc_data.first))) NodeType( treeRef, alloc_data.second,
                tree_node_handle(nullptr) );
        
        auto new_root_node = alloc_data.first;
        auto new_root_handle = alloc_data.second;

        auto left_node = treeRef->get_node( finalSplit.leftBranch.child );
        auto right_node = treeRef->get_node( finalSplit.rightBranch.child );
        left_node->parent_ = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.leftBranch;
        right_node->parent_ = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.rightBranch;

        return new_root_handle;
    }

    return self_handle_;
}

// To be called on a leaf
NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::condenseTree()
{
    tree_node_handle current_handle = self_handle_;

    while( current_handle != nullptr ) {
        auto current_node = treeRef->get_node( current_handle );
        auto parent_handle = current_node->parent_;
        if( parent_handle != nullptr ) {
            auto parent_node = treeRef->get_node( parent_handle );
            if( current_node->cur_offset_ == 0 ) {
                parent_node->removeBranch( current_handle );
            } else {
                Rectangle bb = current_node->boundingBox();
                parent_node->updateBranch( current_handle, bb );
            }
        }
        current_handle = parent_handle;
    }
}

// Always called on root, this = root
NODE_TEMPLATE_TYPES
tree_node_handle NODE_CLASS_TYPES::remove( Point givenPoint ) {
    // D1 [Find node containing record]

    tree_node_handle leaf_handle = findLeaf(givenPoint);
    if( leaf_handle == nullptr ) {
        return self_handle_;
    }

    auto leaf_node = treeRef->get_node( leaf_handle );

    // D2 [Delete record]
    leaf_node->removePoint( givenPoint );
    std::cout << "Remove done." << std::endl;

    // D3 [Propagate changes]
    leaf_node->condenseTree();

    std::cout << "Condense done." << std::endl;

    // D4 [Shorten tree]
    if( not isLeaf() and cur_offset_ == 1 ) {

        // FIXME: GC existing root
        tree_node_handle new_root_handle = std::get<Branch>(
                entries.at(0) ).child;
        auto new_root_node = treeRef->get_node( new_root_handle );
        new_root_node->parent_ = tree_node_handle(nullptr);
        return new_root_handle;
    }

    return self_handle_;
}

NODE_TEMPLATE_TYPES
unsigned NODE_CLASS_TYPES::checksum()
{
    unsigned sum = 0;

    if( isLeaf() ) {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Point &p = std::get<Point>( entries.at(i) );
            for( unsigned d = 0; d < dimensions; d++ ) {
                sum += (unsigned) p[d];
            }
        }
        return sum;
    }

    for( unsigned i = 0; i < cur_offset_; i++) {
        // Recurse
        Branch &b = std::get<Branch>( entries.at(i) );
        auto child_node = treeRef->get_node( b.child );
        sum += child_node->checksum();
    }

    return sum;
}

NODE_TEMPLATE_TYPES
bool NODE_CLASS_TYPES::validate( tree_node_handle expectedParent, unsigned index )
{
    if( parent_ != expectedParent or cur_offset_ > max_branch_factor ) {
        std::cout << "parent = " << parent_ << " expectedParent = " << expectedParent << std::endl;
        std::cout << "maxBranchFactor = " << max_branch_factor << std::endl;
        std::cout << "branches.size() = " << cur_offset_ << std::endl;
        assert( parent_ == expectedParent );
        assert( cur_offset_ <= max_branch_factor );
    }

    if( isLeaf() ) {
        if( expectedParent != nullptr ) {
            auto parent_node = treeRef->get_node( parent_ );
            Branch &parent_branch = std::get<Branch>( parent_node->entries.at( index ) );
            for( unsigned i = 0; i < cur_offset_; i++ ) {
                Point &p = std::get<Point>( entries.at(i) );

                if( not parent_branch.boundingBox.containsPoint( p ) ) {
                    std::cout << parent_branch.boundingBox << " fails to contain " << p << std::endl;
                    assert(parent_branch.boundingBox.containsPoint(p));
                }
            }
        }
        return true;
    }

    // Branch node
    bool valid = true;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );
        auto child_node = treeRef->get_node( b.child );
        
        valid = valid and child_node->validate( self_handle_, i );
    }

    return valid;
}

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::print( unsigned n ) {
    std::string indendtation(n * 4, ' ');
    std::cout << indendtation << "Node " << self_handle_ << std::endl;
    std::cout << indendtation << "(" << std::endl;
    std::cout << indendtation << "    Parent: " << parent_ << std::endl;
    if( not isLeaf() ) {
        std::cout << indendtation << "    Branches: " << std::endl;
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            std::cout << indendtation << "		" << b.child << std::endl;
            std::cout << indendtation << "		" << b.boundingBox << std::endl;
        }
    } else {
        std::cout << indendtation << "    Data: ";
        for( unsigned i = 0; i < cur_offset_; i++) {
            Point &p = std::get<Point>( entries.at(i) );
            std::cout << p << std::endl;
        }
    }
    std::cout << std::endl << indendtation << ")" << std::endl;
}

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::printTree( unsigned n )
{
    // Print this node first
    print( n );

    // Print any of our children with one more level of indentation
    std::string indendtation(n * 4, ' ');
    std::cout << std::endl << indendtation << "{" << std::endl;
    if( not isLeaf() ) {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            // Recurse
            Branch &b = std::get<Branch>( entries.at(i) ) ;
            auto child_node = treeRef->get_node( b.child );
            child_node->printTree( n + 1 );
        }
    }
    std::cout << std::endl << indendtation << "}" << std::endl;
}

//FIXME continue from here.
NODE_TEMPLATE_TYPES
unsigned NODE_CLASS_TYPES::height()
{
    unsigned ret = 0;
    Node *node = this;
    tree_node_handle current_handle = self_handle_;
    for( ;; ) {
        auto current_node = treeRef->get_node( current_handle );
        ret++;
        if( current_node->isLeaf() ) {
            return ret;
        } else {
            Branch &b = std::get<Branch>( current_node->entries.at(0) );
            current_handle = b.child;
        }
    }
}

NODE_TEMPLATE_TYPES
void NODE_CLASS_TYPES::stat()
{
    using NodeType = NODE_CLASS_TYPES;
#ifdef STAT

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push( self_handle_ );

    size_t memoryFootprint = 0;
    unsigned long totalNodes = 1;

    unsigned long singularBranches = 0;
    unsigned long totalLeaves = 0;

    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize( max_branch_factor, 0 );

    double coverage = 0.0;
    double overlap = 0.0;

    while( not context.empty() ) {
        auto current_handle = context.top();
        context.pop();
        auto current_node = treeRef->get_node( current_handle );

        unsigned fanout = current_node->cur_offset_;
        if( fanout >= histogramFanout.size() ) {
            histogramFanout.resize(2*fanout, 0);
        }
        histogramFanout[fanout]++;

        // Compute the overlap and coverage of our children
        if( not current_node->isLeaf() ) {
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>(
                        current_node->entries.at(i) );
                coverage += b.boundingBox.area();

                for( unsigned j = 0; j < current_node->cur_offset_; j++ ) {
                    if( i != j ) {
                        Branch &b_j = std::get<Branch>( current_node->entries.at( j ) );
                        overlap +=
                            b.boundingBox.computeIntersectionArea( b_j.boundingBox );
                    }
                }
            }
        }

        // FIXME: not sure these memory consumption figures are correct.
        if( current_node->isLeaf() ) {
            totalLeaves++;
            memoryFootprint += sizeof(NodeType);
        } else {
            totalNodes += cur_offset_;
            memoryFootprint += sizeof(NodeType);
            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>(
                        current_node->entries.at(i) );
                auto child_node = treeRef->get_node( b.child );
                if( child_node->cur_offset_ == 1 ) {
                    singularBranches++;
                }

                context.push( b.child );
            }
        }
    }

    // Print out what we have found
    STATMEM(memoryFootprint);
    STATHEIGHT(height());
    STATSIZE(totalNodes);
    STATSINGULAR(singularBranches);
    STATLEAF(totalLeaves);
    STATBRANCH(totalNodes - 1);
    STATCOVER(coverage);
    STATOVERLAP(overlap);
    STATAVGCOVER(coverage / totalNodes);
    STATAVGOVERLAP(overlap /totalNodes);
    STATFANHIST();
    for( unsigned i = 0; i < histogramFanout.size(); i++ ) {
        if( histogramFanout[i] > 0 ) {
            STATHIST( i, histogramFanout[i] );
        }
    }
    std::cout << treeRef.stats;
#else
    (void) 0;
#endif
}
