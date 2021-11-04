
template <int min_branch_factor, int max_branch_factor, typename functor>
void treeWalker( RStarTreeDisk<min_branch_factor,max_branch_factor> *treeRef, tree_node_handle root, functor &f ) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    using BranchType = typename NodeType::Branch;
    std::stack<tree_node_handle> context;
    context.push(root);
    tree_node_handle currentContext;

    while (!context.empty())
    {
        currentContext = context.top();
        context.pop();

        pinned_node_ptr<NodeType> currentNode = treeRef->get_node( currentContext );

        
        // Apply the general function to this node
        f( currentNode );

        // Recurse through the rest of the tree
        bool isLeaf = currentNode->isLeafNode();
        if( !isLeaf ) {
            for( unsigned i = 0; i < currentNode->cur_offset_; i++ ) {
                context.push(
                        std::get<BranchType>(currentNode->entries.at(i)).child );
            }
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor,
     max_branch_factor>::Branch::operator==(const
             Node<min_branch_factor,max_branch_factor>::Branch &o) const
{
    return child == o.child && boundingBox == o.boundingBox;
}

template<int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::deleteSubtrees()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // FIXME : I don't think this actually destroys anything.
    if( cur_offset_ == 0 || !std::holds_alternative<Branch>(entries[0]))
    {
        return;
    }

    for( unsigned i = 0; i < cur_offset_; i++ ) { 

        const Branch &b = std::get<Branch>( entries.at(i) );
        tree_node_handle child_handle = b.child;
        tree_node_allocator *allocator  =
            get_node_allocator( treeRef );
        pinned_node_ptr<NodeType> child = treeRef->get_node( child_handle );
        child->deleteSubtrees();
    }
}

template <int min_branch_factor, int max_branch_factor>
Rectangle Node<min_branch_factor, max_branch_factor>::boundingBox() const
{
    assert( cur_offset_ > 0 );
    Rectangle boundingBox( boxFromNodeEntry<min_branch_factor,max_branch_factor>( entries[0] ) );

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        boundingBox.expand(
                boxFromNodeEntry<min_branch_factor,max_branch_factor>(
                    entries.at(i) ) );
    }

    return boundingBox;
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor, max_branch_factor>::updateBoundingBox(tree_node_handle child, Rectangle updatedBoundingBox)
{
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );
        if (b.child == child)
        {
            if (b.boundingBox != updatedBoundingBox)
            {
                b.boundingBox = updatedBoundingBox;
                return true;
            }
            return false;
        }
    }

#ifndef NDEBUG
    print();
    assert(false);
