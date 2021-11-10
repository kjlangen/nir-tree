#include <catch2/catch.hpp>
#include <rstartreedisk/rstartreedisk.h>
#include <util/geometry.h>
#include <iostream>
#include <unistd.h>

using LeafNodeType = rstartreedisk::LeafNode<3,7>;
using BranchNodeType = rstartreedisk::BranchNode<3,7>;
using TreeType = rstartreedisk::RStarTreeDisk<3,7>;
using Branch = rstartreedisk::Branch;

static Branch createBranchEntry(
    const Rectangle &boundingBox,
    tree_node_handle child
) {
	Branch b(boundingBox, child);
	return b;
}

static tree_node_handle
createFullLeafNode(TreeType &tree, tree_node_handle parent, Point p=Point::atOrigin)
{
    // Allocate new node
    auto alloc_data = tree.node_allocator_.create_new_tree_node<LeafNodeType>();
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) LeafNodeType( &tree, node_handle, tree_node_handle() /* nullptr */, 0 );

	std::vector<bool> reInsertedAtLevel = {false};

	for (unsigned i = 0; i < 7; ++i)
	{
		auto new_handle = node->insert(p, reInsertedAtLevel);
        REQUIRE( new_handle == node_handle );
		REQUIRE(reInsertedAtLevel[0] == false);
	}

    node->parent = parent;

	return node_handle;
}

TEST_CASE("R*TreeDisk: testBoundingBox")
{
	// Test set one
    unlink( "rstardiskbacked.txt" );
    {
        TreeType tree( 4096, "rstardiskbacked.txt" );
        auto alloc_data_root =
            tree.node_allocator_.create_new_tree_node<BranchNodeType>( );
        tree_node_handle root = alloc_data_root.second;
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle(nullptr), 0);
        auto rootNode = alloc_data_root.first;
        
        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<LeafNodeType>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child0, root, 1 );
        auto entry =
            createBranchEntry( Rectangle(8.0, 1.0, 12.0, 5.0),
                    alloc_data.second);
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<LeafNodeType>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child1, root, 1 );
        entry =
            createBranchEntry(
                    Rectangle(12.0, -4.0, 16.0, -2.0), child1);
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<LeafNodeType>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child2, root, 1 );
        entry =
            createBranchEntry( Rectangle(8.0, -6.0, 10.0, -4.0), child2 );
        rootNode->addBranchToNode( entry );

        REQUIRE( rootNode->cur_offset_ == 3 );
        REQUIRE( rootNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0) );
    }

    unlink( "rstardiskbacked.txt" );
    {
        // Test set two
        TreeType tree(4096, "rstardiskbacked.txt" );

        auto alloc_data_root = tree.node_allocator_.create_new_tree_node<BranchNodeType>();
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 0 );
        tree_node_handle root = alloc_data_root.second;
        auto rootNode = alloc_data_root.first;
        
        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<LeafNodeType>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child0, root, 1 );
        rootNode->addBranchToNode( 
            createBranchEntry( Rectangle(8.0, 12.0, 10.0, 14.0), child0 ) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<LeafNodeType>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child1, root, 1 );
        rootNode->addBranchToNode( 
            createBranchEntry( Rectangle(10.0, 12.0, 12.0, 14.0), child1) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<LeafNodeType>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child2, root, 1 );
        rootNode->addBranchToNode(
            createBranchEntry( Rectangle(12.0, 12.0, 14.0, 14.0), child2 ) );

        REQUIRE( rootNode->cur_offset_ ==  3 );

        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
    }
    unlink( "rstardiskbacked.txt" );
}

