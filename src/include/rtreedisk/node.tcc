template <int min_branch_factor, int max_branch_factor>
Node<min_branch_factor, max_branch_factor>::Node(RTreeDisk<min_branch_factor, max_branch_factor> *treeRef, tree_node_handle self_handle)
{
    this->parent = tree_node_handle(nullptr);
    this->self_handle_ = self_handle;
    this->treeRef = treeRef;
    this->cur_offset_ = 0;
}

template <int min_branch_factor, int max_branch_factor>
Node<min_branch_factor, max_branch_factor>::Node(RTreeDisk<min_branch_factor, max_branch_factor> *treeRef, tree_node_handle self_handle, tree_node_handle parent)
{
    this->parent = parent;
    this->self_handle_ = self_handle;
    this->treeRef = treeRef;
    this->cur_offset_ = 0;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::deleteSubtrees()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    if (isLeafNode()) {
        return;
    }

    // Note: no actual deletion happening...

    for (unsigned i = 0; i < cur_offset_; i++)
    {
        tree_node_handle child_handle = std::get<Branch>( entries.at(i) ).child;
        assert(child_handle != nullptr);

        pinned_node_ptr<NodeType> child =
            allocator->get_tree_node<NodeType>(child_handle);
        child->deleteSubtrees();
    }
}

template <int min_branch_factor, int max_branch_factor>
Rectangle Node<min_branch_factor, max_branch_factor>::boundingBox()
{
    Rectangle boundingBox;

    if (!isLeafNode())
    {
        boundingBox = std::get<Branch>( entries[0] ).boundingBox;
        for (unsigned i = 1; i < cur_offset_; ++i)
        {
            boundingBox.expand(std::get<Branch>( entries[i] ).boundingBox);
        }
    }
    else
    {
        Point &p = std::get<Point>( entries[0] );
        boundingBox = Rectangle( p, p );
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            p = std::get<Point>( entries[i] );
            boundingBox.expand( p );
        }
    }

    return boundingBox;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::updateBoundingBox(tree_node_handle child, Rectangle updatedBoundingBox)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    using BranchType = NodeType::Branch;
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        Branch &b = std::get<Branch>( entries.at(i) );
        if (b.child == child)
        {
            entries[i] = createBranchEntry<NodeType::NodeEntry, BranchType>(updatedBoundingBox, b.child);
            return;
        }
    }
#ifndef NDEBUG
    print();
    assert(false);
