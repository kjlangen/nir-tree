#include <catch2/catch.hpp>
#include <nirtreedisk/nirtreedisk.h>
#include <storage/page.h>
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
    new (&(*node)) NodeType( &tree, tree_node_handle(nullptr),
            node_handle );

	for (unsigned i = 0; i < 7; i++) {
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
                    InlineBoundedIsotheticPolygon(), child0);

        std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, 1.0, 12.0, 5.0) )
                    );

        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child1, root );
        entry =
            createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,nirtreedisk::Branch>(
                    InlineBoundedIsotheticPolygon(), child1);

        std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, -4.0, 16.0, -2.0) ) );

        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child2, root );
        entry =
            createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,nirtreedisk::Branch>(
                    InlineBoundedIsotheticPolygon( ), child2 );

        std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, -6.0, 10.0, -4.0) ) );
        rootNode->addEntryToNode( entry );

        REQUIRE( rootNode->cur_offset_ == 3 );
        REQUIRE( rootNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0) );

    }
    unlink( "nirdiskbacked.txt" );
    {
        // Test set two
        TreeType tree(4096 * 5, "nirdiskbacked.txt" );
        tree_node_handle root = tree.root;

        auto rootNode =
            tree.node_allocator_.get_tree_node<nirtreedisk::Node<3,7>>( root );
        
        auto  alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child0, root );

        auto entry = createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(), child0); 

        std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, 12.0, 10.0, 14.0) ) );
        rootNode->addEntryToNode( entry );


        alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child1, root );

        entry = createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(), child1);
        std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(10.0, 12.0, 12.0, 14.0) ) );
        rootNode->addEntryToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<nirtreedisk::Node<3,7>>();
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) NodeType( &tree, child2, root );
        entry = createBranchEntry<nirtreedisk::Node<3,7>::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(), child2);
        std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, 12.0, 14.0, 14.0) ) );

        rootNode->addEntryToNode( entry );
            
        REQUIRE( rootNode->cur_offset_ ==  3 );

        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
    }
    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: testUpdateBoundingBox") {

    unlink( "nirdiskbacked.txt" );
	TreeType tree(4096*5, "nirdiskbacked.txt");
    tree_node_handle root = tree.root;

    auto parentNode =
        tree.node_allocator_.get_tree_node<NodeType>( root );

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child0, root );
    auto child0Node = alloc_data.first;
	child0Node->parent = root;
    auto entry = createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(InlineBoundedIsotheticPolygon(), child0);

    std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, -6.0, 10.0, -4.0) ) );

    parentNode->addEntryToNode( entry );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child1, root );
    auto child1Node = alloc_data.first;
    child1Node->parent = root;

    entry = createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(), child1);
    std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, -4.0, 16.0, -2.0) ) );
    parentNode->addEntryToNode(entry);

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child2, root );
    auto child2Node = alloc_data.first;
    child2Node->parent = root;
    
    entry = createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(), child2);
    std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(10.0, 12.0, 12.0, 14.0) ) );

    parentNode->addEntryToNode( entry );
    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child3, root );
    auto child3Node = alloc_data.first;
    child3Node->parent = root;
    entry = createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(), child3);
    std::get<InlineBoundedIsotheticPolygon>( std::get<nirtreedisk::Branch>( entry
                    ).boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, 12.0, 14.0, 14.0) ) );

    parentNode->addEntryToNode( entry );
    REQUIRE( parentNode->cur_offset_ == 4 );

    InlineBoundedIsotheticPolygon stack_poly;
    IsotheticPolygon loc_poly(Rectangle(3.0, 3.0, 5.0,5.0) );
    stack_poly.push_polygon_to_disk( loc_poly );
	parentNode->updateBranch(child3, stack_poly);

	auto &b = std::get<nirtreedisk::Branch>(parentNode->entries[3]);
    auto &poly = std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly );
	REQUIRE(poly.materialize_polygon().boundingBox == Rectangle(3.0, 3.0, 5.0, 5.0));
    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: testRemoveChild" ) {
    unlink( "nirdiskbacked.txt" );
	TreeType tree(4096*5, "nirdiskbacked.txt");
    tree_node_handle root = tree.root;

    auto parentNode =
        tree.node_allocator_.get_tree_node<NodeType>( root );

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child0, root );
    auto child0Node = alloc_data.first;
	child0Node->parent = root;
	parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(
                Rectangle(8.0, -6.0, 10.0, -4.0)), child0) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child1, root );
    auto child1Node = alloc_data.first;
    child1Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, -4.0, 16.0,
                    -2.0)), child1) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child2, root );
    auto child2Node = alloc_data.first;
    child2Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(10.0, 12.0, 12.0,
                    14.0)), child2) );

    alloc_data = tree.node_allocator_.create_new_tree_node<NodeType>();
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) NodeType( &tree, child3, root );
    auto child3Node = alloc_data.first;
    child3Node->parent = root;
    parentNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, 12.0, 14.0,
                    14.0)), child3) );

    REQUIRE( parentNode->cur_offset_ ==  4 );

	// Remove one of the children
	parentNode->removeEntry(child3);
    REQUIRE( parentNode->cur_offset_ == 3 );

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: testRemoveData")
{

    unlink( "nirdiskbacked.txt" );

	// Setup a rtree::Node with some data
	TreeType tree( 4096, "nirdiskbacked.txt" );
	tree_node_handle root = tree.root;
    auto parentNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );

    parentNode->addEntryToNode( Point(9.0, -5.0) );
	parentNode->addEntryToNode( Point(14.0, -3.0) );
	parentNode->addEntryToNode( Point(11.0, 13.0) );
	parentNode->addEntryToNode( Point(13.0, 13.0) );

	REQUIRE(parentNode->cur_offset_ == 4);
	// Remove some of the data
	parentNode->removeEntry( Point(13.0, 13.0) );

	// Test the removal
	REQUIRE(parentNode->cur_offset_ == 3);

    unlink( "nirdiskbacked.txt" );
}


