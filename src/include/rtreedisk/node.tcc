template <int min_branch_factor, int max_branch_factor>
Node<min_branch_factor, max_branch_factor>::Node(RTreeDisk<min_branch_factor, max_branch_factor> *treeRef, tree_node_handle self_handle)
{
    this->parent = tree_node_handle(nullptr);
    this->self_handle_ = self_handle;
    this->treeRef = treeRef;
    this->cur_offset_ = 0;
    this->cur_offset_data_ = 0;
}

template <int min_branch_factor, int max_branch_factor>
Node<min_branch_factor, max_branch_factor>::Node(RTreeDisk<min_branch_factor, max_branch_factor> *treeRef, tree_node_handle self_handle, tree_node_handle parent)
{
    this->parent = parent;
    this->self_handle_ = self_handle;
    this->treeRef = treeRef;
    this->cur_offset_ = 0;
    this->cur_offset_data_ = 0;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::deleteSubtrees()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    // Note: no actual deletion happening...

    for (unsigned i = 0; i < cur_offset_; i++)
    {
        tree_node_handle child_handle = children.at(i);
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
        boundingBox = boundingBoxes[0];
        for (unsigned i = 1; i < cur_offset_; ++i)
        {
            boundingBox.expand(boundingBoxes[i]);
        }
    }
    else
    {
        boundingBox = Rectangle(data[0], data[0]);
        for (unsigned i = 0; i < cur_offset_data_; ++i)
        {
            boundingBox.expand(data[i]);
        }
    }

    return boundingBox;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::updateBoundingBox(tree_node_handle child, Rectangle updatedBoundingBox)
{
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        if (children.at(i) == child)
        {
            boundingBoxes.at(i) = updatedBoundingBox;
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

        if (children[i] == child)
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
    children[idx] = children[cur_offset_ - 1];
    boundingBoxes[idx] = boundingBoxes[cur_offset_ - 1];

    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeData(unsigned idx)
{
    data[idx] = data[cur_offset_data_ - 1];
    cur_offset_data_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::removeData(Point givenPoint)
{
    for (unsigned i = 0; i < cur_offset_data_; ++i)
    {
        if (data[i] == givenPoint)
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
        for (unsigned i = 0; i < cur_offset_data_; ++i)
        {
            if (requestedPoint == data[i])
            {
                accumulator.push_back(data[i]);
                break;
            }
        }
    }
    else
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            tree_node_handle child_handle = children.at(i);
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

    for (; !context.empty();)
    {
        pinned_node_ptr<NodeType> currentContext = context.top();
        context.pop();

        if (currentContext->isLeafNode())
        {
            // Leaf
            for (unsigned i = 0; i < currentContext->cur_offset_data_; ++i)
            {
                if (requestedPoint == currentContext->data[i])
                {
                    matchingPoints.push_back(currentContext->data[i]);
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
                if (currentContext->boundingBoxes[i].containsPoint(requestedPoint))
                {
                    tree_node_handle child_handle = currentContext->children[i];
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
            for (unsigned i = 0; i < curNode->cur_offset_data_; ++i)
            {
                if (requestedRectangle.containsPoint(curNode->data[i]))
                {
                    matchingPoints.push_back(curNode->data[i]);
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
                if (curNode->boundingBoxes[i].intersectsRectangle(requestedRectangle))
                {
                    tree_node_handle child_handle = curNode->children[i];
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

    for (;;)
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
            double smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(givenPoint);
            for (unsigned i = 0; i < node->cur_offset_; ++i)
            {
                double testExpansionArea = node->boundingBoxes[i].computeExpansionArea(givenPoint);
                if (smallestExpansionArea > testExpansionArea)
                {
                    smallestExpansionIndex = i;
                    smallestExpansionArea = testExpansionArea;
                }
            }

            // CL4 [Descend until a leaf is reached]
            tree_node_handle node_handle = node->children[smallestExpansionIndex];
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

    for (;;)
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
            double smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(e.boundingBox);
            for (unsigned i = 0; i < node->cur_offset_; ++i)
            {
                double testExpansionArea = node->boundingBoxes[i].computeExpansionArea(e.boundingBox);
                if (smallestExpansionArea > testExpansionArea)
                {
                    smallestExpansionIndex = i;
                    smallestExpansionArea = testExpansionArea;
                }
            }

            // CL4 [Descend until a leaf is reached]
            tree_node_handle node_handle = node->children[smallestExpansionIndex];
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

    for (; !context.empty();)
    {
        pinned_node_ptr<NodeType> currentContext = context.top();
        context.pop();

        if (currentContext->isLeafNode())
        {
            // Leaf
            for (unsigned i = 0; i < currentContext->cur_offset_data_; ++i)
            {
                if (currentContext->data[i] == givenPoint)
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
                if (currentContext->boundingBoxes[i].containsPoint(givenPoint))
                {
                    // Add the child to the nodes we will consider
                    context.push(allocator->get_tree_node<NodeType>(currentContext->children[i]));
                }
            }
        }
    }
    return tree_node_handle(nullptr);
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveData(unsigned fromIndex, std::vector<Point> &toData)
{
    toData.push_back(data[fromIndex]);
    data[fromIndex] = data[cur_offset_data_ - 1];
    cur_offset_data_--;
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveData(std::vector<Point> &fromData)
{
    cur_offset_data_ = fromData.size();
    for (unsigned i = 0; i < cur_offset_data_; i++)
    {
        data[i] = fromData.at(i);
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveChildren(std::vector<tree_node_handle> &fromChildren, std::vector<Rectangle> &fromBoxes)
{
    assert(fromChildren.size() == fromBoxes.size());
    cur_offset_ = fromChildren.size();
    for (unsigned i = 0; i < cur_offset_; i++)
    {
        children[i] = fromChildren.at(i);
        boundingBoxes[i] = fromBoxes.at(i);
    }

    fromChildren.clear();
    fromBoxes.clear();
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::moveChild(unsigned fromIndex, std::vector<Rectangle> &toRectangles, std::vector<tree_node_handle> &toChildren)
{
    toRectangles.push_back(boundingBoxes[fromIndex]);
    toChildren.push_back(children[fromIndex]);
    boundingBoxes[fromIndex] = boundingBoxes[cur_offset_ - 1];
    children[fromIndex] = children[cur_offset_ - 1];
    cur_offset_--;
}

template <int min_branch_factor, int max_branch_factor>
tree_node_handle Node<min_branch_factor, max_branch_factor>::splitNode(tree_node_handle newChildHandle)
{
    // Consider newChild when splitting
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    pinned_node_ptr<NodeType> newChild = allocator->get_tree_node<NodeType>(newChildHandle);

    boundingBoxes[cur_offset_] = newChild->boundingBox();
    children[cur_offset_] = newChildHandle;
    cur_offset_++;

    newChild->parent = self_handle_;
    unsigned boundingBoxesSize = cur_offset_;

    // Setup the two groups which will be the entries in the two new nodes
    unsigned seedA = 0;
    unsigned seedB = boundingBoxesSize - 1;

    // Compute the first entry in each group based on PS1 & PS2
    double maxWasted = 0;
    Rectangle iBox, jBox;
    for (unsigned i = 0; i < boundingBoxesSize; ++i)
    {
        iBox = boundingBoxes[i];
        for (unsigned j = 0; j < boundingBoxesSize; ++j)
        {
            jBox = boundingBoxes[j];

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
    Rectangle boundingBoxA = boundingBoxes[seedA];
    Rectangle boundingBoxB = boundingBoxes[seedB];

    // seedA and seedB have both already been allocated so put them into the appropriate group
    // and remove them from our boundingBoxes being careful to delete the one which will not
    // affect the index of the other first
    groupABoundingBoxes.push_back(boundingBoxes[seedA]);
    groupAChildren.push_back(children[seedA]);
    groupBBoundingBoxes.push_back(boundingBoxes[seedB]);
    groupBChildren.push_back(children[seedB]);
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
    for (; !boundingBoxes.empty() && (groupABoundingBoxes.size() + cur_offset_ > min_branch_factor) && (groupBBoundingBoxes.size() + cur_offset_ > min_branch_factor);)
    {
        // PN1 [Determine the cost of putting each entry in each group]
        unsigned groupAIndex = 0;
        double groupAMin = std::numeric_limits<double>::infinity();
        unsigned groupBIndex = 0;
        double groupBMin = std::numeric_limits<double>::infinity();

        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            groupAAffinity = boundingBoxA.computeExpansionArea(boundingBoxes[i]);
            groupBAffinity = boundingBoxB.computeExpansionArea(boundingBoxes[i]);
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
                boundingBoxA.expand(boundingBoxes[groupAIndex]);
                moveChild(groupAIndex, groupABoundingBoxes, groupAChildren);
            }
            else
            {
                // Better area or in the worst case an arbitrary choice
                boundingBoxB.expand(boundingBoxes[groupBIndex]);
                moveChild(groupBIndex, groupBBoundingBoxes, groupBChildren);
            }
        }
        else if (groupAMin < groupBMin)
        {
            // Higher affinity for groupA
            boundingBoxA.expand(boundingBoxes[groupAIndex]);
            moveChild(groupAIndex, groupABoundingBoxes, groupAChildren);
        }
        else
        {
            // Higher affinity for groupB
            boundingBoxB.expand(boundingBoxes[groupBIndex]);
            moveChild(groupBIndex, groupBBoundingBoxes, groupBChildren);
        }
    }

    // If we stopped because half the entries were assigned then great put the others in the
    // opposite group
    if (groupABoundingBoxes.size() + cur_offset_ == min_branch_factor)
    {
        //groupABoundingBoxes.insert(groupABoundingBoxes.end(), boundingBoxes.begin(), boundingBoxes.end());
        //groupAChildren.insert(groupAChildren.end(), children.begin(), children.end());

        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            groupABoundingBoxes.emplace_back(boundingBoxes[i]);
            groupAChildren.emplace_back(children[i]);
        }
    }
    else if (groupBBoundingBoxes.size() + cur_offset_ == min_branch_factor)
    {
        //groupBBoundingBoxes.insert(groupBBoundingBoxes.end(), boundingBoxes.begin(), boundingBoxes.end());
        //groupBChildren.insert(groupBChildren.end(), children.begin(), children.end());

        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            groupBBoundingBoxes.emplace_back(boundingBoxes[i]);
            groupBChildren.emplace_back(children[i]);
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
        assert(allocator->get_tree_node<NodeType>(children[i])->parent == self_handle_);
    }
#endif
    newSibling->moveChildren(groupBChildren, groupBBoundingBoxes);
    for (unsigned i = 0; i < newSibling->cur_offset_; i++)
    {
        allocator->get_tree_node<NodeType>(newSibling->children[i])->parent = newSiblingHandle;
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
    data[cur_offset_data_] = newData;
    cur_offset_data_++;
    double dataSize = cur_offset_data_;

    // Compute the first entry in each group based on PS1 & PS2
    unsigned seedA = 0;
    unsigned seedB = dataSize - 1;

    // This rectangle drank too much and represents how wasted iData and jData are
    double maxWasted = 0.0;

    // QS1 [Pick entry for each group]
    Point iData, jData;
    for (unsigned i = 0; i < dataSize; ++i)
    {
        iData = data[i];
        for (unsigned j = 0; j < dataSize; ++j)
        {
            jData = data[j];

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
    Rectangle boundingBoxA = Rectangle(data[seedA], data[seedA]);
    Rectangle boundingBoxB = Rectangle(data[seedB], data[seedB]);

    // seedA and seedB have both already been allocated so put them into the appropriate group
    // and remove them from our data being careful to delete the one which will not affect the
    // index of the other first
    groupAData.push_back(data[seedA]);
    groupBData.push_back(data[seedB]);
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
    for (; cur_offset_data_ > 0 && (groupAData.size() + cur_offset_data_ > min_branch_factor) && (groupBData.size() + cur_offset_data_ > min_branch_factor);)
    {
        // PN1 [Determine the cost of putting each entry in each group]
        unsigned groupAIndex = 0;
        double groupAMin = std::numeric_limits<double>::infinity();
        unsigned groupBIndex = 0;
        double groupBMin = std::numeric_limits<double>::infinity();

        for (unsigned i = 0; i < cur_offset_data_; ++i)
        {
            groupAAffinity = boundingBoxA.computeExpansionArea(data[i]);
            groupBAffinity = boundingBoxB.computeExpansionArea(data[i]);
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
                boundingBoxA.expand(data[groupAIndex]);
                moveData(groupAIndex, groupAData);
            }
            else
            {
                // Better area or in the worst case an arbitrary choice
                boundingBoxB.expand(data[groupBIndex]);
                moveData(groupBIndex, groupBData);
            }
        }
        else if (groupAMin < groupBMin)
        {
            // Higher affinity for groupA
            boundingBoxA.expand(data[groupAIndex]);
            moveData(groupAIndex, groupAData);
        }
        else
        {
            // Higher affinity for groupB
            boundingBoxB.expand(data[groupBIndex]);
            moveData(groupBIndex, groupBData);
        }
    }

    // If we stopped because half the entries were assigned then great put the others in the
    // opposite group
    if (groupAData.size() + cur_offset_data_ == min_branch_factor)
    {
        for (unsigned i = 0; i < cur_offset_data_; ++i)
        {
            groupAData.emplace_back(data[i]);
        }
        //groupAData.insert(groupAData.end(), data.begin(), data.end());
    }
    else if (groupBData.size() + cur_offset_data_ == min_branch_factor)
    {
        for (unsigned i = 0; i < cur_offset_data_; ++i)
        {
            groupBData.emplace_back(data[i]);
        }
        //groupBData.insert(groupBData.end(), data.begin(), data.end());
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
    tree_node_allocator *allocator = get_node_allocator(treeRef); // Helper functions

    // AT1 [Initialize]
    auto node = allocator->get_tree_node<NodeType>(self_handle_);

    for (;;)
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
                if (parentNode->cur_offset_ < max_branch_factor)
                {
                    parentNode->children[parentNode->cur_offset_] = siblingNode->self_handle_;
                    parentNode->boundingBoxes[parentNode->cur_offset_] = siblingNode->boundingBox();
                    parentNode->cur_offset_++;
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
    tree_node_allocator *allocator = get_node_allocator(treeRef); // Helper functions
    pinned_node_ptr<NodeType> leaf = allocator->get_tree_node<NodeType>(chooseLeaf(givenPoint));
    tree_node_handle siblingLeaf = tree_node_handle(nullptr);

    // I2 [Add record to leaf node]
    if (leaf->cur_offset_data_ < max_branch_factor)
    {
        leaf->data[leaf->cur_offset_data_] = givenPoint;
        leaf->cur_offset_data_++;
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

        newRoot->boundingBoxes[newRoot->cur_offset_] = this->boundingBox();
        newRoot->children[newRoot->cur_offset_] = self_handle_;
        newRoot->cur_offset_++;

        siblingNode->parent = newRoot->self_handle_;
        newRoot->boundingBoxes[newRoot->cur_offset_] = siblingNode->boundingBox();
        newRoot->children[newRoot->cur_offset_] = siblingNode->self_handle_;
        newRoot->cur_offset_++;

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
        node->boundingBoxes[node->cur_offset_] = e.boundingBox;
        node->children[node->cur_offset_] = e.child;
        node->cur_offset_++;
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

        newRoot->boundingBoxes[newRoot->cur_offset_] = boundingBox();
        newRoot->children[newRoot->cur_offset_] = self_handle_;
        newRoot->cur_offset_++;

        auto siblingPtr = allocator->get_tree_node<NodeType>(siblingNode);
        siblingPtr->parent = newRoot->self_handle_;
        newRoot->boundingBoxes[newRoot->cur_offset_] = siblingPtr->boundingBox();
        newRoot->children[newRoot->cur_offset_] = siblingNode;
        newRoot->cur_offset_++;

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
    unsigned nodeBoundingBoxesSize, nodeDataSize;
    while (node->parent != nullptr)
    {
        nodeBoundingBoxesSize = node->cur_offset_;
        nodeDataSize = node->cur_offset_data_;
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
                e.data = node->data[i];
                e.level = 0;
                Q.push_back(e);
            }
            for (unsigned i = 0; i < nodeBoundingBoxesSize; ++i)
            {
                ReinsertionEntry e = {};
                e.boundingBox = node->boundingBoxes[i];
                e.child = node->children[i];
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
    if (root->cur_offset_ == 1)
    {
        auto firstChild = allocator->get_tree_node<NodeType>(root->children[0]);
        firstChild->parent = tree_node_handle(nullptr);
        return root->children[0];
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

    if (parent != expectedParent || cur_offset_ > max_branch_factor || cur_offset_data_ > max_branch_factor)
    {
        std::cout << "node = " << this->self_handle_ << std::endl;
        std::cout << "parent = " << parent << " expectedParent = " << expectedParent << std::endl;
        std::cout << "max_branch_factor = " << max_branch_factor << std::endl;
        std::cout << "cur_offset_ = " << cur_offset_ << std::endl;
        std::cout << "cur_offset_data_ = " << cur_offset_data_ << std::endl;
        assert(allocator->get_tree_node<NodeType>(parent) == allocator->get_tree_node<NodeType>(expectedParent));
    }

    if (expectedParent != nullptr)
    {
        auto parentNode = allocator->get_tree_node<NodeType>(parent);
        for (unsigned i; i < cur_offset_data_; i++)
        {
            Point &dataPoint = data[i];

            if (!parentNode->boundingBoxes[index].containsPoint(dataPoint))
            {
                auto parentPtr = allocator->get_tree_node<NodeType>(parent);
                std::cout << parentPtr->boundingBoxes[index] << " fails to contain " << dataPoint << std::endl;
                assert(parentPtr->boundingBoxes[index].containsPoint(dataPoint));
            }
        }
    }

    bool valid = true;
    for (unsigned i = 0; i < children.size(); ++i)
    {
        valid = valid && allocator->get_tree_node<NodeType>(children[i])->validate(this->self_handle_, i);
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
    for (unsigned i = 0; i < cur_offset_; ++i)
    {
        std::cout << indendtation << "		" << boundingBoxes[i] << std::endl;
    }
    std::cout << std::endl
              << indendtation << "    Children: ";
    for (unsigned i = 0; i < cur_offset_; ++i)
    {
        std::cout << children[i] << ' ';
    }
    std::cout << std::endl
              << indendtation << "    Data: ";
    for (unsigned i = 0; i < cur_offset_data_; ++i)
    {
        std::cout << data[i];
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
    for (unsigned i = 0; i < cur_offset_; ++i)
    {
        std::cerr << indendtation << "		" << boundingBoxes[i] << std::endl;
    }
    std::cerr << std::endl
              << indendtation << "    Children: ";
    for (unsigned i = 0; i < cur_offset_; ++i)
    {
        std::cerr << children[i] << ' ';
    }
    std::cerr << std::endl
              << indendtation << "    Data: ";
    for (unsigned i = 0; i < cur_offset_data_; ++i)
    {
        std::cerr << data[i];
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
    if (cur_offset_ > 0)
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            allocator->get_tree_node<NodeType>(children[i])->printTreeErr(n + 1);
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
    if (cur_offset_ > 0)
    {
        for (unsigned i = 0; i < cur_offset_; ++i)
        {
            // Recurse
            allocator->get_tree_node<NodeType>(children[i])->printTree(n + 1);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
unsigned Node<min_branch_factor, max_branch_factor>::checksum()
{
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);

    unsigned sum = 0;

    if (cur_offset_ == 0)
    {
        for (unsigned i = 0; i < cur_offset_data_; ++i)
        {
            for (unsigned d = 0; d < dimensions; ++d)
            {
                sum += (unsigned)data[i][d];
            }
        }
    }
    else
    {
        for (unsigned i = 0; i < boundingBoxes.size(); ++i)
        {
            // Recurse
            sum += allocator->get_tree_node<NodeType>(children[i])->checksum();
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

    for (;;)
    {
        ret++;
        if (node->cur_offset_ == 0)
        {
            return ret;
        }
        else
        {
            node = allocator->get_tree_node<NodeType>(node->children[0]);
        }
    }
}

template <int min_branch_factor, int max_branch_factor>
void Node<min_branch_factor, max_branch_factor>::stat()
{
#ifdef STAT
    using NodeType = Node<min_branch_factor, max_branch_factor>;
    tree_node_allocator *allocator = get_node_allocator(treeRef);
    unsigned long childrenSize;
    unsigned long dataSize;
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

    for (; !context.empty();)
    {
        currentContext = allocator->get_tree_node<NodeType>(context.top());
        context.pop();

        childrenSize = currentContext->cur_offset_;
        dataSize = currentContext->cur_offset_data_;
        unsigned fanout = childrenSize == 0 ? dataSize : childrenSize;
        if (unlikely(fanout >= histogramFanout.size()))
        {
            histogramFanout.resize(2 * fanout, 0);
        }
        ++histogramFanout[fanout];

        // Compute the overlap and coverage of our children
        for (unsigned i = 0; i < currentContext->cur_offset_; ++i)
        {
            coverage += currentContext->boundingBoxes[i].area();

            for (unsigned j = 0; j < currentContext->cur_offset_; ++j)
            {
                if (i != j)
                {
                    overlap += currentContext->boundingBoxes[i].computeIntersectionArea(currentContext->boundingBoxes[j]);
                }
            }
        }

        if (childrenSize == 0 && dataSize > 0)
        {
            ++totalLeaves;
            memoryFootprint += sizeof(Node) + currentContext->cur_offset_data * sizeof(Point);
        }
        else
        {
            totalNodes += childrenSize;
            memoryFootprint += sizeof(Node) + childrenSize * sizeof(Node *) + currentContext->boundingBoxes.size() * sizeof(Rectangle);
            // Determine which branches we need to follow
            for (unsigned i = 0; i < currentContext->cur_offset_; ++i)
            {
                auto child = allocator->get_tree_node<NodeType>(currentContext->children[i]);
                if (child->cur_offset_ == 1 || child->cur_offset_data_ == 1)
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