#endif
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeChild(tree_node_handle child)
{
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        Branch &b = std::get<Branch>( entries.at(i) );
        if (b.child == child)
        {
            removeChild(i);
            return;
        }
    }
    assert(false);
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeChild(unsigned idx)
{
    assert(!isLeafNode());
    entries[idx] = entries[cur_offset_ - 1];
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeData(unsigned idx)
{
    assert(isLeafNode());
    entries[idx] = entries[cur_offset_ - 1];
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeData(Point givenPoint)
{
    for (unsigned i = 0; i < cur_offset_; ++i)
    {
        if (std::get<Point>( entries[i] ) == givenPoint)
        {
            removeData(i);
            return;
        }
    }
    assert(false);
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    if (isLeafNode())
    {
        // Leaf!
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            Point &p = std::get<Point>( entries[i] );
            if (requestedPoint == p)
            {
                accumulator.push_back( p );
                break;
            }
        }
    }
    else
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            tree_node_handle child_handle = std::get<Branch>( entries.at(i) ).child;
            pinned_node_ptr<NodeType> child = allocator->get_tree_node<NodeType>(child_handle);
            child->exhaustiveSearch(requestedPoint, accumulator);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> Node<min_branch_factor, max_branch_factor>::search(Point &requestedPoint)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    std::vector<Point> matchingPoints;

    pinned_node_ptr<NodeType> self_node = allocator->get_tree_node<NodeType>(self_handle_);
    std::stack<pinned_node_ptr<NodeType>> context;
    context.push(self_node);

    while (!context.empty())
    {
        pinned_node_ptr<NodeType> currentContext = context.top();
        context.pop();

        if (currentContext->isLeafNode())
        {
            // Leaf
            for (unsigned i = 0; i < currentContext->cur_offset_; ++i)
            {
                Point &p = std::get<Point>(currentContext->entries[i]);
                if (requestedPoint == p)
                {
                    matchingPoints.push_back(p);
                }
            }
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        }
        else
        {
            for (unsigned i = 0; i < currentContext->cur_offset_; i++)
            {
                Branch &b = std::get<Branch>(currentContext->entries[i]);
                if (b.boundingBox.containsPoint(requestedPoint))
                {
                    tree_node_handle child_handle = b.child;
                    pinned_node_ptr<NodeType> child = allocator->get_tree_node<NodeType>(child_handle);
                    context.push(child);
                }
            }
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
        }
    }

#ifdef STAT
    treeRef.stats.resetSearchTracker<false>();
#endif

    return matchingPoints;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> Node<min_branch_factor, max_branch_factor>::search(Rectangle &requestedRectangle)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    pinned_node_ptr<NodeType> self_node =
        allocator->get_tree_node<NodeType>(self_handle_);
    std::vector<Point> matchingPoints;
    std::stack<pinned_node_ptr<NodeType>> context;
    context.push(self_node);

    while (!context.empty())
    {
        pinned_node_ptr<NodeType> curNode = context.top();
        context.pop();

        if (curNode->isLeafNode())
        {
            // Leaf
            for (unsigned i = 0; i < curNode->cur_offset_; ++i)
            {
                Point &p = std::get<Point>( curNode->entries[i] );
                if (requestedRectangle.containsPoint(p))
                {
                    matchingPoints.push_back(p);
                }
            }
#ifdef STAT
            treeRef->stats.markLeafSearched();
#endif
        }
        else
        {
#ifdef STAT
            treeRef->stats.markNonLeafNodeSearched();
#endif
            for (unsigned i = 0; i < curNode->cur_offset_; i++)
            {
                Branch &b = std::get<Branch>( curNode->entries[i] );
                if (b.boundingBox.intersectsRectangle(requestedRectangle))
                {
                    tree_node_handle child_handle = b.child;
                    tree_node_allocator *allocator = get_node_allocator(treeRef);
                    pinned_node_ptr<NodeType> child =
                        allocator->get_tree_node<NodeType>(child_handle);
                    context.push(child);
                }
            }
        }
    }

    return matchingPoints;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::chooseLeaf(Point givenPoint)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    pinned_node_ptr<NodeType> node = allocator->get_tree_node<NodeType>(self_handle_);

    while (true)
    {
        if (node->isLeafNode())
        {
            // Leaf
            return node->self_handle_;
        }
        else
        {
            // Choose subtree
            unsigned smallestExpansionIndex = 0;
            double smallestExpansionArea = std::get<Branch>( node->entries[0] ).boundingBox.computeExpansionArea(givenPoint);
            for (unsigned i = 0; i < node->cur_offset_; ++i)
            {
                double testExpansionArea = std::get<Branch>( node->entries[i] ).boundingBox.computeExpansionArea(givenPoint);
                if (smallestExpansionArea > testExpansionArea)
                {
                    smallestExpansionIndex = i;
                    smallestExpansionArea = testExpansionArea;
                }
            }

            // CL4 [Descend until a leaf is reached]
            tree_node_handle node_handle = std::get<Branch>( node->entries[smallestExpansionIndex] ).child;
            node = allocator->get_tree_node<NodeType>(node_handle);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::chooseNode(ReinsertionEntry e)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    pinned_node_ptr<NodeType> node = allocator->get_tree_node<NodeType>(self_handle_);

    while (true)
    {
        if (node->isLeafNode())
        {
            for (unsigned i = 0; i < e.level; ++i)
            {
                tree_node_handle parent_handle = node->parent;
                node = allocator->get_tree_node<NodeType>(parent_handle);
            }

            return node->self_handle_;
        }
        else
        {
            // Choose subtree
            unsigned smallestExpansionIndex = 0;
            double smallestExpansionArea = std::get<Branch>( node->entries[0] ).boundingBox.computeExpansionArea(e.boundingBox);
            for (unsigned i = 0; i < node->cur_offset_; ++i)
            {
                double testExpansionArea = std::get<Branch>( node->entries[i] ).boundingBox.computeExpansionArea(e.boundingBox);
                if (smallestExpansionArea > testExpansionArea)
                {
                    smallestExpansionIndex = i;
                    smallestExpansionArea = testExpansionArea;
                }
            }

            // CL4 [Descend until a leaf is reached]
            tree_node_handle node_handle = std::get<Branch>( node->entries[smallestExpansionIndex] ).child;
            node = allocator->get_tree_node<NodeType>(node_handle);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::findLeaf(Point givenPoint)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    pinned_node_ptr<NodeType> node = allocator->get_tree_node<NodeType>(self_handle_);
    std::stack<pinned_node_ptr<NodeType>> context;
    context.push(node);

    while (!context.empty())
    {
        pinned_node_ptr<NodeType> currentContext = context.top();
        context.pop();

        if (currentContext->isLeafNode())
        {
            // Leaf
            for (unsigned i = 0; i < currentContext->cur_offset_; ++i)
            {
                if (std::get<Point>( currentContext->entries[i] ) == givenPoint)
                {
                    return currentContext->self_handle_;
                }
            }
        }
        else
        {
            // Choose subtree
            for (unsigned i = 0; i < currentContext->cur_offset_; ++i)
            {
                Branch &b = std::get<Branch>( currentContext->entries[i] );
                if (b.boundingBox.containsPoint(givenPoint))
                {
                    // Add the child to the nodes we will consider
                    context.push(allocator->get_tree_node<NodeType>(b.child));
                }
            }
        }
    }
    return tree_node_handle(nullptr);
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveData(unsigned fromIndex, std::vector<Point> &toData)
{
    toData.push_back(std::get<Point>( entries[fromIndex] ));
    entries[fromIndex] = entries[cur_offset_ - 1];
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveData(std::vector<Point> &fromData)
{
    cur_offset_ = fromData.size();
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        entries[i] = fromData.at(i);
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveChildren(std::vector<tree_node_handle> &fromChildren, std::vector<Rectangle> &fromBoxes)
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    using BranchType = NodeType::Branch;
    assert(fromChildren.size() == fromBoxes.size());
    cur_offset_ = fromChildren.size();
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        entries[i] = createBranchEntry<NodeType::NodeEntry, BranchType>( fromBoxes.at(i) , fromChildren.at(i) );
    }

    fromChildren.clear();
    fromBoxes.clear();
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveChild(unsigned fromIndex, std::vector<Rectangle> &toRectangles, std::vector<tree_node_handle> &toChildren)
{
    Branch &b = std::get<Branch>( entries[fromIndex] );
    toRectangles.push_back(b.boundingBox);
    toChildren.push_back(b.child);
    entries[fromIndex] = entries[cur_offset_ - 1];
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::splitNode(tree_node_handle newChildHandle)
{
    // Consider newChild when splitting
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    using BranchType = NodeType::Branch;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    pinned_node_ptr<NodeType> newChild = allocator->get_tree_node<NodeType>(newChildHandle);

    addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>( newChild->boundingBox(), newChildHandle ));
    newChild->parent = self_handle_;
    unsigned boundingBoxesSize = (isLeafNode()) ? 0 : cur_offset_;

    // Setup the two groups which will be the entries in the two new nodes
    unsigned seedA = 0;
    unsigned seedB = boundingBoxesSize - 1;

    // Compute the first entry in each group based on PS1 & PS2
    double maxWasted = 0;
    Rectangle iBox, jBox;
    for (unsigned i = 0; i < boundingBoxesSize; ++i)
    {
        iBox = std::get<Branch>( entries[i] ).boundingBox;
        for (unsigned j = 0; j < boundingBoxesSize; ++j)
        {
            jBox = std::get<Branch>( entries[j] ).boundingBox;

            // Calculate the wasted space
            Rectangle temp = iBox;
            temp.expand(jBox);

            double wasted = temp.area() - iBox.area() - jBox.area() + iBox.computeIntersectionArea(jBox);

            if (maxWasted < wasted)
            {
                maxWasted = wasted;

                seedA = i;
                seedB = j;
            }
        }
    }

    // Setup the two groups which will be the entries in the two new nodes
    std::vector<Rectangle> groupABoundingBoxes;
    std::vector<tree_node_handle> groupAChildren;
    std::vector<Rectangle> groupBBoundingBoxes;
    std::vector<tree_node_handle> groupBChildren;

    // Set the bounding rectangles
    Rectangle boundingBoxA = std::get<Branch>( entries[seedA] ).boundingBox;
    Rectangle boundingBoxB = std::get<Branch>( entries[seedB] ).boundingBox;

    // seedA and seedB have both already been allocated so put them into the appropriate group
    // and remove them from our boundingBoxes being careful to delete the one which will not
    // affect the index of the other first
    groupABoundingBoxes.push_back(std::get<Branch>( entries[seedA] ).boundingBox);
    groupAChildren.push_back(std::get<Branch>( entries[seedA] ).child);
    groupBBoundingBoxes.push_back(std::get<Branch>( entries[seedB] ).boundingBox);
    groupBChildren.push_back(std::get<Branch>( entries[seedB] ).child);
    if (seedA > seedB)
    {
        removeChild(seedA);
        removeChild(seedB);
    }
    else
    {
        removeChild(seedB);
        removeChild(seedA);
    }

    // Go through the remaining entries and add them to groupA or groupB
    double groupAAffinity, groupBAffinity;
    // QS2 [Check if done]
    while ( !isLeafNode() && (groupABoundingBoxes.size() + cur_offset_ > min_branch_factor) && (groupBBoundingBoxes.size() + cur_offset_ > min_branch_factor))
    {
        // PN1 [Determine the cost of putting each entry in each group]
        unsigned groupAIndex = 0;
        double groupAMin = std::numeric_limits<double>::infinity();
        unsigned groupBIndex = 0;
        double groupBMin = std::numeric_limits<double>::infinity();

        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            Rectangle r = std::get<Branch>( entries[i] ).boundingBox;
            groupAAffinity = boundingBoxA.computeExpansionArea(r);
            groupBAffinity = boundingBoxB.computeExpansionArea(r);
            // PN2 [Find entry with greatest preference for one group]
            if (groupAAffinity < groupAMin)
            {
                groupAMin = groupAAffinity;
                groupAIndex = i;
            }

            if (groupBAffinity < groupBMin)
            {
                groupBMin = groupBAffinity;
                groupBIndex = i;
            }
        }

        // QS3 [Select where to assign entry]
        if (groupAMin == groupBMin)
        {
            // Tie so use smaller area
            if (boundingBoxA.area() < boundingBoxB.area())
            {
                boundingBoxA.expand( std::get<Branch>( entries[groupAIndex] ).boundingBox );
                moveChild(groupAIndex, groupABoundingBoxes, groupAChildren);
            }
            else
            {
                // Better area or in the worst case an arbitrary choice
                boundingBoxB.expand( std::get<Branch>( entries[groupBIndex] ).boundingBox );
                moveChild(groupBIndex, groupBBoundingBoxes, groupBChildren);
            }
        }
        else if (groupAMin < groupBMin)
        {
            // Higher affinity for groupA
            boundingBoxA.expand(std::get<Branch>( entries[groupAIndex] ).boundingBox );
            moveChild(groupAIndex, groupABoundingBoxes, groupAChildren);
        }
        else
        {
            // Higher affinity for groupB
            boundingBoxB.expand(std::get<Branch>( entries[groupBIndex] ).boundingBox );
            moveChild(groupBIndex, groupBBoundingBoxes, groupBChildren);
        }
    }

    // If we stopped because half the entries were assigned then great put the others in the
    // opposite group
    if (groupABoundingBoxes.size() + cur_offset_ == min_branch_factor)
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            Branch &b = std::get<Branch>( entries[i] );
            groupABoundingBoxes.emplace_back(b.boundingBox);
            groupAChildren.emplace_back(b.child);
        }
    }
    else if (groupBBoundingBoxes.size() + cur_offset_ == min_branch_factor)
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            Branch &b = std::get<Branch>( entries[i] );
            groupBBoundingBoxes.emplace_back(b.boundingBox);
            groupBChildren.emplace_back(b.child);
        }
    }
    else
    {
        // We really shouldn't be here so panic!
        assert(false);
    }

    // Create the new node and fill it
    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle newSiblingHandle = alloc_data.second;
    auto newSibling = alloc_data.first;
    new (&(*newSibling)) NodeType(treeRef, newSiblingHandle, parent);

    // Fill us with groupA and the new node with groupB
    moveChildren(groupAChildren, groupABoundingBoxes);
#ifndef NDEBUG
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        Branch &b = std::get<Branch>( entries[i] );
        assert(allocator->get_tree_node<NodeType>(b.child)->parent == self_handle_);
    }
#endif
    newSibling->moveChildren(groupBChildren, groupBBoundingBoxes);
    for (unsigned i = 0; i < newSibling->cur_offset_; i++)
    {
        Branch &b = std::get<Branch>( newSibling->entries[i] );
        allocator->get_tree_node<NodeType>(b.child)->parent = newSiblingHandle;
    }

    // Return our newly minted sibling
    return newSiblingHandle;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::splitNode(Point newData)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    // Helper functions
    // Include the new point in our split consideration
    addEntryToNode(newData);
    double dataSize = cur_offset_;

    // Compute the first entry in each group based on PS1 & PS2
    unsigned seedA = 0;
    unsigned seedB = dataSize - 1;

    // This rectangle drank too much and represents how wasted iData and jData are
    double maxWasted = 0.0;

    // QS1 [Pick entry for each group]
    Point iData, jData;
    for (unsigned i = 0; i < dataSize && isLeafNode(); ++i)
    {
        iData = std::get<Point>( entries[i] );
        for (unsigned j = 0; j < dataSize; ++j)
        {
            jData = std::get<Point>( entries[j] );

            Rectangle temp = Rectangle(iData, iData);
            temp.expand(jData);

            double wasted = temp.area();

            if (maxWasted < wasted)
            {
                maxWasted = wasted;

                seedA = i;
                seedB = j;
            }
        }
    }

    // Setup the two groups which will be the entries in the two new nodes
    std::vector<Point> groupAData;
    std::vector<Point> groupBData;

    // Set the bounding rectangles
    Rectangle boundingBoxA = Rectangle(std::get<Point>( entries[seedA] ), std::get<Point>( entries[seedA] ));
    Rectangle boundingBoxB = Rectangle(std::get<Point>( entries[seedB] ), std::get<Point>( entries[seedB] ));

    // seedA and seedB have both already been allocated so put them into the appropriate group
    // and remove them from our data being careful to delete the one which will not affect the
    // index of the other first
    groupAData.push_back(std::get<Point>( entries[seedA] ));
    groupBData.push_back(std::get<Point>( entries[seedB] ));
    if (seedA > seedB)
    {
        removeData(seedA);
        removeData(seedB);
    }
    else
    {
        removeData(seedB);
        removeData(seedA);
    }

    // Go through the remaining entries and add them to groupA or groupB
    double groupAAffinity, groupBAffinity;
    // QS2 [Check if done]
    while (isLeafNode() && cur_offset_ > 0 && (groupAData.size() + cur_offset_ > min_branch_factor) && (groupBData.size() + cur_offset_ > min_branch_factor))
    {
        // PN1 [Determine the cost of putting each entry in each group]
        unsigned groupAIndex = 0;
        double groupAMin = std::numeric_limits<double>::infinity();
        unsigned groupBIndex = 0;
        double groupBMin = std::numeric_limits<double>::infinity();

        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            groupAAffinity = boundingBoxA.computeExpansionArea(std::get<Point>( entries[i] ));
            groupBAffinity = boundingBoxB.computeExpansionArea(std::get<Point>( entries[i] ));
            // PN2 [Find entry with greatest preference for one group]
            if (groupAAffinity < groupAMin)
            {
                groupAMin = groupAAffinity;
                groupAIndex = i;
            }

            if (groupBAffinity < groupBMin)
            {
                groupBMin = groupBAffinity;
                groupBIndex = i;
            }
        }

        // QS3 [Select where to assign entry]
        if (groupAMin == groupBMin)
        {
            // Tie so use smaller area
            if (boundingBoxA.area() < boundingBoxB.area())
            {
                boundingBoxA.expand(std::get<Point>( entries[groupAIndex] ));
                moveData(groupAIndex, groupAData);
            }
            else
            {
                // Better area or in the worst case an arbitrary choice
                boundingBoxB.expand( std::get<Point>( entries[groupBIndex] ));
                moveData(groupBIndex, groupBData);
            }
        }
        else if (groupAMin < groupBMin)
        {
            // Higher affinity for groupA
            boundingBoxA.expand(std::get<Point>( entries[groupAIndex] ));
            moveData(groupAIndex, groupAData);
        }
        else
        {
            // Higher affinity for groupB
            boundingBoxB.expand(std::get<Point>( entries[groupBIndex] ));
            moveData(groupBIndex, groupBData);
        }
    }

    // If we stopped because half the entries were assigned then great put the others in the
    // opposite group
    assert(isLeafNode());
    if (isLeafNode() && groupAData.size() + cur_offset_ == min_branch_factor)
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            groupAData.emplace_back(std::get<Point>( entries[i] ));
        }
    }
    else if (isLeafNode() && groupBData.size() + cur_offset_ == min_branch_factor)
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            groupBData.emplace_back(std::get<Point>( entries[i] ));
        }
    }
    else
    {
        // We really shouldn't be here so panic!
        assert(false);
    }

    // Create the new node and fill it
    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle siblingHandle = alloc_data.second;
    auto newSibling = alloc_data.first;
    new (&(*newSibling)) NodeType(treeRef, siblingHandle, parent);

    // Fill us with groupA and the new node with groupB
    moveData(groupAData);
    newSibling->moveData(groupBData);

    // Return our newly minted sibling
    return newSibling->self_handle_;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::adjustTree(tree_node_handle siblingHandle)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    using BranchType = NodeType::Branch;
    tree_node_allocator *allocator = get_node_allocator(treeRef); // Helper functions

    // AT1 [Initialize]
    auto node = allocator->get_tree_node<NodeType>(self_handle_);

    while (true)
    {
        // AT2 [If node is the root, stop]
        if (node->parent == nullptr)
        {
            break;
        }
        else
        {
            // AT3 [Adjust covering rectangle in parent entry]
            tree_node_handle parent_handle = node->parent;
            pinned_node_ptr<NodeType> parentNode = allocator->get_tree_node<NodeType>(parent_handle);
            parentNode->updateBoundingBox(node->self_handle_, node->boundingBox());

            // If we have a split then deal with it otherwise move up the tree
            if (siblingHandle != nullptr)
            {
                pinned_node_ptr<NodeType> siblingNode = allocator->get_tree_node<NodeType>(siblingHandle);
                // AT4 [Propagate the node split upwards]
                if (!parentNode->isLeafNode() && parentNode->cur_offset_ < max_branch_factor)
                {
                    parentNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>( siblingNode->boundingBox(), siblingNode->self_handle_ ));
                    siblingNode->parent = parentNode->self_handle_;

                    node = parentNode;
                    siblingHandle = tree_node_handle(nullptr);
                }
                else
                {
                    tree_node_handle siblingParentHandle = parentNode->splitNode(siblingHandle);

                    node = parentNode;
                    siblingHandle = siblingParentHandle;
                }
            }
            else
            {
                // AT5 [Move up to next level]
                tree_node_handle parent_handle = node->parent;
                pinned_node_ptr<NodeType> parentNode = allocator->get_tree_node<NodeType>(parent_handle);
                node = parentNode;
            }
        }
    }

    return siblingHandle;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::insert(Point givenPoint)
{
    // I1 [Find position for new record]
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    using BranchType = NodeType::Branch;
    tree_node_allocator *allocator = get_node_allocator(treeRef); // Helper functions
    pinned_node_ptr<NodeType> leaf = allocator->get_tree_node<NodeType>(chooseLeaf(givenPoint));
    tree_node_handle siblingLeaf = tree_node_handle(nullptr);

    // I2 [Add record to leaf node]
    if (leaf->isLeafNode() && leaf->cur_offset_ < max_branch_factor)
    {
        leaf->addEntryToNode(givenPoint);
    }
    else
    {
        siblingLeaf = leaf->splitNode(givenPoint);
    }

    // I3 [Propogate changes upward]
    tree_node_handle siblingNodeHandle = leaf->adjustTree(siblingLeaf);

    // I4 [Grow tree taller]
    if (siblingNodeHandle != nullptr)
    {
        auto siblingNode = allocator->get_tree_node<NodeType>(siblingNodeHandle);

        auto alloc_data = allocator->create_new_tree_node<NodeType>();
        tree_node_handle root_handle = alloc_data.second;
        auto newRoot = alloc_data.first;
        new (&(*newRoot)) NodeType(treeRef, root_handle);

        parent = root_handle;
        newRoot->addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>( this->boundingBox(), self_handle_ ));

        siblingNode->parent = newRoot->self_handle_;
        newRoot->addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>( siblingNode->boundingBox(), siblingNode->self_handle_ ));

        return newRoot->self_handle_;
    }
    else
    {
        return self_handle_;
    }
}

