#include <catch2/catch.hpp>
#include <rtreedisk/rtreedisk.h>
#include <storage/page.h>
#include <util/geometry.h>
#include <iostream>
#include <unistd.h>



using NodeType = rtreedisk::Node<3,7>;
using TreeType = rtreedisk::RTreeDisk<3,7>;
/*
tree_node_handle
createFullRtreeDiskLeafNode(TreeType &tree, tree_node_handle parent, Point p=Point::atOrigin)
{
    // Allocate new node
    std::pair<pinned_node_ptr<NodeType>, tree_node_handle> alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) NodeType( &tree, node_handle, tree_node_handle(nullptr));

	for (unsigned i = 0; i < 7; ++i)
	{
		auto new_handle = node->insert(p);
        REQUIRE( new_handle == node_handle );
	}

    node->parent = parent;

	return node_handle;
}*/

TEST_CASE("RTreeDisk: testRemoveData")
{

    unlink( "rstardiskbacked.txt" );

	// Setup a rtree::Node with some data
	TreeType tree( 4096, "rstardiskbacked.txt" );
	tree_node_handle root = tree.root;
    auto parentNode = tree.node_allocator_.get_tree_node<NodeType>( root );

    parentNode->insert( Point(9.0, -5.0) );
	parentNode->insert( Point(14.0, -3.0) );
	parentNode->insert( Point(11.0, 13.0) );
	parentNode->insert( Point(13.0, 13.0) );

	// Remove some of the data
	parentNode->removeData(Point(13.0, 13.0));

	// Test the removal
	REQUIRE(parentNode->cur_offset_ == 3);

    unlink( "rstardiskbacked.txt" );
}
