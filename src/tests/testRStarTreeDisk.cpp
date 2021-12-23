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
    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>( NodeHandleType( LEAF_NODE ) );
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
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        tree_node_handle root = alloc_data_root.second;
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle(nullptr), 1);
        auto rootNode = alloc_data_root.first;
        
        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child0, root, 0 );
        auto entry =
            createBranchEntry( Rectangle(8.0, 1.0, 12.0, 5.0),
                    alloc_data.second);
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ));
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child1, root, 0 );
        entry =
            createBranchEntry(
                    Rectangle(12.0, -4.0, 16.0, -2.0), child1);
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child2, root, 0 );
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

        auto alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 1 );
        tree_node_handle root = alloc_data_root.second;
        auto rootNode = alloc_data_root.first;
        
        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child0, root, 0 );
        rootNode->addBranchToNode( 
            createBranchEntry( Rectangle(8.0, 12.0, 10.0, 14.0), child0 ) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child1, root, 0 );
        rootNode->addBranchToNode( 
            createBranchEntry( Rectangle(10.0, 12.0, 12.0, 14.0), child1) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) LeafNodeType( &tree, child2, root, 0 );
        rootNode->addBranchToNode(
            createBranchEntry( Rectangle(12.0, 12.0, 14.0, 14.0), child2 ) );

        REQUIRE( rootNode->cur_offset_ ==  3 );

        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testUpdateBoundingBox") {

    unlink( "rstardiskbacked.txt" );
	TreeType tree(4096, "rstardiskbacked.txt");

    auto alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle(nullptr), 1 );
    auto parentNode = alloc_data_root.first;
    tree_node_handle root = alloc_data_root.second;

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child0, root, 0 );
    auto child0Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry( Rectangle(8.0, -6.0, 10.0, -4.0), child0 ) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child1, root, 0 );
    auto child1Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry(Rectangle(12.0, -4.0, 16.0, -2.0), child1) );


    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child2, root, 0 );
    auto child2Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry(Rectangle(10.0, 12.0, 12.0, 14.0), child2) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child3, root, 0 );
    auto child3Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry(Rectangle(12.0, 12.0, 14.0, 14.0), child3) );

    REQUIRE( parentNode->cur_offset_ == 4 );

	parentNode->updateBoundingBox(child3, Rectangle(3.0, 3.0, 5.0, 5.0));

	const auto &b = parentNode->entries[3];
	REQUIRE(b.boundingBox == Rectangle(3.0, 3.0, 5.0, 5.0));
    REQUIRE(parentNode->level == 1);
    REQUIRE(child0Node->level == 0);
    REQUIRE(child1Node->level == 0);
    REQUIRE(child2Node->level == 0);
    REQUIRE(child3Node->level == 0);

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE( "R*TreeDisk: testRemoveChild" ) {
    unlink( "rstardiskbacked.txt" );
	TreeType tree(4096, "rstardiskbacked.txt");

    auto alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle(nullptr), 1 );

    tree_node_handle root = alloc_data_root.second;
    auto parentNode = alloc_data_root.first;

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child0, root, 1 );
    auto child0Node = alloc_data.first;
	parentNode->addBranchToNode( createBranchEntry(Rectangle(8.0, -6.0, 10.0, -4.0), child0) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child1, root, 0 );
    auto child1Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry(Rectangle(12.0, -4.0, 16.0, -2.0), child1) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child2, root, 0 );
    auto child2Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry(Rectangle(10.0, 12.0, 12.0, 14.0), child2) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) LeafNodeType( &tree, child3, root, 0 );
    auto child3Node = alloc_data.first;
    parentNode->addBranchToNode( createBranchEntry( Rectangle(12.0, 12.0, 14.0, 14.0), child3) );

    REQUIRE( parentNode->cur_offset_ ==  4 );

	// Remove one of the children
	parentNode->removeChild( child3 );
    REQUIRE( parentNode->cur_offset_ == 3 );

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testRemoveData")
{

    unlink( "rstardiskbacked.txt" );

	// Setup a rtree::Node with some data
	TreeType tree( 4096, "rstardiskbacked.txt" );
    auto parentNode = tree.node_allocator_->get_tree_node<LeafNodeType>(
            tree.root );
    REQUIRE( parentNode->level == 0 );

    parentNode->addPoint( Point(9.0, -5.0) );
	parentNode->addPoint( Point(14.0, -3.0) );
	parentNode->addPoint( Point(11.0, 13.0) );
	parentNode->addPoint( Point(13.0, 13.0) );

	REQUIRE(parentNode->cur_offset_ == 4);

	// Remove some of the data
	parentNode->removePoint( Point(13.0, 13.0) );

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

    auto alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle(nullptr), 2 );
    tree_node_handle root = alloc_data_root.second;
    auto rootNode = alloc_data_root.first;

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto leftNode = alloc_data.first;
    tree_node_handle left = alloc_data.second;
    new (&(*leftNode)) BranchNodeType( &tree, left, root, 1 );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto rightNode = alloc_data.first;
    tree_node_handle right = alloc_data.second;
    new (&(*rightNode)) BranchNodeType( &tree, right, root, 1 );

    tree_node_handle leftChild0 = createFullLeafNode(tree, left);
    tree_node_handle leftChild1 = createFullLeafNode(tree, left);
    tree_node_handle leftChild2 = createFullLeafNode(tree, left);
    leftNode->addBranchToNode(
            createBranchEntry( Rectangle(8.0, 12.0, 10.0, 14.0), leftChild0 ) );
    leftNode->addBranchToNode(
            createBranchEntry( Rectangle(10.0, 12.0, 12.0, 14.0), leftChild1 ) );
    leftNode->addBranchToNode( createBranchEntry( Rectangle(12.0, 12.0, 14.0, 14.0), leftChild2 ) );
    rootNode->addBranchToNode( createBranchEntry( Rectangle(8.0, 12.0, 14.0, 14.0), left ) );

    tree_node_handle rightChild0 = createFullLeafNode(tree,right);
    tree_node_handle rightChild1 = createFullLeafNode(tree,right);
    tree_node_handle rightChild2 = createFullLeafNode(tree,right);
    rightNode->addBranchToNode( createBranchEntry( Rectangle(8.0, 1.0, 12.0, 5.0), rightChild0 ));
    rightNode->addBranchToNode( createBranchEntry( Rectangle(12.0, -4.0, 16.0, -2.0), rightChild1 ));
    rightNode->addBranchToNode( createBranchEntry( Rectangle(8.0, -6.0, 10.0, -4.0), rightChild2 ));
    rootNode->addBranchToNode( createBranchEntry( Rectangle(8.0, -6.0, 16.0, 5.0), right ));

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
    auto alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle( nullptr ), 2 );
    tree_node_handle root = alloc_data_root.second;
    auto rootNode = alloc_data_root.first;

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster4aNode = alloc_data.first;
    tree_node_handle cluster4a = alloc_data.second;
    new (&(*cluster4aNode)) LeafNodeType( &tree, cluster4a,
            tree_node_handle() /*nullptr for now*/, 0 );
    cluster4aNode->addPoint( Point(-10.0, -2.0) );
    cluster4aNode->addPoint( Point(-12.0, -3.0) );
    cluster4aNode->addPoint( Point(-11.0, -3.0) );
    cluster4aNode->addPoint( Point(-10.0, -3.0) );
	
    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster4bNode = alloc_data.first;
    tree_node_handle cluster4b = alloc_data.second;
    new (&(*cluster4bNode)) LeafNodeType( &tree, cluster4b,
            tree_node_handle() /*nullptr for now*/, 0 );

	cluster4bNode->addPoint( Point(-9.0, -3.0) );
	cluster4bNode->addPoint( Point(-7.0, -3.0) );
	cluster4bNode->addPoint( Point(-10.0, -5.0) );

    alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto cluster4Node = alloc_data_root.first;
    tree_node_handle cluster4 = alloc_data_root.second;
    new (&(*cluster4Node)) BranchNodeType( &tree, cluster4, root, 1 );

	cluster4aNode->parent = cluster4;
	cluster4Node->addBranchToNode(
        createBranchEntry( cluster4aNode->boundingBox(), cluster4a) );
	cluster4bNode->parent = cluster4;
	cluster4Node->addBranchToNode(
        createBranchEntry( cluster4bNode->boundingBox(), cluster4b));

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5aNode = alloc_data.first;
    tree_node_handle cluster5a = alloc_data.second;
    new (&(*cluster5aNode)) LeafNodeType( &tree, cluster5a, tree_node_handle() /* nullptr for now */, 0 );
	cluster5aNode->addPoint( Point(-14.5, -13.0) );
	cluster5aNode->addPoint( Point(-14.0, -13.0) );
	cluster5aNode->addPoint( Point(-13.5, -13.5) );
	cluster5aNode->addPoint( Point(-15.0, -14.0) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5bNode = alloc_data.first;
    tree_node_handle cluster5b = alloc_data.second;
    new (&(*cluster5bNode)) LeafNodeType( &tree, cluster5b,
                tree_node_handle() /* nullptr for now */, 0 );
	cluster5bNode->addPoint( Point(-14.0, -14.0) );
	cluster5bNode->addPoint( Point(-13.0, -14.0) );
	cluster5bNode->addPoint( Point(-12.0, -14.0) );
	cluster5bNode->addPoint( Point(-13.5, -16.0) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5cNode = alloc_data.first;
    tree_node_handle cluster5c = alloc_data.second;
    new (&(*cluster5cNode)) LeafNodeType( &tree, cluster5c,
                tree_node_handle() /* nullptr for now */, 0 );

	cluster5cNode->addPoint( Point(-15.0, -14.5) );
	cluster5cNode->addPoint( Point(-14.0, -14.5) );
	cluster5cNode->addPoint( Point(-12.5, -14.5) );
	cluster5cNode->addPoint( Point(-13.5, -15.5) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5dNode = alloc_data.first;
    tree_node_handle cluster5d = alloc_data.second;
    new (&(*cluster5dNode)) LeafNodeType( &tree, cluster5d,
                tree_node_handle() /* nullptr for now */, 0 );
	cluster5dNode->addPoint( Point(-15.0, -15.0));
	cluster5dNode->addPoint( Point(-14.0, -15.0));
	cluster5dNode->addPoint( Point(-13.0, -15.0));
	cluster5dNode->addPoint( Point(-12.0, -15.0));
	cluster5dNode->addPoint( Point(-15.0, -15.0));

    alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto cluster5Node = alloc_data_root.first;
    tree_node_handle cluster5 = alloc_data_root.second;
    new (&(*cluster5Node)) BranchNodeType( &tree, cluster5, root, 1 );

	cluster5aNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(cluster5aNode->boundingBox(),cluster5a));
	cluster5bNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(cluster5bNode->boundingBox(), cluster5b));


	cluster5cNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(cluster5cNode->boundingBox(), cluster5c));
	cluster5dNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(cluster5dNode->boundingBox(), cluster5d));

	// Root
	rootNode->addBranchToNode(
        createBranchEntry(cluster4Node->boundingBox(), cluster4));
	rootNode->addBranchToNode(
        createBranchEntry(cluster5Node->boundingBox(), cluster5));
	REQUIRE( rootNode->level == 2 );


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

        auto alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 2 );
        root = alloc_data_root.second;
        auto rootNode = alloc_data_root.first;

        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster4aNode = alloc_data.first;
        cluster4a = alloc_data.second;
        new (&(*cluster4aNode)) LeafNodeType( &tree, cluster4a, tree_node_handle() /*nullptr for now*/, 0 );
        cluster4aNode->addPoint( Point(-10.0, -2.0) );
        cluster4aNode->addPoint( Point(-12.0, -3.0) );
        cluster4aNode->addPoint( Point(-11.0, -3.0) );
        cluster4aNode->addPoint( Point(-10.0, -3.0) );
        
        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster4bNode = alloc_data.first;
        cluster4b = alloc_data.second;
        new (&(*cluster4bNode)) LeafNodeType( &tree, cluster4b, tree_node_handle() /*nullptr for now*/, 0 );
        cluster4bNode->addPoint( Point(-9.0, -3.0) );
        cluster4bNode->addPoint( Point(-7.0, -3.0) );
        cluster4bNode->addPoint( Point(-10.0, -5.0) );

        alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        auto cluster4Node = alloc_data_root.first;
        cluster4 = alloc_data_root.second;
        new (&(*cluster4Node)) BranchNodeType( &tree, cluster4, root, 1 );
        cluster4aNode->parent = cluster4;
        cluster4Node->addBranchToNode(
            createBranchEntry( cluster4aNode->boundingBox(), cluster4a) );
        cluster4bNode->parent = cluster4;
        cluster4Node->addBranchToNode(
            createBranchEntry( cluster4bNode->boundingBox(), cluster4b));

        // Cluster 5, n = 16
        // (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
        // (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
        // (-14, -15), (-13, -15), (-12, -15)

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5aNode = alloc_data.first;
        cluster5a = alloc_data.second;
        new (&(*cluster5aNode)) LeafNodeType( &tree, cluster5a, tree_node_handle() /* nullptr for now */, 0 );
        cluster5aNode->addPoint( Point(-14.5, -13.0) );
        cluster5aNode->addPoint( Point(-14.0, -13.0) );
        cluster5aNode->addPoint( Point(-13.5, -13.5) );
        cluster5aNode->addPoint( Point(-15.0, -14.0) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5bNode = alloc_data.first;
        cluster5b = alloc_data.second;
        new (&(*cluster5bNode)) LeafNodeType( &tree, cluster5b, tree_node_handle() /* nullptr for now */, 0 );
        cluster5bNode->addPoint( Point(-14.0, -14.0) );
        cluster5bNode->addPoint( Point(-13.0, -14.0) );
        cluster5bNode->addPoint( Point(-12.0, -14.0) );
        cluster5bNode->addPoint( Point(-13.5, -16.0) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5cNode = alloc_data.first;
        cluster5c = alloc_data.second;
        new (&(*cluster5cNode)) LeafNodeType( &tree, cluster5c, tree_node_handle() /* nullptr for now */, 0 );

        cluster5cNode->addPoint( Point(-15.0, -14.5) );
        cluster5cNode->addPoint( Point(-14.0, -14.5) );
        cluster5cNode->addPoint( Point(-12.5, -14.5) );
        cluster5cNode->addPoint( Point(-13.5, -15.5) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5dNode = alloc_data.first;
        cluster5d = alloc_data.second;
        new (&(*cluster5dNode)) LeafNodeType( &tree, cluster5d,
                    tree_node_handle() /* nullptr for now */, 0 );
        cluster5dNode->addPoint( Point(-15.0, -15.0) );
        cluster5dNode->addPoint( Point(-14.0, -15.0) );
        cluster5dNode->addPoint( Point(-13.0, -15.0) );
        cluster5dNode->addPoint( Point(-12.0, -15.0) );
        cluster5dNode->addPoint( Point(-15.0, -15.0) );

        alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        auto cluster5Node = alloc_data_root.first;
        cluster5 = alloc_data_root.second;
        new (&(*cluster5Node)) BranchNodeType( &tree, cluster5, root, 1 );

        cluster5aNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(cluster5aNode->boundingBox(),cluster5a));
        cluster5bNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(cluster5bNode->boundingBox(), cluster5b));

        cluster5cNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(cluster5cNode->boundingBox(), cluster5c));

        cluster5dNode->parent = cluster5;
        cluster5Node->addBranchToNode( createBranchEntry(cluster5dNode->boundingBox(), cluster5d));

        // Root
        rootNode->addBranchToNode(
            createBranchEntry(cluster4Node->boundingBox(), cluster4));
        rootNode->addBranchToNode(
            createBranchEntry(cluster5Node->boundingBox(), cluster5));
        REQUIRE( rootNode->level == 2 );

        tree.root = root;

        tree.write_metadata();
    }

    TreeType tree( 4096 * 5, "rstardiskbacked.txt" );
    REQUIRE( root == tree.root );
    auto rootNode = tree.get_branch_node( root );

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
        auto rootNode = tree.get_leaf_node( root );

        rootNode->addPoint(Point(-3.0, -11.0));
        rootNode->addPoint(Point(-2.0, -9.0));
        rootNode->addPoint(Point(2.0, -10.0));
        rootNode->addPoint(Point(3.0, -11.0));
        rootNode->addPoint(Point(1.0, -9.0));
        rootNode->addPoint(Point(-3.0, -10.0));
        rootNode->addPoint(Point(3.0, -11.0));
        rootNode->addPoint(Point(3.0, -9.0));

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
        auto rootNode = tree2.get_leaf_node( tree2.root );
        rootNode->addPoint(Point(-11.0, -3.0));
        rootNode->addPoint(Point(-9.0, -2.0));
        rootNode->addPoint(Point(-10.0, 2.0));
        rootNode->addPoint(Point(-11.0, 3.0));
        rootNode->addPoint(Point(-9.0, 1.0));
        rootNode->addPoint(Point(-10.0, -3.0));
        rootNode->addPoint(Point(-11.0, 3.0));
        rootNode->addPoint(Point(-9.0, 3.0));
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
        auto rootNode = tree.get_leaf_node( root );
        rootNode->addPoint(Point(-2.0, -6.0));
        rootNode->addPoint(Point(-2.0, -6.0));
        rootNode->addPoint(Point(2.0, -6.0));
        rootNode->addPoint(Point(-1.0, -7.0));
        rootNode->addPoint(Point(1.0, -7.0));
        rootNode->addPoint(Point(3.0, -8.0));
        rootNode->addPoint(Point(-2.0, -9.0));

        // Split the rstartree::Node in two
        rootNode->addPoint(Point(-3.0, -11.0));
        tree_node_handle splitNodeHandle = rootNode->splitNode();
        auto splitNode = tree.get_leaf_node( splitNodeHandle );

        // Test the split
        REQUIRE(rootNode->cur_offset_ == 5);
        REQUIRE(rootNode->entries[0] == Point(-3.0, -11.0));
        REQUIRE(rootNode->entries[1] == Point(-2.0, -6.0));
        REQUIRE(rootNode->entries[2] == Point(-2.0, -6.0));
        REQUIRE(rootNode->entries[3] == Point(-2.0, -9.0));
        REQUIRE(rootNode->entries[4] == Point(-1.0, -7.0));
        REQUIRE(splitNode->cur_offset_ == 3);
        REQUIRE(splitNode->entries[0] == Point(1.0, -7.0));
        REQUIRE(splitNode->entries[1] == Point(2.0, -6.0));
        REQUIRE(splitNode->entries[2] == Point(3.0, -8.0));
        REQUIRE(splitNode->level == rootNode->level);
    }
    unlink( "rstardiskbacked.txt" );

    {
        // Test set two
        // Cluster 2, n = 8
        // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
        TreeType tree2( 4096*5, "rstardiskbacked.txt" );
        auto rootNode =
            tree2.get_leaf_node(tree2.root);
        rootNode->addPoint(Point(-14.0, 8.0));
        rootNode->addPoint(Point(-10.0, 8.0));
        rootNode->addPoint(Point(-9.0, 10.0));
        rootNode->addPoint(Point(-9.0, 9.0));
        rootNode->addPoint(Point(-8.0, 10.0));
        rootNode->addPoint(Point(-9.0, 7.0));
        rootNode->addPoint(Point(-8.0, 8.0));
        rootNode->addPoint(Point(-8.0, 9.0));
        rootNode->level = 0;

        // Split the rstartree::Node in two
        tree_node_handle splitNodeHandle = rootNode->splitNode();
        auto splitNode = tree2.get_leaf_node( splitNodeHandle );

        // Test the split
        REQUIRE(rootNode->cur_offset_ == 4);
        REQUIRE(rootNode->entries[0] == Point(-9.0, 7.0));
        REQUIRE(rootNode->entries[1] == Point(-14.0, 8.0));
        REQUIRE(rootNode->entries[2] == Point(-10.0, 8.0));
        REQUIRE(rootNode->entries[3] == Point(-8.0, 8.0));
        REQUIRE(splitNode->cur_offset_ == 4);
        REQUIRE(splitNode->entries[0] == Point(-9.0, 9.0));
        REQUIRE(splitNode->entries[1] == Point(-8.0, 9.0));
        REQUIRE(splitNode->entries[2] == Point(-9.0, 10.0));
        REQUIRE(splitNode->entries[3] == Point(-8.0, 10.0));
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
        auto alloc_data_root =
            tree3.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree3,
                alloc_data_root.second, tree_node_handle( nullptr ), 1
                );
        tree_node_handle root = alloc_data_root.second;
        tree3.root = root;
        auto rootNode = alloc_data_root.first;

        tree_node_handle dummys[7];
        for( int i = 0; i < 7; i++ ) {
            auto alloc_data =
                tree3.node_allocator_->create_new_tree_node<LeafNodeType>(
                        NodeHandleType( LEAF_NODE ) );
            new (&(*alloc_data.first)) LeafNodeType( &tree3,
                    alloc_data.second, tree3.root, 0 );
            dummys[i] = alloc_data.second;
        }

        REQUIRE( rootNode->level == 1 );
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-6.0, 3.0,
                        nextafter(-4.0,DBL_MAX), nextafter(5.0,DBL_MAX)), dummys[0]));
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-6.0, 3.0,
                        nextafter(-4.0,DBL_MAX), nextafter(5.0,DBL_MAX)), dummys[1]));
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-3.0, 3.0,
                        nextafter(-1.0,DBL_MAX), nextafter(5.0,DBL_MAX)), dummys[2]));
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-2.0, 2.0,
                        nextafter(0.0,DBL_MAX), nextafter(4.0,DBL_MAX)), dummys[3]));
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-2.0, 0.0,
                        nextafter(0.0,DBL_MAX), nextafter(2.0,DBL_MAX)), dummys[4]));
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-4.0, -1.0,
                        nextafter(-2.0,DBL_MAX), nextafter(1.0,DBL_MAX)), dummys[5]));
        rootNode->addBranchToNode(createBranchEntry(Rectangle(-7.0, 1.0,
                        nextafter(-5.0,DBL_MAX), nextafter(3.0,DBL_MAX)), dummys[6]));


        // Extra rstartree::Node causing the split
        auto alloc_data =
            tree3.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree3, alloc_data.second, tree3.root, 0 );


        auto extra_handle = alloc_data.second;
        auto extraNode = alloc_data.first;
        extraNode->addPoint(Point(1.0, 1.0));
        extraNode->addPoint(Point(2.0, 2.0));

        REQUIRE( extraNode->level == 0 );

        // Test the split
        rootNode->addBranchToNode(createBranchEntry(extraNode->boundingBox(), extra_handle) );
        tree_node_handle splitHandle = rootNode->splitNode();
        auto splitNode = tree3.get_branch_node( splitHandle );
        REQUIRE(splitNode->level == rootNode->level);
        REQUIRE(splitNode->level == 1);

        REQUIRE(rootNode->cur_offset_ == 3);
        REQUIRE(rootNode->entries[0].boundingBox
                == Rectangle(-7.0, 1.0, nextafter(-5.0,DBL_MAX),
                    nextafter(3.0,DBL_MAX)));
        REQUIRE(rootNode->entries[1].boundingBox
                == Rectangle(-6.0, 3.0, nextafter(-4.0,DBL_MAX),
                    nextafter(5.0,DBL_MAX)));
        REQUIRE(rootNode->entries[2].boundingBox
                == Rectangle(-6.0, 3.0, nextafter(-4.0,DBL_MAX),
                    nextafter(5.0,DBL_MAX)));
        REQUIRE(splitNode->cur_offset_ == 5);
        REQUIRE(splitNode->entries[0].boundingBox
                == Rectangle(-4.0, -1.0, nextafter(-2.0,DBL_MAX),
                    nextafter(1.0,DBL_MAX)));
        REQUIRE(splitNode->entries[1].boundingBox
                == Rectangle(-3.0, 3.0, nextafter(-1.0,DBL_MAX),
                    nextafter(5.0,DBL_MAX)));
        REQUIRE(splitNode->entries[2].boundingBox
                == Rectangle(-2.0, 2.0, nextafter(0.0,DBL_MAX),
                    nextafter(4.0,DBL_MAX)));
        REQUIRE(splitNode->entries[3].boundingBox
                == Rectangle(-2.0, 0.0, nextafter(0.0,DBL_MAX),
                    nextafter(2.0,DBL_MAX)));
        REQUIRE(splitNode->entries[4].boundingBox
                == Rectangle(1.0, 1.0, nextafter(2.0, DBL_MAX),
                    nextafter(2.0, DBL_MAX)));
        REQUIRE(splitNode->entries[4].child
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
        auto alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 1
                );
        tree_node_handle root = alloc_data_root.second;
        auto rootNode = alloc_data_root.first;
        tree.root = root;

        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto childNode = alloc_data.first;
        new (&(*childNode)) LeafNodeType( &tree, alloc_data.second,
                tree.root, 0 );
        tree_node_handle child_handle = alloc_data.second;
        childNode->addPoint(Point(-30.0, -30.0));
        childNode->addPoint(Point(30.0, 30.0));
        childNode->addPoint(Point(-20.0, -20.0));
        childNode->addPoint(Point(20.0, 20.0));
        childNode->addPoint(Point(-10.0, -10.0));
        childNode->addPoint(Point(10.0, 10.0));
        childNode->addPoint(Point(0.0, 0.0));

        REQUIRE( childNode->level == 0 );

        // Root rstartree::Node
        REQUIRE( rootNode->level == 1 );
        rootNode->addBranchToNode(createBranchEntry(childNode->boundingBox(), child_handle));
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
        Branch bLeft =  rootNode->entries[0];
        Branch bRight = rootNode->entries[1];
        auto leftNode = tree.get_leaf_node( bLeft.child );
        auto rightNode = tree.get_leaf_node( bRight.child );
        REQUIRE(leftNode->cur_offset_ == 3);
        REQUIRE(rightNode->cur_offset_ == 5);

        REQUIRE(leftNode->entries[0] == Point(-30,-30));
        REQUIRE(leftNode->entries[1] == Point(-20,-20));
        REQUIRE(leftNode->entries[2] == Point(-10,-10));

        REQUIRE(rightNode->entries[0] == Point(0,0));
        REQUIRE(rightNode->entries[1] == Point(0,0));
        REQUIRE(rightNode->entries[2] == Point(10,10));
        REQUIRE(rightNode->entries[3] == Point(20,20));
        REQUIRE(rightNode->entries[4] == Point(30,30));
        REQUIRE(leftNode->level == 0);
        REQUIRE(rightNode->level == 0);
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testInsertGrowTreeHeight")
{
    unlink( "rstardiskbacked.txt" );
    {
        unsigned maxBranchFactor = 7;
        TreeType tree(4096*5, "rstardiskbacked.txt");
        auto rootNodeLeaf = tree.get_leaf_node( tree.root );
        REQUIRE( rootNodeLeaf->level == 0 );

        for( unsigned i = 0; i < maxBranchFactor + 1; i++ ) {
            tree.insert(Point(0.0, 0.0));
        }

        REQUIRE( tree.root.get_type() == BRANCH_NODE );
        auto rootNode = tree.get_branch_node( tree.root );
        REQUIRE(rootNode->cur_offset_ == 2);
        Branch bLeft =  rootNode->entries[0];
        Branch bRight = rootNode->entries[1];

        auto left = tree.get_leaf_node( bLeft.child );
        auto right = tree.get_leaf_node( bRight.child );

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

        auto alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 1
                );
        tree_node_handle root_handle = alloc_data_root.second;
        tree.root = root_handle;
        auto rootNode = alloc_data_root.first;

        for( unsigned i = 0; i < maxBranchFactor; i++ ) {
            auto child_handle = createFullLeafNode( tree, tree.root );
            auto childNode = tree.get_leaf_node( child_handle );
            rootNode->addBranchToNode( createBranchEntry( childNode->boundingBox(), child_handle ) );
        }

        unsigned height = rootNode->height();
        tree.hasReinsertedOnLevel = std::vector(height, false);

        // Should be 49 points
        std::vector<Point> accumulator = tree.search( Point(0.0, 0.0) );
        REQUIRE(accumulator.size() == maxBranchFactor * maxBranchFactor);

        REQUIRE(rootNode->cur_offset_ == maxBranchFactor);
        tree.insert(Point(0.0, 0.0));
        auto new_root_handle = tree.root;
        REQUIRE( new_root_handle != root_handle );
        auto newRootNode = tree.get_branch_node( new_root_handle );

        // Confirm tree structure
        REQUIRE(newRootNode->cur_offset_ == 2);
        const Branch &bLeft = newRootNode->entries[0];
        const Branch &bRight = newRootNode->entries[1];
        auto left = tree.get_branch_node( bLeft.child );
        auto right = tree.get_branch_node( bRight.child );

        REQUIRE(left->cur_offset_ == 3);
        REQUIRE(right->cur_offset_ == 5);

        for( size_t i = 0; i < left->cur_offset_; i++ ) {

            auto child_handle = left->entries.at(i).child;

            // These are all leaves
            auto childNode = tree.get_leaf_node( child_handle );
            (void) childNode;
        }

        for( size_t i = 0; i < right->cur_offset_; i++ ) {
            auto child_handle =right->entries.at(i).child;
            auto childNode = tree.get_leaf_node( child_handle );
            (void) childNode;
        }

        // Count
        accumulator = tree.search( Point(0.0, 0.0) );
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
        auto root_node = tree.get_branch_node( root );
        tree_node_handle leaf_handle = root_node->findLeaf( Point(0,0) );
        REQUIRE( leaf_handle.get_type() == LEAF_NODE );

        auto node = tree.get_leaf_node( leaf_handle );

        size_t cnt = node->cur_offset_;
        std::vector<Point> nodesToRemove( node->entries.begin(), node->entries.begin() + (cnt-minBranchFactor + 1));
        for( const auto &entry : nodesToRemove) {
            tree.remove(entry);
        }

        for( unsigned i = 0; i < maxBranchFactor*maxBranchFactor + 1; i++ ) {
            Point p(i, i);
            if( std::find(nodesToRemove.begin(), nodesToRemove.end(), p) == nodesToRemove.end() ) {
                REQUIRE(tree.search(p).size() == 1);
            } else {
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

        auto alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 2
                );
        tree_node_handle root = alloc_data_root.second;
        auto rootNode = alloc_data_root.first;
        tree.root = root;


        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree, alloc_data.second,
                tree_node_handle( nullptr ), 0 );
        auto cluster1a_handle = alloc_data.second;
        auto cluster1a = alloc_data.first;
        cluster1a->addPoint(Point(-3.0, 16.0));
        cluster1a->addPoint(Point(-3.0, 15.0));
        cluster1a->addPoint(Point(-4.0, 13.0));

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree, alloc_data.second,
                tree_node_handle( nullptr ), 0 );
        auto cluster1b_handle = alloc_data.second;
        auto cluster1b = alloc_data.first;
        cluster1b->addPoint(Point(-5.0, 12.0));
        cluster1b->addPoint(Point(-5.0, 15.0));
        cluster1b->addPoint(Point(-6.0, 14.0));
        cluster1b->addPoint(Point(-8.0, 16.0));

        // Cluster 2, n = 8
        // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster2a_handle = alloc_data.second;
        auto cluster2a = alloc_data.first;
        cluster2a->addPoint(Point(-8.0, 10.0));
        cluster2a->addPoint(Point(-9.0, 10.0));
        cluster2a->addPoint(Point(-8.0, 9.0));
        cluster2a->addPoint(Point(-9.0, 9.0));
        cluster2a->addPoint(Point(-8.0, 8.0));

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster2b_handle = alloc_data.second;
        auto cluster2b = alloc_data.first;
        cluster2b->addPoint(Point(-14.0, 8.0));
        cluster2b->addPoint(Point(-10.0, 8.0));
        cluster2b->addPoint(Point(-9.0, 7.0));

        // Cluster 3, n = 9
        // (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster3a_handle = alloc_data.second;
        auto cluster3a = alloc_data.first;
        cluster3a->addPoint(Point(-3.0, 4.0));
        cluster3a->addPoint(Point(-3.0, 0.0));
        cluster3a->addPoint(Point(-2.0, 4.0));
        cluster3a->addPoint(Point(-1.0, 3.0));
        cluster3a->addPoint(Point(-1.0, 1.0));

        alloc_data =
            tree.node_allocator_->create_new_tree_node<LeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        new (&(*alloc_data.first)) LeafNodeType( &tree, alloc_data.second,
                tree_node_handle(nullptr), 0 );
        auto cluster3b_handle = alloc_data.second;
        auto cluster3b = alloc_data.first;
        cluster3b->addPoint(Point(-5.0, 4.0));
        cluster3b->addPoint(Point(-4.0, 3.0));
        cluster3b->addPoint(Point(-4.0, 1.0));
        cluster3b->addPoint(Point(-6.0, 2.0));

        // High level rstartree::Nodes
        alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second,
                tree_node_handle(nullptr), 1 );
        auto left_handle = alloc_data_root.second;
        auto left = alloc_data_root.first;
        cluster1a->parent = left_handle;
        left->addBranchToNode(createBranchEntry(cluster1a->boundingBox(),
                    cluster1a_handle));
        cluster1b->parent = left_handle;
        left->addBranchToNode(createBranchEntry(cluster1b->boundingBox(),
                    cluster1b_handle));
        cluster2a->parent = left_handle;
        left->addBranchToNode(createBranchEntry(cluster2a->boundingBox(),
                    cluster2a_handle));
        cluster2b->parent = left_handle;
        left->addBranchToNode(createBranchEntry(cluster2b->boundingBox(),
                    cluster2b_handle));

        alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree, alloc_data_root.second,
                tree_node_handle(nullptr), 1 );
        auto right_handle = alloc_data_root.second;
        auto right = alloc_data_root.first;
        cluster3a->parent = right_handle;
        right->addBranchToNode(createBranchEntry(cluster3a->boundingBox(),
                    cluster3a_handle));
        cluster3b->parent = right_handle;
        right->addBranchToNode(createBranchEntry(cluster3b->boundingBox(),
                    cluster3b_handle));

        left->parent = root;
        rootNode->addBranchToNode(createBranchEntry(left->boundingBox(),
                    left_handle));
        right->parent = root;
        rootNode->addBranchToNode(createBranchEntry(right->boundingBox(),
                    right_handle));

        // Test search

        // Test set one
        Rectangle sr1 = Rectangle(-9.0, 9.5, nextafter(-5.0, DBL_MAX),
                nextafter(12.5, DBL_MAX));
        std::vector<Point> v1 = tree.search(sr1);
        REQUIRE(v1.size() == 3);
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

        // Test set two
        Rectangle sr2 = Rectangle(-8.0, 4.0, nextafter(-5.0, DBL_MAX),
                nextafter(8.0, DBL_MAX));
        std::vector<Point> v2 = tree.search(sr2);
        REQUIRE(v2.size() == 2);
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

        // Test set three
        Rectangle sr3 = Rectangle(-8.0, 0.0, nextafter(-4.0, DBL_MAX),
                nextafter(16.0, DBL_MAX));
        std::vector<Point> v3 = tree.search(sr3);
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
        std::vector<Point> v4 = tree.search(sr4);
        REQUIRE(v4.size() == 0);

        // Test set five
        Rectangle sr5 = Rectangle(-3.5, 1.0, nextafter(-1.5, DBL_MAX),
                nextafter(3.0, DBL_MAX));
        std::vector<Point> v5 = tree.search(sr5);
        REQUIRE(v5.size() == 0);

        tree.write_metadata();

    }
    {
        // Re-read the tree from disk, doall the searches again
        TreeType tree( 4096*5, "rstardiskbacked.txt");
        auto rootNode = tree.get_branch_node( tree.root );
        // Test set one
        Rectangle sr1 = Rectangle(-9.0, 9.5, nextafter(-5.0, DBL_MAX),
                nextafter(12.5, DBL_MAX));
        std::vector<Point> v1 = tree.search(sr1);
        REQUIRE(v1.size() == 3);
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
        REQUIRE(std::find( v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

        // Test set two
        Rectangle sr2 = Rectangle(-8.0, 4.0, nextafter(-5.0, DBL_MAX),
                nextafter(8.0, DBL_MAX));
        std::vector<Point> v2 = tree.search(sr2);
        REQUIRE(v2.size() == 2);
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
        REQUIRE(std::find( v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

        // Test set three
        Rectangle sr3 = Rectangle(-8.0, 0.0, nextafter(-4.0, DBL_MAX),
                nextafter(16.0, DBL_MAX));
        std::vector<Point> v3 = tree.search(sr3);
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
        std::vector<Point> v4 = tree.search(sr4);
        REQUIRE(v4.size() == 0);

        // Test set five
        Rectangle sr5 = Rectangle(-3.5, 1.0, nextafter(-1.5, DBL_MAX),
                nextafter(3.0, DBL_MAX));
        std::vector<Point> v5 = tree.search(sr5);
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
            tree_node_handle leaf = createFullLeafNode(tree, tree_node_handle( nullptr ));
            leafNodes.push_back(leaf);
        }

        auto alloc_data_root =
            tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                alloc_data_root.second, tree_node_handle( nullptr ), 2
                );
        tree_node_handle root = alloc_data_root.second;
        tree.root = root;
        auto rootNode = alloc_data_root.first;

        // Construct intermediate layer
        std::vector<tree_node_handle> middleLayer;
        for( unsigned i = 0; i < 7; i++ ) {
            alloc_data_root =
                tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                        NodeHandleType( BRANCH_NODE ) );
            new (&(*alloc_data_root.first)) BranchNodeType( &tree,
                    alloc_data_root.second, tree_node_handle( nullptr ), 1 );
            auto child_handle = alloc_data_root.second;
            auto child = alloc_data_root.first;
            child->parent = root;
            for( unsigned j = 0; j < 7; j++ ) {
                auto leaf = leafNodes.at(7*i + j);
                auto leafNode = tree.get_leaf_node( leaf );
                child->addBranchToNode(createBranchEntry(leafNode->boundingBox(), leaf));
                leafNode->parent = child_handle;
            }
            rootNode->addBranchToNode(createBranchEntry(child->boundingBox(), child_handle));
            middleLayer.push_back(child_handle);
        }

        // Emulate a case where we need to reinsert some extra entries in the middle layer,
        // but a reinsertion forces a split while we still have entries outstanding.
        // Shove two extra things into middleLayer[0]

        auto leaf = leafNodes.at(maxBranchFactor*maxBranchFactor);
        auto leafNode = tree.get_leaf_node( leaf);

        leafNode->level = 0;
        leafNode->parent = middleLayer.at(0);

        auto middleNode = tree.get_branch_node( middleLayer.at(0) );
        middleNode->addBranchToNode(createBranchEntry(leafNode->boundingBox(), leaf));


        std::vector<bool> hasReinsertedOnLevel = {false, true, false};

        REQUIRE(middleNode->cur_offset_ > maxBranchFactor );
        middleNode->reInsert(hasReinsertedOnLevel);

        for( auto leaf : leafNodes ) {
            leafNode = tree.get_leaf_node( leaf);
            REQUIRE(leafNode->level == 0);
        }

        for( auto middle : middleLayer ) {
            middleNode = tree.get_branch_node( middle );
            REQUIRE( middleNode->level == 1 );
        }

        REQUIRE( rootNode->level == 2 );
        REQUIRE( rootNode->parent != nullptr );
        auto newRootNode = tree.get_branch_node( rootNode->parent );
        REQUIRE( newRootNode->level == 3 );
    }
    unlink("rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: pack simple leaf node") {
    unlink( "rstardiskbacked.txt" );
    TreeType tree( 4096*5, "rstardiskbacked.txt" );
    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point(i,i) );
    }

    auto root_leaf_node = tree.get_leaf_node( tree.root );
    REQUIRE( root_leaf_node->cur_offset_ == 5 );

    REQUIRE( root_leaf_node->compute_packed_size() <
            sizeof(LeafNodeType) );
    REQUIRE( root_leaf_node->compute_packed_size() ==
            sizeof(void *) + sizeof(tree_node_handle)*2 +
            sizeof(unsigned) + sizeof(Point) * 5 );

    tree_node_handle repacked_handle = root_leaf_node->repack(
            tree.node_allocator_.get() );

    REQUIRE( repacked_handle != nullptr );
    auto packed_leaf =
        tree.node_allocator_->get_tree_node<packed_node>(
                repacked_handle );
    REQUIRE( read_pointer_from_buffer<TreeType>( packed_leaf->buffer_ ) == &tree );
    REQUIRE( * (tree_node_handle *) (packed_leaf->buffer_ + sizeof(void
                    *) ) ==
        repacked_handle );

    REQUIRE( * (tree_node_handle *) (packed_leaf->buffer_ + sizeof(void
                    *) + sizeof(tree_node_handle) ) ==
        root_leaf_node->parent );

    REQUIRE( * (unsigned *) (packed_leaf->buffer_ + sizeof(unsigned
                    *) + sizeof(tree_node_handle)*2 ) ==
        root_leaf_node->cur_offset_ );

    Point *p = (Point *) (packed_leaf->buffer_ + sizeof(void *) +
            sizeof(tree_node_handle)*2 + sizeof(unsigned));
    REQUIRE( *(p++) == root_leaf_node->entries.at(0) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(1) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(2) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(3) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(4) );
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: pack branch structure") {
    unlink( "rstardiskbacked.txt" );
    TreeType tree( 4096*5, "rstardiskbacked.txt" );

    auto alloc_data_root = tree.node_allocator_->create_new_tree_node<BranchNodeType>();
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle(nullptr), 1 );
    auto root_node = alloc_data_root.first;
    auto root_handle = alloc_data_root.second;

    tree_node_handle leaves[5];
    for( unsigned i = 0; i < 5; i++ ) {
        Point p(i,i);
        leaves[i] = createFullLeafNode( tree, root_handle, Point(i,i));
        Rectangle bb( p, Point::closest_larger_point( p ) );
        root_node->addBranchToNode( createBranchEntry( bb, leaves[i] ) );
    }

    REQUIRE( root_node->compute_packed_size() <
            sizeof(BranchNodeType) );
    REQUIRE( root_node->compute_packed_size() ==
            sizeof(void *) + sizeof(tree_node_handle)*2 +
            sizeof(unsigned) + sizeof(Branch) * 5 );

    auto repacked_handle = root_node->repack( tree.node_allocator_.get() );
    REQUIRE( repacked_handle != nullptr );

    auto repacked_node =
        tree.node_allocator_->get_tree_node<packed_node>(
                repacked_handle );

    char *buffer = repacked_node->buffer_;
    REQUIRE( read_pointer_from_buffer<TreeType>( buffer ) == &tree );
    REQUIRE( * (tree_node_handle *) (buffer + sizeof(void*)) ==
            repacked_handle );
    REQUIRE( * (tree_node_handle *) (buffer + sizeof(void*) +
                sizeof(tree_node_handle)) ==
            tree_node_handle(nullptr) );

    REQUIRE( * (unsigned *) (buffer + sizeof(void*) +
                sizeof(tree_node_handle)*2 ) == root_node->cur_offset_ );

    for( unsigned i = 0; i < 5; i++ ) {
        Branch *b = (Branch *) (buffer + (sizeof(void*) +
                sizeof(tree_node_handle)*2 + sizeof(unsigned)+ sizeof(Branch)*i));
        REQUIRE( b->child == leaves[i] );
        Point p = Point(i,i);
        REQUIRE( b->boundingBox == Rectangle( p,
                    Point::closest_larger_point( p ) ) );
    }

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: pack branch subtree") {
    unlink( "rstardiskbacked.txt" );
    TreeType tree( 4096*5, "rstardiskbacked.txt" );

    auto alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle(nullptr), 1 );
    auto root_node = alloc_data_root.first;
    auto root_handle = alloc_data_root.second;

    tree_node_handle leaves[5];
    for( unsigned i = 0; i < 5; i++ ) {
        Point p(i,i);
        leaves[i] = createFullLeafNode( tree, root_handle, Point(i,i));
        Rectangle bb( p, Point::closest_larger_point( p ) );
        root_node->addBranchToNode( createBranchEntry( bb, leaves[i] ) );
    }

    REQUIRE( root_node->compute_packed_size() <
            sizeof(BranchNodeType) );
    REQUIRE( root_node->compute_packed_size() ==
            sizeof(void *) + sizeof(tree_node_handle)*2 +
            sizeof(unsigned) + sizeof(Branch) * 5 );

    auto repacked_handle = rstartreedisk::repack_subtree<3,7>(
            root_handle, tree.node_allocator_.get(),
            tree.node_allocator_.get() );
    REQUIRE( repacked_handle != nullptr );

    auto repacked_node =
        tree.node_allocator_->get_tree_node<packed_node>(
                repacked_handle );

    char *buffer = repacked_node->buffer_;
    REQUIRE( read_pointer_from_buffer<TreeType>( buffer ) == &tree );
    REQUIRE( * (tree_node_handle *) (buffer + sizeof(void*)) ==
            repacked_handle );
    REQUIRE( * (tree_node_handle *) (buffer + sizeof(void*) +
                sizeof(tree_node_handle)) ==
            tree_node_handle(nullptr) );

    REQUIRE( * (unsigned *) (buffer + sizeof(void*) +
                sizeof(tree_node_handle)*2 ) == root_node->cur_offset_ );

    for( unsigned i = 0; i < 5; i++ ) {
        Branch *b = (Branch *) (buffer + (sizeof(void*) +
                sizeof(tree_node_handle)*2 + sizeof(unsigned)+ sizeof(Branch)*i));
        REQUIRE( b->child != leaves[i] );
        leaves[i] = b->child; // we will check this later.
        Point p = Point(i,i);
        REQUIRE( b->boundingBox == Rectangle( p,
                    Point::closest_larger_point( p ) ) );
    }

    for( unsigned i = 0; i < 5; i++ ) {
        auto leaf_node = tree.node_allocator_->get_tree_node<packed_node>( leaves[i] );
        char *buffer = leaf_node->buffer_;
        REQUIRE( read_pointer_from_buffer<TreeType>( buffer ) == &tree );
        REQUIRE( * (tree_node_handle *) (buffer + sizeof(void*)) ==
                leaves[i] );
        REQUIRE( * (tree_node_handle *) (buffer + sizeof(void*) +
                    sizeof(tree_node_handle)) ==
                repacked_handle );
        REQUIRE( * (unsigned *) (buffer + sizeof(void*) +
                sizeof(tree_node_handle)*2) == 7 );
        for( unsigned j = 0; j < 7; j++ ) {
            REQUIRE( * (Point *) (buffer + sizeof(void*) +
                sizeof(tree_node_handle)*2 + sizeof(unsigned)) ==
                Point(i,i));
        }
    }

    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: Search Packed Leaf Node" ) {
    unlink( "rstardiskbacked.txt" );
    TreeType tree( 4096*5, "rstardiskbacked.txt" );

    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point( i, i ) );
    }

    auto repacked_root = rstartreedisk::repack_subtree<3,7>( tree.root,
            tree.node_allocator_.get(),
            tree.node_allocator_.get() );

    for( unsigned i = 0; i < 7; i++ ) {
        Point p(i,i);
        if( i < 5 ) {
            REQUIRE( point_search( repacked_root, p, &tree ).size() == 1 );
        } else {
            REQUIRE( point_search( repacked_root, p, &tree ).size() == 0 );
        }
    }
}

