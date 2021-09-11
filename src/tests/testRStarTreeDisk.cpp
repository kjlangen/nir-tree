#include <catch2/catch.hpp>
#include <rstartreedisk/rstartreedisk.h>
#include <util/geometry.h>
#include <iostream>
#include <unistd.h>

using NodeType = rstartreedisk::Node<3,7>;

template <class NE, class B>
static NE createBranchEntry(const Rectangle
            &boundingBox,
            tree_node_handle child ) {
	B b(boundingBox, child);
	return b;
}

static tree_node_handle
createFullLeafNode(rstartreedisk::RStarTreeDisk &tree, tree_node_handle parent, Point p=Point::atOrigin)
{
    // Allocate new node
    std::pair<pinned_node_ptr<NodeType>, tree_node_handle> alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) NodeType( &tree, node_handle, tree_node_handle() /* nullptr */, 0 );

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
        rstartreedisk::RStarTreeDisk tree( 4096, 3, 7, "rstardiskbacked.txt" );
        tree_node_handle root = tree.root;

        pinned_node_ptr<rstartreedisk::Node<3,7>> rootNode =
            tree.node_allocator_.get_tree_node<rstartreedisk::Node<3,7>>( root );
        
        std::pair<pinned_node_ptr<rstartreedisk::Node<3,7>>, tree_node_handle> alloc_data =
            tree.node_allocator_.create_new_tree_node<rstartreedisk::Node<3,7>>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child0, root, 1 );
        rstartreedisk::Node<3,7>::NodeEntry entry =
            createBranchEntry<rstartreedisk::Node<3,7>::NodeEntry,rstartreedisk::Node<3,7>::Branch>( Rectangle(8.0, 1.0, 12.0, 5.0), child0);
        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rstartreedisk::Node<3,7>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child1, root, 1 );
        entry =
            createBranchEntry<rstartreedisk::Node<3,7>::NodeEntry,rstartreedisk::Node<3,7>::Branch>(
                    Rectangle(12.0, -4.0, 16.0, -2.0), child1);
        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rstartreedisk::Node<3,7>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child2, root, 1 );
        entry =
            createBranchEntry<rstartreedisk::Node<3,7>::NodeEntry,rstartreedisk::Node<3,7>::Branch>(
                    Rectangle(8.0, -6.0, 10.0, -4.0), child2 );
        rootNode->addEntryToNode( entry );

        REQUIRE( rootNode->cur_offset_ == 3 );
        REQUIRE( rootNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0) );
    }


    unlink( "rstardiskbacked.txt" );
    {
        // Test set two
        rstartreedisk::RStarTreeDisk tree(4096, 3, 7, "rstardiskbacked.txt" );
        tree_node_handle root = tree.root;

        pinned_node_ptr<rstartreedisk::Node<3,7>> rootNode =
            tree.node_allocator_.get_tree_node<rstartreedisk::Node<3,7>>( root );
        
        std::pair<pinned_node_ptr<rstartreedisk::Node<3,7>>, tree_node_handle> alloc_data =
            tree.node_allocator_.create_new_tree_node<rstartreedisk::Node<3,7>>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child0, root, 1 );
        rootNode->addEntryToNode( 
            createBranchEntry<rstartreedisk::Node<3,7>::NodeEntry,
            rstartreedisk::Node<3,7>::Branch>(Rectangle(8.0, 12.0, 10.0,
                    14.0), child0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rstartreedisk::Node<3,7>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child1, root, 1 );
        rootNode->addEntryToNode( 
            createBranchEntry<rstartreedisk::Node<3,7>::NodeEntry,
            rstartreedisk::Node<3,7>::Branch>(Rectangle(10.0, 12.0,
                    12.0, 14.0), child1) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<rstartreedisk::Node<3,7>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child2, root, 1 );
        rootNode->addEntryToNode(
            createBranchEntry<rstartreedisk::Node<3,7>::NodeEntry,
            rstartreedisk::Node<3,7>::Branch>(Rectangle(12.0, 12.0,
                    14.0, 14.0), child2) );

        REQUIRE( rootNode->cur_offset_ ==  3 );

        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
    }
    unlink( "rstardiskbacked.txt" );
}

