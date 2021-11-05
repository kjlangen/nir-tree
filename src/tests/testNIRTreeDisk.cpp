#include <catch2/catch.hpp>
#include <nirtreedisk/nirtreedisk.h>
#include <storage/page.h>
#include <util/geometry.h>
#include <iostream>
#include <unistd.h>

#define DefaultLeafNodeType nirtreedisk::LeafNode<3,7,nirtreedisk::LineMinimizeDownsplits>
#define DefaultBranchNodeType nirtreedisk::BranchNode<3,7,nirtreedisk::LineMinimizeDownsplits>
#define DefaulTreeType nirtreedisk::NIRTreeDisk<3,7, nirtreedisk::LineMinimizeDownsplits>

#define MeanBalancedNNType nirtreedisk::Node<3,7,nirtreedisk::LineMinimizeDistanceFromMean>
#define MeanBalancedLeafNodeType nirtreedisk::LeafNode<3,7,nirtreedisk::LineMinimizeDistanceFromMean>
#define MeanBalancedBranchNodeType nirtreedisk::BranchNode<3,7,nirtreedisk::LineMinimizeDistanceFromMean>
#define MeanBalancedTreeType nirtreedisk::NIRTreeDisk<3,7, nirtreedisk::LineMinimizeDistanceFromMean>

static nirtreedisk::Branch createBranchEntry(
    const InlineBoundedIsotheticPolygon &boundingBox,
    tree_node_handle child
) {
    nirtreedisk::Branch b(boundingBox, child);
	return b;
}

static nirtreedisk::Branch createBranchEntry(
    tree_node_handle poly_handle,
    tree_node_handle child
) {
    nirtreedisk::Branch b(poly_handle, child);
	return b;
}