// Always called on root, this = root
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::insert(ReinsertionEntry e)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    using BranchType = NodeType::Branch;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    // If reinserting a leaf then use normal insert
    if (e.level == 0)
    {
        return insert(e.data);
    }

    // I1 [Find position for new record]
    tree_node_handle nodeHandle = chooseNode(e);
    tree_node_handle siblingNode = tree_node_handle(nullptr);
    auto node = allocator->get_tree_node<NodeType>(nodeHandle);

    // I2 [Add record to node]
    if (node->cur_offset_ < max_branch_factor)
    {
        allocator->get_tree_node<NodeType>(e.child)->parent = nodeHandle;
        node->addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>(e.boundingBox, e.child));
    }
    else
    {
        siblingNode = node->splitNode(e.child);
    }

    // I3 [Propogate changes upward]
    siblingNode = node->adjustTree(siblingNode);

    // I4 [Grow tree taller]
    if (siblingNode != nullptr)
    {
        auto alloc_data = allocator->create_new_tree_node<NodeType>();
        tree_node_handle root_handle = alloc_data.second;
        auto newRoot = alloc_data.first;
        new (&(*newRoot)) NodeType(treeRef, root_handle);

        this->parent = root_handle;

        newRoot->addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>( boundingBox(), self_handle_ ));

        auto siblingPtr = allocator->get_tree_node<NodeType>(siblingNode);
        siblingPtr->parent = newRoot->self_handle_;

        siblingPtr->addEntryToNode(createBranchEntry<NodeType::NodeEntry, BranchType>( siblingPtr->boundingBox(), siblingNode ));

        return newRoot->self_handle_;
    }
    else
    {
        return self_handle_;
    }
}

