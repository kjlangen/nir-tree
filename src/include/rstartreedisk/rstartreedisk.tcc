template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RStarTreeDisk<min_branch_factor,max_branch_factor>::exhaustiveSearch( Point requestedPoint )
{
    std::vector<Point> v;
    auto root_ptr = get_node( root );
    root_ptr->exhaustiveSearch( requestedPoint, v );

    return v;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RStarTreeDisk<min_branch_factor, max_branch_factor>::search( Point requestedPoint )
{
    auto  root_ptr = get_node( root );
    assert( !root_ptr->parent );

    return root_ptr->search( requestedPoint );
}


template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RStarTreeDisk<min_branch_factor,max_branch_factor>::search( Rectangle
        requestedRectangle )
{
    auto root_ptr = get_node( root );
    assert( !root_ptr->parent );
    return root_ptr->search( requestedRectangle );
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor, max_branch_factor>::insert( Point givenPoint )
{
    auto root_ptr = get_node( root );
    assert( !root_ptr->parent );

    std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
    root = root_ptr->insert( givenPoint, hasReinsertedOnLevel );
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor, max_branch_factor>::remove( Point givenPoint )
{
    std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
    auto root_ptr = get_node( root );

    root = root_ptr->remove( givenPoint, hasReinsertedOnLevel );

    // Get new root
    root_ptr = get_node( root );
    assert( !root_ptr->parent );
}


template <int min_branch_factor, int max_branch_factor>
unsigned RStarTreeDisk<min_branch_factor,max_branch_factor>::checksum()
{
    auto root_ptr = get_node( root );
    return root_ptr->checksum();
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor,max_branch_factor>::print()
{
    auto root_ptr = get_node( root );
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
    auto root_ptr = get_node( root );
    root_ptr->stat();
}


template <int min_branch_factor, int max_branch_factor>
void RStarTreeDisk<min_branch_factor,max_branch_factor>::visualize()
{
    BMPPrinter p(1000, 1000);
    auto root_ptr = get_node( root );
}