#if 0
TEST_CASE("R*TreeDisk: testUpdateBoundingBox") {

    unlink( "rstardiskbacked.txt" );
	TreeType tree(4096, "rstardiskbacked.txt");
    tree_node_handle root = tree.root;

    pinned_node_ptr<NodeType> parentNode =
        tree.node_allocator_.get_tree_node<NodeType>( root );
	parentNode->level = 1;

    std::pair<pinned_node_ptr<NodeType>,tree_node_handle> alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child0, root, 1 );
    auto child0Node = alloc_data.first;
	child0Node->parent = root;
	child0Node->level = 0;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(8.0, -6.0, 10.0, -4.0), child0) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child1, root, 1 );
    auto child1Node = alloc_data.first;
    child1Node->level = 0;
    child1Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(12.0, -4.0, 16.0, -2.0), child1) );


    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child2, root, 1 );
    auto child2Node = alloc_data.first;
    child2Node->level = 0;
    child2Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(10.0, 12.0, 12.0, 14.0), child2) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child3, root, 1 );
    auto child3Node = alloc_data.first;
    child3Node->level = 0;
    child3Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(12.0, 12.0, 14.0, 14.0), child3) );

    REQUIRE( parentNode->cur_offset_ == 4 );

	parentNode->updateBoundingBox(child3, Rectangle(3.0, 3.0, 5.0, 5.0));

	const auto &b = std::get<NodeType::Branch>(parentNode->entries[3]);
	REQUIRE(b.boundingBox == Rectangle(3.0, 3.0, 5.0, 5.0));
    REQUIRE(parentNode->level == 1);
    REQUIRE(child0Node->level == 0);
    REQUIRE(child1Node->level == 0);
    REQUIRE(child2Node->level == 0);
    REQUIRE(child3Node->level == 0);

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE( "R*TreeDisk: testRemoveChild" ) {
    using NodeType = rstartreedisk::Node<3,7>;

    unlink( "rstardiskbacked.txt" );
	TreeType tree(4096, "rstardiskbacked.txt");
    tree_node_handle root = tree.root;

    pinned_node_ptr<NodeType> parentNode =
        tree.node_allocator_.get_tree_node<NodeType>( root );
	parentNode->level = 1;

    std::pair<pinned_node_ptr<NodeType>,tree_node_handle> alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child0, root, 1 );
    auto child0Node = alloc_data.first;
	child0Node->parent = root;
	child0Node->level = 0;
	parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(8.0, -6.0, 10.0, -4.0), child0) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child1, root, 1 );
    auto child1Node = alloc_data.first;
    child1Node->level = 0;
    child1Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(12.0, -4.0, 16.0, -2.0), child1) );


    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child2, root, 1 );
    auto child2Node = alloc_data.first;
    child2Node->level = 0;
    child2Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(10.0, 12.0, 12.0, 14.0), child2) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child3, root, 1 );
    auto child3Node = alloc_data.first;
    child3Node->level = 0;
    child3Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(Rectangle(12.0, 12.0, 14.0, 14.0), child3) );

    REQUIRE( parentNode->cur_offset_ ==  4 );

	// Remove one of the children
	parentNode->removeChild(child3);
    REQUIRE( parentNode->cur_offset_ == 3 );

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testRemoveData")
{

    unlink( "rstardiskbacked.txt" );

	// Setup a rtree::Node with some data
	TreeType tree( 4096, "rstardiskbacked.txt" );
	tree_node_handle root = tree.root;
    auto parentNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );
    REQUIRE( parentNode->level == 0 );

    parentNode->addEntryToNode( Point(9.0, -5.0) );
	parentNode->addEntryToNode( Point(14.0, -3.0) );
	parentNode->addEntryToNode( Point(11.0, 13.0) );
	parentNode->addEntryToNode( Point(13.0, 13.0) );

	// Remove some of the data
	parentNode->removeData(Point(13.0, 13.0));

	// Test the removal
	REQUIRE(parentNode->cur_offset_ == 3);

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testChooseLeaf")
{
	// Create rtree::Nodes
    unlink( "rstardiskbacked.txt" );

    // Need a bunch of pages so we don't run out of memory while
    // everything is pinned
	TreeType tree( 4096*4, "rstardiskbacked.txt" );
    tree_node_handle root = tree.root;


    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto leftNode = alloc_data.first;
    tree_node_handle left = alloc_data.second;
    new (&(*leftNode)) NodeType( &tree, left, root, 1 );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto rightNode = alloc_data.first;
    tree_node_handle right = alloc_data.second;
    new (&(*rightNode)) NodeType( &tree, right, root, 1 );

    tree_node_handle leftChild0 = createFullLeafNode(tree, left);
    tree_node_handle leftChild1 = createFullLeafNode(tree, left);
    tree_node_handle leftChild2 = createFullLeafNode(tree, left);
    leftNode->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(8.0, 12.0, 10.0, 14.0), leftChild0 ) );
    leftNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(10.0, 12.0, 12.0, 14.0), leftChild1 ) );
    leftNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(12.0, 12.0, 14.0, 14.0), leftChild2 ) );
    rootNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(8.0, 12.0, 14.0, 14.0), left ) );

    tree_node_handle rightChild0 = createFullLeafNode(tree,right);
    tree_node_handle rightChild1 = createFullLeafNode(tree,right);
    tree_node_handle rightChild2 = createFullLeafNode(tree,right);
    rightNode->parent = root;
    rightNode->level = 1;
    rightNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(8.0, 1.0, 12.0, 5.0), rightChild0 ));
    rightNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(12.0, -4.0, 16.0, -2.0), rightChild1 ));
    rightNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(8.0, -6.0, 10.0, -4.0), rightChild2 ));
    rootNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            Rectangle(8.0, -6.0, 16.0, 5.0), right ));

    rootNode->level = 2;

	// Test that we get the correct child for the given point
	REQUIRE(rightChild1 == rootNode->chooseSubtree(Point(13.0, -3.0)));
	REQUIRE(leftChild0 == rootNode->chooseSubtree(Point(8.5, 12.5)));
	REQUIRE(leftChild2 == rootNode->chooseSubtree(Point(13.5, 13.5)));
	REQUIRE(rightChild0 == rootNode->chooseSubtree(Point(7.0, 3.0)));
	REQUIRE(leftChild1 == rootNode->chooseSubtree(Point(11.0, 15.0)));
	REQUIRE(leftChild0 == rootNode->chooseSubtree(Point(4.0, 8.0)));

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testFindLeaf")
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
    unlink( "rstardiskbacked.txt" );

	TreeType tree( 4096 * 5, "rstardiskbacked.txt" );
    tree_node_handle root = tree.root;
    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4aNode = alloc_data.first;
    tree_node_handle cluster4a = alloc_data.second;
    new (&(*cluster4aNode)) NodeType( &tree, cluster4a,
            tree_node_handle() /*nullptr for now*/, 0 );
    cluster4aNode->addEntryToNode( Point(-10.0, -2.0) );
    cluster4aNode->addEntryToNode( Point(-12.0, -3.0) );
    cluster4aNode->addEntryToNode( Point(-11.0, -3.0) );
    cluster4aNode->addEntryToNode( Point(-10.0, -3.0) );
	
    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4bNode = alloc_data.first;
    tree_node_handle cluster4b = alloc_data.second;
    new (&(*cluster4bNode)) NodeType( &tree, cluster4b,
            tree_node_handle() /*nullptr for now*/, 0 );

	cluster4bNode->addEntryToNode( Point(-9.0, -3.0) );
	cluster4bNode->addEntryToNode( Point(-7.0, -3.0) );
	cluster4bNode->addEntryToNode( Point(-10.0, -5.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4Node = alloc_data.first;
    tree_node_handle cluster4 = alloc_data.second;


    new (&(*cluster4Node)) NodeType( &tree, cluster4, root, 1 );
	cluster4aNode->parent = cluster4;
	cluster4Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry, NodeType::Branch>(
                cluster4aNode->boundingBox(), cluster4a) );
	cluster4bNode->parent = cluster4;
	cluster4Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
            cluster4bNode->boundingBox(), cluster4b));

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5aNode = alloc_data.first;
    tree_node_handle cluster5a = alloc_data.second;
    new (&(*cluster5aNode)) NodeType( &tree, cluster5a,
                tree_node_handle() /* nullptr for now */, 0 );
	cluster5aNode->addEntryToNode( Point(-14.5, -13.0) );
	cluster5aNode->addEntryToNode( Point(-14.0, -13.0) );
	cluster5aNode->addEntryToNode( Point(-13.5, -13.5) );
	cluster5aNode->addEntryToNode( Point(-15.0, -14.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5bNode = alloc_data.first;
    tree_node_handle cluster5b = alloc_data.second;
    new (&(*cluster5bNode)) NodeType( &tree, cluster5b,
                tree_node_handle() /* nullptr for now */, 0 );
	cluster5bNode->addEntryToNode( Point(-14.0, -14.0) );
	cluster5bNode->addEntryToNode( Point(-13.0, -14.0) );
	cluster5bNode->addEntryToNode( Point(-12.0, -14.0) );
	cluster5bNode->addEntryToNode( Point(-13.5, -16.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5cNode = alloc_data.first;
    tree_node_handle cluster5c = alloc_data.second;
    new (&(*cluster5cNode)) NodeType( &tree, cluster5c,
                tree_node_handle() /* nullptr for now */, 0 );

	cluster5cNode->addEntryToNode( Point(-15.0, -14.5) );
	cluster5cNode->addEntryToNode( Point(-14.0, -14.5) );
	cluster5cNode->addEntryToNode( Point(-12.5, -14.5) );
	cluster5cNode->addEntryToNode( Point(-13.5, -15.5) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5dNode = alloc_data.first;
    tree_node_handle cluster5d = alloc_data.second;
    new (&(*cluster5dNode)) NodeType( &tree, cluster5d,
                tree_node_handle() /* nullptr for now */, 0 );
	cluster5dNode->addEntryToNode( Point(-15.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-14.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-13.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-12.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-15.0, -15.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5Node = alloc_data.first;
    tree_node_handle cluster5 = alloc_data.second;
    new (&(*cluster5Node)) NodeType( &tree, cluster5, root, 1 );

	cluster5aNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(cluster5aNode->boundingBox(),cluster5a));
	cluster5bNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(cluster5bNode->boundingBox(), cluster5b));


	cluster5cNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(cluster5cNode->boundingBox(), cluster5c));

	cluster5dNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(cluster5dNode->boundingBox(), cluster5d));

	// Root
	rootNode->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(cluster4Node->boundingBox(), cluster4));
	rootNode->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        NodeType::Branch>(cluster5Node->boundingBox(), cluster5));
	rootNode->level = 2;

	// Test finding leaves
	REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testFindLeaf ON DISK")
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
    unlink( "rstardiskbacked.txt" );

    // Same test as above, but shut down the tree and later read it from
    // disk for the search

    tree_node_handle root;
    tree_node_handle cluster4;
    tree_node_handle cluster4a;
    tree_node_handle cluster4b;
    tree_node_handle cluster4c;

    tree_node_handle cluster5;
    tree_node_handle cluster5a;
    tree_node_handle cluster5b;
    tree_node_handle cluster5c;
    tree_node_handle cluster5d;
        
    {
        TreeType tree( 4096 * 5,"rstardiskbacked.txt" );
        root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
                );

        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4aNode = alloc_data.first;
        cluster4a = alloc_data.second;
        new (&(*cluster4aNode)) NodeType( &tree, cluster4a,
                tree_node_handle() /*nullptr for now*/, 0 );
        cluster4aNode->addEntryToNode( Point(-10.0, -2.0) );
        cluster4aNode->addEntryToNode( Point(-12.0, -3.0) );
        cluster4aNode->addEntryToNode( Point(-11.0, -3.0) );
        cluster4aNode->addEntryToNode( Point(-10.0, -3.0) );
        
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4bNode = alloc_data.first;
        cluster4b = alloc_data.second;
        new (&(*cluster4bNode)) NodeType( &tree, cluster4b,
                tree_node_handle() /*nullptr for now*/, 0 );

        cluster4bNode->addEntryToNode( Point(-9.0, -3.0) );
        cluster4bNode->addEntryToNode( Point(-7.0, -3.0) );
        cluster4bNode->addEntryToNode( Point(-10.0, -5.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4Node = alloc_data.first;
        cluster4 = alloc_data.second;


        new (&(*cluster4Node)) NodeType( &tree, cluster4, root, 1 );
        cluster4aNode->parent = cluster4;
        cluster4Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry, NodeType::Branch>(
                    cluster4aNode->boundingBox(), cluster4a) );
        cluster4bNode->parent = cluster4;
        cluster4Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,NodeType::Branch>(
                cluster4bNode->boundingBox(), cluster4b));

        // Cluster 5, n = 16
        // (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
        // (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
        // (-14, -15), (-13, -15), (-12, -15)

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5aNode = alloc_data.first;
        cluster5a = alloc_data.second;
        new (&(*cluster5aNode)) NodeType( &tree, cluster5a,
                    tree_node_handle() /* nullptr for now */, 0 );
        cluster5aNode->addEntryToNode( Point(-14.5, -13.0) );
        cluster5aNode->addEntryToNode( Point(-14.0, -13.0) );
        cluster5aNode->addEntryToNode( Point(-13.5, -13.5) );
        cluster5aNode->addEntryToNode( Point(-15.0, -14.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5bNode = alloc_data.first;
        cluster5b = alloc_data.second;
        new (&(*cluster5bNode)) NodeType( &tree, cluster5b,
                    tree_node_handle() /* nullptr for now */, 0 );
        cluster5bNode->addEntryToNode( Point(-14.0, -14.0) );
        cluster5bNode->addEntryToNode( Point(-13.0, -14.0) );
        cluster5bNode->addEntryToNode( Point(-12.0, -14.0) );
        cluster5bNode->addEntryToNode( Point(-13.5, -16.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5cNode = alloc_data.first;
        cluster5c = alloc_data.second;
        new (&(*cluster5cNode)) NodeType( &tree, cluster5c,
                    tree_node_handle() /* nullptr for now */, 0 );

        cluster5cNode->addEntryToNode( Point(-15.0, -14.5) );
        cluster5cNode->addEntryToNode( Point(-14.0, -14.5) );
        cluster5cNode->addEntryToNode( Point(-12.5, -14.5) );
        cluster5cNode->addEntryToNode( Point(-13.5, -15.5) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5dNode = alloc_data.first;
        cluster5d = alloc_data.second;
        new (&(*cluster5dNode)) NodeType( &tree, cluster5d,
                    tree_node_handle() /* nullptr for now */, 0 );
        cluster5dNode->addEntryToNode( Point(-15.0, -15.0) );
        cluster5dNode->addEntryToNode( Point(-14.0, -15.0) );
        cluster5dNode->addEntryToNode( Point(-13.0, -15.0) );
        cluster5dNode->addEntryToNode( Point(-12.0, -15.0) );
        cluster5dNode->addEntryToNode( Point(-15.0, -15.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5Node = alloc_data.first;
        cluster5 = alloc_data.second;
        new (&(*cluster5Node)) NodeType( &tree, cluster5, root, 1 );

        cluster5aNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            NodeType::Branch>(cluster5aNode->boundingBox(),cluster5a));
        cluster5bNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            NodeType::Branch>(cluster5bNode->boundingBox(), cluster5b));


        cluster5cNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            NodeType::Branch>(cluster5cNode->boundingBox(), cluster5c));

        cluster5dNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            NodeType::Branch>(cluster5dNode->boundingBox(), cluster5d));

        // Root
        rootNode->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            NodeType::Branch>(cluster4Node->boundingBox(), cluster4));
        rootNode->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            NodeType::Branch>(cluster5Node->boundingBox(), cluster5));
        rootNode->level = 2;

        tree.write_metadata();
    }

    TreeType tree( 4096 * 5, "rstardiskbacked.txt" );
    REQUIRE( root == tree.root );
    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );

	// Test finding leaves
	REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testComplexSplitAxis")
{
    unlink( "rstardiskbacked.txt" );
	// Now m=3, M = 7.
	// 2 <= 3 <= 7/2 so we are good.
	
    {
        // Test split with X
        TreeType tree(4096 * 5, "rstardiskbacked.txt");
        tree_node_handle root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
                );
        rootNode->addEntryToNode(Point(-3.0, -11.0));
        rootNode->addEntryToNode(Point(-2.0, -9.0));
        rootNode->addEntryToNode(Point(2.0, -10.0));
        rootNode->addEntryToNode(Point(3.0, -11.0));
        rootNode->addEntryToNode(Point(1.0, -9.0));
        rootNode->addEntryToNode(Point(-3.0, -10.0));
        rootNode->addEntryToNode(Point(3.0, -11.0));
        rootNode->addEntryToNode(Point(3.0, -9.0));

        REQUIRE( rootNode->level == 0 );

        // See above test for margin scores for X and Y.
        // whichever one is lower (here X) is the split axis

        // Split the rstartree::Node in two
        unsigned int axis = rootNode->chooseSplitAxis();
        REQUIRE(axis == 0);
    }
    unlink( "rstardiskbacked.txt" );

    {
        // Test split with Y
        TreeType tree2(4096*5, "rstardiskbacked.txt");
        auto rootNode = tree2.node_allocator_.get_tree_node<NodeType>( tree2.root );
        rootNode->addEntryToNode(Point(-11.0, -3.0));
        rootNode->addEntryToNode(Point(-9.0, -2.0));
        rootNode->addEntryToNode(Point(-10.0, 2.0));
        rootNode->addEntryToNode(Point(-11.0, 3.0));
        rootNode->addEntryToNode(Point(-9.0, 1.0));
        rootNode->addEntryToNode(Point(-10.0, -3.0));
        rootNode->addEntryToNode(Point(-11.0, 3.0));
        rootNode->addEntryToNode(Point(-9.0, 3.0));
        REQUIRE(rootNode->level == 0);

        unsigned int axis = rootNode->chooseSplitAxis();
        REQUIRE(axis == 1);
    }

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testSplitNode")
{
	// Test set one
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	
    unlink( "rstardiskbacked.txt" );

    {
        TreeType tree( 4096 * 5, "rstardiskbacked.txt");
        auto root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
                );
        rootNode->addEntryToNode(Point(-2.0, -6.0));
        rootNode->addEntryToNode(Point(-2.0, -6.0));
        rootNode->addEntryToNode(Point(2.0, -6.0));
        rootNode->addEntryToNode(Point(-1.0, -7.0));
        rootNode->addEntryToNode(Point(1.0, -7.0));
        rootNode->addEntryToNode(Point(3.0, -8.0));
        rootNode->addEntryToNode(Point(-2.0, -9.0));

        // Split the rstartree::Node in two
        rootNode->addEntryToNode(Point(-3.0, -11.0));
        tree_node_handle splitNodeHandle = rootNode->splitNode();
        auto splitNode = tree.node_allocator_.get_tree_node<NodeType>(
                splitNodeHandle );

        // Test the split
        REQUIRE(rootNode->cur_offset_ == 5);
        REQUIRE(std::get<Point>(rootNode->entries[0]) == Point(-3.0, -11.0));
        REQUIRE(std::get<Point>(rootNode->entries[1]) == Point(-2.0, -6.0));
        REQUIRE(std::get<Point>(rootNode->entries[2]) == Point(-2.0, -6.0));
        REQUIRE(std::get<Point>(rootNode->entries[3]) == Point(-2.0, -9.0));
        REQUIRE(std::get<Point>(rootNode->entries[4]) == Point(-1.0, -7.0));
        REQUIRE(splitNode->cur_offset_ == 3);
        REQUIRE(std::get<Point>(splitNode->entries[0]) == Point(1.0, -7.0));
        REQUIRE(std::get<Point>(splitNode->entries[1]) == Point(2.0, -6.0));
        REQUIRE(std::get<Point>(splitNode->entries[2]) == Point(3.0, -8.0));
        REQUIRE(splitNode->level == rootNode->level);
    }
    unlink( "rstardiskbacked.txt" );

    {
        // Test set two
        // Cluster 2, n = 8
        // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
        TreeType tree2( 4096*5, "rstardiskbacked.txt" );
        auto rootNode =
            tree2.node_allocator_.get_tree_node<NodeType>(tree2.root);
        rootNode->addEntryToNode(Point(-14.0, 8.0));
        rootNode->addEntryToNode(Point(-10.0, 8.0));
        rootNode->addEntryToNode(Point(-9.0, 10.0));
        rootNode->addEntryToNode(Point(-9.0, 9.0));
        rootNode->addEntryToNode(Point(-8.0, 10.0));
        rootNode->addEntryToNode(Point(-9.0, 7.0));
        rootNode->addEntryToNode(Point(-8.0, 8.0));
        rootNode->addEntryToNode(Point(-8.0, 9.0));
        rootNode->level = 0;

        // Split the rstartree::Node in two
        tree_node_handle splitNodeHandle = rootNode->splitNode();
        auto splitNode = tree2.node_allocator_.get_tree_node<NodeType>(
                splitNodeHandle );

        // Test the split
        REQUIRE(rootNode->cur_offset_ == 4);
        REQUIRE(std::get<Point>(rootNode->entries[0]) == Point(-9.0, 7.0));
        REQUIRE(std::get<Point>(rootNode->entries[1]) == Point(-14.0, 8.0));
        REQUIRE(std::get<Point>(rootNode->entries[2]) == Point(-10.0, 8.0));
        REQUIRE(std::get<Point>(rootNode->entries[3]) == Point(-8.0, 8.0));
        REQUIRE(splitNode->cur_offset_ == 4);
        REQUIRE(std::get<Point>(splitNode->entries[0]) == Point(-9.0, 9.0));
        REQUIRE(std::get<Point>(splitNode->entries[1]) == Point(-8.0, 9.0));
        REQUIRE(std::get<Point>(splitNode->entries[2]) == Point(-9.0, 10.0));
        REQUIRE(std::get<Point>(splitNode->entries[3]) == Point(-8.0, 10.0));
        REQUIRE(rootNode->level == splitNode->level);
    }
    unlink( "rstardiskbacked.txt" );


	// Test set three
	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// {(-5, 4), 1, 1}, {(-2, 4), 1, 1}, {(-1, 3), 1, 1}, {(-1, 1), 1, 1}, {(-3, 0), 1, 1},
	// {(-6, 2), 1, 1}
    {
        TreeType tree3( 4096 * 5, "rstardiskbacked.txt" );
        auto rootNode = tree3.node_allocator_.get_tree_node<NodeType>(
                tree3.root );
        tree_node_handle dummys[7];
        for( int i = 0; i < 7; i++ ) {
            auto alloc_data = tree3.node_allocator_.create_new_tree_node<NodeType>();
            new (&(*alloc_data.first)) NodeType( &tree3,
                    alloc_data.second,
                    tree3.root, 0 );
            dummys[i] = alloc_data.second;
        }

        rootNode->level = 1;
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-6.0, 3.0,
                        nextafter(-4.0,DBL_MAX), nextafter(5.0,DBL_MAX)), dummys[0]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-6.0, 3.0,
                        nextafter(-4.0,DBL_MAX), nextafter(5.0,DBL_MAX)), dummys[1]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-3.0, 3.0,
                        nextafter(-1.0,DBL_MAX), nextafter(5.0,DBL_MAX)), dummys[2]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-2.0, 2.0,
                        nextafter(0.0,DBL_MAX), nextafter(4.0,DBL_MAX)), dummys[3]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-2.0, 0.0,
                        nextafter(0.0,DBL_MAX), nextafter(2.0,DBL_MAX)), dummys[4]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-4.0, -1.0,
                        nextafter(-2.0,DBL_MAX), nextafter(1.0,DBL_MAX)), dummys[5]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-7.0, 1.0,
                        nextafter(-5.0,DBL_MAX), nextafter(3.0,DBL_MAX)), dummys[6]));


        // Extra rstartree::Node causing the split
        auto alloc_data =
            tree3.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree3, alloc_data.second,
                tree3.root, 0 );


        auto extra_handle = alloc_data.second;
        auto extraNode = alloc_data.first;
        extraNode->addEntryToNode(Point(1.0, 1.0));
        extraNode->addEntryToNode(Point(2.0, 2.0));

        REQUIRE( extraNode->level == 0 );

        // Test the split
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(extraNode->boundingBox(), extra_handle) );
        tree_node_handle splitHandle = rootNode->splitNode();
        auto splitNode = tree3.node_allocator_.get_tree_node<NodeType>(
                splitHandle );
        REQUIRE(splitNode->level == rootNode->level);
        REQUIRE(splitNode->level == 1);

        REQUIRE(rootNode->cur_offset_ == 3);
        REQUIRE(std::get<NodeType::Branch>(rootNode->entries[0]).boundingBox
                == Rectangle(-7.0, 1.0, nextafter(-5.0,DBL_MAX),
                    nextafter(3.0,DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(rootNode->entries[1]).boundingBox
                == Rectangle(-6.0, 3.0, nextafter(-4.0,DBL_MAX),
                    nextafter(5.0,DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(rootNode->entries[2]).boundingBox
                == Rectangle(-6.0, 3.0, nextafter(-4.0,DBL_MAX),
                    nextafter(5.0,DBL_MAX)));
        REQUIRE(splitNode->cur_offset_ == 5);
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[0]).boundingBox
                == Rectangle(-4.0, -1.0, nextafter(-2.0,DBL_MAX),
                    nextafter(1.0,DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[1]).boundingBox
                == Rectangle(-3.0, 3.0, nextafter(-1.0,DBL_MAX),
                    nextafter(5.0,DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[2]).boundingBox
                == Rectangle(-2.0, 2.0, nextafter(0.0,DBL_MAX),
                    nextafter(4.0,DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[3]).boundingBox
                == Rectangle(-2.0, 0.0, nextafter(0.0,DBL_MAX),
                    nextafter(2.0,DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[4]).boundingBox
                == Rectangle(1.0, 1.0, nextafter(2.0, DBL_MAX),
                    nextafter(2.0, DBL_MAX)));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[4]).child
                == extra_handle);
    }
    unlink( "rstardiskbacked.txt" );
	
}


TEST_CASE("R*TreeDisk: testInsertOverflowReInsertAndSplit")
{
    unlink( "rstardiskbacked.txt" );
	// From paper:
	// If level is not the root level and this is the first call of
	// overflowTreatment in the current level during a single rectangle insertion,
	// then reInsert. Else, split.

	// Leaf rtree::Node and new sibling leaf
	// Cluster 4, n = 7
    {
        TreeType tree(4096*5, "rstardiskbacked.txt");
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                tree.root ); 
        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto childNode = alloc_data.first;
        new (&(*childNode)) NodeType( &tree, alloc_data.second,
                tree.root, 0 );
        tree_node_handle child_handle = alloc_data.second;
        childNode->addEntryToNode(Point(-30.0, -30.0));
        childNode->addEntryToNode(Point(30.0, 30.0));
        childNode->addEntryToNode(Point(-20.0, -20.0));
        childNode->addEntryToNode(Point(20.0, 20.0));
        childNode->addEntryToNode(Point(-10.0, -10.0));
        childNode->addEntryToNode(Point(10.0, 10.0));
        childNode->addEntryToNode(Point(0.0, 0.0));

        REQUIRE( childNode->level == 0 );

        // Root rstartree::Node
        rootNode->level = 1;
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(childNode->boundingBox(), child_handle));
        childNode->parent = tree.root;

        Point point(0.0,0.0);

        REQUIRE(childNode->cur_offset_  == 7);
        Point center = childNode->boundingBox().centrePoint();
        REQUIRE(center[0] == (nextafter(30.0,DBL_MAX) -30.0)/2 );
        REQUIRE(center[1] == (nextafter(30.0,DBL_MAX) -30.0)/2 );
        REQUIRE(rootNode->checksum() == 0);

        // We are not the root and we haven't inserted anything yet. Should call reinsert
        // But then we will fill out the node again. Then we call split.
        std::vector<bool> reInsertedAtLevel = { false, false };
        rootNode->insert(point, reInsertedAtLevel);

        // We tried to reinsert, but it won't work.
        REQUIRE(reInsertedAtLevel[0] == true);
        REQUIRE(reInsertedAtLevel[1] == false);

        // Should force split
        REQUIRE(rootNode->cur_offset_ == 2);
        REQUIRE(rootNode->level == 1);

        // We will have split along the x axis (y axis isomorphic so we prefer x).
        // Overlap is always zero between any cut along X. Cumulative area is minimized at 3,5 or 5,3 split.
        // We prefer 3,5.
        NodeType::Branch bLeft = std::get<NodeType::Branch>(rootNode->entries[0]);
        NodeType::Branch bRight = std::get<NodeType::Branch>(rootNode->entries[1]);
        auto leftNode = tree.node_allocator_.get_tree_node<NodeType>(
                bLeft.child );
        auto rightNode = tree.node_allocator_.get_tree_node<NodeType>(
                bRight.child );
        REQUIRE(leftNode->cur_offset_ == 3);
        REQUIRE(rightNode->cur_offset_ == 5);

        REQUIRE(std::get<Point>(leftNode->entries[0]) == Point(-30,-30));
        REQUIRE(std::get<Point>(leftNode->entries[1]) == Point(-20,-20));
        REQUIRE(std::get<Point>(leftNode->entries[2]) == Point(-10,-10));

        REQUIRE(std::get<Point>(rightNode->entries[0]) == Point(0,0));
        REQUIRE(std::get<Point>(rightNode->entries[1]) == Point(0,0));
        REQUIRE(std::get<Point>(rightNode->entries[2]) == Point(10,10));
        REQUIRE(std::get<Point>(rightNode->entries[3]) == Point(20,20));
        REQUIRE(std::get<Point>(rightNode->entries[4]) == Point(30,30));
        REQUIRE(leftNode->level == 0);
        REQUIRE(rightNode->level == 0);
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testInsertGrowTreeHeight")
{
    unlink( "rstardiskbacked.txt" );
    {
        std::vector<bool> reInsertedAtLevel = { false };
        unsigned maxBranchFactor = 7;
        TreeType tree(4096*5, "rstardiskbacked.txt");
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                tree.root );
        REQUIRE( rootNode->level == 0 );

        for( unsigned i = 0; i < maxBranchFactor + 1; i++) {
            tree_node_handle root = rootNode->insert(Point(0.0, 0.0), reInsertedAtLevel);
            rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                    root );
        }

        REQUIRE(rootNode->cur_offset_ == 2);
        NodeType::Branch bLeft = std::get<NodeType::Branch>(rootNode->entries[0]);
        NodeType::Branch bRight = std::get<NodeType::Branch>(rootNode->entries[1]);

        auto left = tree.node_allocator_.get_tree_node<NodeType>(
                bLeft.child );
        auto right = tree.node_allocator_.get_tree_node<NodeType>(
                bRight.child );

        REQUIRE(left->cur_offset_ == 3);
        REQUIRE(left->level == 0);
        REQUIRE(right->cur_offset_ == 5);
        REQUIRE(right->level == 0);
        REQUIRE(rootNode->level == 1);
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testSplitNonLeafNode")
{
    unlink( "rstardiskbacked.txt" );
    {
        unsigned maxBranchFactor = 7;
        TreeType tree(4096*5, "rstardiskbacked.txt" );
        auto root_handle = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                root_handle );
        rootNode->level = 1;

        for( unsigned i = 0; i < maxBranchFactor; i++) {
            auto child_handle = createFullLeafNode( tree, tree.root );
            auto childNode = tree.node_allocator_.get_tree_node<NodeType>(
                    child_handle );
            rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                    NodeType::Branch>( childNode->boundingBox(),
                        child_handle ) );
        }

        unsigned height = rootNode->height();
        tree.hasReinsertedOnLevel = std::vector(height, false);

        // Should be 49 points
        std::vector<Point> accumulator = tree.search(Point(0.0, 0.0));
        REQUIRE(accumulator.size() == maxBranchFactor * maxBranchFactor);

        REQUIRE(rootNode->cur_offset_ == maxBranchFactor);
        tree.insert(Point(0.0, 0.0));
        auto new_root_handle = tree.root;
        REQUIRE( new_root_handle != root_handle );
        auto newRootNode = tree.node_allocator_.get_tree_node<NodeType>(
                new_root_handle );

        // Confirm tree structure
        REQUIRE(newRootNode->cur_offset_ == 2);
        const NodeType::Branch &bLeft = std::get<NodeType::Branch>(
                newRootNode->entries[0] );
        const NodeType::Branch &bRight = std::get<NodeType::Branch>(
                newRootNode->entries[1] );
        auto left = tree.node_allocator_.get_tree_node<NodeType>(
                bLeft.child );
        auto right = tree.node_allocator_.get_tree_node<NodeType>(
                bRight.child );

        REQUIRE(left->cur_offset_ == 3);
        REQUIRE(right->cur_offset_ == 5);

        for( size_t i = 0; i < left->cur_offset_; i++ ) {

            auto child_handle =
                std::get<NodeType::Branch>(left->entries.at(i)).child;
            auto childNode =
                tree.node_allocator_.get_tree_node<NodeType>(
                        child_handle );

            // These are all leaves
            REQUIRE(std::holds_alternative<Point>(childNode->entries[0]));
        }

        for( size_t i = 0; i < right->cur_offset_; i++ ) {
            auto child_handle =
                std::get<NodeType::Branch>(right->entries.at(i)).child;
            auto childNode =
                tree.node_allocator_.get_tree_node<NodeType>(
                        child_handle );
        }

        // Count
        accumulator = newRootNode->search( Point(0.0, 0.0));
        REQUIRE(accumulator.size() == maxBranchFactor * maxBranchFactor + 1);
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: RemoveLeafNode")
{

    unlink( "rstardiskbacked.txt" );
    {
        unsigned maxBranchFactor = 7;
        unsigned minBranchFactor = 3;
        TreeType tree(4096*5, "rstardiskbacked.txt");

        for( unsigned i = 0; i < maxBranchFactor*maxBranchFactor + 1;
                i++) {
            tree.insert(Point(i, i));
        }

        for (unsigned i = 0; i < maxBranchFactor*maxBranchFactor + 1;
                i++) {
            Point p(i, i);
            REQUIRE(tree.search(p).size() == 1);
        }

        //Find a leaf
        auto root = tree.root;
        auto node = tree.node_allocator_.get_tree_node<NodeType>(
                root );
        while( std::holds_alternative<NodeType::Branch>(node->entries[0])) {
            const NodeType::Branch &b = std::get<NodeType::Branch>(node->entries[0]);
            node = tree.node_allocator_.get_tree_node<NodeType>( b.child
                    );
        }

        REQUIRE( std::holds_alternative<Point>(node->entries[0]) );
        size_t cnt = node->cur_offset_;
        std::vector<NodeType::NodeEntry> nodesToRemove( node->entries.begin(), node->entries.begin() + (cnt-minBranchFactor + 1));
        for (const auto &entry : nodesToRemove)
        {
            const Point &p = std::get<Point>(entry);
            tree.remove(p);
        }

        for (unsigned i = 0; i < maxBranchFactor*maxBranchFactor + 1; ++i)
        {
            Point p(i, i);
            NodeType::NodeEntry ne = p;
            if (std::find(nodesToRemove.begin(), nodesToRemove.end(), ne) == nodesToRemove.end())
            {
                REQUIRE(tree.search(p).size() == 1);
            }
            else
            {
                REQUIRE(tree.search(p).size() == 0);
            }
        }
    }

    unlink( "rstardiskbacked.txt" );
	
}

TEST_CASE("R*TreeDisk: testSearch")
{
	// Build the tree directly

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
    unlink( "rstardiskbacked.txt" );
    {
        TreeType tree( 4096*5, "rstardiskbacked.txt");
        auto root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                root );

        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle( nullptr ), 0 );
        auto cluster1a_handle = alloc_data.second;
        auto cluster1a = alloc_data.first;
        cluster1a->addEntryToNode(Point(-3.0, 16.0));
        cluster1a->addEntryToNode(Point(-3.0, 15.0));
        cluster1a->addEntryToNode(Point(-4.0, 13.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle( nullptr ), 0 );
        auto cluster1b_handle = alloc_data.second;
        auto cluster1b = alloc_data.first;
        cluster1b->addEntryToNode(Point(-5.0, 12.0));
        cluster1b->addEntryToNode(Point(-5.0, 15.0));
        cluster1b->addEntryToNode(Point(-6.0, 14.0));
        cluster1b->addEntryToNode(Point(-8.0, 16.0));

        // Cluster 2, n = 8
        // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster2a_handle = alloc_data.second;
        auto cluster2a = alloc_data.first;
        cluster2a->addEntryToNode(Point(-8.0, 10.0));
        cluster2a->addEntryToNode(Point(-9.0, 10.0));
        cluster2a->addEntryToNode(Point(-8.0, 9.0));
        cluster2a->addEntryToNode(Point(-9.0, 9.0));
        cluster2a->addEntryToNode(Point(-8.0, 8.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster2b_handle = alloc_data.second;
        auto cluster2b = alloc_data.first;
        cluster2b->addEntryToNode(Point(-14.0, 8.0));
        cluster2b->addEntryToNode(Point(-10.0, 8.0));
        cluster2b->addEntryToNode(Point(-9.0, 7.0));

        // Cluster 3, n = 9
        // (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster3a_handle = alloc_data.second;
        auto cluster3a = alloc_data.first;
        cluster3a->addEntryToNode(Point(-3.0, 4.0));
        cluster3a->addEntryToNode(Point(-3.0, 0.0));
        cluster3a->addEntryToNode(Point(-2.0, 4.0));
        cluster3a->addEntryToNode(Point(-1.0, 3.0));
        cluster3a->addEntryToNode(Point(-1.0, 1.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster3b_handle = alloc_data.second;
        auto cluster3b = alloc_data.first;
        cluster3b->addEntryToNode(Point(-5.0, 4.0));
        cluster3b->addEntryToNode(Point(-4.0, 3.0));
        cluster3b->addEntryToNode(Point(-4.0, 1.0));
        cluster3b->addEntryToNode(Point(-6.0, 2.0));

        // High level rstartree::Nodes
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto left_handle = alloc_data.second;
        auto left = alloc_data.first;
        cluster1a->parent = left_handle;
        left->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(cluster1a->boundingBox(),
                    cluster1a_handle));
        cluster1b->parent = left_handle;
        left->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(cluster1b->boundingBox(),
                    cluster1b_handle));
        cluster2a->parent = left_handle;
        left->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(cluster2a->boundingBox(),
                    cluster2a_handle));
        cluster2b->parent = left_handle;
        left->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(cluster2b->boundingBox(),
                    cluster2b_handle));
        left->level = 1;

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        new (&(*alloc_data.first)) NodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto right_handle = alloc_data.second;
        auto right = alloc_data.first;
        cluster3a->parent = right_handle;
        right->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(cluster3a->boundingBox(),
                    cluster3a_handle));
        cluster3b->parent = right_handle;
        right->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(cluster3b->boundingBox(),
                    cluster3b_handle));
        right->level = 1;

        left->parent = root;
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(left->boundingBox(),
                    left_handle));
        right->parent = root;
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(right->boundingBox(),
                    right_handle));
        rootNode->level = 2;

        // Test search

        // Test set one
        Rectangle sr1 = Rectangle(-9.0, 9.5, nextafter(-5.0, DBL_MAX),
                nextafter(12.5, DBL_MAX));
        std::vector<Point> v1 = rootNode->search(sr1);
        REQUIRE(v1.size() == 3);
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

        // Test set two
        Rectangle sr2 = Rectangle(-8.0, 4.0, nextafter(-5.0, DBL_MAX),
                nextafter(8.0, DBL_MAX));
        std::vector<Point> v2 = rootNode->search(sr2);
        REQUIRE(v2.size() == 2);
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

        // Test set three
        Rectangle sr3 = Rectangle(-8.0, 0.0, nextafter(-4.0, DBL_MAX),
                nextafter(16.0, DBL_MAX));
        std::vector<Point> v3 = rootNode->search(sr3);
        REQUIRE(v3.size() == 12);
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-5.0, 4.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-4.0, 3.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-4.0, 1.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-6.0, 2.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 10.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 9.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 8.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-5.0, 12.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-5.0, 15.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-6.0, 14.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 16.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-4.0, 13.0)) != v3.end());

        // Test set four
        Rectangle sr4 = Rectangle(2.0, -4.0, nextafter(4.0, DBL_MAX),
                nextafter(-2.0, DBL_MAX));
        std::vector<Point> v4 = rootNode->search(sr4);
        REQUIRE(v4.size() == 0);

        // Test set five
        Rectangle sr5 = Rectangle(-3.5, 1.0, nextafter(-1.5, DBL_MAX),
                nextafter(3.0, DBL_MAX));
        std::vector<Point> v5 = rootNode->search(sr5);
        REQUIRE(v5.size() == 0);
    }
    {
        // Re-read the tree from disk, doall the searches again
        TreeType tree( 4096*5, "rstardiskbacked.txt");
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                tree.root );
        // Test set one
        Rectangle sr1 = Rectangle(-9.0, 9.5, nextafter(-5.0, DBL_MAX),
                nextafter(12.5, DBL_MAX));
        std::vector<Point> v1 = rootNode->search(sr1);
        REQUIRE(v1.size() == 3);
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

        // Test set two
        Rectangle sr2 = Rectangle(-8.0, 4.0, nextafter(-5.0, DBL_MAX),
                nextafter(8.0, DBL_MAX));
        std::vector<Point> v2 = rootNode->search(sr2);
        REQUIRE(v2.size() == 2);
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

        // Test set three
        Rectangle sr3 = Rectangle(-8.0, 0.0, nextafter(-4.0, DBL_MAX),
                nextafter(16.0, DBL_MAX));
        std::vector<Point> v3 = rootNode->search(sr3);
        REQUIRE(v3.size() == 12);
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-5.0, 4.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-4.0, 3.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-4.0, 1.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-6.0, 2.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 10.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 9.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 8.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-5.0, 12.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-5.0, 15.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-6.0, 14.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-8.0, 16.0)) != v3.end());
        REQUIRE(std::find( v3.begin(), v3.end(), Point(-4.0, 13.0)) != v3.end());

        // Test set four
        Rectangle sr4 = Rectangle(2.0, -4.0, nextafter(4.0, DBL_MAX),
                nextafter(-2.0, DBL_MAX));
        std::vector<Point> v4 = rootNode->search(sr4);
        REQUIRE(v4.size() == 0);

        // Test set five
        Rectangle sr5 = Rectangle(-3.5, 1.0, nextafter(-1.5, DBL_MAX),
                nextafter(3.0, DBL_MAX));
        std::vector<Point> v5 = rootNode->search(sr5);
        REQUIRE(v5.size() == 0);
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: reInsertAccountsForNewTreeDepth")
{
    unlink( "rstardiskbacked.txt" );
    {
        // Need to construct a tree of depth at least 3.
        unsigned maxBranchFactor = 7;
        TreeType tree(4096*10, "rstardiskbacked.txt");
        std::vector<tree_node_handle> leafNodes;
        for (unsigned i = 0; i < maxBranchFactor*maxBranchFactor + 1; i++)
        {
            tree_node_handle leaf = createFullLeafNode(tree,
                    tree_node_handle( nullptr ));
            auto leafNode =
                tree.node_allocator_.get_tree_node<NodeType>( leaf );
            leafNode->level = 0;
            leafNodes.push_back(leaf);
        }

        auto root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
                );
        rootNode->level = 2;

        // Construct intermediate layer
        std::vector<tree_node_handle> middleLayer;
        for (unsigned i = 0; i < 7; i++)
        {
            auto alloc_data =
                tree.node_allocator_.create_new_tree_node<NodeType>();
            new (&(*alloc_data.first)) NodeType( &tree,
                    alloc_data.second, tree_node_handle( nullptr ), 0 );
            auto child_handle = alloc_data.second;
            auto child = alloc_data.first;
            child->level = 1;
            child->parent = root;
            for (unsigned j = 0; j < 7; j++)
            {
                auto leaf = leafNodes.at(7*i + j);
                auto leafNode =
                    tree.node_allocator_.get_tree_node<NodeType>( leaf
                            );
                child->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                        NodeType::Branch>(leafNode->boundingBox(), leaf));
                leafNode->parent = child_handle;
            }
            rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                    NodeType::Branch>(child->boundingBox(), child_handle));
            middleLayer.push_back(child_handle);
        }

        // Emulate a case where we need to reinsert some extra entries in the middle layer,
        // but a reinsertion forces a split while we still have entries outstanding.
        // Shove two extra things into middleLayer[0]

        auto leaf = leafNodes.at(maxBranchFactor*maxBranchFactor);
        auto leafNode = tree.node_allocator_.get_tree_node<NodeType>(
                leaf);

        leafNode->level = 0;
        leafNode->parent = middleLayer.at(0);

        auto middleNode = tree.node_allocator_.get_tree_node<NodeType>(
                middleLayer.at(0) );
        middleNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(leafNode->boundingBox(), leaf));


        std::vector<bool> hasReinsertedOnLevel = {false, true, false};

        REQUIRE(middleNode->cur_offset_ > maxBranchFactor );
        middleNode->reInsert(hasReinsertedOnLevel);

        for(auto leaf : leafNodes) {
            leafNode = tree.node_allocator_.get_tree_node<NodeType>(
                leaf);

            REQUIRE(leafNode->level == 0);
        }

        for (auto middle : middleLayer)
        {
            middleNode = tree.node_allocator_.get_tree_node<NodeType>(
                middle);

            REQUIRE(middleNode->level == 1);
        }

        REQUIRE(rootNode->level == 2 );
        REQUIRE(rootNode->parent != nullptr);
        auto newRootNode = tree.node_allocator_.get_tree_node<NodeType>(
                rootNode->parent );
        REQUIRE(newRootNode->level == 3);
    }
    unlink("rstardiskbacked.txt" );
}
#endif
