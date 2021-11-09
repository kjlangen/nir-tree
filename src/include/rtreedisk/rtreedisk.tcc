template <int min_branch_factor, int max_branch_factor>
RTreeDisk<min_branch_factor,max_branch_factor>::RTreeDisk(size_t memory_budget, std::string backing_file): node_allocator_(memory_budget, backing_file)
{
    // Initialize buffer pool
    node_allocator_.initialize();

    backing_file_ = backing_file;

    /* We need to figure out if there was already data, and read
        * that into memory if we have it. */

    size_t existing_page_count =
        node_allocator_.buffer_pool_.get_preexisting_page_count();

    // If this is a fresh tree, then make a fresh root
    if (existing_page_count == 0)
    {
        std::pair<pinned_node_ptr<Node<min_branch_factor, max_branch_factor>>, tree_node_handle> alloc =
            node_allocator_.create_new_tree_node<Node<min_branch_factor, max_branch_factor>>();
        root = alloc.second;
        new (&(*(alloc.first))) Node<min_branch_factor, max_branch_factor>(this, root, tree_node_handle( nullptr ) /*nullptr*/);
        return;
    }

    std::string meta_file = backing_file_ + ".meta";
    int fd = open( meta_file.c_str(), O_RDONLY );
    assert( fd >= 0 );

    int rc = read( fd, (char *) &root, sizeof( root ) );
    assert( rc == sizeof( root ) );
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RTreeDisk<min_branch_factor,max_branch_factor>::exhaustiveSearch( Point requestedPoint )
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    std::vector<Point> v;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    root_ptr->exhaustiveSearch( requestedPoint, v );

    return v;
}

template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RTreeDisk<min_branch_factor, max_branch_factor>::search( Point requestedPoint )
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    assert( !root_ptr->parent );

    return root_ptr->search( requestedPoint );
}


template <int min_branch_factor, int max_branch_factor>
std::vector<Point> RTreeDisk<min_branch_factor,max_branch_factor>::search( Rectangle
        requestedRectangle )
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    assert( !root_ptr->parent );
    return root_ptr->search( requestedRectangle );
}

template <int min_branch_factor, int max_branch_factor>
void RTreeDisk<min_branch_factor, max_branch_factor>::insert( Point givenPoint )
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    assert( !root_ptr->parent );

    root = root_ptr->insert( givenPoint );
}

template <int min_branch_factor, int max_branch_factor>
void RTreeDisk<min_branch_factor, max_branch_factor>::remove( Point givenPoint )
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    root = root_ptr->remove( givenPoint );

#ifndef NDEBUG
    // Get new root
    root_ptr = get_node( root );
    assert( !root_ptr->parent );
#endif
}

template <int min_branch_factor, int max_branch_factor>
unsigned RTreeDisk<min_branch_factor,max_branch_factor>::checksum()
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    return root_ptr->checksum();
}


template <int min_branch_factor, int max_branch_factor>
void RTreeDisk<min_branch_factor,max_branch_factor>::print()
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    root_ptr->printTree();
}


template <int min_branch_factor, int max_branch_factor>
bool RTreeDisk<min_branch_factor,max_branch_factor>::validate()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    return root_ptr->validate( tree_node_handle( nullptr ), 0);
}


template <int min_branch_factor, int max_branch_factor>
void RTreeDisk<min_branch_factor,max_branch_factor>::stat()
{

    using NodeType = Node<min_branch_factor,max_branch_factor>;
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
    root_ptr->stat();
}


template <int min_branch_factor, int max_branch_factor>
void RTreeDisk<min_branch_factor,max_branch_factor>::visualize()
{
    using NodeType = Node<min_branch_factor,max_branch_factor>;

    BMPPrinter p(1000, 1000);
    pinned_node_ptr<NodeType> root_ptr = get_node( root );
}

