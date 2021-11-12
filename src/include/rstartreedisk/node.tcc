#define NODE_TEMPLATE_PARAMS template <int min_branch_factor, int max_branch_factor>
#define LEAF_NODE_CLASS_TYPES LeafNode<min_branch_factor, max_branch_factor>
#define BRANCH_NODE_CLASS_TYPES BranchNode<min_branch_factor, max_branch_factor>

template <int min_branch_factor, int max_branch_factor, typename functor>
void treeWalker( RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef, tree_node_handle root, functor &f ) {
    std::stack<tree_node_handle> context;
    context.push( root );
    tree_node_handle currentContext;

    while( !context.empty() ) {
        currentContext = context.top();
        context.pop();

        f( treeRef, currentContext );

        if( currentContext.get_type() == BRANCH_NODE ) {
            auto node = treeRef->get_branch_node( currentContext );
            for( unsigned i = 0; i < node->cur_offset_; i++ ) {
                context.push( node->entries.at(i).child );
            }
        }
    }
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::deleteSubtrees()
{
    return;
}

NODE_TEMPLATE_PARAMS
Rectangle LEAF_NODE_CLASS_TYPES::boundingBox() const
{
    assert( cur_offset_ > 0 );
    Rectangle boundingBox( entries[0], Point::closest_larger_point(
                entries[0] ) );

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        boundingBox.expand( entries.at(i) );
    }

    return boundingBox;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::removePoint( const Point &givenPoint )
{
    unsigned i = 0;
    for( ; i < cur_offset_; i++ ) {
        if( entries.at(i) == givenPoint ) {
            break;
        }
    }
    assert( givenPoint == entries.at(i) );

    entries.at(i) = entries.at(cur_offset_-1);
    cur_offset_--;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::exhaustiveSearch(
    const Point &requestedPoint,
    std::vector<Point> &accumulator
) const {
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        const Point &p = entries.at(i);

        if( p == requestedPoint ) {
            accumulator.push_back( p );
        }
    }
}

// Only gets called if the root is a leaf
NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::searchSub(
    const Point &requestedPoint,
    std::vector<Point> &accumulator
) {
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        const Point &p = entries.at(i);

        if( p == requestedPoint ) {
            accumulator.push_back( p );
        }
    }
}

// Only gets called if the root is a leaf
NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::searchSub(
    const Rectangle &rectangle,
    std::vector<Point> &accumulator
) {
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        const Point &p = entries.at(i);

        if( rectangle.containsPoint( p ) ) {
            accumulator.push_back( p );
        }
    }
}

// Only called if the root is a leaf
NODE_TEMPLATE_PARAMS
std::vector<Point> LEAF_NODE_CLASS_TYPES::search(
    const Point &requestedPoint
) {
    std::vector<Point> accumulator;

    searchSub( requestedPoint, accumulator );

#ifdef STAT
    treeRef->stats.resetSearchTracker( false );
#endif
    return accumulator;
}

NODE_TEMPLATE_PARAMS
std::vector<Point> LEAF_NODE_CLASS_TYPES::search(
    const Rectangle &requestedRectangle
) {
    std::vector<Point> matchingPoints;

    searchSub( requestedRectangle, matchingPoints );

#ifdef STAT
    treeRef->stats.resetSearchTracker( true );
#endif
    return matchingPoints;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::chooseSubtree(
    const NodeEntry &givenNodeEntry
) {
    return self_handle_;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::findLeaf(
    const Point &givenPoint
) {
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        if( entries.at(i) == givenPoint ) {
            return self_handle_;
        }
    }

    return tree_node_handle( nullptr );
}

NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::chooseSplitLeafAxis()
{
    unsigned optimalAxis = 0;
    double optimalMargin = std::numeric_limits<double>::infinity();

    // Make the entries easier to work with
    std::vector<Point *> entriesCopy;
    entriesCopy.reserve( cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        entriesCopy.push_back( &(entries.at(i)) );
    }

    // Consider all M-2m+2 distributions in each dimension
    for( unsigned d = 0; d < dimensions; d++ ) {
        // First sort in the current dimension
        std::sort( entriesCopy.begin(), entriesCopy.end(), [d](Point *a, Point *b)
        {
            return (*a)[d] < (*b)[d];
        });

        // Setup groups
        std::vector<Point *> groupA( entriesCopy.begin(), entriesCopy.begin() + min_branch_factor );
        std::vector<Point *> groupB( entriesCopy.begin() + min_branch_factor, entriesCopy.end() );

        // Cycle through all M-2m+2 distributions
        double totalMargin = 0.0;
        while( groupA.size() <= max_branch_factor and groupB.size() >= min_branch_factor ) {
            // Compute the margin of groupA and groupB
            Rectangle boundingBoxA( *groupA[0], Point::closest_larger_point( *groupA[0] ) );
            for( unsigned i = 1; i < groupA.size(); i++ ) {
                boundingBoxA.expand( *groupA[i] );
            }

            Rectangle boundingBoxB( *groupB[0],
                    Point::closest_larger_point( *groupB[0] ) );
            for( unsigned i = 1; i < groupB.size(); i++ ) {
                boundingBoxB.expand( *groupB[i] );
            }

            // Add to the total margin sum
            totalMargin += boundingBoxA.margin() + boundingBoxB.margin();

            // Add one new value to groupA and remove one from groupB to obtain next distribution
            Point *transferPoint = groupB.front();
            groupB.erase( groupB.begin() );
            groupA.push_back( transferPoint );
        }

        if( totalMargin < optimalMargin ) {
            optimalMargin = totalMargin;
            optimalAxis = d;
        }
    }

    // Sort along our best axis
    std::sort( entries.begin(), entries.begin() +cur_offset_,
            [optimalAxis]( Point &a, Point &b ) {
        return a[optimalAxis] < b[optimalAxis];
    });

    return optimalAxis;
}

// CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all
//  margin values for the different distributions. This can be stored in a array of variable
//  that we keep in a loop -> and the just compare to the others?
// 	We can first call a helper function that returns an array of all possible distributions for it?
// CSA2: Return the Axis that has the minimum total sum of all the distributions
NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::chooseSplitAxis()
{
    return chooseSplitLeafAxis();
}

// CSI1: Given the chosen split index
// 	group all the entries into multiple groups and choose the one that has the least
// 	overlap value; resolve ties with the minimum area
// 	returns tuple of best distribution group indices
NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::chooseSplitIndex( unsigned axis ) {
    // We assume this is called after we have sorted this->data according to axis.

    const auto groupABegin = entries.begin();
    const auto groupAEnd = entries.begin() + min_branch_factor;
    const auto groupBBegin = entries.begin() + min_branch_factor;
    const auto groupBEnd = entries.begin() + cur_offset_;

    std::vector<Point> groupA( groupABegin, groupAEnd );
    std::vector<Point> groupB( groupBBegin, groupBEnd );
    unsigned splitIndex = cur_offset_  / 2;

    // Find the best size out of all the distributions
    double minOverlap = std::numeric_limits<double>::infinity();
    double minArea = std::numeric_limits<double>::infinity();

    // Tracking what the current "cut" mark is
    unsigned currentSplitPoint = min_branch_factor;

    // Try each of the M-2m + 2 groups
    while( groupA.size() <= max_branch_factor and groupB.size() >= min_branch_factor ) {
        // Compute the margin of groupA and groupB
        Rectangle boundingBoxA( groupA[0], Point::closest_larger_point( groupA[0] ) );
        for( unsigned i = 1; i < groupA.size(); i++ ) {
            boundingBoxA.expand( groupA[i] );
        }

        Rectangle boundingBoxB( groupB[0], Point::closest_larger_point(
                    groupB[0] ) );
        for( unsigned i = 1; i < groupB.size(); i++ ) {
            boundingBoxB.expand( groupB[i] );
        }

        // Compute intersection area to determine best grouping of data points
        double evalDistOverlap = boundingBoxA.computeIntersectionArea(boundingBoxB);

        if( evalDistOverlap < minOverlap ) {
            // We save this current distribution of indices to return
            minOverlap = evalDistOverlap;
            splitIndex = currentSplitPoint;

            // Set this if we haven't already
            if( minArea == std::numeric_limits<double>::infinity() ) {
                minArea = boundingBoxA.area() + boundingBoxB.area();
            }
        } else if( evalDistOverlap == minOverlap ) {
            // If overlap is equal, we use the distribution that creates the smallest areas
            double evalMinArea = boundingBoxA.area() + boundingBoxB.area();

            if( evalMinArea < minArea ) {
                // Save this current distribution of indices to return
                minArea = evalMinArea;
                splitIndex = currentSplitPoint;
            }
        }
        
        // Add one new value to groupA and remove one from groupB to obtain next distribution
        Point transferPoint = groupB.front();
        groupB.erase( groupB.begin() );
        groupA.push_back( transferPoint );

        // Push the split point forward.
        currentSplitPoint++;
    }

    return splitIndex;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::splitNode()
{
    using NodeType = LEAF_NODE_CLASS_TYPES;
    // S1: Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
    // S2: Invoke chooseSplitIndex given the axis to determine the best distribution along this axis
    // S3: Distribute the entries among these two groups

    // Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
    // For now we will save the axis as a int -> since this allows for room for growth in the future
    // Call ChooseSplitIndex to create optimal splitting of data array
    unsigned splitAxis = chooseSplitAxis();
    unsigned splitIndex = chooseSplitIndex(splitAxis);

    std::cout << "Split Leaf Node: {" <<  std::endl;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        std::cout << entries.at(i) << std::endl;
    }
    std::cout << "}" << std::endl;
    std::cout << "Split on axis" << splitAxis << ", index: " <<
        splitIndex << std::endl;

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    auto alloc_data =
        allocator->create_new_tree_node<NodeType>( NodeHandleType(
                    LEAF_NODE ) );

    auto newSibling = alloc_data.first;
    tree_node_handle sibling_handle = alloc_data.second;

    new (&(*(newSibling))) NodeType( treeRef, sibling_handle, parent,
            level );

    newSibling->parent = parent;
    newSibling->level = level;
    newSibling->treeRef = treeRef; 
    newSibling->self_handle_ = sibling_handle;

#if !defined( NDEDBUG )
    if( parent ) {
        auto parent_ptr = treeRef->get_branch_node( parent );
        assert( level + 1 == parent_ptr->level );

    }
#endif

    // Copy everything to the right of the splitPoint (inclusive) to the new sibling
    std::copy( entries.begin() + splitIndex, entries.begin() + cur_offset_,
            newSibling->entries.begin() );

    newSibling->cur_offset_ = cur_offset_ - splitIndex;

    // Chop our node's data down
    cur_offset_ = splitIndex;

    assert( cur_offset_ > 0 );
    assert( newSibling->cur_offset_ > 0 );

    std::cout << "New Node1: {" <<  std::endl;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        std::cout << entries.at(i) << std::endl;
    }
    std::cout << "}" << std::endl;

    std::cout << "New Node2: {" <<  std::endl;
    for( unsigned i = 0; i < newSibling->cur_offset_; i++ ) {
        std::cout << newSibling->entries.at(i) << std::endl;
    }
    std::cout << "}" << std::endl;

    // Return our newly minted sibling
    return sibling_handle;
}