// To be called on a leaf
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::condenseTree()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    // CT1 [Initialize]
    tree_node_handle nodeHandle = self_handle_;
    auto node = allocator->get_tree_node<NodeType>(nodeHandle);
    unsigned level = 0;

    std::vector<ReinsertionEntry> Q;

    // CT2 [Find parent entry]
    while (node->parent != nullptr)
    {
        unsigned nodeBoundingBoxesSize = (node->isLeafNode()) ? 0 : node->cur_offset_;
        unsigned nodeDataSize = (node->isLeafNode()) ? node->cur_offset_ : 0;
        auto parentNode = allocator->get_tree_node<NodeType>(node->parent);
        // CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
        if (nodeBoundingBoxesSize >= min_branch_factor || nodeDataSize >= min_branch_factor)
        {
            parentNode->updateBoundingBox(node->self_handle_, node->boundingBox());

            // CT5 [Move up one level in the tree]
            // Move up a level without deleting ourselves
            node = allocator->get_tree_node<NodeType>(node->parent);
            nodeHandle = node->self_handle_;
            level++;
        }
        else
        {
            // Remove ourselves from our parent
            parentNode->removeChild(nodeHandle);

            // Add a reinsertion entry for each data point or branch of this node
            for (unsigned i = 0; i < nodeDataSize; ++i)
            {
                ReinsertionEntry e = {};
                e.child = nullptr;
                e.data = std::get<Point>( node->entries[i] );
                e.level = 0;
                Q.push_back(e);
            }
            for (unsigned i = 0; i < nodeBoundingBoxesSize; ++i)
            {
                ReinsertionEntry e = {};
                e.boundingBox = std::get<Branch>( node->entries[i] ).boundingBox;
                e.child = std::get<Branch>( node->entries[i] ).child;
                e.level = level;
                Q.push_back(e);
            }

            // Prepare for garbage collection
            auto garbage = node;

            // CT5 [Move up one level in the tree]
            // Move up a level before deleting ourselves
            node = parentNode;
            nodeHandle = node->self_handle_;
            level++;

            // Cleanup ourselves without deleting children b/c they will be reinserted
            // TODO: how to delete a node
            // delete garbage;
        }
    }

    // CT6 [Re-insert oprhaned entries]
    for (unsigned i = 0; i < Q.size(); ++i)
    {
        node = allocator->get_tree_node<NodeType>(node->insert(Q[i]));
    }

    return node->self_handle_;
}

