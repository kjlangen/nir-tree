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
    AbstractIsotheticPolygon *poly;
    tree_node_allocator *allocator = get_node_allocator( treeRef );
    pinned_node_ptr<InlineUnboundedIsotheticPolygon> poly_pin(
            allocator->buffer_pool_, nullptr, nullptr );

    Branch &b = std::get<Branch>(entries.at(0));
    if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                b.boundingPoly ) ) {
        poly = &(std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly )
            );
    } else {
        tree_node_handle poly_handle = std::get<tree_node_handle>( b.boundingPoly );
        poly_pin =
            allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
        poly = &(*poly_pin);
    }
    bb = poly->getBoundingBox();
    for( size_t i = 1; i < cur_offset_; i++ ) {
        Branch &b2 = std::get<Branch>(entries.at(i));
        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                    b2.boundingPoly ) ) {
            poly = &(std::get<InlineBoundedIsotheticPolygon>( b2.boundingPoly )
                );
        } else {
            tree_node_handle poly_handle = std::get<tree_node_handle>(
                    b2.boundingPoly );
            poly_pin =
                allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                        poly_handle );
            poly = &(*poly_pin);
        }

        bb.expand( poly->getBoundingBox() );
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
        Branch &b = std::get<Branch>( entries.at( found_index ) );
        if( b.child == entry ) {
            break;
        }
        found_index++;
    }
    entries.at( found_index ) = entries.at( cur_offset_-1 );
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
                AbstractIsotheticPolygon *poly;
                pinned_node_ptr<InlineUnboundedIsotheticPolygon>
                    poly_pin( allocator->buffer_pool_, nullptr, nullptr );
                Branch &b = std::get<Branch>( current_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    poly = &(std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly ) );
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    poly_pin =
                        allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                poly_handle );
                   
                    poly = &(*poly_pin);
                }
                if( poly->containsPoint( requestedPoint) ) {
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
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    InlineBoundedIsotheticPolygon &poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly );
                    if( poly.intersectsRectangle( requestedRectangle ) ) {
                        // Add to the nodes we will check
                        context.push( b.child );
                    }
                } else {
                    assert( false ); // FIXME: out of band poly
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

    assert( cur_node_handle != nullptr );

    for( ;; ) {
        assert( cur_node_handle != nullptr );
        auto cur_node = allocator->get_tree_node<NodeType>(
                cur_node_handle );

        // CL2 [Leaf check]
        if( cur_node->isLeaf() ) {
            return cur_node_handle;
        } else {
            // Compute the smallest expansion
            assert( cur_node->cur_offset_ > 0 );

            unsigned smallestExpansionBranchIndex = 0;

            tree_node_allocator *allocator = get_node_allocator( treeRef );
            AbstractIsotheticPolygon *node_poly;
            pinned_node_ptr<InlineUnboundedIsotheticPolygon> node_pin(
                    allocator->buffer_pool_, nullptr, nullptr );


            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(std::get<Branch>(
                    cur_node->entries.at(0)).boundingPoly ) ) {
                node_poly =
                    &(std::get<InlineBoundedIsotheticPolygon>(std::get<Branch>(
                        cur_node->entries.at(0) ).boundingPoly )
                );
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>(std::get<Branch>(
                    cur_node->entries.at(0)).boundingPoly );
                node_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>
                    ( poly_handle );
                node_poly = &(*node_pin);
            }

            assert( node_poly->end() - node_poly->begin() > 0 );

            AbstractIsotheticPolygon::OptimalExpansion smallestExpansion =
                node_poly->computeExpansionArea(givenPoint);
            AbstractIsotheticPolygon::OptimalExpansion evalExpansion;

            for( size_t i = 1; i < cur_node->cur_offset_ &&
                    smallestExpansion.area != -1.0; i++ ) {
                Branch &b = std::get<Branch>( cur_node->entries.at(i) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    node_poly =
                        &(std::get<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly ));
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    node_pin =
                        allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                poly_handle );
                    node_poly = &(*node_pin);
                }

                assert( node_poly->end() - node_poly->begin() > 0 );

                evalExpansion = node_poly->computeExpansionArea(givenPoint);
                if( evalExpansion.area < smallestExpansion.area && evalExpansion.area != 0.0 ) {
                    smallestExpansionBranchIndex = i;
                    smallestExpansion = evalExpansion;
                }
            }


            if( smallestExpansion.area != -1.0 ) {
                // FIXME: out of band poly
                Branch &b = std::get<Branch>( cur_node->entries.at(
                    smallestExpansionBranchIndex ) );
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                      b.boundingPoly ) ) {
                    node_poly =
                        &(std::get<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly ) );
                } else {
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b.boundingPoly );
                    node_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    node_poly = &(*node_pin);
                }

                assert( node_poly->end() - node_poly->begin() > 0 );

                // N.B., we use malloc because
                // InlineUnboundedIsotheticPolygon
                // is allocated using malloc, and this pointer may
                // be changed to point to such an object later
                AbstractIsotheticPolygon *subsetPolygon =
                    (InlineBoundedIsotheticPolygon *) malloc( sizeof(
                                InlineBoundedIsotheticPolygon ) );
                new (subsetPolygon) InlineBoundedIsotheticPolygon(
                     ((InlineBoundedIsotheticPolygon *) node_poly)->basicRectangles[smallestExpansion.index]);

                assert( subsetPolygon->end() - subsetPolygon->begin() > 0 );

                // FIXME: start here
                subsetPolygon->expand(givenPoint);

                for( size_t i = 0; i < cur_node->cur_offset_; i++ ) {
                    if( i != smallestExpansionBranchIndex ) {
                        Branch &b = std::get<Branch>(
                                cur_node->entries.at(i) );
                        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                                    b.boundingPoly ) ) {
                            node_poly =
                                &(std::get<InlineBoundedIsotheticPolygon>(b.boundingPoly));
                        } else {
                            tree_node_handle poly_handle =
                                std::get<tree_node_handle>(
                                        b.boundingPoly );
                            node_pin =
                                allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                        poly_handle );

                            node_poly = &(*node_pin);
                        }

                        assert( node_poly->end() - node_poly->begin() > 0 );
                        AbstractIsotheticPolygon *modified_poly =
                            subsetPolygon->increaseResolution(
                                    givenPoint, *node_poly );

                        if( modified_poly != subsetPolygon ) {
                            // We overflowed, and are now an
                            // InlineUnboundedIsotheticPolygon
                            // Free old memory and use new poly
                            free( subsetPolygon );
                            subsetPolygon = modified_poly;
                        }
                    }
                }

                if( cur_node->parent != nullptr ) {
                    auto parent =
                        allocator->get_tree_node<NodeType>(
                                cur_node->parent );
                    Branch &b = std::get<Branch>(
                            parent->entries.at(enclosingPolyIndex));
                    if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly ) ) {
                        node_poly =
                            &(std::get<InlineBoundedIsotheticPolygon>(
                                        b.boundingPoly ) );
                    } else {
                        tree_node_handle poly_handle =
                            std::get<tree_node_handle>( b.boundingPoly
                                    );
                        node_pin =
                            allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                                    poly_handle );
                        node_poly = &(*node_pin);
                    }
                    assert( node_poly->end() - node_poly->begin() > 0 );

                    AbstractIsotheticPolygon *modified_poly =
                        subsetPolygon->intersection( *node_poly );
                    // We overflowed, and are now an
                    // InlineUnboundedIsotheticPolygon
                    // Free old memory and use new poly
                    if( modified_poly != subsetPolygon ) {
                        free( subsetPolygon );
                        subsetPolygon = modified_poly;
                    }
                }

                Branch &b2 = std::get<Branch>(
                        cur_node->entries.at(smallestExpansionBranchIndex) );

                // If existing poly is inline in the page
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b2.boundingPoly ) ) {

                    node_poly = &(std::get<InlineBoundedIsotheticPolygon>(
                                b2.boundingPoly ));
                } else {
                    // Otherwise, read the special page with the polygon
                    tree_node_handle poly_handle =
                        std::get<tree_node_handle>( b2.boundingPoly );
                    // pull pin ptr out of scope so this doesn't get
                    // immediately unpinned
                    node_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                    node_poly = &(*node_pin);
                }
                assert( node_poly->end() - node_poly->begin() > 0 );

                node_poly->remove(smallestExpansion.index);

                AbstractIsotheticPolygon *modified_poly =
                    node_poly->merge( *subsetPolygon );

                free( subsetPolygon );

                // If we have the same address, then we just updated the
                // poly inline, carry on.
                if( modified_poly != node_poly ) {
                    // Address is different, so we overflowed
                    // This puts InlineUnboundedIsotheticPolygon
                    // on the heap. Need to copy this poly to a
                    // valid page location.
                    
                    std::cout << "Allocating unbounded poly on disk" <<
                        std::endl;
                    auto alloc_data = allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                            compute_sizeof_inline_unbounded_polygon(
                                modified_poly->end() -
                                modified_poly->begin() ) );

                    new (&(*(alloc_data.first)))
                        InlineUnboundedIsotheticPolygon(
                                // Downcast, we know we overflowed
                                * (InlineUnboundedIsotheticPolygon *) modified_poly );


                    // Don't need our temporary memory anymore
                    free( modified_poly );
                    
                    // Point to page position where we copied our
                    // object
                    node_poly  = &(*(alloc_data.first));
                    assert( node_poly->end() - node_poly->begin() > 0 );

                    // Put the handle of what we allocated in the
                    // branch. It is out of line now.
                    b.boundingPoly = alloc_data.second;

                } // update out of line
                assert( node_poly->end() - node_poly->begin() > 0 );

                auto b_child =
                    allocator->get_tree_node<NodeType>( b.child );

                if( b_child->isLeaf() and b_child->cur_offset_ > 0 ) {
                    b_child->entries.at( b_child->cur_offset_ ) = givenPoint;
                    b_child->cur_offset_++;
                    node_poly->shrink( b_child->entries.begin(),
                            b_child->entries.begin() +
                            b_child->cur_offset_ );
                    b_child->cur_offset_--;
                    for( unsigned i = 0; i < b_child->cur_offset_; i++ ) {
                        assert( std::holds_alternative<Point>(
                                    b_child->entries.at(i) ) or
                                std::holds_alternative<Branch>(
                                    b_child->entries.at(i) ) );
                    }
                }
                node_poly->refine();
                assert( node_poly->end() - node_poly->begin() > 0 );
            }

            assert( cur_node->cur_offset_ > smallestExpansionBranchIndex  );

            // Descend
            Branch &b = std::get<Branch>(
                    cur_node->entries.at(smallestExpansionBranchIndex) );
            cur_node_handle = b.child;
            enclosingPolyIndex = smallestExpansionBranchIndex;
            assert( cur_node_handle != nullptr );
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
                if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) ) {
                    InlineBoundedIsotheticPolygon &poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                b.boundingPoly );
                    if( poly.containsPoint(givenPoint) ) {
                        // Add the child to the nodes we will consider
                        context.push( b.child );
                    }
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
    std::cout << "Choosing partition location for node: " <<
        self_handle_ << std::endl;

    if( isLeaf() ) {
        std::cout << "Node is a leaf. " << std::endl;
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
        std::cout << "Node is a branch. " << std::endl;
        Point centreOfMass = Point::atOrigin;
        std::vector<Rectangle> sortable;
        double location;
        tree_node_allocator *allocator = get_node_allocator( treeRef );

        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            AbstractIsotheticPolygon *poly;
            pinned_node_ptr<InlineUnboundedIsotheticPolygon> poly_pin(
                    allocator->buffer_pool_, nullptr, nullptr );

            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        b.boundingPoly ) ) {
                std::cout << "InlinePoly!" << std::endl;
                poly = &(std::get<InlineBoundedIsotheticPolygon>(
                            b.boundingPoly ) );
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( b.boundingPoly );
                std::cout << "OutoflinePoly!" << std::endl;
                poly_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                poly = &(*poly_pin);
            }
            std::cout << "Poly has: " << poly->end() - poly->begin() <<
                " rectangles." << std::endl;
            sortable.insert(sortable.end(),
                    poly->begin(),
                    poly->end() );
        }
        std::cout << "All poly rectangles added!" << std::endl;
        std::cout << "Total count: " << sortable.size() << std::endl;
        for( Rectangle &boundingBox : sortable ) {
            centreOfMass += boundingBox.lowerLeft;
            centreOfMass += boundingBox.upperRight;
            totalMass += 2.0;
        }

        centreOfMass /= totalMass;

        for( unsigned d = 0; d < dimensions; d++ ) {
            location = centreOfMass[d];
            std::cout << "Center of mass in " << d << " is " << location
                << std::endl;

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

template <int min_branch_factor, int max_branch_factor>
AbstractIsotheticPolygon *Node<min_branch_factor,max_branch_factor>::fix_polygon(
    AbstractIsotheticPolygon *existing_polygon
) {
    AbstractIsotheticPolygon *original_poly = existing_polygon;

    // Hopefully we can reuse this polygon, its on a page somewhere
    existing_polygon->clear();

    tree_node_allocator *allocator = get_node_allocator( treeRef );
    // Dead pin for now, but will be set to keep the poly page pinned
    // in internal scope if needed
    pinned_node_ptr<InlineUnboundedIsotheticPolygon> poly_pin(
            allocator->buffer_pool_, nullptr, nullptr );

    // Walk over all our children's polygons, merge 'em all in
    for( unsigned i = 0; i < cur_offset_; i++ ) {
        Branch &b = std::get<Branch>( entries.at(i) );
        AbstractIsotheticPolygon *branch_poly;

        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                    b.boundingPoly ) ) {
            // It's inline in the pinned node, jsut get the address
            branch_poly = &( std::get<InlineBoundedIsotheticPolygon>(
                        b.boundingPoly ) );
        } else {
            // It's on another page, figure out where, read that page,
            // set the address
            tree_node_handle poly_handle =
                std::get<tree_node_handle>( b.boundingPoly );
            poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
            branch_poly = &(*poly_pin);
        }
        
        // Merge the existing polygon in with our child
        AbstractIsotheticPolygon *new_poly = existing_polygon->merge(
                *branch_poly );

        // We overflowed and put the new poly on the heap
        if( new_poly != existing_polygon ) {

            // The original poly is on a page somewhere
            // So don't free that one
            if( existing_polygon != original_poly ) {
                free( existing_polygon );
            }

            // Update, continue
            existing_polygon = new_poly;
        }
    }
    return existing_polygon;
}