template <class NT>
std::pair<tree_node_handle,tree_node_handle> adjustTreeBottomHalf(
    NT node,
    NT sibling,
    std::vector<bool> &hasReinsertedOnLevel,
    int max_branch_factor
) {

    auto *tree_ref_backup = node->treeRef;

    tree_node_handle parent_handle = node->parent;
    tree_node_handle node_handle = node->self_handle_;
    auto parent_ptr = tree_ref_backup->get_branch_node( parent_handle );

    bool didUpdateBoundingBox = parent_ptr->updateBoundingBox( node_handle, node->boundingBox() );

    // If we have a split then deal with it otherwise move up the tree
    if( sibling != nullptr ) {
        tree_node_handle sibling_handle = sibling->self_handle_;
        Rectangle bb = sibling->boundingBox();
        // AT4 [Propogate the node split upwards]
        Branch b( bb, sibling_handle );
        parent_ptr->addBranchToNode( b );

        unsigned sz = parent_ptr->cur_offset_;
        if( sz >= (unsigned) max_branch_factor ) {
            tree_node_handle parent_before_handle = node->parent;
            tree_node_handle sibling_parent_handle = parent_ptr->overflowTreatment(hasReinsertedOnLevel);

            if( sibling_parent_handle ) {
                // We split our parent, so now we have two (possible) parents
                assert( node->parent == sibling_parent_handle||
                        node->parent == parent_before_handle );
                assert( sibling->parent ==
                        sibling_parent_handle ||
                        sibling->parent ==
                        parent_before_handle );

                // Need to keep traversing up
                node_handle = parent_before_handle;
                sibling_handle = sibling_parent_handle;
                assert(node != sibling);

                return std::make_pair( node_handle, sibling_handle );
            }
        }

        node_handle = parent_ptr->self_handle_;
        sibling_handle = tree_node_handle( nullptr );

        return std::make_pair( node_handle, sibling_handle );

    }
    // AT5 [Move up to next level]
    if( didUpdateBoundingBox ) {
        node_handle = parent_ptr->self_handle_;
    } else {
        // If we didn't update our bounding box and there was no split, no reason to keep
        // going.
        return std::make_pair( tree_node_handle(nullptr),
                tree_node_handle( nullptr ) );
    }
    return std::make_pair( node_handle, tree_node_handle( nullptr ) );
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle adjustTreeSub(
    tree_node_handle node_handle,
    tree_node_handle sibling_handle,
    std::vector<bool> &hasReinsertedOnLevel,
    RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef
) {
    // AT1 [Initialize]
    for( ;; ) {
        assert( node_handle );

        if( node_handle.get_type() == LEAF_NODE ) {
            auto node = treeRef->get_leaf_node( node_handle );
            if( !node->parent ) {
                break;
            }

            pinned_node_ptr<LEAF_NODE_CLASS_TYPES>
                sibling_node( treeRef->node_allocator_.buffer_pool_, nullptr,
                        nullptr );
            assert( sibling_node == nullptr );

            if( sibling_handle ) {
                assert( sibling_handle.get_type() == LEAF_NODE );
                sibling_node = treeRef->get_leaf_node( sibling_handle );
            }

            auto ret_data = adjustTreeBottomHalf(
                node,
                sibling_node,
                hasReinsertedOnLevel,
                max_branch_factor
            );
            node_handle = ret_data.first;
            sibling_handle = ret_data.second;
        } else {
            auto node = treeRef->get_branch_node( node_handle );
            if( !node->parent ) {
                break;
            }
            pinned_node_ptr<BRANCH_NODE_CLASS_TYPES>
                sibling_node( treeRef->node_allocator_.buffer_pool_, nullptr,
                        nullptr );
            assert( sibling_node == nullptr );

            if( sibling_handle ) {
                assert( sibling_handle.get_type() == BRANCH_NODE );
                sibling_node = treeRef->get_branch_node( sibling_handle );
            }

            auto ret_data = adjustTreeBottomHalf(
                node,
                sibling_node,
                hasReinsertedOnLevel,
                max_branch_factor
            );
            node_handle = ret_data.first;
            sibling_handle = ret_data.second;
        }
        if( !node_handle ) {
            break;
        }
    }

    return sibling_handle;
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::adjustTree(
    tree_node_handle sibling_handle,
    std::vector<bool> &hasReinsertedOnLevel
) {
    return adjustTreeSub( self_handle_, sibling_handle, hasReinsertedOnLevel, treeRef );
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::adjustTree(
    tree_node_handle sibling_handle,
    std::vector<bool> &hasReinsertedOnLevel
) {
    return adjustTreeSub( self_handle_, sibling_handle, hasReinsertedOnLevel, treeRef );
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::reInsert(
    std::vector<bool> &hasReinsertedOnLevel
) {
    // 1. RI1 Compute distance between each of the points and the bounding box containing them.
    // 2. RI2 Sort the entries by DECREASING index -> ok let's define an
    // 		extra helper function that gets to do this and pass it into sort

    Point globalCenterPoint = boundingBox().centrePoint();

    assert( hasReinsertedOnLevel.at(level) );

    std::sort( entries.begin(), entries.begin() + cur_offset_,
        [&globalCenterPoint](Point &a, Point &b)
        {
            Rectangle rectA( a, Point::closest_larger_point( a ) );
            Rectangle rectB( b, Point::closest_larger_point( b ) );
            return rectA.centrePoint().distance( globalCenterPoint ) > rectB.centrePoint().distance( globalCenterPoint );
        });

    // 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
    //		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
    unsigned numNodesToReinsert = get_p_value( treeRef ) * cur_offset_;

    // 4. Insert the removed entries -> OK we can also specify a flag that is
    //		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)

    unsigned remainder = cur_offset_ - numNodesToReinsert;

    // We need to reinsert these entries
    // We pop them all off before hand so that any reorganization of the tree during this recursive
    // insert does not affect which entries get popped off
    std::vector<Point> entriesToReinsert;
    entriesToReinsert.reserve( numNodesToReinsert );

    // copy these out
    std::copy(
        entries.begin() + remainder,
        entries.begin() + cur_offset_,
        std::back_inserter(entriesToReinsert)
    );

    //adjust ending of array
    cur_offset_ = remainder; 

    // During this recursive insert (we are already in an insert, since we are reInserting), we
    // may end up here again. If we do, we should still be using the same hasReinsertedOnLevel
    // vector because it corresponds to the activities we have performed during a single
    // point/rectangle insertion (the top level one)

    // Find the root node
    tree_node_handle root_handle = self_handle_;
    for( ;; ) {

        // Get the node and check if it has a parent
        if( root_handle.get_type() == LEAF_NODE ) {
            auto root_node = treeRef->get_leaf_node( root_handle );

            // If it does not, then we've found the root
            if( !root_node->parent ) {
                break;
            }

            root_handle = root_node->parent;
        } else {
            auto root_node = treeRef->get_branch_node( root_handle );

            // If it does not, then we've found the root
            if( !root_node->parent ) {
                break;
            }

            root_handle = root_node->parent;
        }
    }

    std::cout << "Overflow treatment, need to reinsert nodes: {" <<
        std::endl;
    for( size_t i = 0; i < entriesToReinsert.size(); i++ ) {
        std::cout << entriesToReinsert.at(i) << std::endl;
    }
    std::cout << "}" << std::endl;

    for( const Point &entry : entriesToReinsert ) {
        if( root_handle.get_type() == LEAF_NODE ) {
            auto root_node = treeRef->get_leaf_node( root_handle );
            root_handle = root_node->insert( entry, hasReinsertedOnLevel );
        } else {
            auto root_node = treeRef->get_branch_node( root_handle );
            root_handle = root_node->insert( entry, hasReinsertedOnLevel );
        }
    }

    return tree_node_handle( nullptr );
}

// Overflow treatement for dealing with a node that is too big (overflow)
NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::overflowTreatment(
    std::vector<bool> &hasReinsertedOnLevel
) {
    assert( hasReinsertedOnLevel.size() > level );

    if( hasReinsertedOnLevel.at(level) ) {
        std::cout << "Overflow treatment on leaf node, splitting." <<
            std::endl;
        return splitNode();
    } else {
        hasReinsertedOnLevel.at(level) = true;
        std::cout << "Overflow treatment on leaf node, reinserting." <<
            std::endl;
        return reInsert( hasReinsertedOnLevel );
    }
}

NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::insert(
    Point nodeEntry,
    std::vector<bool> &hasReinsertedOnLevel
) {
    tree_node_allocator *allocator = get_node_allocator( treeRef );

    // Always called on root, this = root
    assert( !parent );

    // I1 [Find position for new record]
    tree_node_handle sibling_handle = tree_node_handle( nullptr );

    // I2 [Add record to leaf node]
    addPoint( nodeEntry );

    // If we exceed treeRef->maxBranchFactor we need to do something about it
    if( cur_offset_ > max_branch_factor ) {
        // We call overflow treatment to determine how our sibling node is treated if we do a
        // reInsert, sibling is nullptr. This is properly dealt with in adjustTree
        sibling_handle = overflowTreatment( hasReinsertedOnLevel );
    }

    // I3 [Propogate overflow treatment changes upward]
    sibling_handle = adjustTree( sibling_handle, hasReinsertedOnLevel );

    // I4 [Grow tree taller]
    if( sibling_handle ) {

        assert( !parent );
        auto alloc_data =
            allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                    NodeHandleType( BRANCH_NODE ) );
        auto newRoot = alloc_data.first;

        tree_node_handle root_handle = alloc_data.second;
        auto sibling = treeRef->get_leaf_node( sibling_handle );

        new (&(*(newRoot)))
            BRANCH_NODE_CLASS_TYPES( treeRef, root_handle,
                tree_node_handle( nullptr ), this->level+1 );
        
        this->parent = root_handle;

        // Make the existing root a child of newRoot
        Branch b1( boundingBox(), self_handle_ );

        newRoot->addBranchToNode( b1 );

        // Make the new sibling node a child of newRoot
        sibling->parent = root_handle;

        Branch b2( sibling->boundingBox(), sibling_handle );
        newRoot->addBranchToNode( b2 );

        // Ensure newRoot has both children
        assert(newRoot->cur_offset_ == 2);
        assert(sibling->level+1 == newRoot->level);

        // Fix the reinserted length
        hasReinsertedOnLevel.push_back(false);

        return root_handle;
    } else {
        // We might no longer be the parent.  If we hit overflowTreatment, we may have triggered
        // reInsert, which then triggered a split. That insert will have returned newRoot, but
        // because reInsert() returns nullptr, we don't know about it
        tree_node_handle root_handle = self_handle_; 

        for( ;; ) {
            if( root_handle.get_type() == LEAF_NODE ) {
                auto node_data = treeRef->get_leaf_node( root_handle );
                if( !node_data->parent ) {
                    return root_handle;
                }

                root_handle = node_data->parent;
            } else {
                auto node_data = treeRef->get_branch_node( root_handle );
                if( !node_data->parent ) {
                    return root_handle;
                }

                root_handle = node_data->parent;
            }
        }
        return root_handle;
    }
}

// To be called on a leaf
NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::condenseTree(std::vector<bool> &hasReinsertedOnLevel)
{
    // CT1 [Initialize]
    tree_node_handle node_handle = self_handle_;

    std::vector<NodeEntry> Q;

    // CT2 [Find parent entry]
    unsigned entriesSize;

    for( ;; ) {

        tree_node_handle parent_handle;
        Rectangle bb;
        if( node_handle.get_type() == LEAF_NODE ) {
            auto node = treeRef->get_leaf_node( node_handle );
            parent_handle = node->parent;
            entriesSize = node->cur_offset_;
            bb = node->boundingBox();
        } else {
            auto node = treeRef->get_branch_node( node_handle );
            parent_handle = node->parent;
            entriesSize = node->cur_offset_;
            bb = node->boundingBox();
        }

        if( !parent_handle ) {
            break;
        }

        // CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
        if( entriesSize >= min_branch_factor ) {
            auto parent = treeRef->get_branch_node( parent_handle );
            parent->updateBoundingBox( node_handle, bb );

            // CT5 [Move up one level in the tree]
            // Move up a level without deleting ourselves
            node_handle = parent->self_handle_;
        } else {
            auto parent = treeRef->get_branch_node( parent_handle );
            // Remove ourselves from our parent
            parent->removeChild( node_handle );

            if( node_handle.get_type() == LEAF_NODE ) {
                auto node = treeRef->get_leaf_node( node_handle );
                // Push these entries into Q
                std::copy(node->entries.begin(),
                        node->entries.begin() + node->cur_offset_, std::back_inserter(Q));
            } else {
                auto node = treeRef->get_leaf_node( node_handle );
                // Push these entries into Q
                std::copy(node->entries.begin(),
                        node->entries.begin() + node->cur_offset_, std::back_inserter(Q));
            }

            // FIXME: Should garbage collect node_ptr, it is dead now
            //tree_node_handle garbage = node_handle;

            node_handle = parent->self_handle_;
            // Cleanup ourselves without deleting children b/c they will be reinserted
            // GarbageCollect( node_ptr );

        }
    }


    // CT6 [Re-insert oprhaned entries]
    for( const auto &entry : Q ) {
        if( node_handle.get_type() == LEAF_NODE ) {
            auto root = treeRef->get_leaf_node( node_handle );
            assert( !root->parent );
            node_handle = root->insert(std::get<Point>(entry), hasReinsertedOnLevel);
        } else {
            auto root = treeRef->get_branch_node( node_handle );
            assert( !root->parent );
            node_handle = root->insert(entry, hasReinsertedOnLevel);
        }
    }

    return node_handle;
}

// Always called on root, this = root
NODE_TEMPLATE_PARAMS
tree_node_handle LEAF_NODE_CLASS_TYPES::remove(
    Point &givenPoint,
    std::vector<bool> hasReinsertedOnLevel
) {
    removePoint(givenPoint);

    // D3 [Propagate changes]
    tree_node_handle root_handle = condenseTree( hasReinsertedOnLevel );
    if( root_handle.get_type() == LEAF_NODE ) {
        return root_handle;
    }

    auto root = treeRef->get_branch_node( root_handle );

    // D4 [Shorten tree]
    if (root->cur_offset_ == 1 ) {
        // Slice the hasReinsertedOnLevel
        hasReinsertedOnLevel.pop_back();

        // We are removing the root to shorten the tree so we then decide to remove the root
        Branch &b = root->entries[0];

        // Get rid of the old root

        if( b.child.get_type() == LEAF_NODE ) {
            auto child = treeRef->get_leaf_node( b.child );
            child->parent = tree_node_handle( nullptr );
            // Garbage Collect Root
            // FIXME GC(root);

            return b.child;
        } else {
            auto child = treeRef->get_branch_node( b.child );
            child->parent = tree_node_handle( nullptr );
            // Garbage Collect Root
            // FIXME GC(root);
            return b.child;
        }
    }
    return root_handle;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::print() const
{

    std::string indentation(4, ' ');
    std::cout << indentation << "Node " << (void *)this << std::endl;
    std::cout << indentation << "{" << std::endl;
    std::cout << indentation << "    BoundingBox: " << boundingBox() << std::endl;
    std::cout << indentation << "    Parent: " << parent << std::endl;
    std::cout << indentation << "    Entries: " << std::endl;
    
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        std::cout << indentation << "		" << entries.at(i) <<
            std::endl;
    }
    std::cout << std::endl << indentation << "}" << std::endl;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::printTree() const
{
    // No op
}

NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::checksum() const
{
    unsigned checksum = 0;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        auto &p = entries.at(i);
        for( unsigned d = 0; d < dimensions; d++ ) {
            checksum += (unsigned) p[d];
        }
    }

    return checksum;
}

NODE_TEMPLATE_PARAMS
unsigned LEAF_NODE_CLASS_TYPES::height() const
{
    assert( parent == nullptr );
    return level+1;
}

NODE_TEMPLATE_PARAMS
void LEAF_NODE_CLASS_TYPES::stat() const
{
    std::cout << "Called stat on a single-node tree?" << std::endl;
    assert( false );
}

//// BRANCH STARTS 
NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::deleteSubtrees()
{
    for( unsigned i = 0; i < cur_offset_; i++ ) { 
        const Branch &b = entries.at(i);
        tree_node_handle child_handle = b.child;
        if( child_handle.get_type() == LEAF_NODE ) {
            auto child = treeRef->get_leaf_node( child_handle );
            child->deleteSubtrees();
        } else {
            auto child = treeRef->get_branch_node( child_handle );
            child->deleteSubtrees();
        }
    }
}

NODE_TEMPLATE_PARAMS
Rectangle BRANCH_NODE_CLASS_TYPES::boundingBox() const
{
    assert( cur_offset_ > 0 );
    Rectangle boundingBox = entries.at(0).boundingBox;

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        boundingBox.expand( entries.at(i).boundingBox );
    }

    return boundingBox;
}

NODE_TEMPLATE_PARAMS
bool BRANCH_NODE_CLASS_TYPES::updateBoundingBox(
    tree_node_handle child,
    Rectangle updatedBoundingBox
) {
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = entries.at(i);
        if( b.child == child ) {
            if( b.boundingBox != updatedBoundingBox ) {
                b.boundingBox = updatedBoundingBox;
                return true;
            }
            return false;
        }
    }

    assert(false);
    return false;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::removeChild( tree_node_handle child ) {
    unsigned i = 0;
    for( ; i < cur_offset_; i++ ) {
        if( entries.at(i).child == child ) {
            break;
        }
    }
    assert( entries.at(i).child == child );
    entries.at(i) = entries.at(cur_offset_-1);
    cur_offset_--;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::exhaustiveSearch(
    const Point &requestedPoint,
    std::vector<Point> &accumulator
) const {
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        tree_node_handle child_handle = entries.at(i).child;
        if( child_handle.get_type() == LEAF_NODE ) {
            auto child = treeRef->get_leaf_node( child_handle );
            child->exhaustiveSearch( requestedPoint, accumulator );
        } else {
            auto child = treeRef->get_branch_node( child_handle );
            child->exhaustiveSearch( requestedPoint, accumulator );
        }
    }
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::searchSub(
    const Point &requestedPoint,
    std::vector<Point> &accumulator
) {
    std::stack<tree_node_handle> context;
    context.push( self_handle_ );

    while( !context.empty() ) {
        auto cur_handle = context.top();
        context.pop();
        if( cur_handle.get_type() == LEAF_NODE ) {
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
            auto curNode = treeRef->get_leaf_node( cur_handle );
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Point &p = curNode->entries.at(i);
                if( p == requestedPoint ) {
                    accumulator.push_back( p );
                }
            }
        } else {
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
            auto curNode = treeRef->get_branch_node( cur_handle );
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Branch &b = curNode->entries.at( i );

                if( b.boundingBox.containsPoint( requestedPoint ) ) {
                    context.push( b.child );
                }
            }
        }
    }
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::searchSub(
    const Rectangle &rectangle,
    std::vector<Point> &accumulator
) {
    std::stack<tree_node_handle> context;
    context.push( self_handle_ );

    while( !context.empty() ) {
        auto cur_handle = context.top();
        context.pop();
        
        if( cur_handle.get_type() == LEAF_NODE ) {
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
            auto curNode = treeRef->get_leaf_node( cur_handle );
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Point &p = curNode->entries.at(i);

                if( rectangle.containsPoint( p ) ) {
                    accumulator.push_back( p );
                }
            }
        } else {
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
            auto curNode = treeRef->get_branch_node( cur_handle );
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Branch &b = curNode->entries.at(i);

                if( b.boundingBox.intersectsRectangle(rectangle) ) {
                    context.push( b.child );
                }
            }
        }
    }
}