#endif
    return false;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeChild(tree_node_handle child)
{
    using BranchType = typename
        Node<min_branch_factor,max_branch_factor>::Branch;

    auto iter = std::remove_if( entries.begin(),
       entries.begin() + cur_offset_, [&child]( NodeEntry &entry ) {
        return std::get<BranchType>( entry ).child == child; });
    cur_offset_ = (iter - entries.begin());
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeData(const Point &givenPoint)
{
    auto iter = std::remove_if( entries.begin(),
            entries.begin() + cur_offset_, [&givenPoint]( NodeEntry &entry
                ) { return std::get<Point>( entry) == givenPoint; }
            );
    cur_offset_ = (iter - entries.begin());
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::exhaustiveSearch(const Point &requestedPoint, std::vector<Point> &accumulator) const
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // Am I a leaf?
    bool isLeaf = isLeafNode();
    if( isLeaf ) {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            const Point &p = std::get<Point>( entries.at(i) );

            if( p == requestedPoint ) {
                accumulator.push_back( p );
            }
        }
    }
    else
    {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            tree_node_handle child_handle = std::get<Branch>(
                    entries.at(i) ).child;
            pinned_node_ptr<NodeType> child = treeRef->get_node( child_handle );
            child->exhaustiveSearch( requestedPoint, accumulator );
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::searchSub(const Point &requestedPoint, std::vector<Point> &accumulator)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    // FIXME: Could probably improve performance here with a move
    // operator on pinned_node_ptr
    std::stack<pinned_node_ptr<NodeType>> context;

    pinned_node_ptr<NodeType> self_node = treeRef->get_node( self_handle_ );
    context.push( self_node );

    while (!context.empty())
    {

        pinned_node_ptr<NodeType> curNode = context.top();
        context.pop();

        // Am I a leaf?
        bool isLeaf = curNode->isLeafNode();
        if( isLeaf )
        {
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Point &p = std::get<Point>( curNode->entries.at(i) );

                if( p == requestedPoint ) {
                    accumulator.push_back( p );
                }
            }
        }
        else
        {
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Branch &b = std::get<Branch>( curNode->entries.at(
                            i ) );


                if( b.boundingBox.containsPoint( requestedPoint ) ) {
                    tree_node_handle child_handle = b.child;
                    pinned_node_ptr<NodeType> child = treeRef->get_node( child_handle );
                    context.push( child );
                }
            }
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::searchSub(const Rectangle &rectangle, std::vector<Point> &accumulator)
{
    // FIXME: Could probably improve performance here with a
    // pinned_node_ptr move method

    using NodeType = Node<min_branch_factor,max_branch_factor>;

    pinned_node_ptr<NodeType> self_node = treeRef->get_node( self_handle_ );
    std::stack<pinned_node_ptr<NodeType>> context;
    context.push(self_node);

    while (!context.empty())
    {
        pinned_node_ptr<NodeType> curNode = context.top();
        context.pop();

        bool isLeaf = curNode->isLeafNode();
        if (isLeaf)
        {
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Point &p = std::get<Point>( curNode->entries.at(i) );

                if( rectangle.containsPoint( p ) )
                {
                    accumulator.push_back( p );
                }
            }
        }
        else
        {
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
            for( unsigned i = 0; i < curNode->cur_offset_; i++ ) {
                const Branch &b = std::get<Branch>(
                        curNode->entries.at(i) );

                if (b.boundingBox.intersectsRectangle(rectangle))
                {

                    tree_node_handle child_handle = b.child;
                    pinned_node_ptr<NodeType> child = treeRef->get_node( child_handle );
                    context.push( child );
                }
            }
        }
    }
}


template <int min_branch_factor, int max_branch_factor>
std::vector<Point> Node<min_branch_factor, max_branch_factor>::search(const Point &requestedPoint)
{
    std::vector<Point> accumulator;

    searchSub(requestedPoint, accumulator);

#ifdef STAT
    treeRef->stats.resetSearchTracker( false );
#endif
    return accumulator;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> Node<min_branch_factor, max_branch_factor>::search(const Rectangle &requestedRectangle)
{
    std::vector<Point> matchingPoints;

    searchSub(requestedRectangle, matchingPoints);

#ifdef STAT
    treeRef->stats.resetSearchTracker( true );
#endif
    return matchingPoints;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::chooseSubtree(const NodeEntry &givenNodeEntry)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
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
    if (entryIsBranch)
    {
        const Branch &b = std::get<Branch>(givenNodeEntry);
        tree_node_handle child_handle = b.child;
        pinned_node_ptr<Node> child = treeRef->get_node( child_handle );
        stoppingLevel = child->level + 1;
    }
    Rectangle givenEntryBoundingBox = boxFromNodeEntry<min_branch_factor,max_branch_factor>(givenNodeEntry);

    for (;;)
    {
        pinned_node_ptr<NodeType> node = treeRef->get_node( node_handle );

        if (node->level == stoppingLevel)
        {
            return node_handle;
        }

        assert(!node->isLeafNode());

        // Our children point to leaves
#ifndef NDEBUG
        const Branch &firstBranch = std::get<Branch>(node->entries[0]);
        {
            pinned_node_ptr<NodeType> child = treeRef->get_node( firstBranch.child );
            assert( child->cur_offset_ > 0 );
        }
#endif

        unsigned descentIndex = 0;
        
        pinned_node_ptr<NodeType> child = treeRef->get_node(
                        std::get<Branch>(node->entries[0]).child );

        bool childrenAreLeaves =
            !std::holds_alternative<Branch>(child->entries[0]);
        if (childrenAreLeaves)
        {
            double smallestOverlapExpansion = std::numeric_limits<double>::infinity();
            double smallestExpansionArea = std::numeric_limits<double>::infinity();
            double smallestArea = std::numeric_limits<double>::infinity();

            // Choose the entry in N whose rectangle needs least overlap enlargement
            unsigned num_entries_els = node->cur_offset_;
            for (unsigned i = 0; i < num_entries_els; ++i)
            {
                const NodeEntry &entry = node->entries[i];
                const Branch &b = std::get<Branch>(entry);

                // Compute overlap
                double testOverlapExpansionArea =
                    computeOverlapGrowth<NodeType::NodeEntry,NodeType::Branch,max_branch_factor>(i, node->entries,
                            node->cur_offset_, givenEntryBoundingBox);

                // Take largest overlap
                if (smallestOverlapExpansion > testOverlapExpansionArea)
                {
                    descentIndex = i;
                    smallestOverlapExpansion = testOverlapExpansionArea;
                    smallestExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
                    smallestArea = b.boundingBox.area();
                } 
                else if (smallestOverlapExpansion == testOverlapExpansionArea)
                {
                    // Use expansion area to break tie
                    double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
                    if (smallestExpansionArea > testExpansionArea)
                    {
                        descentIndex = i;
                        // Don't need to update smallestOverlapExpansion, its the same
                        smallestExpansionArea = testExpansionArea;
                        smallestArea = b.boundingBox.area();
                    }
                    else if (smallestExpansionArea == testExpansionArea)
                    {
                        // Use area to break tie
                        double testArea = b.boundingBox.area();
                        if (smallestArea > testArea)
                        {
                            descentIndex = i;
                            // Don't need to update smallestOverlapExpansion, its the same
                            // Don't need to update smallestExpansionArea, its the same
                            smallestArea = testArea;
                        }
                    }
                }
            }
        }
        else
        {
            double smallestExpansionArea = std::numeric_limits<double>::infinity();
            double smallestArea = std::numeric_limits<double>::infinity();

            // CL2 [Choose subtree]
            // Find the bounding box with least required expansion/overlap
            unsigned num_entries_els = node->cur_offset_;
            for (unsigned i = 0; i < num_entries_els; ++i)
            {
                const NodeEntry &entry = node->entries[i];
                const Branch &b = std::get<Branch>(entry);

                double testExpansionArea = b.boundingBox.computeExpansionArea(givenEntryBoundingBox);
                if (smallestExpansionArea > testExpansionArea)
                {
                    descentIndex = i;
                    smallestExpansionArea = testExpansionArea;
                    smallestArea = b.boundingBox.area();
                }
                else if (smallestExpansionArea == testExpansionArea)
                {
                    // Use area to break tie
                    double testArea = b.boundingBox.area();
                    if (smallestArea > testArea)
                    {
                        descentIndex = i;
                        // Don't need to update smallestExpansionArea
                        smallestArea = testArea;
                    }
                }
            }
        }

        // Descend
        node_handle = std::get<Branch>(node->entries[descentIndex]).child;
    }
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::findLeaf(const Point &givenPoint)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    assert( cur_offset_ > 0 );

    // Am I a leaf?
    bool isLeaf = isLeafNode();
    if( isLeaf ) {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            if( std::get<Point>( entries.at(i) ) == givenPoint ) {
                return self_handle_;
            }
        }

        return tree_node_handle();
    }

    for( unsigned i = 0; i < cur_offset_; i++ ) {
        const Branch &b = std::get<Branch>( entries.at( i ) );

        if (b.boundingBox.containsPoint(givenPoint))
        {
            tree_node_handle child_handle = b.child;
            pinned_node_ptr<NodeType> child = treeRef->get_node( child_handle );
            tree_node_handle ret_handle = child->findLeaf( givenPoint );

            if( ret_handle ) {
                return ret_handle;
            }

            // Nope, keep looking...
        }
    }

    return tree_node_handle( nullptr );
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::chooseSplitLeafAxis()
{
    unsigned optimalAxis = 0;
    double optimalMargin = std::numeric_limits<double>::infinity();

    // Make the entries easier to work with
    std::vector<NodeEntry *> entriesCopy;
    entriesCopy.reserve( cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        entriesCopy.push_back( &(entries.at(i)) );
    }

    // Consider all M-2m+2 distributions in each dimension
    for (unsigned d = 0; d < dimensions; d++)
    {
        // First sort in the current dimension
        std::sort(entriesCopy.begin(), entriesCopy.end(), [d](NodeEntry *a, NodeEntry *b)
        {
            return std::get<Point>(*a)[d] < std::get<Point>(*b)[d];
        });

        // Setup groups
        std::vector<NodeEntry *> groupA(entriesCopy.begin(),
                entriesCopy.begin() + min_branch_factor );
        std::vector<NodeEntry *> groupB(entriesCopy.begin() +
                min_branch_factor, entriesCopy.end());

        // Cycle through all M-2m+2 distributions
        double totalMargin = 0.0;
        for (;groupA.size() <= max_branch_factor && groupB.size() >=
                min_branch_factor;)
        {
            // Compute the margin of groupA and groupB
            Rectangle boundingBoxA = boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupA[0]);
            for (unsigned i = 1; i < groupA.size(); ++i)
            {
                boundingBoxA.expand(std::get<Point>(*groupA[i]));
            }

            Rectangle boundingBoxB = boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupB[0]);
            for (unsigned i = 1; i < groupB.size(); ++i)
            {
                boundingBoxB.expand(std::get<Point>(*groupB[i]));
            }

            // Add to the total margin sum
            totalMargin += boundingBoxA.margin() + boundingBoxB.margin();

            // Add one new value to groupA and remove one from groupB to obtain next distribution
            NodeEntry *transferPoint = groupB.front();
            groupB.erase(groupB.begin());
            groupA.push_back(transferPoint);
        }

        if (totalMargin < optimalMargin)
        {
            optimalMargin = totalMargin;
            optimalAxis = d;
        }
    }

    // Sort along our best axis
    std::sort(entries.begin(), entries.begin()+cur_offset_, [optimalAxis](NodeEntry &a, NodeEntry &b)
    {
        return std::get<Point>(a)[optimalAxis] < std::get<Point>(b)[optimalAxis];
    });

    return optimalAxis;
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::chooseSplitNonLeafAxis()
{
    unsigned optimalAxisLower = 0;
    unsigned optimalAxisUpper = 0;
    double optimalMarginLower = std::numeric_limits<double>::infinity();
    double optimalMarginUpper = std::numeric_limits<double>::infinity();

    // Make entries easier to work with
    std::vector<NodeEntry *> lowerEntries;
    lowerEntries.reserve( cur_offset_ );
    std::vector<NodeEntry *> upperEntries;
    upperEntries.reserve( cur_offset_ );
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        lowerEntries.push_back( &(entries.at(i)) );
        upperEntries.push_back( &(entries.at(i)) );
    }

    // Consider all M-2m+2 distributions in each dimension
    for (unsigned d = 0; d < dimensions; d++)
    {
        // First sort in the current dimension sorting both the lower and upper arrays
        std::sort(lowerEntries.begin(), lowerEntries.end(), [d](NodeEntry *a, NodeEntry *b)
        {
            return std::get<Branch>(*a).boundingBox.lowerLeft[d] < std::get<Branch>(*b).boundingBox.lowerLeft[d];
        });
        std::sort(upperEntries.begin(), upperEntries.end(), [d](NodeEntry *a, NodeEntry *b)
        {
            return std::get<Branch>(*a).boundingBox.upperRight[d] < std::get<Branch>(*b).boundingBox.upperRight[d];
        });

        // Setup groups
        std::vector<NodeEntry *> groupALower(lowerEntries.begin(),
                lowerEntries.begin() + min_branch_factor );
        std::vector<NodeEntry *> groupAUpper(upperEntries.begin(),
                upperEntries.begin() + min_branch_factor );

        std::vector<NodeEntry *> groupBLower(lowerEntries.begin() +
                min_branch_factor, lowerEntries.end());
        std::vector<NodeEntry *> groupBUpper(upperEntries.begin() +
                min_branch_factor, upperEntries.end());

        // Cycle through all M-2m+2 distributions
        double totalMarginLower = 0.0;
        double totalMarginUpper = 0.0;
        for (;groupALower.size() <= max_branch_factor &&
                groupBLower.size() >= min_branch_factor; )
        {
            // Compute the margin of groupA and groupB
            Rectangle boundingBoxALower = boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupALower[0]);
            Rectangle boundingBoxAUpper = boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupAUpper[0]);
            for (unsigned i = 1; i < groupALower.size(); ++i)
            {
                boundingBoxALower.expand(boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupALower[i]));
                boundingBoxAUpper.expand(boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupAUpper[i]));
            }

            Rectangle boundingBoxBLower = boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupBLower[0]);
            Rectangle boundingBoxBUpper = boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupBUpper[0]);
            for (unsigned i = 1; i < groupBLower.size(); ++i)
            {
                boundingBoxBLower.expand(boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupBLower[i]));
                boundingBoxBUpper.expand(boxFromNodeEntry<min_branch_factor,max_branch_factor>(*groupBUpper[i]));
            }

            // Add to the total margin sum
            totalMarginLower += boundingBoxALower.margin() + boundingBoxBLower.margin();
            totalMarginUpper += boundingBoxAUpper.margin() + boundingBoxBUpper.margin();

            // Add one new value to groupA and remove one from groupB to obtain next distribution
            NodeEntry *transferPointLower = groupBLower.front();
            NodeEntry *transferPointUpper = groupBUpper.front();
            groupBLower.erase(groupBLower.begin());
            groupBUpper.erase(groupBUpper.begin());
            groupALower.push_back(transferPointLower);
            groupAUpper.push_back(transferPointUpper);
        }

        if (totalMarginLower < optimalMarginLower)
        {
            optimalMarginLower = totalMarginLower;
            optimalAxisLower = d;
        }

        if (totalMarginUpper < optimalMarginUpper)
        {
            optimalMarginUpper = totalMarginUpper;
            optimalAxisUpper = d;
        }
    }

    bool sortLower = optimalMarginUpper > optimalMarginLower ? true : false;
    unsigned optimalAxis = sortLower ? optimalAxisLower : optimalAxisUpper;

    // Sort to match the optimal axis
    if (sortLower)
    {

        std::sort(entries.begin(), entries.begin() + cur_offset_, [optimalAxis](NodeEntry &a, NodeEntry &b)
        {
            return std::get<Branch>(a).boundingBox.lowerLeft[optimalAxis] < 
                    std::get<Branch>(b).boundingBox.lowerLeft[optimalAxis];
        });
    }
    else
    {
        std::sort(entries.begin(), entries.begin() + cur_offset_, [optimalAxis](NodeEntry &a, NodeEntry &b)
        {
            return std::get<Branch>(a).boundingBox.upperRight[optimalAxis] <
                    std::get<Branch>(b).boundingBox.upperRight[optimalAxis];
        });
    }

    return optimalAxis;
}