// Always called on root, this = root
template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::remove(Point givenPoint)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    // D1 [Find node containing record]
    tree_node_handle leafHandle = findLeaf(givenPoint);
    auto leaf = allocator->get_tree_node<NodeType>(leafHandle);

    if (leafHandle == nullptr)
    {
        return tree_node_handle(nullptr);
    }

    // D2 [Delete record]
    leaf->removeData(givenPoint);

    // D3 [Propagate changes]
    auto root = allocator->get_tree_node<NodeType>(leaf->condenseTree());

    // D4 [Shorten tree]
    if (!root->isLeafNode() && root->cur_offset_ == 1)
    {
        auto firstChild = allocator->get_tree_node<NodeType>(std::get<Branch>( root->entries[0] ).child);
        firstChild->parent = tree_node_handle(nullptr);
        return std::get<Branch>( root->entries[0] ).child;
    }
    else
    {
        return root->self_handle_;
    }
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor, max_branch_factor>::validate(tree_node_handle expectedParent, unsigned index)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    if ( parent != expectedParent || cur_offset_ > max_branch_factor )
    {
        std::cout << "node = " << this->self_handle_ << std::endl;
        std::cout << "parent = " << parent << " expectedParent = " << expectedParent << std::endl;
        std::cout << "max_branch_factor = " << max_branch_factor << std::endl;
        std::cout << "cur_offset_ = " << cur_offset_ << std::endl;
        assert(parent == expectedParent);
    }

    if (expectedParent != nullptr)
    {
        auto parentNode = allocator->get_tree_node<NodeType>(parent);
        for (unsigned i = 0; i < cur_offset_ && isLeafNode(); i++)
        {
            Point &dataPoint = std::get<Point>( entries[i] );

            Rectangle parentBox = std::get<Branch>( allocator->get_tree_node<NodeType>(parent)->entries[index] ).boundingBox;
            if (!parentBox.containsPoint(dataPoint))
            {
                auto parentPtr = allocator->get_tree_node<NodeType>(parent);
                std::cout << parentBox << " fails to contain " << dataPoint << std::endl;
                assert(parentBox.containsPoint(dataPoint));
            }
        }
    }

    bool valid = true;
    if( !isLeafNode() ) {
        for (unsigned i = 0; i < cur_offset_; i++ ) {
            valid = valid && allocator->get_tree_node<NodeType>(std::get<Branch>(entries[i]).child)->validate(this->self_handle_, i);
        }
    }

    return valid;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::print(unsigned n)
{
    std::string indendtation(n * 4, ' ');
    std::cout << indendtation << "Node " << this->self_handle_ << std::endl;
    std::cout << indendtation << "{" << std::endl;
    std::cout << indendtation << "    Parent: " << parent << std::endl;
    std::cout << indendtation << "    Bounding Boxes: " << std::endl;
    for (unsigned i = 0; i < cur_offset_ && !isLeafNode(); ++i)
    {
        std::cout << indendtation << "		" << std::get<Branch>( entries[i] ).boundingBox << std::endl;
    }
    std::cout << std::endl
              << indendtation << "    Children: ";
    for (unsigned i = 0; i < cur_offset_ && !isLeafNode(); ++i)
    {
        std::cout << std::get<Branch>( entries[i] ).child << ' ';
    }
    std::cout << std::endl
              << indendtation << "    Data: ";
    for (unsigned i = 0; i < cur_offset_ && isLeafNode(); ++i)
    {
        std::cout << std::get<Point>( entries[i] );
    }
    std::cout << std::endl
              << indendtation << "}" << std::endl;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::printErr(unsigned n)
{
    std::string indendtation(n * 4, ' ');
    std::cerr << indendtation << "Node " << this->self_handle_ << std::endl;
    std::cerr << indendtation << "{" << std::endl;
    std::cerr << indendtation << "    Parent: " << parent << std::endl;
    std::cerr << indendtation << "    Bounding Boxes: " << std::endl;
    for (unsigned i = 0; i < cur_offset_ && !isLeafNode(); ++i)
    {
        std::cerr << indendtation << "		" << std::get<Branch>( entries[i] ).boundingBox << std::endl;
    }
    std::cerr << std::endl
              << indendtation << "    Children: ";
    for (unsigned i = 0; i < cur_offset_ && !isLeafNode(); ++i)
    {
        std::cerr << std::get<Branch>( entries[i] ).child << ' ';
    }
    std::cerr << std::endl
              << indendtation << "    Data: ";
    for (unsigned i = 0; i < cur_offset_ && isLeafNode(); ++i)
    {
        std::cerr << std::get<Point>( entries[i] );
    }
    std::cerr << std::endl
              << indendtation << "}" << std::endl;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::printTreeErr(unsigned n)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    // Print this node first
    printErr(n);

    // Print any of our children with one more level of indentation
    if (cur_offset_ > 0 && !isLeafNode())
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            allocator->get_tree_node<NodeType>(std::get<Branch>( entries[i] ).child)->printTreeErr(n + 1);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::printTree(unsigned n)
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    // Print this node first
    print(n);

    // Print any of our children with one more level of indentation
    if (cur_offset_ > 0 && !isLeafNode())
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            allocator->get_tree_node<NodeType>(std::get<Branch>( entries[i] ).child)->printTree(n + 1);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor, max_branch_factor>::checksum()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    unsigned sum = 0;

    if (isLeafNode())
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            for (unsigned d = 0; d < dimensions; ++d)
            {
                sum += (unsigned)std::get<Point>( entries[i] )[d];
            }
        }
    }
    else
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            sum += allocator->get_tree_node<NodeType>(std::get<Branch>( entries[i] ).child)->checksum();
        }
    }

    return sum;
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor, max_branch_factor>::height()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    unsigned ret = 0;
    auto node = allocator->get_tree_node<NodeType>(self_handle_);

    while (true)
    {
        ret++;
        if (isLeafNode())
        {
            return ret;
        }
        else
        {
            node = allocator->get_tree_node<NodeType>(std::get<Branch>( node->entries[0] ).child);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::stat()
{
#ifdef STAT
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    size_t memoryFootprint = 0;
    unsigned long totalNodes = 1;
    unsigned long singularBranches = 0;
    unsigned long totalLeaves = 0;

    std::vector<unsigned long> histogramFanout;
    histogramFanout.resize(max_branch_factor + 10, 0);

    double coverage = 0.0;
    double overlap = 0.0;

    // Initialize our context stack
    std::stack<tree_node_handle> context;
    context.push(self_handle_);
    pinned_node_ptr<NodeType> currentContext;

    while (!context.empty())
    {
        currentContext = allocator->get_tree_node<NodeType>(context.top());
        context.pop();

        unsigned long childrenSize = (!currentContext->isLeafNode()) ? currentContext->cur_offset_ : 0;
        unsigned long dataSize = (currentContext->isLeafNode()) ? currentContext->cur_offset_ : 0;
        unsigned fanout = childrenSize == 0 ? dataSize : childrenSize;
        if (unlikely(fanout >= histogramFanout.size()))
        {
            histogramFanout.resize(2 * fanout, 0);
        }
        ++histogramFanout[fanout];

        // Compute the overlap and coverage of our children
        for (unsigned i = 0; i < currentContext->cur_offset_ && !currentContext->isLeafNode(); ++i)
        {
            Rectangle box_i = std::get<Branch>(currentContext->entries[i]).boundingBox;
            coverage += box_i.area();

            for (unsigned j = 0; j < currentContext->cur_offset_; ++j)
            {
                if (i != j)
                {
                    Rectangle box_j = std::get<Branch>(currentContext->entries[j]).boundingBox;
                    overlap += box_i.computeIntersectionArea(box_j);
                }
            }
        }

        if (childrenSize == 0 && dataSize > 0)
        {
            ++totalLeaves;
            memoryFootprint += sizeof(Node) + currentContext->cur_offset_ * sizeof(Point);
        }
        else
        {
            totalNodes += childrenSize;
            memoryFootprint += sizeof(Node) + childrenSize * sizeof(Node *) + currentContext->cur_offset * sizeof(Rectangle);
            // Determine which branches we need to follow
            for (unsigned i = 0; i < currentContext->cur_offset_ && !currentContext->isLeafNode(); ++i)
            {
                auto child = allocator->get_tree_node<NodeType>(std::get<Branch>( currentContext->entries[i] ).child);
                if (child->cur_offset_ == 1)
                {
                    singularBranches++;
                }

                context.push(child);
            }
        }
    }

    // Print out statistics
    STATMEM(memoryFootprint);
    STATHEIGHT(height());
    STATSIZE(totalNodes);
    STATSINGULAR(singularBranches);
    STATLEAF(totalLeaves);
    STATBRANCH(totalNodes - 1);
    STATCOVER(coverage);
    STATOVERLAP(overlap);
    STATAVGCOVER(coverage / totalNodes);
    STATAVGOVERLAP(overlap / totalNodes);
    STATFANHIST();
    for (unsigned i = 0; i < histogramFanout.size(); ++i)
    {
        if (histogramFanout[i] > 0)
        {
            STATHIST(i, histogramFanout[i]);
        }
    }
    std::cout << treeRef.stats;

    STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
    (void)0;
#endif
}