TEST_CASE("NIRTreeDisk: testFindLeaf")
{
	// Create rtree::Nodes
    unlink( "nirdiskbacked.txt" );

    // Need a bunch of pages so we don't run out of memory while
    // everything is pinned
	TreeType tree( 4096*5, "nirdiskbacked.txt" );
    tree_node_handle root = tree.root;

    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto leftNode = alloc_data.first;
    tree_node_handle left = alloc_data.second;
    new (&(*leftNode)) NodeType( &tree, left, root );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto rightNode = alloc_data.first;
    tree_node_handle right = alloc_data.second;
    new (&(*rightNode)) NodeType( &tree, right, root );

    tree_node_handle leftChild0 = createFullLeafNode(
            tree, left, Point(8.5, 12.5));
    tree_node_handle leftChild1 = createFullLeafNode(tree, left,
            Point(11.0,15.0));
    tree_node_handle leftChild2 = createFullLeafNode(tree, left,
            Point(13.5,13.5));
    leftNode->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(8.0, 12.0, 10.0,
                    14.0)), leftChild0 ) );
    leftNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, 12.0, 12.0,
                    15.0)), leftChild1 ) );
    leftNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, 12.0, 14.0,
                    14.0)), leftChild2 ) );
    rootNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, 12.0, 14.0,
                    15.0)), left ) );

    tree_node_handle rightChild0 = createFullLeafNode(tree,right,
            Point(7.0, 3.0) );
    tree_node_handle rightChild1 = createFullLeafNode(tree,right,
            Point(13.0,-3.0));
    tree_node_handle rightChild2 = createFullLeafNode(tree,right);
    rightNode->parent = root;
    rightNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, 1.0, 12.0,
                    5.0)), rightChild0 ));
    rightNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, -4.0, 16.0,
                    -2.0)), rightChild1 ));
    rightNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(8.0, -6.0, 10.0,
                    -4.0)), rightChild2 ));
    rootNode->addEntryToNode( createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, -6.0, 16.0,
                    5.0)), right ));

	// Test that we get the correct child for the given point
	REQUIRE(rightChild1 == rootNode->findLeaf(Point(13.0, -3.0)));
	REQUIRE(leftChild0 == rootNode->findLeaf(Point(8.5, 12.5)));
	REQUIRE(leftChild2 == rootNode->findLeaf(Point(13.5, 13.5)));
	REQUIRE(rightChild0 == rootNode->findLeaf(Point(7.0, 3.0)));
	REQUIRE(leftChild1 == rootNode->findLeaf(Point(11.0, 15.0)));

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: testFindLeaf2")
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
    unlink( "nirdiskbacked.txt" );

	TreeType tree( 4096 * 10, "nirdiskbacked.txt" );
    tree_node_handle root = tree.root;
    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
            );

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4aNode = alloc_data.first;
    tree_node_handle cluster4a = alloc_data.second;
    new (&(*cluster4aNode)) NodeType( &tree, tree_node_handle(nullptr), cluster4a );
    cluster4aNode->addEntryToNode( Point(-10.0, -2.0) );
    cluster4aNode->addEntryToNode( Point(-12.0, -3.0) );
    cluster4aNode->addEntryToNode( Point(-11.0, -3.0) );
    cluster4aNode->addEntryToNode( Point(-10.0, -3.0) );
	
    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4bNode = alloc_data.first;
    tree_node_handle cluster4b = alloc_data.second;
    new (&(*cluster4bNode)) NodeType( &tree, tree_node_handle(nullptr), cluster4b );

	cluster4bNode->addEntryToNode( Point(-9.0, -3.0) );
	cluster4bNode->addEntryToNode( Point(-7.0, -3.0) );
	cluster4bNode->addEntryToNode( Point(-10.0, -5.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster4Node = alloc_data.first;
    tree_node_handle cluster4 = alloc_data.second;


    new (&(*cluster4Node)) NodeType( &tree, root, cluster4);
	cluster4aNode->parent = cluster4;
	cluster4Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry, nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster4aNode->boundingBox()), cluster4a) );
	cluster4bNode->parent = cluster4;
	cluster4Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster4bNode->boundingBox()), cluster4b));

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5aNode = alloc_data.first;
    tree_node_handle cluster5a = alloc_data.second;
    new (&(*cluster5aNode)) NodeType( &tree, cluster5a,
                tree_node_handle(nullptr) );
	cluster5aNode->addEntryToNode( Point(-14.5, -13.0) );
	cluster5aNode->addEntryToNode( Point(-14.0, -13.0) );
	cluster5aNode->addEntryToNode( Point(-13.5, -13.5) );
	cluster5aNode->addEntryToNode( Point(-15.0, -14.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5bNode = alloc_data.first;
    tree_node_handle cluster5b = alloc_data.second;
    new (&(*cluster5bNode)) NodeType( &tree, tree_node_handle(), cluster5b );
	cluster5bNode->addEntryToNode( Point(-14.0, -14.0) );
	cluster5bNode->addEntryToNode( Point(-13.0, -14.0) );
	cluster5bNode->addEntryToNode( Point(-12.0, -14.0) );
	cluster5bNode->addEntryToNode( Point(-13.5, -16.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5cNode = alloc_data.first;
    tree_node_handle cluster5c = alloc_data.second;
    new (&(*cluster5cNode)) NodeType( &tree,
                tree_node_handle(nullptr), cluster5c );

	cluster5cNode->addEntryToNode( Point(-15.0, -14.5) );
	cluster5cNode->addEntryToNode( Point(-14.0, -14.5) );
	cluster5cNode->addEntryToNode( Point(-12.5, -14.5) );
	cluster5cNode->addEntryToNode( Point(-13.5, -15.5) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5dNode = alloc_data.first;
    tree_node_handle cluster5d = alloc_data.second;
    new (&(*cluster5dNode)) NodeType( &tree, tree_node_handle( nullptr
                ), cluster5d );
	cluster5dNode->addEntryToNode( Point(-15.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-14.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-13.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-12.0, -15.0));
	cluster5dNode->addEntryToNode( Point(-15.0, -15.0));

    alloc_data =
        tree.node_allocator_.create_new_tree_node<NodeType>();
    auto cluster5Node = alloc_data.first;
    tree_node_handle cluster5 = alloc_data.second;
    new (&(*cluster5Node)) NodeType( &tree, root, cluster5 );

	cluster5aNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster5aNode->boundingBox()),cluster5a));
	cluster5bNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster5bNode->boundingBox()), cluster5b));


	cluster5cNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster5cNode->boundingBox()), cluster5c));

	cluster5dNode->parent = cluster5;
	cluster5Node->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster5dNode->boundingBox()), cluster5d));

	// Root
	rootNode->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster4Node->boundingBox()), cluster4));
	rootNode->addEntryToNode(
        createBranchEntry<NodeType::NodeEntry,
        nirtreedisk::Branch>(
            InlineBoundedIsotheticPolygon(cluster5Node->boundingBox()), cluster5));

	// Test finding leaves
	REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: testFindLeaf2 ON DISK")
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
    unlink( "nirdiskbacked.txt" );
    tree_node_handle root;
    tree_node_handle cluster4a;
    tree_node_handle cluster4b;
    tree_node_handle cluster4;
    tree_node_handle cluster5a;
    tree_node_handle cluster5b;
    tree_node_handle cluster5c;
    tree_node_handle cluster5d;
    tree_node_handle cluster5;

    {
        TreeType tree( 4096 * 10, "nirdiskbacked.txt" );
        root = tree.root;
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( root
                );

        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4aNode = alloc_data.first;
        cluster4a = alloc_data.second;
        new (&(*cluster4aNode)) NodeType( &tree, tree_node_handle(nullptr), cluster4a );
        cluster4aNode->addEntryToNode( Point(-10.0, -2.0) );
        cluster4aNode->addEntryToNode( Point(-12.0, -3.0) );
        cluster4aNode->addEntryToNode( Point(-11.0, -3.0) );
        cluster4aNode->addEntryToNode( Point(-10.0, -3.0) );
        
        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4bNode = alloc_data.first;
        cluster4b = alloc_data.second;
        new (&(*cluster4bNode)) NodeType( &tree, tree_node_handle(nullptr), cluster4b );

        cluster4bNode->addEntryToNode( Point(-9.0, -3.0) );
        cluster4bNode->addEntryToNode( Point(-7.0, -3.0) );
        cluster4bNode->addEntryToNode( Point(-10.0, -5.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster4Node = alloc_data.first;
        cluster4 = alloc_data.second;

        new (&(*cluster4Node)) NodeType( &tree, root, cluster4);
        cluster4aNode->parent = cluster4;
        cluster4Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry, nirtreedisk::Branch>(
                    InlineBoundedIsotheticPolygon(cluster4aNode->boundingBox()), cluster4a) );
        cluster4bNode->parent = cluster4;
        cluster4Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster4bNode->boundingBox()), cluster4b));

        // Cluster 5, n = 16
        // (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
        // (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
        // (-14, -15), (-13, -15), (-12, -15)

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5aNode = alloc_data.first;
        cluster5a = alloc_data.second;
        new (&(*cluster5aNode)) NodeType( &tree, cluster5a,
                    tree_node_handle(nullptr) );
        cluster5aNode->addEntryToNode( Point(-14.5, -13.0) );
        cluster5aNode->addEntryToNode( Point(-14.0, -13.0) );
        cluster5aNode->addEntryToNode( Point(-13.5, -13.5) );
        cluster5aNode->addEntryToNode( Point(-15.0, -14.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5bNode = alloc_data.first;
        cluster5b = alloc_data.second;
        new (&(*cluster5bNode)) NodeType( &tree, tree_node_handle(), cluster5b );
        cluster5bNode->addEntryToNode( Point(-14.0, -14.0) );
        cluster5bNode->addEntryToNode( Point(-13.0, -14.0) );
        cluster5bNode->addEntryToNode( Point(-12.0, -14.0) );
        cluster5bNode->addEntryToNode( Point(-13.5, -16.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5cNode = alloc_data.first;
        cluster5c = alloc_data.second;
        new (&(*cluster5cNode)) NodeType( &tree,
                    tree_node_handle(nullptr), cluster5c );

        cluster5cNode->addEntryToNode( Point(-15.0, -14.5) );
        cluster5cNode->addEntryToNode( Point(-14.0, -14.5) );
        cluster5cNode->addEntryToNode( Point(-12.5, -14.5) );
        cluster5cNode->addEntryToNode( Point(-13.5, -15.5) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5dNode = alloc_data.first;
        cluster5d = alloc_data.second;
        new (&(*cluster5dNode)) NodeType( &tree, tree_node_handle( nullptr
                    ), cluster5d );
        cluster5dNode->addEntryToNode( Point(-15.0, -15.0));
        cluster5dNode->addEntryToNode( Point(-14.0, -15.0));
        cluster5dNode->addEntryToNode( Point(-13.0, -15.0));
        cluster5dNode->addEntryToNode( Point(-12.0, -15.0));
        cluster5dNode->addEntryToNode( Point(-15.0, -15.0));

        alloc_data =
            tree.node_allocator_.create_new_tree_node<NodeType>();
        auto cluster5Node = alloc_data.first;
        cluster5 = alloc_data.second;
        new (&(*cluster5Node)) NodeType( &tree, root, cluster5 );

        cluster5aNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster5aNode->boundingBox()),cluster5a));
        cluster5bNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster5bNode->boundingBox()), cluster5b));


        cluster5cNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster5cNode->boundingBox()), cluster5c));

        cluster5dNode->parent = cluster5;
        cluster5Node->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster5dNode->boundingBox()), cluster5d));

        // Root
        rootNode->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster4Node->boundingBox()), cluster4));
        rootNode->addEntryToNode(
            createBranchEntry<NodeType::NodeEntry,
            nirtreedisk::Branch>(
                InlineBoundedIsotheticPolygon(cluster5Node->boundingBox()), cluster5));

        // Destroy tree
    }

    // Read existing tree from disk
    TreeType tree( 4096 * 5, "nirdiskbacked.txt" );
    auto rootNode = tree.node_allocator_.get_tree_node<NodeType>( tree.root
            );

	// Test finding leaves
	REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);


    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: testInsertGrowTreeHeight")
{
    unlink( "nirdiskbacked.txt" );
    {
        unsigned maxBranchFactor = 7;
        TreeType tree(4096*5, "nirdiskbacked.txt");
        auto rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                tree.root );

        for( unsigned i = 0; i < maxBranchFactor + 1; i++) {
            tree_node_handle root = rootNode->insert(Point(0.0, 0.0));
            rootNode = tree.node_allocator_.get_tree_node<NodeType>(
                    root );
        }

        REQUIRE(rootNode->cur_offset_ == 2);
        nirtreedisk::Branch bLeft = std::get<nirtreedisk::Branch>(rootNode->entries[0]);
        nirtreedisk::Branch bRight = std::get<nirtreedisk::Branch>(rootNode->entries[1]);

        auto left = tree.node_allocator_.get_tree_node<NodeType>(
                bLeft.child );
        auto right = tree.node_allocator_.get_tree_node<NodeType>(
                bRight.child );

        REQUIRE(left->cur_offset_ == 4);
        REQUIRE(right->cur_offset_ == 4);
    }
    unlink( "nirdiskbacked.txt" );
}

