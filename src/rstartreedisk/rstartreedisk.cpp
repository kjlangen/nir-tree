#include <rstartreedisk/rstartreedisk.h>
#include <storage/tree_node_allocator.h>
namespace rstartreedisk
{
    RStarTreeDisk::RStarTreeDisk(
        size_t memory_budget,
        unsigned minBranchFactor,
        unsigned maxBranchFactor,
        std::string backing_file_name )
    : node_allocator_( memory_budget, backing_file_name ),
    minBranchFactor( minBranchFactor ),
    maxBranchFactor( maxBranchFactor )
	{
        // Initialize buffer pool
        node_allocator_.initialize();

		hasReinsertedOnLevel = {false};

        /* We need to figure out if there was already data, and read
         * that into memory if we have it. */

        size_t existing_page_count =
            node_allocator_.buffer_pool_.get_preexisting_page_count();

        std::cout << "EXISTING PAGE COUNT: " << existing_page_count <<
            std::endl;

        // If this is a fresh tree, then make a fresh root
        if( existing_page_count == 0 ) { 
            std::pair<pinned_node_ptr<Node<3,7>>, tree_node_handle> alloc =
                node_allocator_.create_new_tree_node<Node<3,7>>();
            root = alloc.second;
            new (&(*(alloc.first))) Node<3,7>( this, root, tree_node_handle() /*nullptr*/, 0
                    );
            return;
        }

        // Hunt for the root
        size_t node_size = sizeof( Node<3,7> );
        for( size_t i = 0; i < existing_page_count; i++ ) {
            page *p = node_allocator_.buffer_pool_.get_page( i );
            node_allocator_.buffer_pool_.pin_page( p );
            char *page_bytes = p->data_;
            for( size_t offset_multiplier = 0; offset_multiplier <
                    (PAGE_DATA_SIZE / sizeof(Node<3,7>));
                     offset_multiplier++ ) { 
                Node<3,7> *interpreted_ptr = (Node<3,7> *) (page_bytes +
                        offset_multiplier * sizeof( Node<3,7> ));
                if( interpreted_ptr->parent == tree_node_handle() ) {
                    // Found the root
                    root = tree_node_handle( i, offset_multiplier *
                            sizeof( Node<3,7> ));
                    std::cout << "Found root at Page " << i << " offset "
                        << offset_multiplier * sizeof( Node<3,7> ) <<
                        std::endl;
                    node_allocator_.buffer_pool_.unpin_page( p );
                    return;
                } else {
                    std::cout << "Not root node. Has parent" <<
                        std::endl;
                    std::cout << interpreted_ptr->parent << std::endl;
                }
            }
            node_allocator_.buffer_pool_.unpin_page( p );
        }

        assert( false );
	}

	RStarTreeDisk::~RStarTreeDisk()
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		root_ptr->deleteSubtrees();
	}

	std::vector<Point> RStarTreeDisk::exhaustiveSearch( Point requestedPoint )
	{
		std::vector<Point> v;
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		root_ptr->exhaustiveSearch( requestedPoint, v );

		return v;
	}

	std::vector<Point> RStarTreeDisk::search( Point requestedPoint )
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		assert( !root_ptr->parent );

		return root_ptr->search( requestedPoint );
	}

	std::vector<Point> RStarTreeDisk::search( Rectangle
            requestedRectangle )
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		assert( !root_ptr->parent );
		return root_ptr->search( requestedRectangle );
	}

	void RStarTreeDisk::insert( Point givenPoint )
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		assert( !root_ptr->parent );

		std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
		root = root_ptr->insert( givenPoint, hasReinsertedOnLevel );
	}

	void RStarTreeDisk::remove( Point givenPoint )
	{
		std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );

		root = root_ptr->remove( givenPoint, hasReinsertedOnLevel );

        // Get new root
        root_ptr = node_allocator_.get_tree_node<Node<3,7>>( root );
        assert( !root_ptr->parent );
	}

	unsigned RStarTreeDisk::checksum()
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		return root_ptr->checksum();
	}

	void RStarTreeDisk::print()
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		root_ptr->printTree();
	}

	bool RStarTreeDisk::validate()
	{
		return true;
	}

	void RStarTreeDisk::stat()
	{
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		root_ptr->stat();
	}

	void RStarTreeDisk::visualize()
	{
		BMPPrinter p(1000, 1000);
        pinned_node_ptr<Node<3,7>> root_ptr =
            node_allocator_.get_tree_node<Node<3,7>>( root );
		//p.printToBMP( root_ptr );
	}
}