static tree_node_handle
createFullLeafNode(DefaulTreeType &tree, tree_node_handle parent, Point p=Point::atOrigin)
{
    // Allocate new node
    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
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
        DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

        // The root node is a LeafNode by default. To avoid the
        // annoyance of making it into a Branch node, just make a fresh
        // node usingthe tree's allocator.

        auto root_alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( nirtreedisk::BRANCH_NODE ) );
        new (&(*root_alloc_data.first)) DefaultBranchNodeType( &tree,
                 tree_node_handle( nullptr ), root_alloc_data.second );

        pinned_node_ptr<DefaultBranchNodeType> rootNode =
            root_alloc_data.first;
        REQUIRE( rootNode->parent == tree_node_handle( nullptr ) );
        tree_node_handle root = root_alloc_data.second;
        
        // Make a bunch of Leaf Nodes
        std::pair<pinned_node_ptr<DefaultLeafNodeType>, tree_node_handle> alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child0);
        nirtreedisk::Branch entry =
            createBranchEntry( InlineBoundedIsotheticPolygon(), child0);

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, 1.0, 12.0, 5.0) ) );

        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child1 );
        entry = createBranchEntry( InlineBoundedIsotheticPolygon(), child1);

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, -4.0, 16.0, -2.0) ) );

        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child2 );
        entry = createBranchEntry( InlineBoundedIsotheticPolygon( ), child2 );

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, -6.0, 10.0, -4.0) ) );
        rootNode->addBranchToNode( entry );

        REQUIRE( rootNode->cur_offset_ == 3 );
        REQUIRE( rootNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0) );

    }

    unlink( "nirdiskbacked.txt" );

    {
        // Test set two
        DefaulTreeType tree(4096 * 5, "nirdiskbacked.txt" );

        auto alloc_root_data =
            tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( nirtreedisk::BRANCH_NODE ) );
        new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
                 tree_node_handle( nullptr ), alloc_root_data.second);
        tree_node_handle root = alloc_root_data.second;
        auto rootNode = alloc_root_data.first;

        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, child0, root );

        nirtreedisk::Branch entry = createBranchEntry( InlineBoundedIsotheticPolygon(), child0); 

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, 12.0, 10.0, 14.0) ) );
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child1 );

        entry = createBranchEntry(
                InlineBoundedIsotheticPolygon(), child1);
        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(10.0, 12.0, 12.0, 14.0) ) );
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child2 );
        entry = createBranchEntry( InlineBoundedIsotheticPolygon(), child2);
        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, 12.0, 14.0, 14.0) ) );

        rootNode->addBranchToNode( entry );
            
        REQUIRE( rootNode->cur_offset_ ==  3 );

        REQUIRE(rootNode->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
    }

    unlink( "nirdiskbacked.txt" );
}
TEST_CASE("NIRTreeDisk: testUpdateBoundingBox") {

    unlink( "nirdiskbacked.txt" );
	DefaulTreeType tree(4096*5, "nirdiskbacked.txt");

    auto alloc_root_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), alloc_root_data.second );
    tree_node_handle root = alloc_root_data.second;
    auto parentNode = alloc_root_data.first;

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child0 );
    auto child0Node = alloc_data.first;
	REQUIRE( child0Node->parent == root );

    auto entry = createBranchEntry(InlineBoundedIsotheticPolygon(), child0);

    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, -6.0, 10.0, -4.0) ) );

    parentNode->addBranchToNode( entry );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child1 );
    auto child1Node = alloc_data.first;
    REQUIRE( child1Node->parent == root );

    entry = createBranchEntry(
            InlineBoundedIsotheticPolygon(), child1);
    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, -4.0, 16.0, -2.0) ) );
    parentNode->addBranchToNode(entry);

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child2);
    auto child2Node = alloc_data.first;
    REQUIRE( child2Node->parent == root );
    
    entry = createBranchEntry(
            InlineBoundedIsotheticPolygon(), child2);
    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(10.0, 12.0, 12.0, 14.0) ) );
    parentNode->addBranchToNode( entry );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child3);
    auto child3Node = alloc_data.first;
    REQUIRE( child3Node->parent == root );
    entry = createBranchEntry( InlineBoundedIsotheticPolygon(), child3);
    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, 12.0, 14.0, 14.0) ) );

    parentNode->addBranchToNode( entry );
    REQUIRE( parentNode->cur_offset_ == 4 );

    InlineBoundedIsotheticPolygon stack_poly;
    IsotheticPolygon loc_poly(Rectangle(3.0, 3.0, 5.0,5.0) );
    stack_poly.push_polygon_to_disk( loc_poly );
	parentNode->updateBranch(child3, stack_poly);

	auto &b = parentNode->entries[3];
    auto &poly = std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly );
	REQUIRE(poly.materialize_polygon().boundingBox == Rectangle(3.0, 3.0, 5.0, 5.0));
    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: testRemoveChild" ) {
    unlink( "nirdiskbacked.txt" );
	DefaulTreeType tree(4096*5, "nirdiskbacked.txt");

    auto alloc_root_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), alloc_root_data.second );
    tree_node_handle root = alloc_root_data.second;
    auto parentNode = alloc_root_data.first;

    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child0
            );
    auto child0Node = alloc_data.first;
	REQUIRE( child0Node->parent == root );
	parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(
                Rectangle(8.0, -6.0, 10.0, -4.0)), child0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child1 );
    auto child1Node = alloc_data.first;
    REQUIRE( child1Node->parent == root );
    parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, -4.0, 16.0,
                    -2.0)), child1) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child2
            );
    auto child2Node = alloc_data.first;
    REQUIRE( child2Node->parent == root );
    parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(10.0, 12.0, 12.0,
                    14.0)), child2) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child3);
    auto child3Node = alloc_data.first;
    REQUIRE( child3Node->parent == root );
    parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, 12.0, 14.0, 14.0)), child3) );

    REQUIRE( parentNode->cur_offset_ ==  4 );

	// Remove one of the children
	parentNode->removeBranch(child3);
    REQUIRE( parentNode->cur_offset_ == 3 );

    unlink( "nirdiskbacked.txt" );

}