TEST_CASE("R*TreeDisk: testUpdateBoundingBox") {

    unlink( "rstardiskbacked.txt" );
	rstartreedisk::RStarTreeDisk tree(4096, 3, 7, "rstardiskbacked.txt");
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
	rstartreedisk::RStarTreeDisk tree(4096, 3, 7, "rstardiskbacked.txt");
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
	rstartreedisk::RStarTreeDisk tree( 4096, 3, 7, "rstardiskbacked.txt" );
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
	rstartreedisk::RStarTreeDisk tree( 4096*4, 3, 7, "rstardiskbacked.txt" );
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

	rstartreedisk::RStarTreeDisk tree( 4096 * 5, 3, 7, "rstardiskbacked.txt" );
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
        rstartreedisk::RStarTreeDisk tree( 4096 * 5, 3, 7, "rstardiskbacked.txt" );
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
    }

    rstartreedisk::RStarTreeDisk tree( 4096 * 5, 3, 7, "rstardiskbacked.txt" );
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
        rstartreedisk::RStarTreeDisk tree(4096 * 5, 3, 7, "rstardiskbacked.txt");
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
        rstartreedisk::RStarTreeDisk tree2(4096*5,3,7, "rstardiskbacked.txt");
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
        rstartreedisk::RStarTreeDisk tree( 4096 * 5, 3,7,
                "rstardiskbacked.txt");
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
        REQUIRE(rootNode->cur_offset_ == 4);
        REQUIRE(std::get<Point>(rootNode->entries[0]) == Point(-3.0, -11.0));
        REQUIRE(std::get<Point>(rootNode->entries[1]) == Point(-2.0, -6.0));
        REQUIRE(std::get<Point>(rootNode->entries[2]) == Point(-2.0, -6.0));
        REQUIRE(std::get<Point>(rootNode->entries[3]) == Point(-2.0, -9.0));
        REQUIRE(splitNode->cur_offset_ == 4);
        REQUIRE(std::get<Point>(splitNode->entries[0]) == Point(-1.0, -7.0));
        REQUIRE(std::get<Point>(splitNode->entries[1]) == Point(1.0, -7.0));
        REQUIRE(std::get<Point>(splitNode->entries[2]) == Point(2.0, -6.0));
        REQUIRE(std::get<Point>(splitNode->entries[3]) == Point(3.0, -8.0));
        REQUIRE(splitNode->level == rootNode->level);
    }
    unlink( "rstardiskbacked.txt" );

    {
        // Test set two
        // Cluster 2, n = 8
        // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
        rstartreedisk::RStarTreeDisk tree2( 4096*5, 3,7,
                "rstardiskbacked.txt" );
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
        REQUIRE(rootNode->cur_offset_ == 3);
        REQUIRE(std::get<Point>(rootNode->entries[0]) == Point(-9.0, 7.0));
        REQUIRE(std::get<Point>(rootNode->entries[1]) == Point(-14.0, 8.0));
        REQUIRE(std::get<Point>(rootNode->entries[2]) == Point(-10.0, 8.0));
        REQUIRE(splitNode->cur_offset_ == 5);
        REQUIRE(std::get<Point>(splitNode->entries[0]) == Point(-8.0, 8.0));
        REQUIRE(std::get<Point>(splitNode->entries[1]) == Point(-9.0, 9.0));
        REQUIRE(std::get<Point>(splitNode->entries[2]) == Point(-8.0, 9.0));
        REQUIRE(std::get<Point>(splitNode->entries[3]) == Point(-9.0, 10.0));
        REQUIRE(std::get<Point>(splitNode->entries[4]) == Point(-8.0, 10.0));
        REQUIRE(rootNode->level == splitNode->level);
    }
    unlink( "rstardiskbacked.txt" );


	// Test set three
	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// {(-5, 4), 1, 1}, {(-2, 4), 1, 1}, {(-1, 3), 1, 1}, {(-1, 1), 1, 1}, {(-3, 0), 1, 1},
	// {(-6, 2), 1, 1}
    {
        rstartreedisk::RStarTreeDisk tree3( 4096 * 5, 3, 7,
                "rstardiskbacked.txt" );
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
                NodeType::Branch>(Rectangle(-6.0, 3.0, -4.0, 5.0), dummys[0]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-6.0, 3.0, -4.0, 5.0), dummys[1]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-3.0, 3.0, -1.0, 5.0), dummys[2]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-2.0, 2.0, 0.0, 4.0), dummys[3]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-2.0, 0.0, 0.0, 2.0), dummys[4]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-4.0, -1.0, -2.0, 1.0), dummys[5]));
        rootNode->addEntryToNode(createBranchEntry<NodeType::NodeEntry,
                NodeType::Branch>(Rectangle(-7.0, 1.0, -5.0, 3.0), dummys[6]));


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
        REQUIRE(std::get<NodeType::Branch>(rootNode->entries[0]).boundingBox == Rectangle(-7.0, 1.0, -5.0, 3.0));
        REQUIRE(std::get<NodeType::Branch>(rootNode->entries[1]).boundingBox == Rectangle(-6.0, 3.0, -4.0, 5.0));
        REQUIRE(std::get<NodeType::Branch>(rootNode->entries[2]).boundingBox == Rectangle(-6.0, 3.0, -4.0, 5.0));
        REQUIRE(splitNode->cur_offset_ == 5);
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[0]).boundingBox == Rectangle(-4.0, -1.0, -2.0, 1.0));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[1]).boundingBox == Rectangle(-3.0, 3.0, -1.0, 5.0));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[2]).boundingBox == Rectangle(-2.0, 2.0, 0.0, 4.0));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[3]).boundingBox == Rectangle(-2.0, 0.0, 0.0, 2.0));
        REQUIRE(std::get<NodeType::Branch>(splitNode->entries[4]).boundingBox == Rectangle(1.0, 1.0, 2.0, 2.0));
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
        rstartreedisk::RStarTreeDisk tree(4096*5, 3, 7, "rstardiskbacked.txt");
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
        REQUIRE(childNode->boundingBox().centrePoint() == Point(0.0,0.0));
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
        rstartreedisk::RStarTreeDisk tree(4096*5, 3, 7, "rstardiskbacked.txt");
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
        rstartreedisk::RStarTreeDisk tree(4096*5, 3, 7,
                "rstardiskbacked.txt" );
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