// CSA1: Sort entries by lower and upper bound along each axis and compute S -> sum of all
//  margin values for the different distributions. This can be stored in a array of variable
//  that we keep in a loop -> and the just compare to the others?
// 	We can first call a helper function that returns an array of all possible distributions for it?
// CSA2: Return the Axis that has the minimum total sum of all the distributions
template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::chooseSplitAxis()
{
    return isLeafNode() ? chooseSplitLeafAxis() : chooseSplitNonLeafAxis();
}

// CSI1: Given the chosen split index
// 	group all the entries into multiple groups and choose the one that has the least
// 	overlap value; resolve ties with the minimum area
// 	returns tuple of best distribution group indices
template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::chooseSplitIndex(unsigned axis)
{
    // We assume this is called after we have sorted this->data according to axis.

    const auto groupABegin = entries.begin();
    const auto groupAEnd = entries.begin() + min_branch_factor;
    const auto groupBBegin = entries.begin() + min_branch_factor;
    const auto groupBEnd = entries.begin() + cur_offset_;

    std::vector<NodeEntry> groupA(groupABegin, groupAEnd);
    std::vector<NodeEntry> groupB(groupBBegin, groupBEnd);
    unsigned splitIndex = cur_offset_  / 2;

    // Find the best size out of all the distributions
    double minOverlap = std::numeric_limits<double>::infinity();
    double minArea = std::numeric_limits<double>::infinity();

    // Tracking what the current "cut" mark is
    unsigned currentSplitPoint = min_branch_factor;

    // Try each of the M-2m + 2 groups
    while (groupA.size() <= max_branch_factor && groupB.size() >=
            min_branch_factor )
    {
        // Compute the margin of groupA and groupB
        Rectangle boundingBoxA = boxFromNodeEntry<min_branch_factor,max_branch_factor>(groupA[0]);
        for (unsigned i = 1; i < groupA.size(); ++i)
        {
            boundingBoxA.expand(boxFromNodeEntry<min_branch_factor,max_branch_factor>(groupA[i]));
        }

        Rectangle boundingBoxB = boxFromNodeEntry<min_branch_factor,max_branch_factor>(groupB[0]);
        for (unsigned i = 1; i < groupB.size(); ++i)
        {
            boundingBoxB.expand(boxFromNodeEntry<min_branch_factor,max_branch_factor>(groupB[i]));
        }

        // Compute intersection area to determine best grouping of data points
        double evalDistOverlap = boundingBoxA.computeIntersectionArea(boundingBoxB);

        if (evalDistOverlap < minOverlap)
        {
            // We save this current distribution of indices to return
            minOverlap = evalDistOverlap;
            splitIndex = currentSplitPoint;

            // Set this if we haven't already
            if (minArea == std::numeric_limits<double>::infinity())
            {
                minArea = boundingBoxA.area() + boundingBoxB.area();
            }
        }
        else if (evalDistOverlap == minOverlap)
        {
            // If overlap is equal, we use the distribution that creates the smallest areas
            double evalMinArea = boundingBoxA.area() + boundingBoxB.area();

            if (evalMinArea < minArea)
            {
                // Save this current distribution of indices to return
                minArea = evalMinArea;
                splitIndex = currentSplitPoint;
            }
        }
        
        // Add one new value to groupA and remove one from groupB to obtain next distribution
        NodeEntry transferPoint = groupB.front();
        groupB.erase(groupB.begin());
        groupA.push_back(transferPoint);

        // Push the split point forward.
        currentSplitPoint++;
    }

    return splitIndex;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::splitNode()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    // S1: Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
    // S2: Invoke chooseSplitIndex given the axis to determine the best distribution along this axis
    // S3: Distribute the entries among these two groups

    // Call chooseSplitAxis to determine the axis perpendicular to which the split is performed
    // For now we will save the axis as a int -> since this allows for room for growth in the future
    // Call ChooseSplitIndex to create optimal splitting of data array
    unsigned splitAxis = chooseSplitAxis();
    unsigned splitIndex = chooseSplitIndex(splitAxis);

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    std::pair<pinned_node_ptr<NodeType>, tree_node_handle> alloc_data =
        allocator->create_new_tree_node<NodeType>();

    pinned_node_ptr<NodeType> newSibling = alloc_data.first;
    tree_node_handle sibling_handle = alloc_data.second;

    new (&(*(newSibling))) NodeType( treeRef, sibling_handle, parent,
            level );

    newSibling->parent = parent;
    newSibling->level = level;
    newSibling->treeRef = treeRef; 
    newSibling->self_handle_ = sibling_handle;

#if !defined( NDEDBUG )
    if( parent ) {
        pinned_node_ptr<NodeType> parent_ptr = treeRef->get_node( parent );
        assert( level + 1 == parent_ptr->level );

    }
#endif

    // Copy everything to the right of the splitPoint (inclusive) to the new sibling
    std::copy( entries.begin() + splitIndex, entries.begin() + cur_offset_,
            newSibling->entries.begin() );

    newSibling->cur_offset_ = cur_offset_ - splitIndex;

    if (std::holds_alternative<Branch>(newSibling->entries[0]))
    {
        for( unsigned i = 0; i < newSibling->cur_offset_; i++ ) {
            // Update parents
            Branch &b = std::get<Branch>( newSibling->entries.at(i) );
            pinned_node_ptr<NodeType> child = treeRef->get_node( b.child );
            child->parent = sibling_handle;

            assert(level == child->level + 1);
            assert(newSibling->level == child->level + 1);
        }
    }

    // Chop our node's data down
    cur_offset_ = splitIndex;

    assert( cur_offset_ > 0 );
    assert( newSibling->cur_offset_ > 0 );

    // Return our newly minted sibling
    return sibling_handle;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::adjustTree(tree_node_handle sibling, std::vector<bool> &hasReinsertedOnLevel)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    // AT1 [Initialize]
    tree_node_handle node_handle = self_handle_;
    tree_node_handle sibling_handle = sibling;
    for (;;)
    {
        assert( node_handle );

        pinned_node_ptr<NodeType> node = treeRef->get_node( node_handle );

        // AT2 [If node is the root, stop]
        if (!node->parent)
        {
            break;
        }
        else
        {
            // AT3 [Adjust covering rectangle in parent entry]
            tree_node_handle parent_handle = node->parent;
            pinned_node_ptr<NodeType> parent_ptr = treeRef->get_node( parent_handle );
            bool didUpdateBoundingBox =
                parent_ptr->updateBoundingBox(node_handle, node->boundingBox());

            // If we have a split then deal with it otherwise move up the tree
            if (sibling_handle)
            {
                pinned_node_ptr<NodeType> sibling_ptr = treeRef->get_node( sibling_handle );
                assert(node->level + 1 == parent_ptr->level);
                assert(sibling_ptr->level + 1 == parent_ptr->level);

                // AT4 [Propogate the node split upwards]
                Branch b(sibling_ptr->boundingBox(), sibling_handle);
                parent_ptr->entries.at(parent_ptr->cur_offset_) = std::move(b) ;
                parent_ptr->cur_offset_++;
                // FIXME check for overflow
#ifndef NDEBUG
                for( unsigned i = 0; i < parent_ptr->cur_offset_; i++ ) {
                    assert( std::holds_alternative<Branch>(
                                parent_ptr->entries.at( i ) ) );
                    pinned_node_ptr<NodeType> child = treeRef->get_node(
                                std::get<Branch>( parent_ptr->entries.at(i) ).child );
                    assert(child->level + 1 == parent_ptr->level);
                }
#endif
                unsigned sz = parent_ptr->cur_offset_;
                if ( sz >= max_branch_factor )
                {
                    tree_node_handle parent_before_handle = node->parent;
                    tree_node_handle sibling_parent_handle = parent_ptr->overflowTreatment(hasReinsertedOnLevel);

                    if( sibling_parent_handle ) {
                        // We split our parent, so now we have two (possible) parents
                        assert( parent == sibling_parent_handle||
                                parent == parent_before_handle );
                        assert( sibling_ptr->parent ==
                                sibling_parent_handle ||
                                sibling_ptr->parent ==
                                parent_before_handle );

                        // Need to keep traversing up
                        node_handle = parent_before_handle;
                        sibling_handle = sibling_parent_handle;
                        assert(node != sibling_ptr);

                        continue;
                    }
                }

                node_handle = parent_ptr->self_handle_;
                sibling_handle = tree_node_handle( nullptr );
            }
            else
            {
                // AT5 [Move up to next level]
                if (didUpdateBoundingBox)
                {
                    node_handle = parent_ptr->self_handle_;
                } else {

                    // If we didn't update our bounding box and there was no split, no reason to keep
                    // going.
                    return tree_node_handle( nullptr );
                }
            }
        }
    }

    return sibling_handle;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::reInsert(std::vector<bool> &hasReinsertedOnLevel)
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;

    // 1. RI1 Compute distance between each of the points and the bounding box containing them.
    // 2. RI2 Sort the entries by DECREASING index -> ok let's define an
    // 		extra helper function that gets to do this and pass it into sort

    Point globalCenterPoint = boundingBox().centrePoint();

    assert(hasReinsertedOnLevel.at(level));

    std::sort(entries.begin(), entries.begin() + cur_offset_,
        [&globalCenterPoint](NodeEntry &a, NodeEntry &b)
        {
            Rectangle rectA = boxFromNodeEntry<min_branch_factor,max_branch_factor>(a);
            Rectangle rectB = boxFromNodeEntry<min_branch_factor,max_branch_factor>(b);
            return rectA.centrePoint().distance(globalCenterPoint) > rectB.centrePoint().distance(globalCenterPoint);
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
    std::vector<NodeEntry> entriesToReinsert;
    entriesToReinsert.reserve(numNodesToReinsert);

    // copy these out
    std::copy(entries.begin() + remainder, entries.begin() + cur_offset_, std::back_inserter(entriesToReinsert));
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
        pinned_node_ptr<NodeType> root_node = treeRef->get_node( root_handle );

        // If it does not, then we've found the root
        if( !root_node->parent ) {
            break;
        }

        root_handle = root_node->parent;
    }

    pinned_node_ptr<NodeType> root_node = treeRef->get_node( root_handle );

    for( const NodeEntry &entry : entriesToReinsert ) {
        assert( !root_node->parent );

        root_handle = root_node->insert(entry, hasReinsertedOnLevel);
        root_node = treeRef->get_node( root_handle );
    }

    return tree_node_handle( nullptr );
}

// Overflow treatement for dealing with a node that is too big (overflow)
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::overflowTreatment(std::vector<bool> &hasReinsertedOnLevel)
{
    assert(hasReinsertedOnLevel.size() > level);

    if (hasReinsertedOnLevel.at(level))
    {
        return splitNode();
    }
    else
    {
        hasReinsertedOnLevel.at(level) = true;
        return reInsert(hasReinsertedOnLevel);
    }
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::insert(NodeEntry nodeEntry, std::vector<bool> &hasReinsertedOnLevel)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator( treeRef );
    // Always called on root, this = root
    assert( !parent );

    // I1 [Find position for new record]
    tree_node_handle insertion_point_handle = chooseSubtree(nodeEntry);
    pinned_node_ptr<NodeType> insertion_point = treeRef->get_node( insertion_point_handle );

    tree_node_handle sibling_handle = tree_node_handle( nullptr );

    // I2 [Add record to leaf node]
    bool givenIsLeaf = std::holds_alternative<Point>(nodeEntry);
#ifndef NDEBUG
    bool firstIsPoint = entries.empty() ||
        std::holds_alternative<Point>(insertion_point->entries[0]);
    assert((givenIsLeaf && firstIsPoint) || (!givenIsLeaf && !firstIsPoint));
#endif
    insertion_point->entries.at(insertion_point->cur_offset_) = nodeEntry;
    insertion_point->cur_offset_++;

    if (!givenIsLeaf)
    {
        const Branch &b = std::get<Branch>(nodeEntry);
        pinned_node_ptr<NodeType> child = treeRef->get_node( b.child );
        assert(insertion_point->level == child->level + 1);
        child->parent = insertion_point_handle;
    }

    unsigned num_els = insertion_point->cur_offset_;

    // If we exceed treeRef->maxBranchFactor we need to do something about it
    if ( num_els > max_branch_factor )
    {
        // We call overflow treatment to determine how our sibling node is treated if we do a
        // reInsert, sibling is nullptr. This is properly dealt with in adjustTree
        sibling_handle = insertion_point->overflowTreatment(hasReinsertedOnLevel);
    }

    // I3 [Propogate overflow treatment changes upward]
    sibling_handle = insertion_point->adjustTree(
            sibling_handle, hasReinsertedOnLevel );

    // I4 [Grow tree taller]
    if( sibling_handle ) {

        assert( !parent );
        std::pair<pinned_node_ptr<NodeType>, tree_node_handle> alloc_data =
            allocator->create_new_tree_node<NodeType>();
        pinned_node_ptr<NodeType> newRoot = alloc_data.first;
        tree_node_handle root_handle = alloc_data.second;

        pinned_node_ptr<NodeType> sibling = treeRef->get_node( sibling_handle );

        new (&(*(newRoot)))
            NodeType( treeRef, root_handle,
                tree_node_handle( nullptr ), this->level+1 );
        
        this->parent = root_handle;

        // Make the existing root a child of newRoot
        Branch b1( boundingBox(), self_handle_ );
        newRoot->entries.at(newRoot->cur_offset_) = std::move( b1 );
        newRoot->cur_offset_++;

        // Make the new sibling node a child of newRoot
        sibling->parent = root_handle;
        Branch b2( sibling->boundingBox(), sibling_handle );
        newRoot->entries.at( newRoot->cur_offset_ ) = std::move( b2 );
        newRoot->cur_offset_++;

        // Ensure newRoot has both children
        assert(newRoot->cur_offset_ == 2);
        assert(sibling->level+1 == newRoot->level);

        // Fix the reinserted length
        hasReinsertedOnLevel.push_back(false);

        return root_handle;
    }
    else
    {
        // We might no longer be the parent.  If we hit overflowTreatment, we may have triggered
        // reInsert, which then triggered a split. That insert will have returned newRoot, but
        // because reInsert() returns nullptr, we don't know about it
        tree_node_handle root_handle = self_handle_; 

        for( ;; ) {
            pinned_node_ptr<NodeType> node_data = treeRef->get_node( root_handle );
            if( !node_data->parent ) {
                return root_handle;
            }

            root_handle = node_data->parent;
        }
        return root_handle;
    }
}

// To be called on a leaf
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor,max_branch_factor>::condenseTree(std::vector<bool> &hasReinsertedOnLevel)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    // CT1 [Initialize]
    tree_node_handle node_handle = self_handle_;

    // Is Leaf
    assert( cur_offset_ == 0 || std::holds_alternative<Point>(entries[0]));

    std::vector<NodeEntry> Q;

    // CT2 [Find parent entry]
    unsigned entriesSize;

    for( ;; ) {
        pinned_node_ptr<NodeType> node = treeRef->get_node( node_handle );

        if( !node->parent ) {
            break;
        }

        entriesSize = node->cur_offset_;

        // CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
        if (entriesSize >= min_branch_factor )
        {
            pinned_node_ptr<NodeType> parent = treeRef->get_node( node->parent );
            parent->updateBoundingBox( node_handle, node->boundingBox());

            // CT5 [Move up one level in the tree]
            // Move up a level without deleting ourselves
            node_handle = parent->self_handle_;
        }
        else
        {

            pinned_node_ptr<NodeType> parent = treeRef->get_node( node->parent );
            // Remove ourselves from our parent
            parent->removeChild( node_handle );
            assert( !node->entries.empty() );

            // Push these entries into Q
            std::copy(node->entries.begin(),
                    node->entries.begin() + node->cur_offset_, std::back_inserter(Q));

            // FIXME: Should garbage collect node_ptr, it is dead now
            //tree_node_handle garbage = node_handle;

            node_handle = parent->self_handle_;
            // Cleanup ourselves without deleting children b/c they will be reinserted
            // GarbageCollect( node_ptr );

        }
    }

    // CT6 [Re-insert oprhaned entries]
    for (const auto &entry : Q)
    {
        pinned_node_ptr<NodeType> root = treeRef->get_node( node_handle );
        assert( !root->parent );
        node_handle = root->insert(entry, hasReinsertedOnLevel);
    }

    return node_handle;
}