TEST_CASE("NIRTreeDisk: testRemoveData")
{

    unlink( "nirdiskbacked.txt" );

	// Setup a rtree::Node with some data
	DefaulTreeType tree( 4096, "nirdiskbacked.txt" );
	tree_node_handle root = tree.root;
    auto parentNode = tree.get_leaf_node( root );

    parentNode->addPoint( Point(9.0, -5.0) );
	parentNode->addPoint( Point(14.0, -3.0) );
	parentNode->addPoint( Point(11.0, 13.0) );
	parentNode->addPoint( Point(13.0, 13.0) );

	REQUIRE(parentNode->cur_offset_ == 4);
	// Remove some of the data
	parentNode->removePoint( Point(13.0, 13.0) );

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
	DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto root_alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    new (&(*root_alloc_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), root_alloc_data.second );

    pinned_node_ptr<DefaultBranchNodeType> rootNode =
        root_alloc_data.first;
    REQUIRE( rootNode->parent == tree_node_handle( nullptr ) );
    tree_node_handle root = root_alloc_data.second;

    auto alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    auto leftNode = alloc_branch_data.first;
    tree_node_handle left = alloc_branch_data.second;
    new (&(*leftNode)) DefaultBranchNodeType( &tree, root, left );

    alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    auto rightNode = alloc_branch_data.first;
    tree_node_handle right = alloc_branch_data.second;
    new (&(*rightNode)) DefaultBranchNodeType( &tree, root, right );

    tree_node_handle leftChild0 = createFullLeafNode(
            tree, left, Point(8.5, 12.5));
    tree_node_handle leftChild1 = createFullLeafNode(tree, left,
            Point(11.0,15.0));
    tree_node_handle leftChild2 = createFullLeafNode(tree, left,
            Point(13.5,13.5));
    leftNode->addBranchToNode(
        createBranchEntry( InlineBoundedIsotheticPolygon(Rectangle(8.0, 12.0,
                    nextafter(10.0, DBL_MAX),
                    nextafter(14.0, DBL_MAX))), leftChild0 ) );
    leftNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, 12.0,
                    nextafter(12.0, DBL_MAX),
                    nextafter(15.0, DBL_MAX))), leftChild1 ) );
    leftNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, 12.0,
                    nextafter(14.0, DBL_MAX),
                    nextafter(14.0, DBL_MAX))), leftChild2 ) );
    rootNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, 12.0,
                    nextafter(14.0, DBL_MAX),
                    nextafter(15.0, DBL_MAX))), left ) );

    tree_node_handle rightChild0 = createFullLeafNode(tree,right,
            Point(7.0, 3.0) );
    tree_node_handle rightChild1 = createFullLeafNode(tree,right,
            Point(13.0,-3.0));
    tree_node_handle rightChild2 = createFullLeafNode(tree,right);
    REQUIRE( rightNode->parent == root );
    rightNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, 1.0,
                    nextafter(12.0, DBL_MAX),
                    nextafter(5.0, DBL_MAX))), rightChild0 ));
    rightNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, -4.0,
                    nextafter(16.0, DBL_MAX),
                    nextafter(-2.0, DBL_MAX))), rightChild1 ));
    rightNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(8.0, -6.0,
                    nextafter(10.0, DBL_MAX),
                    nextafter(-4.0, DBL_MAX))), rightChild2 ));
    rootNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(7.0, -6.0,
                    nextafter(16.0, DBL_MAX),
                    nextafter(5.0, DBL_MAX))), right ));

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

	DefaulTreeType tree( 4096 * 10, "nirdiskbacked.txt" );
    auto alloc_root_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), alloc_root_data.second );
    tree_node_handle root = alloc_root_data.second;
    auto rootNode= alloc_root_data.first;


    auto alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    auto cluster4aNode = alloc_data.first;
    tree_node_handle cluster4a = alloc_data.second;
    new (&(*cluster4aNode)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr), cluster4a );
    cluster4aNode->addPoint( Point(-10.0, -2.0) );
    cluster4aNode->addPoint( Point(-12.0, -3.0) );
    cluster4aNode->addPoint( Point(-11.0, -3.0) );
    cluster4aNode->addPoint( Point(-10.0, -3.0) );
	
    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    auto cluster4bNode = alloc_data.first;
    tree_node_handle cluster4b = alloc_data.second;
    new (&(*cluster4bNode)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr), cluster4b );

	cluster4bNode->addPoint( Point(-9.0, -3.0) );
	cluster4bNode->addPoint( Point(-7.0, -3.0) );
	cluster4bNode->addPoint( Point(-10.0, -5.0) );

    auto alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    auto cluster4Node = alloc_branch_data.first;
    tree_node_handle cluster4 = alloc_branch_data.second;
    new (&(*cluster4Node)) DefaultBranchNodeType( &tree, root, cluster4);

	cluster4aNode->parent = cluster4;
	cluster4Node->addBranchToNode(
        createBranchEntry( InlineBoundedIsotheticPolygon(cluster4aNode->boundingBox()), cluster4a) );
	cluster4bNode->parent = cluster4;
	cluster4Node->addBranchToNode(
        createBranchEntry( InlineBoundedIsotheticPolygon(cluster4bNode->boundingBox()), cluster4b));

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    auto cluster5aNode = alloc_data.first;
    tree_node_handle cluster5a = alloc_data.second;
    new (&(*cluster5aNode)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr), cluster5a );
	cluster5aNode->addPoint( Point(-14.5, -13.0) );
	cluster5aNode->addPoint( Point(-14.0, -13.0) );
	cluster5aNode->addPoint( Point(-13.5, -13.5) );
	cluster5aNode->addPoint( Point(-15.0, -14.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    auto cluster5bNode = alloc_data.first;
    tree_node_handle cluster5b = alloc_data.second;
    new (&(*cluster5bNode)) DefaultLeafNodeType( &tree,
            tree_node_handle(nullptr), cluster5b );
	cluster5bNode->addPoint( Point(-14.0, -14.0) );
	cluster5bNode->addPoint( Point(-13.0, -14.0) );
	cluster5bNode->addPoint( Point(-12.0, -14.0) );
	cluster5bNode->addPoint( Point(-13.5, -16.0) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    auto cluster5cNode = alloc_data.first;
    tree_node_handle cluster5c = alloc_data.second;
    new (&(*cluster5cNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster5c );

	cluster5cNode->addPoint( Point(-15.0, -14.5) );
	cluster5cNode->addPoint( Point(-14.0, -14.5) );
	cluster5cNode->addPoint( Point(-12.5, -14.5) );
	cluster5cNode->addPoint( Point(-13.5, -15.5) );

    alloc_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( nirtreedisk::LEAF_NODE ) );
    auto cluster5dNode = alloc_data.first;
    tree_node_handle cluster5d = alloc_data.second;
    new (&(*cluster5dNode)) DefaultLeafNodeType( &tree, tree_node_handle( nullptr
                ), cluster5d );
	cluster5dNode->addPoint( Point(-15.0, -15.0));
	cluster5dNode->addPoint( Point(-14.0, -15.0));
	cluster5dNode->addPoint( Point(-13.0, -15.0));
	cluster5dNode->addPoint( Point(-12.0, -15.0));
	cluster5dNode->addPoint( Point(-15.0, -15.0));

    alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( nirtreedisk::BRANCH_NODE ) );
    auto cluster5Node = alloc_branch_data.first;
    tree_node_handle cluster5 = alloc_branch_data.second;
    new (&(*cluster5Node)) DefaultBranchNodeType( &tree, root, cluster5 );

	cluster5aNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(
            InlineBoundedIsotheticPolygon(cluster5aNode->boundingBox()),cluster5a));
	cluster5bNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(
            InlineBoundedIsotheticPolygon(cluster5bNode->boundingBox()), cluster5b));
	cluster5cNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(
            InlineBoundedIsotheticPolygon(cluster5cNode->boundingBox()), cluster5c));
	cluster5dNode->parent = cluster5;
	cluster5Node->addBranchToNode(
        createBranchEntry(
            InlineBoundedIsotheticPolygon(cluster5dNode->boundingBox()), cluster5d));

	// Root
	rootNode->addBranchToNode(
        createBranchEntry(
            InlineBoundedIsotheticPolygon(cluster4Node->boundingBox()), cluster4));
	rootNode->addBranchToNode(
        createBranchEntry(
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
        DefaulTreeType tree( 4096 * 10, "nirdiskbacked.txt" );
        auto alloc_root_data =
            tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( nirtreedisk::BRANCH_NODE ) );
        new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
                 tree_node_handle( nullptr ), alloc_root_data.second );
        root = alloc_root_data.second;
        auto rootNode= alloc_root_data.first;


        auto alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        auto cluster4aNode = alloc_data.first;
        cluster4a = alloc_data.second;
        new (&(*cluster4aNode)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr), cluster4a );
        cluster4aNode->addPoint( Point(-10.0, -2.0) );
        cluster4aNode->addPoint( Point(-12.0, -3.0) );
        cluster4aNode->addPoint( Point(-11.0, -3.0) );
        cluster4aNode->addPoint( Point(-10.0, -3.0) );
        
        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        auto cluster4bNode = alloc_data.first;
        cluster4b = alloc_data.second;
        new (&(*cluster4bNode)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr), cluster4b );

        cluster4bNode->addPoint( Point(-9.0, -3.0) );
        cluster4bNode->addPoint( Point(-7.0, -3.0) );
        cluster4bNode->addPoint( Point(-10.0, -5.0) );

        auto alloc_branch_data =
            tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( nirtreedisk::BRANCH_NODE ) );
        auto cluster4Node = alloc_branch_data.first;
        cluster4 = alloc_branch_data.second;
        new (&(*cluster4Node)) DefaultBranchNodeType( &tree, root, cluster4);

        cluster4aNode->parent = cluster4;
        cluster4Node->addBranchToNode(
            createBranchEntry( InlineBoundedIsotheticPolygon(cluster4aNode->boundingBox()), cluster4a) );
        cluster4bNode->parent = cluster4;
        cluster4Node->addBranchToNode(
            createBranchEntry( InlineBoundedIsotheticPolygon(cluster4bNode->boundingBox()), cluster4b));

        // Cluster 5, n = 16
        // (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
        // (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
        // (-14, -15), (-13, -15), (-12, -15)

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        auto cluster5aNode = alloc_data.first;
        cluster5a = alloc_data.second;
        new (&(*cluster5aNode)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr), cluster5a );
        cluster5aNode->addPoint( Point(-14.5, -13.0) );
        cluster5aNode->addPoint( Point(-14.0, -13.0) );
        cluster5aNode->addPoint( Point(-13.5, -13.5) );
        cluster5aNode->addPoint( Point(-15.0, -14.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        auto cluster5bNode = alloc_data.first;
        cluster5b = alloc_data.second;
        new (&(*cluster5bNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster5b );
        cluster5bNode->addPoint( Point(-14.0, -14.0) );
        cluster5bNode->addPoint( Point(-13.0, -14.0) );
        cluster5bNode->addPoint( Point(-12.0, -14.0) );
        cluster5bNode->addPoint( Point(-13.5, -16.0) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        auto cluster5cNode = alloc_data.first;
        cluster5c = alloc_data.second;
        new (&(*cluster5cNode)) DefaultLeafNodeType( &tree,
                    tree_node_handle(nullptr), cluster5c );

        cluster5cNode->addPoint( Point(-15.0, -14.5) );
        cluster5cNode->addPoint( Point(-14.0, -14.5) );
        cluster5cNode->addPoint( Point(-12.5, -14.5) );
        cluster5cNode->addPoint( Point(-13.5, -15.5) );

        alloc_data =
            tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( nirtreedisk::LEAF_NODE ) );
        auto cluster5dNode = alloc_data.first;
        cluster5d = alloc_data.second;
        new (&(*cluster5dNode)) DefaultLeafNodeType( &tree, tree_node_handle( nullptr
                    ), cluster5d );
        cluster5dNode->addPoint( Point(-15.0, -15.0));
        cluster5dNode->addPoint( Point(-14.0, -15.0));
        cluster5dNode->addPoint( Point(-13.0, -15.0));
        cluster5dNode->addPoint( Point(-12.0, -15.0));
        cluster5dNode->addPoint( Point(-15.0, -15.0));

        alloc_branch_data =
            tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( nirtreedisk::BRANCH_NODE ) );
        auto cluster5Node = alloc_branch_data.first;
        cluster5 = alloc_branch_data.second;
        new (&(*cluster5Node)) DefaultBranchNodeType( &tree, root, cluster5 );

        cluster5aNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(
                InlineBoundedIsotheticPolygon(cluster5aNode->boundingBox()),cluster5a));
        cluster5bNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(
                InlineBoundedIsotheticPolygon(cluster5bNode->boundingBox()), cluster5b));
        cluster5cNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(
                InlineBoundedIsotheticPolygon(cluster5cNode->boundingBox()), cluster5c));
        cluster5dNode->parent = cluster5;
        cluster5Node->addBranchToNode(
            createBranchEntry(
                InlineBoundedIsotheticPolygon(cluster5dNode->boundingBox()), cluster5d));

        // Root
        rootNode->addBranchToNode(
            createBranchEntry(
                InlineBoundedIsotheticPolygon(cluster4Node->boundingBox()), cluster4));
        rootNode->addBranchToNode(
            createBranchEntry(
                InlineBoundedIsotheticPolygon(cluster5Node->boundingBox()), cluster5));

        // Test finding leaves
        REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
        REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
        REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
        REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
        REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
        REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);

        tree.root = root;

        // Destroy tree
        tree.write_metadata();
    }

    // Read existing tree from disk
    DefaulTreeType tree( 4096 * 5, "nirdiskbacked.txt" );
    auto rootNode = tree.get_branch_node( tree.root );

	// Test finding leaves
	REQUIRE(rootNode->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(rootNode->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(rootNode->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(rootNode->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(rootNode->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(rootNode->findLeaf(Point(-13.0, -15.0)) == cluster5d);


    unlink( "nirdiskbacked.txt" );
    unlink( "nirdiskbacked.txt.meta" );
}

TEST_CASE("NIRTreeDisk: testInsertGrowTreeHeight")
{
    unlink( "nirdiskbacked.txt" );
    {
        unsigned maxBranchFactor = 7;
        DefaulTreeType tree(4096*5, "nirdiskbacked.txt");

        for( unsigned i = 0; i < maxBranchFactor + 1; i++) {
            tree.insert( Point(i,i) );
        }

        auto rootNode =
            tree.node_allocator_.get_tree_node<DefaultBranchNodeType>(
                    tree.root );
        REQUIRE(rootNode->cur_offset_ == 2);

        nirtreedisk::Branch bLeft = rootNode->entries[0];
        nirtreedisk::Branch bRight = rootNode->entries[1];
        REQUIRE( bLeft.child.get_type() ==  nirtreedisk::LEAF_NODE );
        REQUIRE( bRight.child.get_type() ==  nirtreedisk::LEAF_NODE );

        auto left = tree.get_leaf_node( bLeft.child );
        auto right = tree.get_leaf_node( bRight.child );

        REQUIRE(left->cur_offset_ == 4);
        REQUIRE(right->cur_offset_ == 4);
    }
    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: doubleGrowTreeHeight")
{
    unlink( "nirdiskbacked.txt" );
    {
        unsigned max_branch_factor = 7;
        unsigned insertion_count = max_branch_factor*7 + 1;

        MeanBalancedTreeType tree(4096*20, "nirdiskbacked.txt");
        for( unsigned i = 0; i < insertion_count; i++) {
            tree.insert(Point(i,i));
        }

        tree_node_handle root = tree.root;
        auto root_node = tree.get_branch_node( root );

        for( unsigned i = 0; i < insertion_count; i++) {
            REQUIRE( tree.search( Point(i,i) ).size() == 1 );
        }

        REQUIRE( root_node->cur_offset_ == 3 );

        nirtreedisk::Branch bLeft = root_node->entries[0];
        nirtreedisk::Branch bRight = root_node->entries[1];

        auto left = tree.get_branch_node( bLeft.child );
        auto right = tree.get_branch_node( bRight.child );

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
        // Guestimate way more nodes.
        size_t nodes_per_page = PAGE_DATA_SIZE / sizeof(
                DefaultLeafNodeType);

        // We need a decent number of pages in memory because during
        // searches the whole path down to the leaf is pinned.
        size_t page_count = 20;

        size_t insertion_count = max_branch_factor * (nodes_per_page *
                page_count) * 4;

        MeanBalancedTreeType tree(4096*page_count, "nirdiskbacked.txt");
        for( unsigned i = 0; i < insertion_count; i++) {
            tree.insert(Point(i,i));
        }

        for( unsigned i = 0; i < insertion_count; i++) {
            REQUIRE( tree.search( Point(i,i) ).size() == 1);
        }
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: pack simple leaf node") {
    unlink( "nirdiskbacked.txt" );
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point(i,i) );
    }

    auto root_leaf_node = tree.get_leaf_node( tree.root );
    REQUIRE( root_leaf_node->cur_offset_ == 5 );

    REQUIRE( root_leaf_node->compute_packed_size() <
            sizeof(DefaultLeafNodeType) );
    REQUIRE( root_leaf_node->compute_packed_size() ==
            sizeof(void *) + sizeof(tree_node_handle)*2 +
            sizeof(size_t) + sizeof(Point) * 5 );

    tree_node_handle repacked_handle = root_leaf_node->repack(
            &(tree.node_allocator_) );

    REQUIRE( repacked_handle != nullptr );
    auto packed_leaf =
        tree.node_allocator_.get_tree_node<nirtreedisk::packed_node>(
                repacked_handle );
    REQUIRE( nirtreedisk::read_pointer_from_buffer<DefaulTreeType>( packed_leaf->buffer_ ) == &tree );
    REQUIRE( * (tree_node_handle *) (packed_leaf->buffer_ + sizeof(void
                    *) ) ==
        root_leaf_node->self_handle_ );

    REQUIRE( * (tree_node_handle *) (packed_leaf->buffer_ + sizeof(void
                    *) + sizeof(tree_node_handle) ) ==
        root_leaf_node->parent );

    REQUIRE( * (size_t *) (packed_leaf->buffer_ + sizeof(void
                    *) + sizeof(tree_node_handle)*2 ) ==
        root_leaf_node->cur_offset_ );

    Point *p = (Point *) (packed_leaf->buffer_ + sizeof(void *) +
            sizeof(tree_node_handle)*2 + sizeof(size_t));
    REQUIRE( *(p++) == root_leaf_node->entries.at(0) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(1) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(2) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(3) );
    REQUIRE( *(p++) == root_leaf_node->entries.at(4) );
    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: pack branch node all inline") {
    unlink( "nirdiskbacked.txt" );
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point(i,i) );
    }

    auto alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second );

    auto alloc_leaf_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second );

    auto branch_node = alloc_branch_data.first;
    auto leaf_node = alloc_leaf_data.first;
    auto leaf_handle = alloc_leaf_data.second;

    // Add five different branches for the same leaf.
    // This isn't valid in practice but for testing purposes it is fine.
    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(
                Rectangle(0.0, 0.0, 1.0, 1.0)), leaf_handle );
    branch_node->addBranchToNode( b );
    b = createBranchEntry( InlineBoundedIsotheticPolygon(
                Rectangle(0.0, 0.0, 2.0, 2.0)), leaf_handle );
    branch_node->addBranchToNode( b );
    b = createBranchEntry( InlineBoundedIsotheticPolygon(
                Rectangle(0.0, 0.0, 3.0, 3.0)), leaf_handle );
    branch_node->addBranchToNode( b );
    b = createBranchEntry( InlineBoundedIsotheticPolygon(
                Rectangle(0.0, 0.0, 4.0, 4.0)), leaf_handle );
    branch_node->addBranchToNode( b );
    b = createBranchEntry( InlineBoundedIsotheticPolygon(
                Rectangle(0.0, 0.0, 5.0, 5.0)), leaf_handle );
    branch_node->addBranchToNode( b );

    REQUIRE( branch_node->cur_offset_ == 5 );

    tree_node_handle packed_handle = branch_node->repack(
            &(tree.node_allocator_) );
    REQUIRE( packed_handle != nullptr );

    auto packed_branch= tree.node_allocator_.get_tree_node<nirtreedisk::packed_node>(
            packed_handle );
    
    REQUIRE( nirtreedisk::read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->self_handle_ );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (size_t *) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(size_t);

    // There are 5 entries

    //Entry 1
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,1.0,1.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            1U );
    offset += sizeof(unsigned);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,1.0,1.0) );
    offset += sizeof(Rectangle);

    //Entry 2
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,2.0,2.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            1U );
    offset += sizeof(unsigned);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,2.0,2.0) );
    offset += sizeof(Rectangle);

    //Entry 3
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,3.0,3.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            1U );
    offset += sizeof(unsigned);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,3.0,3.0) );
    offset += sizeof(Rectangle);

    //Entry 4
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,4.0,4.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            1U );
    offset += sizeof(unsigned);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,4.0,4.0) );
    offset += sizeof(Rectangle);

    //Entry 5
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,5.0,5.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            1U );
    offset += sizeof(unsigned);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,5.0,5.0) );
    offset += sizeof(Rectangle);

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE("NIRTreeDisk: pack complex inline polygon") {
    unlink( "nirdiskbacked.txt" );
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point(i,i) );
    }

    auto alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second );

    auto alloc_leaf_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second );

    auto branch_node = alloc_branch_data.first;
    auto leaf_node = alloc_leaf_data.first;
    auto leaf_handle = alloc_leaf_data.second;

    IsotheticPolygon polygon( Rectangle(0.0,0.0,1.0,1.0) );
    polygon.basicRectangles.push_back( Rectangle( 1.0, 2.0, 3.0, 4.0 ) );
    polygon.basicRectangles.push_back( Rectangle( -1.0, -2.0, -3.0, -4.0 ) );
    polygon.basicRectangles.push_back( Rectangle( 10.0, 10.0, 30.0, 40.0 ) );
    polygon.basicRectangles.push_back( Rectangle( 1.0, 3.0, 3.0, 7.0 ) );
    polygon.recomputeBoundingBox();

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(), leaf_handle );
    std::get<InlineBoundedIsotheticPolygon>(b.boundingPoly).push_polygon_to_disk( polygon );
    branch_node->addBranchToNode( b );

    tree_node_handle packed_handle = branch_node->repack( &(tree.node_allocator_) ); 

    auto packed_branch= tree.node_allocator_.get_tree_node<nirtreedisk::packed_node>(
            packed_handle );
    
    REQUIRE( nirtreedisk::read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->self_handle_ );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (size_t *) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(size_t);


    //Entry 1
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            polygon.boundingBox );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            5U );
    offset += sizeof(unsigned);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(0.0,0.0,1.0,1.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(1.0,2.0,3.0,4.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(-1.0,-2.0,-3.0,-4.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(10.0,10.0,30.0,40.0) );
    offset += sizeof(Rectangle);
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
            Rectangle(1.0,3.0,3.0,7.0) );
    offset += sizeof(Rectangle);
}

