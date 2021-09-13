template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RStarTreeDisk<min_branch_factor,max_branch_factor>::exhaustiveSearch( Point requestedPoint )
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    std::vector<Point> v;
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    root_ptr->exhaustiveSearch( requestedPoint, v );

    return v;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RStarTreeDisk<min_branch_factor, max_branch_factor>::search( Point requestedPoint )
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    assert( !root_ptr->parent );

    return root_ptr->search( requestedPoint );
}


template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RStarTreeDisk<min_branch_factor,max_branch_factor>::search( Rectangle
        requestedRectangle )
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    assert( !root_ptr->parent );
    return root_ptr->search( requestedRectangle );
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor, max_branch_factor>::insert( Point givenPoint )
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    assert( !root_ptr->parent );

    std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
    root = root_ptr->insert( givenPoint, hasReinsertedOnLevel );
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor, max_branch_factor>::remove( Point givenPoint )
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );

    root = root_ptr->remove( givenPoint, hasReinsertedOnLevel );

    // Get new root
    root_ptr = node_allocator_.get_tree_node<NodeType>( root );
    assert( !root_ptr->parent );
}


template <int min_branch_factor, int max_branch_factor>
unsigned RStarTreeDisk<min_branch_factor,max_branch_factor>::checksum()
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    return root_ptr->checksum();
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor,max_branch_factor>::print()
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    root_ptr->printTree();
}


template <int min_branch_factor, int max_branch_factor>
bool RStarTreeDisk<min_branch_factor,max_branch_factor>::validate()
{
    return true;
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor,max_branch_factor>::stat()
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
    root_ptr->stat();
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor,max_branch_factor>::visualize()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    BMPPrinter p(1000, 1000);
    pinned_node_ptr<NodeType> root_ptr =
        node_allocator_.get_tree_node<NodeType>( root );
}