// Always called on root, this = root
template <int min_branch_factor, int max_branch_factor> 
tree_node_handle Node<min_branch_factor,max_branch_factor>::remove(Point &givenPoint, std::vector<bool> hasReinsertedOnLevel)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    assert( !parent );

    // D1 [Find node containing record]
    tree_node_handle leaf_ptr = findLeaf(givenPoint);
    if(!leaf_ptr) {
        return leaf_ptr; /*nullptr*/ 
    }

    pinned_node_ptr<NodeType> leaf = treeRef->get_node( leaf_ptr );
    // D2 [Delete record]

    leaf->removeData(givenPoint);

    // D3 [Propagate changes]
    tree_node_handle root_handle = leaf->condenseTree(hasReinsertedOnLevel);
    pinned_node_ptr<NodeType> root = treeRef->get_node( root_handle );

    // D4 [Shorten tree]
    if (root->cur_offset_ == 1 and !root->isLeafNode())
    {
        // Slice the hasReinsertedOnLevel
        hasReinsertedOnLevel.pop_back();

        // We are removing the root to shorten the tree so we then decide to remove the root
        Branch &b = std::get<Branch>(root->entries[0]);

        // Get rid of the old root
        pinned_node_ptr<NodeType> child = treeRef->get_node( b.child );

        // Garbage Collect Root
        // FIXME GC(root);


        // I'm the root now!
        child->parent = tree_node_handle( nullptr );

        return b.child;
    }
    else
    {
        return root_handle;
    }
}

