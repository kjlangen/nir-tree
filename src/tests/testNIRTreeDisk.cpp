#include <catch2/catch.hpp>
#include <nirtreedisk/nirtreedisk.h>
#include <util/geometry.h>
#include <iostream>
#include <unistd.h>

using NodeType = nirtreedisk::Node<3,7>;
using TreeType = nirtreedisk::NIRTreeDisk<3,7>;

template <class NE, class B>
static NE createBranchEntry(
    const InlineBoundedIsotheticPolygon &boundingBox,
    tree_node_handle child
) {
	B b(boundingBox, child);
	return b;
}

static tree_node_handle
createFullLeafNode(TreeType &tree, tree_node_handle parent, Point p=Point::atOrigin)
{
    // Allocate new node
    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) NodeType( &tree, node_handle,
            tree_node_handle(nullptr) );


	for (unsigned i = 0; i < 7; ++i)
	{
		auto new_handle = node->insert(p);
        REQUIRE( new_handle == node_handle );
	}
    node->parent = parent;
	return node_handle;
}


TEST_CASE("NIRTreeDisk: testBoundingBox")
{
	// Test set one

    unlink( "nirdiskbacked.txt" );
    {
        TreeType tree( 4096*5, "nirdiskbacked.txt" );
        tree_node_handle root = tree.root;

        pinned_node_ptr<nirtreedisk::Node<3,7>> rootNode =
            tree.node_allocator_.get_tree_node<nirtreedisk::Node<3,7>>( root );
        
        std::pair<pinned_node_ptr<nirtreedisk::Node<3,7>>, tree_node_handle> alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child0, root );
        nirtreedisk::Node<3,7>::NodeEntry entry =
            createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,nirtreedisk::Branch>(
                    InlineBoundedIsotheticPolygon( Rectangle(8.0, 1.0,
                            12.0, 5.0) ), child0);
        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child1, root );
        entry =
            createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,nirtreedisk::Branch>(
                    InlineBoundedIsotheticPolygon( Rectangle(12.0, -4.0,
                            16.0, -2.0) ), child1);
        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child2, root );
        entry =
            createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,nirtreedisk::Branch>(
                    InlineBoundedIsotheticPolygon( Rectangle(8.0, -6.0,
                            10.0, -4.0) ), child2 );
        rootNode->addEntryToNode( entry );

        REQUIRE( rootNode->cur_offset_ == 3 );
        REQUIRE( rootNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0) );

    }
    unlink( "nirdiskbacked.txt" );
}
