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
    bb = std::get<Branch>( entries.at(0) ).boundingPoly.boundingBox;
    for( size_t i = 1; i < cur_offset_; i++ ) {
        bb.expand( std::get<Branch>( entries.at(i) ).boundingPoly.boundingBox );
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
        Branch &b = std::get<Branch>( entries[ found_index ] );
        if( b.child == entry ) {
            break;
        }
        found_index++;
    }
    entries[ found_index ] = entries[ cur_offset_-1];
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
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( b.boundingPoly.containsPoint(requestedPoint) ) {
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
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( b.boundingPoly.intersectsRectangle( requestedRectangle ) ) {
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

    for( ;; ) {
        auto cur_node = allocator->get_tree_node<NodeType>(
                cur_node_handle );

        // CL2 [Leaf check]
        if( cur_node->isLeaf() ) {
            return cur_node_handle;
        } else {
            // Compute the smallest expansion
            unsigned smallestExpansionBranchIndex = 0;
            InlineBoundedIsotheticPolygon::OptimalExpansion smallestExpansion =
                std::get<Branch>( cur_node->entries.at(0) ).boundingPoly.computeExpansionArea(givenPoint);
            InlineBoundedIsotheticPolygon::OptimalExpansion evalExpansion;
            for( size_t i = 1; i < cur_node->cur_offset_ &&
                    smallestExpansion.area != -1.0; i++ ) {
                evalExpansion = std::get<Branch>(
                        cur_node->entries.at(i) ).boundingPoly.computeExpansionArea(givenPoint);
                if( evalExpansion.area < smallestExpansion.area && evalExpansion.area != 0.0 ) {
                    smallestExpansionBranchIndex = i;
                    smallestExpansion = evalExpansion;
                }
            }

            if( smallestExpansion.area != -1.0 ) {
                InlineBoundedIsotheticPolygon subsetPolygon( std::get<Branch>(
                            cur_node->entries.at(smallestExpansionBranchIndex) ).boundingPoly.basicRectangles[smallestExpansion.index]);
                subsetPolygon.expand(givenPoint);

                for( size_t i = 0; i < cur_node->cur_offset_; i++ ) {
                    if( i != smallestExpansionBranchIndex ) {
                        subsetPolygon.increaseResolution( givenPoint,
                                std::get<Branch>( cur_node->entries.at(i) ).boundingPoly);
                    }
                }

                if( cur_node->parent != nullptr ) {
                    auto parent =
                        allocator->get_tree_node<NodeType>(
                                cur_node->parent );
                    subsetPolygon.intersection(std::get<Branch>(
                                parent->entries.at(enclosingPolyIndex)).boundingPoly);
                }

                Branch &b = std::get<Branch>(
                        cur_node->entries.at(smallestExpansionBranchIndex) );

                b.boundingPoly.remove(smallestExpansion.index);
                b.boundingPoly.merge(subsetPolygon);

                auto b_child =
                    allocator->get_tree_node<NodeType>( b.child );

                if( b_child->isLeaf() and b_child->cur_offset_ > 0 ) {
                    b_child->entries.at( b_child->cur_offset_ ) = givenPoint;
                    b_child->cur_offset_++;
                    b.boundingPoly.shrink( b_child->entries.begin(),
                            b_child->entries.begin() +
                            b_child->cur_offset_ );
                    b_child->cur_offset_--;
                }
                b.boundingPoly.refine();
            }

            // Descend
            Branch &b = std::get<Branch>(
                    cur_node->entries.at(smallestExpansionBranchIndex) );
            cur_node_handle = b.child;
            enclosingPolyIndex = smallestExpansionBranchIndex;
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
                if( b.boundingPoly.containsPoint(givenPoint) ) {
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
        std::cout << "Choosing parition location for branch node." <<
            std::endl;
        Point centreOfMass = Point::atOrigin;
        std::vector<Rectangle> sortable;
        double location;

        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            sortable.insert(sortable.end(),
                    b.boundingPoly.basicRectangles.begin(),
                    b.boundingPoly.basicRectangles.begin() +
                    b.boundingPoly.rectangle_count_ );
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

// Splitting a node will remove it from its parent node and its memory will be freed
template <int min_branch_factor, int max_branch_factor>
SplitResult
Node<min_branch_factor,max_branch_factor>::splitNode(
    Partition p
) {
    std::cout << "Splitting Node!" << std::endl;
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    // Initialize our context stack
    InlineBoundedIsotheticPolygon referencePoly;
    if( parent != nullptr ) {
        auto parent_node =
            allocator->get_tree_node<NodeType>( parent );
        referencePoly = parent_node->locateBranch(this->self_handle_).boundingPoly;
    } else {
        referencePoly = InlineBoundedIsotheticPolygon(boundingBox());
        std::cout << "Constructed referencePoly: " << referencePoly <<
            std::endl;
    }

    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first;
    new (&(*left_node)) NodeType( treeRef, parent, left_handle );

    alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first;
    new (&(*right_node)) NodeType( treeRef, parent, right_handle );


    SplitResult split = {{referencePoly, left_handle}, {referencePoly,
        right_handle}};

    split.leftBranch.boundingPoly.maxLimit(p.location, p.dimension);
    split.rightBranch.boundingPoly.minLimit(p.location, p.dimension);

    split.leftBranch.boundingPoly.refine();
    split.rightBranch.boundingPoly.refine();

    if( isLeaf() ) {
        std::cout << "We are splitting a leaf." << std::endl;
        bool containedLeft, containedRight;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &dataPoint = std::get<Point>( entries.at(i) );
            containedLeft = split.leftBranch.boundingPoly.containsPoint( dataPoint );
            containedRight = split.rightBranch.boundingPoly.containsPoint( dataPoint );

            if( containedLeft && !containedRight ) {
                std::cout << "containedLeft, !containedRight" <<
                    std::endl;
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else if( !containedLeft && containedRight ) {
                std::cout << "!containedLeft, containedRight" <<
                    std::endl;
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            } else if( left_node->cur_offset_ < right_node->cur_offset_
                    ) {
                std::cout << "Both contained, going left" << std::endl;
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else {
                std::cout << "Both contained, going right" << std::endl;
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            }
        }
        cur_offset_ = 0;

        split.leftBranch.boundingPoly.shrink( left_node->entries.begin(),
                left_node->entries.begin() + left_node->cur_offset_ );
        split.rightBranch.boundingPoly.shrink(
                right_node->entries.begin(), right_node->entries.begin() +
                right_node->cur_offset_ );
    } else {
        std::cout << "Splitting Branch Node." << std::endl;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );
            if( branch.boundingPoly.boundingBox.upperRight[p.dimension] <= p.location) {
                std::cout << "Added branch left." << std::endl;
                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );
                child->parent = split.leftBranch.child;
                assert( split.leftBranch.child == left_handle );
                left_node->entries.at( left_node->cur_offset_++ ) =
                    branch;
            } else if (branch.boundingPoly.boundingBox.lowerLeft[p.dimension] >= p.location) {
                std::cout << "Added branch right." << std::endl;
                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );

                child->parent = split.rightBranch.child;
                assert( split.rightBranch.child == right_handle );
                right_node->entries.at( right_node->cur_offset_++ ) = branch;
            } else {
                std::cout << "Downward split!" << std::endl;
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
        std::cout << "isroot? " << (treeRef->root == current_handle) <<
            std::endl;
        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            std::cout << "Handling a split!" << std::endl;
            std::cout << "Node currently has " <<
                current_node->cur_offset_ << " entries!" << std::endl;
            {
                auto left_node =
                    allocator->get_tree_node<NodeType>(
                            propagationSplit.leftBranch.child );
                std::cout << "Left_node has cur_offset" <<
                    left_node->cur_offset_ << std::endl;
                if( left_node->cur_offset_ > 0 ) {
                    current_node->entries[ current_node->cur_offset_++ ] =
                        propagationSplit.leftBranch;
                }
            }
            {
                auto right_node =
                    allocator->get_tree_node<NodeType>(
                            propagationSplit.rightBranch.child );

                std::cout << "Right_node has cur_offset" <<
                    right_node->cur_offset_ << std::endl;
                if( right_node->cur_offset_  > 0  ) {
                    current_node->entries[ current_node->cur_offset_++ ] =
                        propagationSplit.rightBranch;
                }
            }
            std::cout << "Now Node has " << current_node->cur_offset_ <<
                " entries." << std::endl;
        }

        // Early exit if this node does not overflow
        if( current_node->cur_offset_ <= max_branch_factor ) {
            std::cout << "Do not need to split." << std::endl;
            propagationSplit = {
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
            };
            break;
        }

        std::cout << "Need to split!" << std::endl;
        std::cout << "isroot? " << (treeRef->root == current_handle) <<
            std::endl;


        // Otherwise, split node
        propagationSplit = current_node->splitNode();

        // Cleanup before ascending
        if( current_node->parent != nullptr ) {
            auto parent_node =
                allocator->get_tree_node<NodeType>(
                        current_node->parent );
            std::cout << "Removing node " << current_handle << " from parent" << current_node->parent << std::endl;
            parent_node->removeEntry( current_handle );
        }

        // Ascend, propagating splits
        std::cout << "Going up, taking left node." << std::endl;
        auto left_node =
            allocator->get_tree_node<NodeType>(
                    propagationSplit.leftBranch.child );
        std::cout << "Left Node parent is: " << left_node->parent <<
            std::endl;
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
        current_node->entries[ current_node->cur_offset_++ ] =
            givenPoint;
    }


    SplitResult finalSplit = current_node->adjustTree();


    // Grow the tree taller if we need to
    if( finalSplit.leftBranch.child != nullptr and finalSplit.rightBranch.child != nullptr ) {
        std::cout << "Growing tree." << std::endl;
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
        new_root_node->entries[ new_root_node->cur_offset_++ ] =
            finalSplit.leftBranch;

        auto right_node =
            allocator->get_tree_node<NodeType>(
                    finalSplit.rightBranch.child );

        right_node->parent = new_root_handle;
        new_root_node->entries[ new_root_node->cur_offset_++ ] =
            finalSplit.rightBranch;

        // FIXME: GC original root node
        //delete this;

        std::cout << "Created new root node: " << new_root_handle << std::endl;

        return new_root_handle;
    }
    std::cout << "Not Growing Tree." << std::endl;

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
    if( cur_offset_ == 1 and std::holds_alternative<Branch>( entries[0]
                ) ) {
        tree_node_handle new_root_handle = std::get<Branch>( entries[0]
                ).child;
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
            Point &dataPoint = std::get<Point>( entries[i] );
            for( unsigned d = 0; d < dimensions; d++ ) {
                sum += (unsigned) dataPoint[d];
            }
        }
    }
    else {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( size_t i = 0; i < cur_offset_; i++ ) {
            // Recurse
            Branch &branch = std::get<Branch>( entries[i] );
            auto child =
                allocator->get_tree_node<NodeType>(
                        branch.child );
            sum += child->checksum();
        }
    }

    return sum;
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor,max_branch_factor>::validate( tree_node_handle expectedParent, unsigned index) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    if( parent != expectedParent || cur_offset_ > max_branch_factor ) {
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
                    parent_node->entries[index] );

            for( size_t i = 0; i < cur_offset_; i++ ) {
                Point &dataPoint = std::get<Point>( entries[i] );
                if( !branch.boundingPoly.containsPoint( dataPoint ) ) {
                    std::cout << branch.boundingPoly << " fails to contain " << dataPoint << std::endl;
                    assert( false );
                }
            }

            return true;
        }

        for( size_t i = 0; i < cur_offset_; i++ ) {
            for( size_t j = 0; j < cur_offset_; j++ ) {
                if( i != j ) {
                    Branch &b_i = std::get<Branch>( entries[i] );
                    Branch &b_j = std::get<Branch>( entries[j] );
                    if( b_i.boundingPoly.disjoint(b_j.boundingPoly) ) {
                        std::cout << "Branch " << i << " is not disjoint from sibling Branch " << j << std::endl;
                        std::cout << "branches[" << i << "].boundingPoly = " << b_i.boundingPoly << std::endl;
                        std::cout << "branches[" << j << "].boundingPoly = " << b_j.boundingPoly << std::endl;
                        assert( b_i.boundingPoly.disjoint( b_j.boundingPoly) );
                    }
                }
            }
        }

        bool valid = true;
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries[i] );
            auto child =
                allocator->get_tree_node<NodeType>( b.child );
            valid = valid and child->validate( self_handle_, i );
        }

        return valid;
    }
    return true;
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
            Point &dataPoint = std::get<Point>( entries[i] );
            std::cout << dataPoint;
        }
    } else {
        std::cout << indendtation << "    Branches: " << std::endl;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries[i] );
            std::cout << indendtation << "		" << branch.child << std::endl;
            std::cout << indendtation << "		" << branch.boundingPoly << std::endl;
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
            Branch &branch = std::get<Branch>( entries[i] );
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
        current_handle = std::get<Branch>( node->entries[0] ).child;
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
                Branch &b = std::get<Branch>( current_node->entries[i]
                        );
                coverage += b.boundingPoly.area();
            }

            totalNodes += current_node->cur_offset_;
            memoryFootprint += sizeof(Node) + current_node->cur_offset_ * sizeof(Node::Branch);
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries[i]
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