template <int min_branch_factor, int max_branch_factor> 
void Node<min_branch_factor,max_branch_factor>::print() const
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    tree_node_handle root_handle = get_root_handle( treeRef );

    pinned_node_ptr<NodeType> root = treeRef->get_node( root_handle );
    unsigned max_level = root->level;

    std::string indentation((max_level - level) * 4, ' ');
    std::cout << indentation << "Node " << (void *)this << std::endl;
    std::cout << indentation << "{" << std::endl;
    std::cout << indentation << "    BoundingBox: " << boundingBox() << std::endl;
    std::cout << indentation << "    Parent: " << parent << std::endl;
    std::cout << indentation << "    Entries: " << std::endl;
    
    bool isLeaf = isLeafNode();
    if (isLeaf)
    {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            std::cout << indentation << "		" <<
                std::get<Point>( entries.at(i) ) << std::endl;
        }
    }
    else
    {
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            const Branch &b = std::get<Branch>( entries.at(i) );
            std::cout << indentation << "		" << b.boundingBox << ", ptr: " << b.child << std::endl;
        }
    }
    std::cout << std::endl << indentation << "}" << std::endl;
}

template <int min_branch_factor, int max_branch_factor> 
void Node<min_branch_factor,max_branch_factor>::printTree() const
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    // Print this node first
    struct Printer
    {
        void operator()( pinned_node_ptr<NodeType> node) {
            node->print();
        }
    };

    Printer p;
    treeWalker<min_branch_factor,max_branch_factor>(treeRef, self_handle_, p);
}