NODE_TEMPLATE_PARAMS
std::vector<Point> BRANCH_NODE_CLASS_TYPES::search( const Point &requestedPoint ) {
    std::vector<Point> accumulator;

    searchSub( requestedPoint, accumulator );

#ifdef STAT
    treeRef->stats.resetSearchTracker( false );
#endif
    return accumulator;
}

NODE_TEMPLATE_PARAMS
std::vector<Point> BRANCH_NODE_CLASS_TYPES::search( const Rectangle &requestedRectangle ) {
    std::vector<Point> matchingPoints;

    searchSub( requestedRectangle, matchingPoints );

#ifdef STAT
    treeRef->stats.resetSearchTracker( true );
#endif
    return matchingPoints;
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::chooseSubtree( const NodeEntry &givenNodeEntry ) {
    // CS1: This is CAlled on the root! Just like above
    // CS2: If N is a leaf return N (same)
    // CS3: If the child pointers (bounding boxes) -> choose reactangle that needs least
    // 		overlap enlargment to fit the new Point/bounding rectangle if tie return smallest area
    // 		i.e. the rectangle that has the least overlap -> tbh I'm not sure we can just leave this
    // 	Else: 
    // 		If not child pointers (bounding boxes) -> choose reactangle that needs least
    // 		overlap enlargment to fit the new Point (same as before) if tie return smallest area (same)


    // CL1 [Initialize]
    tree_node_handle node_handle = self_handle_;

    // Always called on root, this = root
    assert( !parent );

    unsigned stoppingLevel = 0;
    bool entryIsBranch = std::holds_alternative<Branch>(givenNodeEntry);
    Rectangle givenEntryBoundingBox;
    if( entryIsBranch ) {
        const Branch &b = std::get<Branch>(givenNodeEntry);
        givenEntryBoundingBox = b.boundingBox;
        tree_node_handle child_handle = b.child;
        if( child_handle.get_type() == LEAF_NODE ) {
            auto child = treeRef->get_leaf_node( child_handle );
            stoppingLevel = child->level + 1;
        } else {
            auto child = treeRef->get_branch_node( child_handle );
            stoppingLevel = child->level + 1;
        }
    } else {
        const Point &p = std::get<Point>( givenNodeEntry );
        givenEntryBoundingBox = Rectangle( p, Point::closest_larger_point( p ) );
    }
    for( ;; ) {
        if( node_handle.get_type() == LEAF_NODE ) {
            // This is a deviation from before, but i presume if this a
            // point we want to hit leaves. Otherwise we want to go deep
            // enough to stop
            assert( std::holds_alternative<Point>(givenNodeEntry) );
            return node_handle;
        }

        auto node = treeRef->get_branch_node( node_handle );
        if( node->level == stoppingLevel ) {
            return node_handle;
        }


        unsigned descentIndex = 0;

        auto child_handle = node->entries.at(0).child;
        bool childrenAreLeaves = (child_handle.get_type() == LEAF_NODE);
        if( childrenAreLeaves ) {
            double smallestOverlapExpansion = std::numeric_limits<double>::infinity();
            double smallestExpansionArea = std::numeric_limits<double>::infinity();
            double smallestArea = std::numeric_limits<double>::infinity();

            // Choose the entry in N whose rectangle needs least overlap enlargement
            unsigned num_entries_els = node->cur_offset_;
            for( unsigned i = 0; i < num_entries_els; i++ ) {
                const Branch &b = entries.at(i);

                // Compute overlap
                double testOverlapExpansionArea =
                    computeOverlapGrowth<NodeEntry,Branch,max_branch_factor>(i, node->entries,
                            node->cur_offset_, givenEntryBoundingBox);

                // Take largest overlap
                if( smallestOverlapExpansion > testOverlapExpansionArea ) {
                    descentIndex = i;
                    smallestOverlapExpansion = testOverlapExpansionArea;
                    smallestExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
                    smallestArea = b.boundingBox.area();
                } else if( smallestOverlapExpansion == testOverlapExpansionArea ) {
                    // Use expansion area to break tie
                    double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
                    if( smallestExpansionArea > testExpansionArea ) {
                        descentIndex = i;
                        // Don't need to update smallestOverlapExpansion, its the same
                        smallestExpansionArea = testExpansionArea;
                        smallestArea = b.boundingBox.area();
                    } else if( smallestExpansionArea == testExpansionArea ) {
                        // Use area to break tie
                        double testArea = b.boundingBox.area();
                        if( smallestArea > testArea ) {
                            descentIndex = i;
                            // Don't need to update smallestOverlapExpansion, its the same
                            // Don't need to update smallestExpansionArea, its the same
                            smallestArea = testArea;
                        }
                    }
                }
            }
        // childrenAreLeaves end
        } else {
            double smallestExpansionArea = std::numeric_limits<double>::infinity();
            double smallestArea = std::numeric_limits<double>::infinity();

            // CL2 [Choose subtree]
            // Find the bounding box with least required expansion/overlap
            unsigned num_entries_els = node->cur_offset_;
            for( unsigned i = 0; i < num_entries_els; i++ ) {
                const Branch &b = entries.at(i);
                double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
                if( smallestExpansionArea > testExpansionArea ) {
                    descentIndex = i;
                    smallestExpansionArea = testExpansionArea;
                    smallestArea = b.boundingBox.area();
                } else if( smallestExpansionArea == testExpansionArea ) {
                    // Use area to break tie
                    double testArea = b.boundingBox.area();
                    if( smallestArea > testArea ) {
                        descentIndex = i;
                        // Don't need to update smallestExpansionArea
                        smallestArea = testArea;
                    }
                }
            }
        }

        // Descend
        node_handle = node->entries[descentIndex].child;
    }
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::findLeaf( const Point &givenPoint ) {
    assert( cur_offset_ > 0 );

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        const Branch &b = entries.at( i );

        if( b.boundingBox.containsPoint( givenPoint ) ) {
            tree_node_handle ret_handle(nullptr);
            tree_node_handle child_handle = b.child;
            if( child_handle.get_type() == LEAF_NODE ) {
                auto child = treeRef->get_leaf_node( child_handle );
                ret_handle = child->findLeaf( givenPoint );
            } else {
                auto child = treeRef->get_branch_node( child_handle );
                ret_handle = child->findLeaf( givenPoint );
            }

            if( ret_handle ) {
                return ret_handle;
            }

            // Nope, keep looking...
        }
    }

    return tree_node_handle( nullptr );
}

NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::chooseSplitNonLeafAxis()
{
    unsigned optimalAxisLower = 0;
    unsigned optimalAxisUpper = 0;
    double optimalMarginLower = std::numeric_limits<double>::infinity();
    double optimalMarginUpper = std::numeric_limits<double>::infinity();

    // Make entries easier to work with
    std::vector<Branch *> lowerEntries;
    lowerEntries.reserve( cur_offset_ );
    std::vector<Branch *> upperEntries;
    upperEntries.reserve( cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        lowerEntries.push_back( &(entries.at(i)) );
        upperEntries.push_back( &(entries.at(i)) );
    }

    // Consider all M-2m+2 distributions in each dimension
    for( unsigned d = 0; d < dimensions; d++ ) {
        // First sort in the current dimension sorting both the lower and upper arrays
        std::sort( lowerEntries.begin(), lowerEntries.end(), [d]( Branch
                    *a, Branch *b)
        {
            return a->boundingBox.lowerLeft[d] < b->boundingBox.lowerLeft[d];
        });
        std::sort(upperEntries.begin(), upperEntries.end(), [d](Branch
                    *a, Branch *b)
        {
            return a->boundingBox.upperRight[d] < b->boundingBox.upperRight[d];
        });

        // Setup groups
        std::vector<Branch *> groupALower(lowerEntries.begin(),
                lowerEntries.begin() + min_branch_factor );
        std::vector<Branch *> groupAUpper(upperEntries.begin(),
                upperEntries.begin() + min_branch_factor );

        std::vector<Branch *> groupBLower(lowerEntries.begin() +
                min_branch_factor, lowerEntries.end());
        std::vector<Branch *> groupBUpper(upperEntries.begin() +
                min_branch_factor, upperEntries.end());

        // Cycle through all M-2m+2 distributions
        double totalMarginLower = 0.0;
        double totalMarginUpper = 0.0;
        while( groupALower.size() <= max_branch_factor and groupBLower.size() >= min_branch_factor ) {
            // Compute the margin of groupA and groupB
            Rectangle boundingBoxALower = groupALower[0]->boundingBox;
            Rectangle boundingBoxAUpper = groupAUpper[0]->boundingBox;
            for( unsigned i = 1; i < groupALower.size(); i++ ) {
                boundingBoxALower.expand(groupALower[i]->boundingBox);
                boundingBoxAUpper.expand(groupAUpper[i]->boundingBox);
            }

            Rectangle boundingBoxBLower = groupBLower[0]->boundingBox;
            Rectangle boundingBoxBUpper = groupBUpper[0]->boundingBox;
            for( unsigned i = 1; i < groupBLower.size(); i++ ) {
                boundingBoxBLower.expand(groupBLower[i]->boundingBox);
                boundingBoxBUpper.expand(groupBUpper[i]->boundingBox);
            }

            // Add to the total margin sum
            totalMarginLower += boundingBoxALower.margin() + boundingBoxBLower.margin();
            totalMarginUpper += boundingBoxAUpper.margin() + boundingBoxBUpper.margin();

            // Add one new value to groupA and remove one from groupB to obtain next distribution
            Branch *transferPointLower = groupBLower.front();
            Branch *transferPointUpper = groupBUpper.front();
            groupBLower.erase(groupBLower.begin());
            groupBUpper.erase(groupBUpper.begin());
            groupALower.push_back(transferPointLower);
            groupAUpper.push_back(transferPointUpper);
        }

        if( totalMarginLower < optimalMarginLower ) {
            optimalMarginLower = totalMarginLower;
            optimalAxisLower = d;
        }

        if( totalMarginUpper < optimalMarginUpper ) {
            optimalMarginUpper = totalMarginUpper;
            optimalAxisUpper = d;
        }
    }

    bool sortLower = optimalMarginUpper > optimalMarginLower ? true : false;
    unsigned optimalAxis = sortLower ? optimalAxisLower : optimalAxisUpper;

    // Sort to match the optimal axis
    if( sortLower ) {

        std::sort( entries.begin(), entries.begin() + cur_offset_,
                [optimalAxis](Branch &a, Branch &b)
        {
            return a.boundingBox.lowerLeft[optimalAxis] < 
                    b.boundingBox.lowerLeft[optimalAxis];
        });
    } else {
        std::sort(entries.begin(), entries.begin() + cur_offset_,
                [optimalAxis](Branch &a, Branch &b)
        {
            return a.boundingBox.upperRight[optimalAxis] <
                    b.boundingBox.upperRight[optimalAxis];
        });
    }

    return optimalAxis;
}


// CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all
//  margin values for the different distributions. This can be stored in a array of variable
//  that we keep in a loop -> and the just compare to the others?
// 	We can first call a helper function that returns an array of all possible distributions for it?
// CSA2: Return the Axis that has the minimum total sum of all the distributions
NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::chooseSplitAxis() {
    return chooseSplitNonLeafAxis();
}

// CSI1: Given the chosen split index
// 	group all the entries into multiple groups and choose the one that has the least
// 	overlap value; resolve ties with the minimum area
// 	returns tuple of best distribution group indices
NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::chooseSplitIndex(
    unsigned axis
) {
    // We assume this is called after we have sorted this->data according to axis.

    const auto groupABegin = entries.begin();
    const auto groupAEnd = entries.begin() + min_branch_factor;
    const auto groupBBegin = entries.begin() + min_branch_factor;
    const auto groupBEnd = entries.begin() + cur_offset_;

    std::vector<Branch> groupA(groupABegin, groupAEnd);
    std::vector<Branch> groupB(groupBBegin, groupBEnd);
    unsigned splitIndex = cur_offset_  / 2;

    // Find the best size out of all the distributions
    double minOverlap = std::numeric_limits<double>::infinity();
    double minArea = std::numeric_limits<double>::infinity();

    // Tracking what the current "cut" mark is
    unsigned currentSplitPoint = min_branch_factor;

    // Try each of the M-2m + 2 groups
    while( groupA.size() <= max_branch_factor and groupB.size() >= min_branch_factor ) {
        // Compute the margin of groupA and groupB
        Rectangle boundingBoxA = groupA[0].boundingBox;
        for( unsigned i = 1; i < groupA.size(); i++ ) {
            boundingBoxA.expand( groupA[i].boundingBox );
        }

        Rectangle boundingBoxB = groupB[0].boundingBox;
        for( unsigned i = 1; i < groupB.size(); i++ ) {
            boundingBoxB.expand( groupB[i].boundingBox );
        }

        // Compute intersection area to determine best grouping of data points
        double evalDistOverlap = boundingBoxA.computeIntersectionArea(boundingBoxB);

        if( evalDistOverlap < minOverlap ) {
            // We save this current distribution of indices to return
            minOverlap = evalDistOverlap;
            splitIndex = currentSplitPoint;

            // Set this if we haven't already
            if( minArea == std::numeric_limits<double>::infinity() ) {
                minArea = boundingBoxA.area() + boundingBoxB.area();
            }
        } else if( evalDistOverlap == minOverlap ) {
            // If overlap is equal, we use the distribution that creates the smallest areas
            double evalMinArea = boundingBoxA.area() + boundingBoxB.area();

            if( evalMinArea < minArea ) {
                // Save this current distribution of indices to return
                minArea = evalMinArea;
                splitIndex = currentSplitPoint;
            }
        }
        
        // Add one new value to groupA and remove one from groupB to obtain next distribution
        Branch transferPoint = groupB.front();
        groupB.erase( groupB.begin() );
        groupA.push_back( transferPoint );

        // Push the split point forward.
        currentSplitPoint++;
    }

    return splitIndex;
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::splitNode()
{
    // S1: Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
    // S2: Invoke chooseSplitIndex given the axis to determine the best distribution along this axis
    // S3: Distribute the entries among these two groups

    // Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
    // For now we will save the axis as a int -> since this allows for room for growth in the future
    // Call ChooseSplitIndex to create optimal splitting of data array
    unsigned splitAxis = chooseSplitAxis();
    unsigned splitIndex = chooseSplitIndex( splitAxis );

    std::cout << "Split Branch Node: {" <<  std::endl;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        std::cout << entries.at(i).boundingBox << std::endl;
    }
    std::cout << "}" << std::endl;
    std::cout << "Split on axis" << splitAxis << ", index: " <<
        splitIndex << std::endl;

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    auto alloc_data =
        allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                NodeHandleType( BRANCH_NODE ) );

    auto newSibling = alloc_data.first;
    tree_node_handle sibling_handle = alloc_data.second;

    new (&(*(newSibling))) BRANCH_NODE_CLASS_TYPES( treeRef, sibling_handle, parent,
            level );

    newSibling->parent = parent;
    newSibling->level = level;
    newSibling->treeRef = treeRef; 
    newSibling->self_handle_ = sibling_handle;

#if !defined( NDEDBUG )
    if( parent ) {
        auto parent_ptr = treeRef->get_branch_node( parent );
        assert( level + 1 == parent_ptr->level );

    }
#endif

    // Copy everything to the right of the splitPoint (inclusive) to the new sibling
    std::copy( entries.begin() + splitIndex, entries.begin() + cur_offset_,
            newSibling->entries.begin() );

    newSibling->cur_offset_ = cur_offset_ - splitIndex;

    for( unsigned i = 0; i < newSibling->cur_offset_; i++ ) {
        // Update parents
        Branch &b = newSibling->entries.at(i);
        if( b.child.get_type() == LEAF_NODE ) {
            auto child = treeRef->get_leaf_node( b.child );
            child->parent = sibling_handle;
            assert(level == child->level + 1);
            assert(newSibling->level == child->level + 1);
        } else {
            auto child = treeRef->get_branch_node( b.child );
            child->parent = sibling_handle;
            assert(level == child->level + 1);
            assert(newSibling->level == child->level + 1);
        }
    }

    // Chop our node's data down
    cur_offset_ = splitIndex;

    assert( cur_offset_ > 0 );
    assert( newSibling->cur_offset_ > 0 );

    std::cout << "New Node1: {" <<  std::endl;
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        std::cout << entries.at(i).boundingBox << std::endl;
    }
    std::cout << "}" << std::endl;

    std::cout << "New Node2: {" <<  std::endl;
    for( unsigned i = 0; i < newSibling->cur_offset_; i++ ) {
        std::cout << newSibling->entries.at(i).boundingBox << std::endl;
    }
    std::cout << "}" << std::endl;


    // Return our newly minted sibling
    return sibling_handle;
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::reInsert(
    std::vector<bool> &hasReinsertedOnLevel
) {
    // 1. RI1 Compute distance between each of the points and the bounding box containing them.
    // 2. RI2 Sort the entries by DECREASING index -> ok let's define an
    // 		extra helper function that gets to do this and pass it into sort

    Point globalCenterPoint = boundingBox().centrePoint();

    assert(hasReinsertedOnLevel.at(level));

    std::sort( entries.begin(), entries.begin() + cur_offset_,
        [&globalCenterPoint]( Branch &a, Branch &b ) {
            Rectangle rectA = a.boundingBox;
            Rectangle rectB = b.boundingBox;
            return rectA.centrePoint().distance( globalCenterPoint ) > rectB.centrePoint().distance(globalCenterPoint);
        });

    // 3. RI3 Remove the first p entries from N and adjust the bounding box -> OK so we need to adjust the data model
    //		to include a specified "p" value -> this should be unique to the node -> so it's a node variable
    unsigned numNodesToReinsert = get_p_value( treeRef ) * cur_offset_;

    // 4. Insert the removed entries -> OK we can also specify a flag that is
    //		if you want to reinsert starting with largest values (i.e. start at index 0) or closest values (Start at index p)


    unsigned remainder = cur_offset_ - numNodesToReinsert;

    // We need to reinsert these entries
    // We pop them all off before hand so that any reorganization of the tree during this recursive
    // insert does not affect which entries get popped off
    std::vector<Branch> entriesToReinsert;
    entriesToReinsert.reserve( numNodesToReinsert );

    // copy these out
    std::copy( entries.begin() + remainder, entries.begin() + cur_offset_, std::back_inserter(entriesToReinsert) );

    //adjust ending of array
    cur_offset_ = remainder; 

    // During this recursive insert (we are already in an insert, since we are reInserting), we
    // may end up here again. If we do, we should still be using the same hasReinsertedOnLevel
    // vector because it corresponds to the activities we have performed during a single
    // point/rectangle insertion (the top level one)

    // Find the root node
    tree_node_handle root_handle = self_handle_;
    for( ;; ) {

        // Get the node and check if it has a parent
        // Since I am branch, whatever root_handle is must also be a
        // branch
        auto root_node = treeRef->get_branch_node( root_handle );

        // If it does not, then we've found the root
        if( !root_node->parent ) {
            break;
        }

        root_handle = root_node->parent;
    }

    auto root_node = treeRef->get_branch_node( root_handle );

    for( const Branch &entry : entriesToReinsert ) {
        assert( !root_node->parent );

        root_handle = root_node->insert(entry, hasReinsertedOnLevel);
        root_node = treeRef->get_branch_node( root_handle );
    }

    return tree_node_handle( nullptr );
}