// Splitting a node will remove it from its parent node and its memory will be freed
template <int min_branch_factor, int max_branch_factor>
SplitResult
Node<min_branch_factor,max_branch_factor>::splitNode(
    Partition p
) {
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    tree_node_allocator *allocator = get_node_allocator( treeRef );

    pinned_node_ptr<InlineUnboundedIsotheticPolygon> left_poly_pin(
            allocator->buffer_pool_, nullptr, nullptr );
    AbstractIsotheticPolygon *node_poly;
    InlineBoundedIsotheticPolygon stub_poly;
    tree_node_handle poly_handle( nullptr );

    if( parent != nullptr ) {
        auto parent_node =
            allocator->get_tree_node<NodeType>( parent );
        Branch &parent_branch = parent_node->locateBranch(this->self_handle_);
        if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                    parent_branch.boundingPoly ) ) {

            // Copy because we will unpin parent now
            stub_poly = std::get<InlineBoundedIsotheticPolygon>(
                        parent_branch.boundingPoly );
            node_poly = &stub_poly;
        } else {
            poly_handle = std::get<tree_node_handle>(
                    parent_branch.boundingPoly );
            // Pin is out of scope so that we don't destroy this
            // reference
            left_poly_pin = allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                    poly_handle );
            node_poly = &(*left_poly_pin);
        }
    } else {
        stub_poly = InlineBoundedIsotheticPolygon(boundingBox());
        node_poly = &stub_poly;
    }

    auto alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle left_handle = alloc_data.second;
    auto left_node = alloc_data.first;
    new (&(*left_node)) NodeType( treeRef, parent, left_handle );

    alloc_data = allocator->create_new_tree_node<NodeType>();
    tree_node_handle right_handle = alloc_data.second;
    auto right_node = alloc_data.first;
    new (&(*right_node)) NodeType( treeRef, parent, right_handle );

    std::cout << "Split of " << self_handle_ << " resulted in two handles: " << left_handle << " and " << right_handle << std::endl;


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
        { tree_node_handle(nullptr), tree_node_handle(nullptr) }, 
        { tree_node_handle(nullptr), tree_node_handle(nullptr) } };

    // If these are unbounded polys, then we need to pin the 
    // polygon copy we will create now. The left is already pinned from
    // before
    pinned_node_ptr<InlineUnboundedIsotheticPolygon> right_poly_pin(
            allocator->buffer_pool_, nullptr, nullptr );

    if( !poly_handle ) {
        // Copy the poly twice
        split = {{ * (InlineBoundedIsotheticPolygon *) node_poly, left_handle}, 
            {* (InlineBoundedIsotheticPolygon *) node_poly, right_handle}};

    } else {
        // We need another copy of the InlineUnboundedIsotheticPolygon
        // node on a page somewhere.
        std::cout << "Creating unbounded poly on disk." << std::endl;
        auto alloc_data = allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                compute_sizeof_inline_unbounded_polygon(
                    ((InlineUnboundedIsotheticPolygon *)
                     node_poly)->max_rectangle_count_ ) );
        // Copy
        new (&(*alloc_data.first)) InlineUnboundedIsotheticPolygon(
                *(InlineUnboundedIsotheticPolygon *) node_poly );

        tree_node_handle new_handle = alloc_data.second;

        split = { {poly_handle, left_handle}, {new_handle,
            right_handle} };

        // Set the pin so this doesn't get evicted from memory yet
        right_poly_pin = alloc_data.first;
    }

    AbstractIsotheticPolygon *left_polygon;
    AbstractIsotheticPolygon *right_polygon;
    if( !poly_handle ) { 
        left_polygon =
            &(std::get<InlineBoundedIsotheticPolygon>(split.leftBranch.boundingPoly));
        right_polygon = 
            &(std::get<InlineBoundedIsotheticPolygon>(split.rightBranch.boundingPoly));
    } else {
        left_polygon = &(*left_poly_pin);
        right_polygon = &(*right_poly_pin);
    }
    std::cout << "Produced left polygon (before): " <<
        left_polygon->getBoundingBox() << std::endl;
    std::cout << "Produced right polygon (before): " <<
        right_polygon->getBoundingBox() << std::endl;


    std::cout << "Bounding point " << p.location << " in dimension: " <<
        p.dimension << std::endl;
    // this is the left polygon
    left_polygon->maxLimit( p.location, p.dimension );
    left_polygon->refine();

    // this is the right polygon
    right_polygon->minLimit( p.location, p.dimension );
    right_polygon->refine();

    std::cout << "Produced left polygon: " <<
        left_polygon->getBoundingBox() << std::endl;
    std::cout << "Produced right polygon: " <<
        right_polygon->getBoundingBox() << std::endl;

    if( isLeaf() ) {
        std::cout << "This split was for a leaf node" << std::endl;

        bool containedLeft, containedRight;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Point &dataPoint = std::get<Point>( entries.at(i) );
            containedLeft = left_polygon->containsPoint( dataPoint );
            containedRight = right_polygon->containsPoint( dataPoint );

            if( containedLeft && !containedRight ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else if( !containedLeft && containedRight ) {
                right_node->entries.at( right_node->cur_offset_++ ) =
                    dataPoint;
            } else if( left_node->cur_offset_ < right_node->cur_offset_
                    ) {
                left_node->entries.at( left_node->cur_offset_++ ) =
                    dataPoint;
            } else {
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
        std:: cout << "EXISTING ENTRIES: " << cur_offset_ << std::endl;

        std::cout << "LEFT NODE ENTRIES: " << left_node->cur_offset_ <<
            std::endl;
        std::cout << "RIGHT NODE ENTRIES: " << right_node->cur_offset_ <<
            std::endl;


        cur_offset_ = 0;

        left_polygon->shrink( left_node->entries.begin(),
                left_node->entries.begin() + left_node->cur_offset_ );
        right_polygon->shrink(
                right_node->entries.begin(), right_node->entries.begin() +
                right_node->cur_offset_ );
    } else {
        std::cout << "This is a branch split!" << std::endl;
        std::cout << "Partition Dimension: " << p.dimension << std::endl;
        std::cout << "Partition location: " << p.location << std::endl;

        for( size_t i = 0; i < cur_offset_; i++ ) {
            std::cout << "Operating over branch number: " << i <<
                std::endl;
            Branch &branch = std::get<Branch>( entries.at(i) );

            pinned_node_ptr<InlineUnboundedIsotheticPolygon>
                branch_poly_pin( allocator->buffer_pool_, nullptr,
                        nullptr );
            AbstractIsotheticPolygon *poly;
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly ) ) {
                poly = &(std::get<InlineBoundedIsotheticPolygon>(
                        branch.boundingPoly ));
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( branch.boundingPoly );
                branch_poly_pin =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                poly = &(*branch_poly_pin);
            }

            std::cout << "Got branch polygon: " <<
                poly->getBoundingBox() << std::endl;

            bool is_contained_left =
                poly->getBoundingBox().upperRight[p.dimension]
                <= p.location;
            bool is_contained_right =
                poly->getBoundingBox().lowerLeft[p.dimension]
                >= p.location;
            if( is_contained_left and (not is_contained_right or
                (left_node->cur_offset_ < right_node->cur_offset_ ) ) ){
                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );
                child->parent = split.leftBranch.child;
                assert( split.leftBranch.child == left_handle );
                std::cout << "assigned branch " << i << ", " << branch.child
                    << " to left node."
                    << std::endl;
                left_node->entries.at( left_node->cur_offset_++ ) =
                    branch;
            } else if( is_contained_right and (not is_contained_left or
                    (left_node->cur_offset_ >= right_node->cur_offset_) ) ) {
                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );

                std::cout << "assigned branch " << i << ", " << branch.child
                    <<" to right node."
                    << std::endl;

                child->parent = split.rightBranch.child;
                assert( split.rightBranch.child == right_handle );
                right_node->entries.at( right_node->cur_offset_++ ) = branch;
            } else {

                std::cout << "Branch " << i << "does not neatly fit." <<
                    std::endl;

                auto child =
                    allocator->get_tree_node<NodeType>(
                            branch.child );

                std::cout << "Need to split down!" << std::endl;

                SplitResult downwardSplit = child->splitNode( p );

                std::cout << "Split down done!" << std::endl;

                // FIXME: GC branch.child
                // delete branch.child;

                auto left_child =
                    allocator->get_tree_node<NodeType>(
                            downwardSplit.leftBranch.child );
                if( left_child->cur_offset_ > 0 ) {
                    std::cout << "adding branch to left_side" <<
                        std::endl;
                    left_child->parent = split.leftBranch.child;
                    left_node->entries.at( left_node->cur_offset_++ ) =
                        downwardSplit.leftBranch;
                }
                auto right_child =
                    allocator->get_tree_node<NodeType>(
                            downwardSplit.rightBranch.child );
                if( right_child->cur_offset_ > 0 ) {
                    std::cout << "Adding branch to right side." <<
                        std::endl;
                    right_child->parent = split.rightBranch.child;
                    right_node->entries.at( right_node->cur_offset_++ )
                        = downwardSplit.rightBranch;
                }
            }

            std::cout << "Done branch " << i << std::endl;
        }

        std::cout << "Done all branch loops for " << self_handle_ <<
            std::endl;

        // It is possible that after splitting on the geometric median,
        // we still end up with an overfull node. This can happen
        // everything gets assigned to the left node except for one
        // branch that needs a downward split. That downward split
        // results in a node added to the left and to the right,
        // resulting in an overfull left node.

        // FIXME: presumes sorted to preserve no intersection
        if( left_node->cur_offset_ > max_branch_factor ) {
            // Move last entry to the right
            right_node->entries.at( right_node->cur_offset_++ ) =
                left_node->entries.at( left_node->cur_offset_-1 );
            left_node->cur_offset_--;

        } else if( right_node->cur_offset_ > max_branch_factor ) {
            // Move first entry to the left
            left_node->entries.at( left_node->cur_offset_++ ) =
                right_node->entries.at( right_node->cur_offset_-1 );
            right_node->cur_offset_--;
        }

        assert( left_node->cur_offset_ <= max_branch_factor );
        assert( right_node->cur_offset_ <= max_branch_factor );

        // We used the polygon definitions above to split the data on a
        // partition line. But after the split, the actual boundingBox
        // we have for the underlying nodes is probably different.
        AbstractIsotheticPolygon *fixed_left = left_node->fix_polygon(
                left_polygon );
        // FIXME: decide if we should free existing left_polygon here
        if( fixed_left != left_polygon ) {
            // Need a new node
            auto alloc_data = 
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            ((InlineUnboundedIsotheticPolygon *)
                             fixed_left)->rectangle_count_ ) );
            new (&(*(alloc_data.first)))
                InlineUnboundedIsotheticPolygon( fixed_left->begin(),
                        fixed_left->end() );
            free( fixed_left );
            split.leftBranch.boundingPoly = alloc_data.second;
        }

        AbstractIsotheticPolygon *fixed_right = right_node->fix_polygon(
                right_polygon );
        // FIXME: decide if we should free existing right_polygon here
        if( fixed_right != right_polygon ) {
            // Need a new node
            auto alloc_data = 
                allocator->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                        compute_sizeof_inline_unbounded_polygon(
                            ((InlineUnboundedIsotheticPolygon *)
                             fixed_right)->rectangle_count_ ) );
            new (&(*(alloc_data.first)))
                InlineUnboundedIsotheticPolygon( fixed_right->begin(),
                        fixed_right->end() );
            free( fixed_right );
            split.rightBranch.boundingPoly = alloc_data.second;
        }

        
        std:: cout << "EXISTING ENTRIES: " << cur_offset_ << std::endl;

        std::cout << "LEFT NODE ENTRIES (" << left_handle << "): " << left_node->cur_offset_ <<
            std::endl;
        std::cout << "RIGHT NODE ENTRIES: (" << right_handle << "): " << right_node->cur_offset_ <<
            std::endl;

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
    std::cout << "Adjust tree, starting at node: " << current_handle <<
        std::endl;
    while( current_handle != nullptr ) {
        auto current_node =
            allocator->get_tree_node<NodeType>(
                    current_handle );
        std::cout << "Adjust tree, at handle: " << current_handle <<
            std::endl;

        // If there was a split we were supposed to propagate then propagate it
        if( propagationSplit.leftBranch.child != nullptr and propagationSplit.rightBranch.child != nullptr ) {
            {
                auto left_node =
                    allocator->get_tree_node<NodeType>(
                            propagationSplit.leftBranch.child );
                if( left_node->cur_offset_ > 0 ) {
                    std::cout << "Adding entry to node: " <<
                        current_handle << " has prior count: " <<
                        current_node->cur_offset_ << std::endl;
                    current_node->entries.at(
                            current_node->cur_offset_++ ) =
                        propagationSplit.leftBranch;
                }
            }

            {
                auto right_node =
                    allocator->get_tree_node<NodeType>(
                            propagationSplit.rightBranch.child );

                if( right_node->cur_offset_  > 0 ) {
                    std::cout << "Adding entry to node: " <<
                        current_handle << " has prior count: " <<
                        current_node->cur_offset_ << std::endl;

                    current_node->entries.at( current_node->cur_offset_++ ) =
                        propagationSplit.rightBranch;
                }
            }
            if( current_node->cur_offset_ > 8 ) {
                std::cout << "THIS NODE IS WAY TOO BIG: " <<
                    current_node->cur_offset_ <<
                    std::endl;
                std::cout << current_handle << std::endl;
            }

            for( unsigned i = 0; i < current_node->cur_offset_; i++ ) {
                assert( std::holds_alternative<Point>(
                            current_node->entries.at(i) ) or
                        std::holds_alternative<Branch>(
                            current_node->entries.at(i) ) );
            }

        }

        // Early exit if this node does not overflow
        if( current_node->cur_offset_ <= max_branch_factor ) {
            propagationSplit = {
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)},
                {InlineBoundedIsotheticPolygon(), tree_node_handle(nullptr)}
            };
            break;
        }

        std::cout << "I am supposed to split: " << current_handle <<
            std::endl;
        // Otherwise, split node
        propagationSplit = current_node->splitNode();


        // Cleanup before ascending
        if( current_node->parent != nullptr ) {
            auto parent_node =
                allocator->get_tree_node<NodeType>(
                        current_node->parent );
            std::cout << "Removing entry from parent " <<
                current_node->parent << std::endl;
            std::cout << "Parent entry count: " <<
                parent_node->cur_offset_ << std::endl;

            parent_node->removeEntry( current_handle );

            std::cout << "Parent new entry count: " <<
                parent_node->cur_offset_ << std::endl;

        }

        // Ascend, propagating splits
        auto left_node =
            allocator->get_tree_node<NodeType>(
                    propagationSplit.leftBranch.child );
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

        auto left_node =
            allocator->get_tree_node<NodeType>(
                    finalSplit.leftBranch.child );
        left_node->parent = new_root_handle;
        new_root_node->entries.at( new_root_node->cur_offset_++ ) =
            finalSplit.leftBranch;

        auto right_node =
            allocator->get_tree_node<NodeType>(
                    finalSplit.rightBranch.child );

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
    if( cur_offset_ == 1 and std::holds_alternative<Branch>( entries.at(0)
                ) ) {
        tree_node_handle new_root_handle = std::get<Branch>(
                entries.at(0) ).child;
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
            Point &dataPoint = std::get<Point>( entries.at(i) );
            for( unsigned d = 0; d < dimensions; d++ ) {
                sum += (unsigned) dataPoint[d];
            }
        }
    }
    else {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( size_t i = 0; i < cur_offset_; i++ ) {
            // Recurse
            Branch &branch = std::get<Branch>( entries.at(i) );
            auto child =
                allocator->get_tree_node<NodeType>(
                        branch.child );
            sum += child->checksum();
        }
    }

    return sum;
}