/*TEST_CASE("NIRTreeDisk: doubleGrowTreeHeight")
{
    unlink( "nirdiskbacked.txt" );
    {
        unsigned max_branch_factor = 7;
        unsigned insertion_count = max_branch_factor*7 + 1;

        TreeType tree(4096*20, "nirdiskbacked.txt");
        for( unsigned i = 0; i < insertion_count; i++) {
            tree.insert(Point(i,i));
        }


        //REQUIRE(rootNode->cur_offset_ == 2);
        auto root = tree.root;
        auto root_node = tree.node_allocator_.get_tree_node<NodeType>(
                root );

        for( unsigned i = 0; i < insertion_count; i++) {
            REQUIRE( tree.search( Point(i,i) ).size() == 1 );
        }

        REQUIRE( root_node->cur_offset_ == 3 );
        nirtreedisk::Branch bLeft = std::get<nirtreedisk::Branch>(root_node->entries[0]);
        nirtreedisk::Branch bRight = std::get<nirtreedisk::Branch>(root_node->entries[1]);

        auto left = tree.node_allocator_.get_tree_node<NodeType>(
                bLeft.child );
        auto right = tree.node_allocator_.get_tree_node<NodeType>(
                bRight.child );

        REQUIRE(left->cur_offset_ == 4);
        REQUIRE(right->cur_offset_ == 4);
    }
    unlink( "nirdiskbacked.txt" );
}
*/