// Overflow treatement for dealing with a node that is too big (overflow)
NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::overflowTreatment(
    std::vector<bool> &hasReinsertedOnLevel
) {
    assert( hasReinsertedOnLevel.size() > level );

    if( hasReinsertedOnLevel.at(level) ) {
        std::cout << "Overflow treatment on branch node, splitting." <<
            std::endl;
        return splitNode();
    } else {
        hasReinsertedOnLevel.at(level) = true;
        std::cout << "Overflow treatment on branch node, reinserting." <<
            std::endl;
        return reInsert( hasReinsertedOnLevel );
    }
}

NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::insert(
    NodeEntry nodeEntry,
    std::vector<bool> &hasReinsertedOnLevel
) {
    tree_node_allocator *allocator = get_node_allocator( treeRef );
    // Always called on root, this = root
    assert( !parent );

    // I1 [Find position for new record]
    tree_node_handle insertion_point_handle = chooseSubtree( nodeEntry );

    tree_node_handle sibling_handle = tree_node_handle( nullptr );

    // I2 [Add record to leaf node]
    bool givenIsLeaf = std::holds_alternative<Point>(nodeEntry);
    if( givenIsLeaf ) {
        auto insertion_point = treeRef->get_leaf_node( insertion_point_handle );
        insertion_point->addPoint( std::get<Point>( nodeEntry ) );

        std::cout << "Inserted Point: " << std::get<Point>( nodeEntry ) <<
            std::endl;
        std::cout << "Insertion point now has points: { " << std::endl;
        for( size_t i = 0; i < insertion_point->cur_offset_; i++ ) {
            std::cout << insertion_point->entries.at(i) << std::endl;
        }
        std::cout << "}" << std::endl;

        unsigned num_els = insertion_point->cur_offset_;

        // If we exceed treeRef->maxBranchFactor we need to do something about it
        if( num_els > max_branch_factor ) {
            // We call overflow treatment to determine how our sibling node is treated if we do a
            // reInsert, sibling is nullptr. This is properly dealt with in adjustTree
            sibling_handle = insertion_point->overflowTreatment(hasReinsertedOnLevel);
        }

        // I3 [Propogate overflow treatment changes upward]
        sibling_handle = insertion_point->adjustTree(
                sibling_handle, hasReinsertedOnLevel );

    } else {
        auto insertion_point = treeRef->get_branch_node( insertion_point_handle );
        Branch &b = std::get<Branch>( nodeEntry );
        insertion_point->addBranchToNode( b );
        std::cout << "Inserted branch with bounding box: " <<
            b.boundingBox << std::endl;
        std::cout << "Insertion point now has boundingBoxes: {" <<
            std::endl;
        for( size_t i = 0; i < insertion_point->cur_offset_; i++ ) {
            std::cout << insertion_point->entries.at(i).boundingBox <<
                std::endl;
        }
        std::cout << "}" << std::endl;
        if( b.child.get_type() == LEAF_NODE ) { 
            auto child = treeRef->get_leaf_node( b.child );
            assert( insertion_point->level == child->level + 1 );
            child->parent = insertion_point_handle;
        } else {
            auto child = treeRef->get_branch_node( b.child );
            assert( insertion_point->level == child->level + 1 );
            child->parent = insertion_point_handle;
        }

        unsigned num_els = insertion_point->cur_offset_;

        // If we exceed treeRef->maxBranchFactor we need to do something about it
        if( num_els > max_branch_factor ) {
            // We call overflow treatment to determine how our sibling node is treated if we do a
            // reInsert, sibling is nullptr. This is properly dealt with in adjustTree
            sibling_handle = insertion_point->overflowTreatment(hasReinsertedOnLevel);
        }

        // I3 [Propogate overflow treatment changes upward]
        sibling_handle = insertion_point->adjustTree(
                sibling_handle, hasReinsertedOnLevel );


    }

    // I4 [Grow tree taller]
    if( sibling_handle ) {

        assert( !parent );
        auto alloc_data =
            allocator->create_new_tree_node<BRANCH_NODE_CLASS_TYPES>(
                    NodeHandleType( BRANCH_NODE ) );
        auto  newRoot = alloc_data.first;
        tree_node_handle root_handle = alloc_data.second;

        auto  sibling = treeRef->get_branch_node( sibling_handle );

        new (&(*(newRoot)))
            BRANCH_NODE_CLASS_TYPES( treeRef, root_handle, tree_node_handle( nullptr ), this->level+1 );
        
        this->parent = root_handle;

        // Make the existing root a child of newRoot
        Branch b1( boundingBox(), self_handle_ );
        newRoot->addBranchToNode( b1 );

        // Make the new sibling node a child of newRoot
        sibling->parent = root_handle;
        Branch b2( sibling->boundingBox(), sibling_handle );
        newRoot->addBranchToNode( b2 );

        // Ensure newRoot has both children
        assert(newRoot->cur_offset_ == 2);
        assert(sibling->level+1 == newRoot->level);

        // Fix the reinserted length
        hasReinsertedOnLevel.push_back(false);

        return root_handle;
    } else {

        // We might no longer be the parent.  If we hit overflowTreatment, we may have triggered
        // reInsert, which then triggered a split. That insert will have returned newRoot, but
        // because reInsert() returns nullptr, we don't know about it
        tree_node_handle root_handle = self_handle_; 

        for( ;; ) {
            auto node_data = treeRef->get_branch_node( root_handle );
            if( !node_data->parent ) {
                return root_handle;
            }

            root_handle = node_data->parent;
        }
        return root_handle;
    }
}