template <int min_branch_factor, int max_branch_factor>
bool Node<min_branch_factor,max_branch_factor>::bounding_box_validate()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    if( !isLeaf() ) {
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( unsigned i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
            Rectangle boundingBox;
            if( std::holds_alternative<InlineBoundedIsotheticPolygon>(
                        b.boundingPoly ) ) {
                auto &boundingPoly = std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
                        );
                boundingBox = boundingPoly.getBoundingBox();
            } else {
                tree_node_handle poly_handle =
                    std::get<tree_node_handle>( b.boundingPoly );
                auto poly_ptr =
                    allocator->get_tree_node<InlineUnboundedIsotheticPolygon>(
                            poly_handle );
                boundingBox = poly_ptr->getBoundingBox();
            }
            tree_node_handle child_handle = b.child;
            auto child = allocator->get_tree_node<NodeType>(
                    child_handle );
            if( child->boundingBox() != boundingBox ) {
                std::cout << "Expected child " << child_handle << " to have box " << boundingBox << " but had: " << child->boundingBox() << std::endl;
            }
            assert( child->boundingBox() == boundingBox );
            child->bounding_box_validate();
        }
    }
    return true;
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
                    parent_node->entries.at(index) );
            auto &poly = std::get<InlineBoundedIsotheticPolygon>(
                    branch.boundingPoly ); //FIXME: out of band poly

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
                    // FIXME: out of band polys
                    auto &b_i_poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                b_i.boundingPoly );
                    auto &b_j_poly =
                        std::get<InlineBoundedIsotheticPolygon>(
                                b_j.boundingPoly );

                    if( b_i_poly.disjoint(b_j_poly) ) {
                        std::cout << "Branch " << i << " is not disjoint from sibling Branch " << j << std::endl;
                        std::cout << "branches[" << i << "].boundingPoly= " << b_i_poly << std::endl;
                        std::cout << "branches[" << j << "].boundingPoly = " << b_j_poly << std::endl;
                        assert( b_i_poly.disjoint( b_j_poly ) );
                    }
                }
            }
        }

        bool valid = true;
        tree_node_allocator *allocator = get_node_allocator( treeRef );
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &b = std::get<Branch>( entries.at(i) );
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
            Point &dataPoint = std::get<Point>( entries.at(i) );
            std::cout << dataPoint;
        }
    } else {
        std::cout << indendtation << "    Branches: " << std::endl;
        for( size_t i = 0; i < cur_offset_; i++ ) {
            Branch &branch = std::get<Branch>( entries.at(i) );
            // FIXME: out of band poly
            auto &poly = std::get<InlineBoundedIsotheticPolygon>(
                    branch.boundingPoly );
            std::cout << indendtation << "		" << branch.child << std::endl;
            std::cout << indendtation << "		" << poly << std::endl;
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
            Branch &branch = std::get<Branch>( entries.at(i) );
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
        current_handle = std::get<Branch>( node->entries.at(0) ).child;
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
                Branch &b = std::get<Branch>( current_node->entries.at(i)
                        );
                coverage += b.boundingPoly.area();
            }

            totalNodes += current_node->cur_offset_;
            memoryFootprint += sizeof(Node) + current_node->cur_offset_ * sizeof(Node::Branch);
            for( size_t i = 0; i < current_node->cur_offset_; i++ ) {
                Branch &b = std::get<Branch>( current_node->entries.at(i)
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