template <int min_branch_factor, int max_branch_factor> 
unsigned Node<min_branch_factor,max_branch_factor>::checksum() const
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    struct ChecksumFunctor
    {
        unsigned checksum;

        ChecksumFunctor() {
            checksum = 0;
        }

        void operator()( pinned_node_ptr<NodeType> node ) {
            bool isLeaf = node->entries.empty() || std::holds_alternative<Point>(node->entries[0]);
            if( isLeaf ) {
                for( unsigned i = 0; i < node->cur_offset_; i++ ) {
                    const Point &p = std::get<Point>(
                            node->entries.at(i) );
                    for( unsigned d = 0; d < dimensions; ++d ) {
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

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor,max_branch_factor>::height() const
{
    assert( parent == nullptr );
    return level+1;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor,max_branch_factor>::stat() const
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
                pinned_node_ptr<Node<min_branch_factor,max_branch_factor>> node ) {

            unsigned entriesSize = node->cur_offset_;

            if (entriesSize == 1)
            {
                ++singularBranches;
            }

            ++totalNodes;

            if (unlikely(entriesSize >= histogramFanout.size()))
            {
                //Avoid reallocing
                histogramFanout.resize(2*entriesSize, 0);
            }
            ++histogramFanout[entriesSize];

            bool isLeaf = node->isLeafNode();
            if (isLeaf)
            {
                ++totalLeaves;
                memoryFootprint += sizeof(Node) + entriesSize * sizeof(Point);
            }
            else
            {
                // Compute the overlap and coverage of our children
                for (unsigned i = 0; i < entriesSize; ++i)
                {
                    coverage += boxFromNodeEntry<min_branch_factor,max_branch_factor>(node->entries[i]).area();

                    for (unsigned j = 0; j < node->entries.size(); ++j)
                    {
                        if (i != j)
                        {
                            overlap += boxFromNodeEntry<min_branch_factor,max_branch_factor>(node->entries[i]).computeIntersectionArea(boxFromNodeEntry<min_branch_factor,max_branch_factor>(node->entries[j]));
                        }
                    }
                }

                memoryFootprint += sizeof(Node) + entriesSize * sizeof(Node *) + entriesSize * sizeof(Rectangle);
            }
        }
    };

    StatWalker sw;
    treeWalker( treeRef, treeRef->root, sw);

    // Print out what we have found
    STATMEM(sw.memoryFootprint);
    STATHEIGHT(height());
    STATSIZE(sw.totalNodes);
    STATSINGULAR(sw.singularBranches);
    STATLEAF(sw.totalLeaves);
    STATBRANCH(sw.totalNodes - 1);
    STATCOVER(sw.coverage);
    STATOVERLAP(sw.overlap);
    STATAVGCOVER(sw.coverage / sw.totalNodes);
    STATAVGOVERLAP(sw.overlap / sw.totalNodes);
    STATFANHIST();
    for (unsigned i = 0; i < sw.histogramFanout.size(); ++i)
    {
        if (sw.histogramFanout[i] > 0)
        {
            STATHIST(i, sw.histogramFanout[i]);
        }
    }
    std::cout << treeRef->stats;

    STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
    (void) 0;
#endif
}

/*
template <class NE, class B>
Rectangle boxFromNodeEntry<min_branch_factor,max_branch_factor>(const NE &entry)
{
}
*/