// Always called on root, this = root
NODE_TEMPLATE_PARAMS
tree_node_handle BRANCH_NODE_CLASS_TYPES::remove(
    Point &givenPoint,
    std::vector<bool> hasReinsertedOnLevel
) {
    assert( !parent );

    // D1 [Find node containing record]
    tree_node_handle leaf_ptr = findLeaf(givenPoint);
    if( !leaf_ptr ) {
        return leaf_ptr; /*nullptr*/ 
    }

    auto leaf = treeRef->get_leaf_node( leaf_ptr );
    
    // D2 [Delete record]
    leaf->removePoint( givenPoint );

    // D3 [Propagate changes]
    auto root_handle = leaf->condenseTree( hasReinsertedOnLevel );
    if( root_handle.get_type() == BRANCH_NODE ) {
        auto root = treeRef->get_branch_node( root_handle );
        if( root->cur_offset_ == 1 ) {
            // Slice the hasReinsertedOnLevel
            hasReinsertedOnLevel.pop_back();

            // We are removing the root to shorten the tree so we then decide to remove the root
            Branch &b = root->entries[0];

            // Get rid of the old root
            if( b.child.get_type() == LEAF_NODE ) {
                auto child = treeRef->get_leaf_node( b.child );
                child->parent = tree_node_handle( nullptr );
            } else {
                auto child = treeRef->get_branch_node( b.child );
                child->parent = tree_node_handle( nullptr );
            }

            // Garbage Collect Root
            // FIXME GC(root);

            return b.child;
        }
    }
    return root_handle;

}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::print() const
{
    std::string indentation(4, ' ');
    std::cout << indentation << "Node " << (void *)this << std::endl;
    std::cout << indentation << "{" << std::endl;
    std::cout << indentation << "    BoundingBox: " << boundingBox() << std::endl;
    std::cout << indentation << "    Parent: " << parent << std::endl;
    std::cout << indentation << "    Entries: " << std::endl;
    
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        const Branch &b = entries.at(i);
        std::cout << indentation << "		" << b.boundingBox << ", ptr: " << b.child << std::endl;
    }
    std::cout << std::endl << indentation << "}" << std::endl;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::printTree() const
{
    // Print this node first
    struct Printer
    {
        void operator()(
            RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef,
            tree_node_handle node_handle
        ) {
            if( node_handle.get_type() == LEAF_NODE ) {
                auto node = treeRef->get_leaf_node( node_handle );
                node->print();
            } else { 
                auto node = treeRef->get_branch_node( node_handle );
                node->print();
            }
        }
    };

    Printer p;
    treeWalker<min_branch_factor,max_branch_factor>( treeRef, self_handle_, p );
}

NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::checksum() const
{
    struct ChecksumFunctor
    {
        unsigned checksum;

        ChecksumFunctor() {
            checksum = 0;
        }

        void operator()(
                RStarTreeDisk<min_branch_factor,max_branch_factor>
                *treeRef, tree_node_handle node_handle ) {
            if( node_handle.get_type() == LEAF_NODE ) {
                auto node = treeRef->get_leaf_node( node_handle );
                for( unsigned i = 0; i < node->cur_offset_; i++ ) {
                    const Point &p = node->entries.at(i);
                    for( unsigned d = 0; d < dimensions; d++ ) {
                        checksum += (unsigned)p[d];
                    }
                }
            } 
        }
    };

    ChecksumFunctor cf;
    treeWalker<min_branch_factor,max_branch_factor>( treeRef, self_handle_, cf );

    return cf.checksum;
}

NODE_TEMPLATE_PARAMS
unsigned BRANCH_NODE_CLASS_TYPES::height() const
{
    assert( parent == nullptr );
    return level+1;
}

NODE_TEMPLATE_PARAMS
void BRANCH_NODE_CLASS_TYPES::stat() const
{
#ifdef STAT
    struct StatWalker
    {
        size_t memoryFootprint;
        unsigned long totalNodes;
        unsigned long singularBranches;
        unsigned long totalLeaves;
        double coverage;
        double overlap;
        std::vector<unsigned long> histogramFanout;

        StatWalker()
        {
            memoryFootprint = 0;
            totalNodes = 0;
            singularBranches = 0;
            totalLeaves = 0;
            coverage = 0.0;
            overlap = 0.0;
            histogramFanout.resize(max_branch_factor, 0);
        }

        void operator()(
                RStarTreeDisk<min_branch_factor,max_branch_factor>
                * treeRef, tree_node_handle node_handle ) {

            totalNodes++;
            if( node_handle.get_type() == LEAF_NODE ) {
                auto node = treeRef->get_leaf_node( node_handle );
                totalLeaves++;
                memoryFootprint += sizeof(LEAF_NODE_CLASS_TYPES);

                if( node->cur_offset_ >= histogramFanout.size() ) {
                    //Avoid reallocing
                    histogramFanout.resize(2*node->cur_offset_, 0);
                }
                histogramFanout[node->cur_offset_]++;

            } else {
                auto node = treeRef->get_branch_node( node_handle );
                // Compute the overlap and coverage of our children
                for( unsigned i = 0; i < node->cur_offset_; i++ ) {
                    coverage += node->entries[i].boundingBox.area();

                    for( unsigned j = 0; j < node->cur_offset_; j++ ) {
                        if (i != j) {
                            overlap +=
                                node->entries[i].boundingBox.computeIntersectionArea(node->entries[j].boundingBox);
                        }
                    }
                }

                memoryFootprint += sizeof(BRANCH_NODE_CLASS_TYPES);

                if( node->cur_offset_ >= histogramFanout.size() ) {
                    //Avoid reallocing
                    histogramFanout.resize(2*node->cur_offset_, 0);
                }
                histogramFanout[node->cur_offset_]++;
            }
        }
    };

    StatWalker sw;
    treeWalker( treeRef, treeRef->root, sw );

    // Print out what we have found
    STATMEM(sw.memoryFootprint);
    STATHEIGHT(height());
    STATSIZE(sw.totalNodes);
    STATLEAF(sw.totalLeaves);
    STATBRANCH(sw.totalNodes - sw.totalLeaves);
    STATCOVER(sw.coverage);
    STATOVERLAP(sw.overlap);
    STATAVGCOVER(sw.coverage / sw.totalNodes);
    STATAVGOVERLAP(sw.overlap / sw.totalNodes);
    STATFANHIST();
    for( unsigned i = 0; i < sw.histogramFanout.size(); i++) {
        if( sw.histogramFanout[i] > 0 ) {
            STATHIST( i, sw.histogramFanout[i] );
        }
    }
    std::cout << treeRef->stats;

    STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
    (void) 0;
#endif
}

#undef NODE_TEMPLATE_PARAMS
#undef LEAF_NODE_CLASS_TYPES
#undef BRANCH_NODE_CLASS_TYPES