TEST_CASE("NIRTreeDisk: pack out of line polygon") {
    unlink( "nirdiskbacked.txt" );
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point(i,i) );
    }

    auto alloc_branch_data =
        tree.node_allocator_.create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second );

    auto alloc_leaf_data =
        tree.node_allocator_.create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second );

    auto branch_node = alloc_branch_data.first;
    auto leaf_node = alloc_leaf_data.first;
    auto leaf_handle = alloc_leaf_data.second;

    IsotheticPolygon polygon( Rectangle(0.0, 0.0, 1.0, 1.0) );
    for( double i = 1.0; i < 20.0; i += 1.0 ) {
        polygon.basicRectangles.push_back( Rectangle( i, i, i+1.0,
                    i+1.0) );
    }
    polygon.recomputeBoundingBox();

    auto alloc_poly_data = tree.node_allocator_.create_new_tree_node<InlineUnboundedIsotheticPolygon>(
            compute_sizeof_inline_unbounded_polygon(
                polygon.basicRectangles.size()
                 ), NodeHandleType(nirtreedisk::BIG_POLYGON));
    new (&(*alloc_poly_data.first)) InlineUnboundedIsotheticPolygon(
            &(tree.node_allocator_), polygon.basicRectangles.size() );
    alloc_poly_data.first->push_polygon_to_disk( polygon );

    nirtreedisk::Branch b = createBranchEntry( alloc_poly_data.second, leaf_handle );
    branch_node->addBranchToNode( b );

    tree_node_handle packed_handle = branch_node->repack( &(tree.node_allocator_) ); 

    auto packed_branch= tree.node_allocator_.get_tree_node<nirtreedisk::packed_node>(
            packed_handle );

    REQUIRE( nirtreedisk::read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->self_handle_ );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (size_t *) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(size_t);

    // Entry 1
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
           leaf_handle ); 
    offset += sizeof( tree_node_handle );
    REQUIRE( * (Rectangle *) (packed_branch->buffer_ + offset) ==
           polygon.boundingBox );
    offset += sizeof(Rectangle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
           std::numeric_limits<unsigned>::max() );
    offset += sizeof(unsigned);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
           alloc_poly_data.second );

}