TEST_CASE("R*TreeDisk: Search packed subtree") {
    unlink( "rstardiskbacked.txt" );
    TreeType tree( 4096*5, "rstardiskbacked.txt" );

    auto alloc_data_root =
        tree.node_allocator_->create_new_tree_node<BranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_data_root.first)) BranchNodeType( &tree,
            alloc_data_root.second, tree_node_handle(nullptr), 1 );
    auto root_node = alloc_data_root.first;
    auto root_handle = alloc_data_root.second;

    tree_node_handle leaves[5];
    for( unsigned i = 0; i < 5; i++ ) {
        Point p(i,i);
        leaves[i] = createFullLeafNode( tree, root_handle, Point(i,i));
        Rectangle bb( p, Point::closest_larger_point( p ) );
        root_node->addBranchToNode( createBranchEntry( bb, leaves[i] ) );
    }

    auto repacked_handle = rstartreedisk::repack_subtree<3,7>(
            root_handle, tree.node_allocator_.get(),
            tree.node_allocator_.get() );
    REQUIRE( repacked_handle != nullptr );

    tree.root = repacked_handle;
    for( unsigned i = 0; i < 7; i++ ) {
        Point p(i,i);
        if( i < 5 ) {
            REQUIRE( tree.search( p ).size() == 7 );
        } else {
            REQUIRE( tree.search( p ).size() == 0 );
        }
    }

    for( unsigned i = 0; i < 5; i++ ) {
        Point p(i,i);
        Point p2(i+1,i+1);
        Rectangle range( p, Point::closest_larger_point( p2 ) );
        if( i < 4 ) {
            REQUIRE( tree.search( range ).size() == 14 );
        } else {
            REQUIRE( tree.search( range ).size() == 7 );
        }
    }

    unlink( "rstardiskbacked.txt" );
}
