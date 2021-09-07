#include <rstartreedisk/rstartreedisk.h>
#include <storage/tree_node_allocator.h>
namespace rstartreedisk
{
    RStarTreeDisk::RStarTreeDisk(
        size_t memory_budget,
        unsigned minBranchFactor,
        unsigned maxBranchFactor )
    : node_allocator_( memory_budget ),
    minBranchFactor( minBranchFactor ),
    maxBranchFactor( maxBranchFactor )
	{
        // Initialize buffer pool
        node_allocator_.initialize();

		hasReinsertedOnLevel = {false};

        std::pair<pinned_node_ptr<Node>, tree_node_handle> alloc = node_allocator_.create_new_tree_node<Node>();
        root = alloc.second;
        new (&(*(alloc.first))) Node( this, tree_node_handle() /*nullptr*/, 0
                );
	}

	RStarTreeDisk::~RStarTreeDisk()
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		root_ptr->deleteSubtrees();
	}

	std::vector<Point> RStarTreeDisk::exhaustiveSearch( Point requestedPoint )
	{
		std::vector<Point> v;
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		root_ptr->exhaustiveSearch( requestedPoint, v );

		return v;
	}

	std::vector<Point> RStarTreeDisk::search( Point requestedPoint )
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		assert( !root_ptr->parent );

		return root_ptr->search( requestedPoint );
	}

	std::vector<Point> RStarTreeDisk::search( Rectangle requestedRectangle )
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		assert( !root_ptr->parent );
		return root_ptr->search( requestedRectangle );
	}

	void RStarTreeDisk::insert( Point givenPoint )
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		assert( !root_ptr->parent );

		std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
		root = root_ptr->insert( givenPoint, hasReinsertedOnLevel );
	}

	void RStarTreeDisk::remove( Point givenPoint )
	{
		std::fill( hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false );
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );

		root = root_ptr->remove( givenPoint, hasReinsertedOnLevel );

        // Get new root
        root_ptr = node_allocator_.get_tree_node<Node>( root );
        assert( !root_ptr->parent );
	}

	unsigned RStarTreeDisk::checksum()
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		return root_ptr->checksum();
	}

	void RStarTreeDisk::print()
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		root_ptr->printTree();
	}

	bool RStarTreeDisk::validate()
	{
		return true;
	}

	void RStarTreeDisk::stat()
	{
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		root_ptr->stat();
	}

	void RStarTreeDisk::visualize()
	{
		BMPPrinter p(1000, 1000);
        pinned_node_ptr<Node> root_ptr = node_allocator_.get_tree_node<Node>( root );
		//p.printToBMP( root_ptr );
	}
}