/*
TEST_CASE("NIRTreeDisk: grow tree branch all same point")
{
    unlink( "nirdiskbacked.txt" );
    {
        unsigned max_branch_factor = 7;
        unsigned insertion_count = max_branch_factor*max_branch_factor + 1;

        TreeType tree(4096*10, "nirdiskbacked.txt");
        for( unsigned i = 0; i < insertion_count; i++) {
            tree.insert(Point(0,0));
        }

        auto root = tree.root;
        auto root_node = tree.node_allocator_.get_tree_node<NodeType>(
                root );

        for( unsigned i = 0; i < insertion_count; i++) {
            REQUIRE( tree.search( Point(0,0) ).size() ==
                    max_branch_factor*max_branch_factor+1 );
        }

        REQUIRE( root_node->cur_offset_ == 3 );
        nirtreedisk::Branch bLeft = std::get<nirtreedisk::Branch>(root_node->entries[0]);
        nirtreedisk::Branch bRight = std::get<nirtreedisk::Branch>(root_node->entries[1]);

        auto left = tree.node_allocator_.get_tree_node<NodeType>(
                bLeft.child );
        auto right = tree.node_allocator_.get_tree_node<NodeType>(
                bRight.child );

        REQUIRE(left->cur_offset_ == 4);
        REQUIRE(right->cur_offset_ == 4);
    }
    unlink( "nirdiskbacked.txt" );
}


TEST_CASE( "NIRTreeDisk: grow well-beyond memory provisions" )
{
    unlink( "nirdiskbacked.txt" );
    {
        unsigned max_branch_factor = 7;
        size_t nodes_per_page = PAGE_DATA_SIZE / sizeof(
                NodeType);

        // We need a decent number of pages in memory because during
        // searches the whole path down to the leaf is pinned.
        size_t page_count = 20;

        size_t insertion_count = max_branch_factor * (nodes_per_page *
                page_count) * 4;

        TreeType tree(4096*page_count, "nirdiskbacked.txt");
        for( unsigned i = 0; i < insertion_count; i++) {
            tree.insert(Point(i,i));
        }

        for( unsigned i = 0; i < insertion_count; i++) {
            REQUIRE( tree.search( Point(i,i) ).size() == 1);
        }
    }
    unlink( "nirdiskbacked.txt" );

}
*/
