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
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle node_handle = alloc_data.second;
    auto node = alloc_data.first;
    new (&(*node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            node_handle, 0 );

    std::vector<bool> hasReinsertedOnLevel = {false};
	for (unsigned i = 0; i < 7; i++) {
		node->insert(p, hasReinsertedOnLevel );
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
            tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*root_alloc_data.first)) DefaultBranchNodeType( &tree,
                 tree_node_handle( nullptr ), root_alloc_data.second, 1 );

        pinned_node_ptr<DefaultBranchNodeType> rootNode =
            root_alloc_data.first;
        REQUIRE( rootNode->parent == tree_node_handle( nullptr ) );
        tree_node_handle root = root_alloc_data.second;
        
        // Make a bunch of Leaf Nodes
        std::pair<pinned_node_ptr<DefaultLeafNodeType>, tree_node_handle> alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root,
                child0, 0);
        nirtreedisk::Branch entry =
            createBranchEntry( InlineBoundedIsotheticPolygon(), child0);

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, 1.0, 12.0, 5.0) ) );

        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root,
                child1, 0 );
        entry = createBranchEntry( InlineBoundedIsotheticPolygon(), child1);

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, -4.0, 16.0, -2.0) ) );

        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root,
                child2, 0 );
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
            tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
                 tree_node_handle( nullptr ), alloc_root_data.second, 1);
        tree_node_handle root = alloc_root_data.second;
        auto rootNode = alloc_root_data.first;

        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child0 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, child0,
                root, 0 );

        nirtreedisk::Branch entry = createBranchEntry( InlineBoundedIsotheticPolygon(), child0); 

        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, 12.0, 10.0, 14.0) ) );
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child1 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root,
                child1, 0 );

        entry = createBranchEntry(
                InlineBoundedIsotheticPolygon(), child1);
        std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(10.0, 12.0, 12.0, 14.0) ) );
        rootNode->addBranchToNode( entry );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        tree_node_handle child2 = alloc_data.second;
        new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root,
                child2, 0 );
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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), alloc_root_data.second, 1 );
    tree_node_handle root = alloc_root_data.second;
    auto parentNode = alloc_root_data.first;

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child0,
            0 );
    auto child0Node = alloc_data.first;
	REQUIRE( child0Node->parent == root );

    auto entry = createBranchEntry(InlineBoundedIsotheticPolygon(), child0);

    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(8.0, -6.0, 10.0, -4.0) ) );

    parentNode->addBranchToNode( entry );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root,
            child1,0 );
    auto child1Node = alloc_data.first;
    REQUIRE( child1Node->parent == root );

    entry = createBranchEntry(
            InlineBoundedIsotheticPolygon(), child1);
    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(12.0, -4.0, 16.0, -2.0) ) );
    parentNode->addBranchToNode(entry);

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child2,
            0);
    auto child2Node = alloc_data.first;
    REQUIRE( child2Node->parent == root );
    
    entry = createBranchEntry(
            InlineBoundedIsotheticPolygon(), child2);
    std::get<InlineBoundedIsotheticPolygon>( entry.boundingPoly ).push_polygon_to_disk(
                    IsotheticPolygon( Rectangle(10.0, 12.0, 12.0, 14.0) ) );
    parentNode->addBranchToNode( entry );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child3,
            0);
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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), alloc_root_data.second, 1 );
    tree_node_handle root = alloc_root_data.second;
    auto parentNode = alloc_root_data.first;

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child0 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child0,
            0
            );
    auto child0Node = alloc_data.first;
	REQUIRE( child0Node->parent == root );
	parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(
                Rectangle(8.0, -6.0, 10.0, -4.0)), child0) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child1 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child1,
            0 );
    auto child1Node = alloc_data.first;
    REQUIRE( child1Node->parent == root );
    parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(12.0, -4.0, 16.0,
                    -2.0)), child1) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child2 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child2,
            0
            );
    auto child2Node = alloc_data.first;
    REQUIRE( child2Node->parent == root );
    parentNode->addBranchToNode( createBranchEntry(
            InlineBoundedIsotheticPolygon(Rectangle(10.0, 12.0, 12.0,
                    14.0)), child2) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle child3 = alloc_data.second;
    new (&(*alloc_data.first)) DefaultLeafNodeType( &tree, root, child3,
            0);
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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*root_alloc_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), root_alloc_data.second, 2 );

    pinned_node_ptr<DefaultBranchNodeType> rootNode =
        root_alloc_data.first;
    REQUIRE( rootNode->parent == tree_node_handle( nullptr ) );
    tree_node_handle root = root_alloc_data.second;

    auto alloc_branch_data =
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto leftNode = alloc_branch_data.first;
    tree_node_handle left = alloc_branch_data.second;
    new (&(*leftNode)) DefaultBranchNodeType( &tree, root, left, 1 );

    alloc_branch_data =
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto rightNode = alloc_branch_data.first;
    tree_node_handle right = alloc_branch_data.second;
    new (&(*rightNode)) DefaultBranchNodeType( &tree, root, right, 1 );

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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
             tree_node_handle( nullptr ), alloc_root_data.second, 2 );
    tree_node_handle root = alloc_root_data.second;
    auto rootNode= alloc_root_data.first;


    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster4aNode = alloc_data.first;
    tree_node_handle cluster4a = alloc_data.second;
    new (&(*cluster4aNode)) DefaultLeafNodeType( &tree,
            tree_node_handle(nullptr), cluster4a, 0 );
    cluster4aNode->addPoint( Point(-10.0, -2.0) );
    cluster4aNode->addPoint( Point(-12.0, -3.0) );
    cluster4aNode->addPoint( Point(-11.0, -3.0) );
    cluster4aNode->addPoint( Point(-10.0, -3.0) );
	
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster4bNode = alloc_data.first;
    tree_node_handle cluster4b = alloc_data.second;
    new (&(*cluster4bNode)) DefaultLeafNodeType( &tree,
            tree_node_handle(nullptr), cluster4b, 0 );

	cluster4bNode->addPoint( Point(-9.0, -3.0) );
	cluster4bNode->addPoint( Point(-7.0, -3.0) );
	cluster4bNode->addPoint( Point(-10.0, -5.0) );

    auto alloc_branch_data =
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto cluster4Node = alloc_branch_data.first;
    tree_node_handle cluster4 = alloc_branch_data.second;
    new (&(*cluster4Node)) DefaultBranchNodeType( &tree, root, cluster4,
            1);

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
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5aNode = alloc_data.first;
    tree_node_handle cluster5a = alloc_data.second;
    new (&(*cluster5aNode)) DefaultLeafNodeType( &tree,
            tree_node_handle(nullptr), cluster5a, 0 );
	cluster5aNode->addPoint( Point(-14.5, -13.0) );
	cluster5aNode->addPoint( Point(-14.0, -13.0) );
	cluster5aNode->addPoint( Point(-13.5, -13.5) );
	cluster5aNode->addPoint( Point(-15.0, -14.0) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5bNode = alloc_data.first;
    tree_node_handle cluster5b = alloc_data.second;
    new (&(*cluster5bNode)) DefaultLeafNodeType( &tree,
            tree_node_handle(nullptr), cluster5b, 0 );
	cluster5bNode->addPoint( Point(-14.0, -14.0) );
	cluster5bNode->addPoint( Point(-13.0, -14.0) );
	cluster5bNode->addPoint( Point(-12.0, -14.0) );
	cluster5bNode->addPoint( Point(-13.5, -16.0) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5cNode = alloc_data.first;
    tree_node_handle cluster5c = alloc_data.second;
    new (&(*cluster5cNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster5c, 0 );

	cluster5cNode->addPoint( Point(-15.0, -14.5) );
	cluster5cNode->addPoint( Point(-14.0, -14.5) );
	cluster5cNode->addPoint( Point(-12.5, -14.5) );
	cluster5cNode->addPoint( Point(-13.5, -15.5) );

    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    auto cluster5dNode = alloc_data.first;
    tree_node_handle cluster5d = alloc_data.second;
    new (&(*cluster5dNode)) DefaultLeafNodeType( &tree, tree_node_handle( nullptr
                ), cluster5d, 0 );
	cluster5dNode->addPoint( Point(-15.0, -15.0));
	cluster5dNode->addPoint( Point(-14.0, -15.0));
	cluster5dNode->addPoint( Point(-13.0, -15.0));
	cluster5dNode->addPoint( Point(-12.0, -15.0));
	cluster5dNode->addPoint( Point(-15.0, -15.0));

    alloc_branch_data =
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType( BRANCH_NODE ) );
    auto cluster5Node = alloc_branch_data.first;
    tree_node_handle cluster5 = alloc_branch_data.second;
    new (&(*cluster5Node)) DefaultBranchNodeType( &tree, root, cluster5,
            1 );

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
            tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        new (&(*alloc_root_data.first)) DefaultBranchNodeType( &tree,
                 tree_node_handle( nullptr ), alloc_root_data.second, 2 );
        root = alloc_root_data.second;
        auto rootNode= alloc_root_data.first;


        auto alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster4aNode = alloc_data.first;
        cluster4a = alloc_data.second;
        new (&(*cluster4aNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster4a, 0 );
        cluster4aNode->addPoint( Point(-10.0, -2.0) );
        cluster4aNode->addPoint( Point(-12.0, -3.0) );
        cluster4aNode->addPoint( Point(-11.0, -3.0) );
        cluster4aNode->addPoint( Point(-10.0, -3.0) );
        
        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster4bNode = alloc_data.first;
        cluster4b = alloc_data.second;
        new (&(*cluster4bNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster4b, 0 );

        cluster4bNode->addPoint( Point(-9.0, -3.0) );
        cluster4bNode->addPoint( Point(-7.0, -3.0) );
        cluster4bNode->addPoint( Point(-10.0, -5.0) );

        auto alloc_branch_data =
            tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        auto cluster4Node = alloc_branch_data.first;
        cluster4 = alloc_branch_data.second;
        new (&(*cluster4Node)) DefaultBranchNodeType( &tree, root,
                cluster4, 1);

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
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5aNode = alloc_data.first;
        cluster5a = alloc_data.second;
        new (&(*cluster5aNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster5a, 0 );
        cluster5aNode->addPoint( Point(-14.5, -13.0) );
        cluster5aNode->addPoint( Point(-14.0, -13.0) );
        cluster5aNode->addPoint( Point(-13.5, -13.5) );
        cluster5aNode->addPoint( Point(-15.0, -14.0) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5bNode = alloc_data.first;
        cluster5b = alloc_data.second;
        new (&(*cluster5bNode)) DefaultLeafNodeType( &tree,
                tree_node_handle(nullptr), cluster5b, 0 );
        cluster5bNode->addPoint( Point(-14.0, -14.0) );
        cluster5bNode->addPoint( Point(-13.0, -14.0) );
        cluster5bNode->addPoint( Point(-12.0, -14.0) );
        cluster5bNode->addPoint( Point(-13.5, -16.0) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5cNode = alloc_data.first;
        cluster5c = alloc_data.second;
        new (&(*cluster5cNode)) DefaultLeafNodeType( &tree,
                    tree_node_handle(nullptr), cluster5c, 0 );

        cluster5cNode->addPoint( Point(-15.0, -14.5) );
        cluster5cNode->addPoint( Point(-14.0, -14.5) );
        cluster5cNode->addPoint( Point(-12.5, -14.5) );
        cluster5cNode->addPoint( Point(-13.5, -15.5) );

        alloc_data =
            tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                    NodeHandleType( LEAF_NODE ) );
        auto cluster5dNode = alloc_data.first;
        cluster5d = alloc_data.second;
        new (&(*cluster5dNode)) DefaultLeafNodeType( &tree, tree_node_handle( nullptr
                    ), cluster5d, 0 );
        cluster5dNode->addPoint( Point(-15.0, -15.0));
        cluster5dNode->addPoint( Point(-14.0, -15.0));
        cluster5dNode->addPoint( Point(-13.0, -15.0));
        cluster5dNode->addPoint( Point(-12.0, -15.0));
        cluster5dNode->addPoint( Point(-15.0, -15.0));

        alloc_branch_data =
            tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                    NodeHandleType( BRANCH_NODE ) );
        auto cluster5Node = alloc_branch_data.first;
        cluster5 = alloc_branch_data.second;
        new (&(*cluster5Node)) DefaultBranchNodeType( &tree, root,
                cluster5, 0 );

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
            tree.node_allocator_->get_tree_node<DefaultBranchNodeType>(
                    tree.root );
        REQUIRE(rootNode->cur_offset_ == 2);
        REQUIRE( rootNode->level_ == 1 );

        nirtreedisk::Branch bLeft = rootNode->entries[0];
        nirtreedisk::Branch bRight = rootNode->entries[1];
        REQUIRE( bLeft.child.get_type() ==  LEAF_NODE );
        REQUIRE( bRight.child.get_type() ==  LEAF_NODE );

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
        unsigned insertion_count = max_branch_factor*max_branch_factor + 1;

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
        REQUIRE( root_node->level_ == 2 );

        nirtreedisk::Branch bLeft = root_node->entries[0];
        nirtreedisk::Branch bRight = root_node->entries[1];

        auto left = tree.get_branch_node( bLeft.child );
        auto right = tree.get_branch_node( bRight.child );

        REQUIRE(left->cur_offset_ == 4);
        REQUIRE(right->cur_offset_ == 4);
        REQUIRE( left->level_ == 1 );
        REQUIRE( right->level_ == 1 );
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
            sizeof(unsigned) + sizeof(Point) * 5 );

    tree_node_handle repacked_handle = root_leaf_node->repack(
            tree.node_allocator_.get() );

    REQUIRE( repacked_handle != nullptr );
    auto packed_leaf =
        tree.node_allocator_->get_tree_node<packed_node>(
                repacked_handle );
    REQUIRE( read_pointer_from_buffer<DefaulTreeType>( packed_leaf->buffer_ ) == &tree );
    REQUIRE( * (tree_node_handle *) (packed_leaf->buffer_ + sizeof(void
                    *) ) ==
        repacked_handle );

    REQUIRE( * (tree_node_handle *) (packed_leaf->buffer_ + sizeof(void
                    *) + sizeof(tree_node_handle) ) ==
        root_leaf_node->parent );

    REQUIRE( * (unsigned *) (packed_leaf->buffer_ + sizeof(void
                    *) + sizeof(tree_node_handle)*2 ) ==
        root_leaf_node->cur_offset_ );

    Point *p = (Point *) (packed_leaf->buffer_ + sizeof(void *) +
            sizeof(tree_node_handle)*2 + sizeof(unsigned));
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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second, 0 );

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
            tree.node_allocator_.get(), tree.node_allocator_.get() );
    REQUIRE( packed_handle != nullptr );

    auto packed_branch= tree.node_allocator_->get_tree_node<packed_node>(
            packed_handle );
    
    REQUIRE( read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            packed_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (unsigned *) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(unsigned);

    // There are 5 entries

    //Entry 1
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second, 0 );

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

    tree_node_handle packed_handle = branch_node->repack(
            tree.node_allocator_.get(), tree.node_allocator_.get() ); 

    auto packed_branch= tree.node_allocator_->get_tree_node<packed_node>(
            packed_handle );
    
    REQUIRE( read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            packed_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (unsigned*) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(unsigned);


    //Entry 1
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            leaf_handle );
    offset += sizeof(tree_node_handle);
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
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second, 0 );

    auto branch_node = alloc_branch_data.first;
    auto leaf_node = alloc_leaf_data.first;
    auto leaf_handle = alloc_leaf_data.second;

    IsotheticPolygon polygon( Rectangle(0.0, 0.0, 1.0, 1.0) );
    for( double i = 1.0; i < 30.0; i += 1.0 ) {
        polygon.basicRectangles.push_back( Rectangle( i, i, i+1.0,
                    i+1.0) );
    }
    polygon.recomputeBoundingBox();

    auto alloc_poly_data = tree.node_allocator_->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
            compute_sizeof_inline_unbounded_polygon(
                polygon.basicRectangles.size()
                 ), NodeHandleType(BIG_POLYGON));
    new (&(*alloc_poly_data.first)) InlineUnboundedIsotheticPolygon(
            tree.node_allocator_.get(), polygon.basicRectangles.size() );
    alloc_poly_data.first->push_polygon_to_disk( polygon );

    nirtreedisk::Branch b = createBranchEntry( alloc_poly_data.second, leaf_handle );
    branch_node->addBranchToNode( b );

    tree_node_handle packed_handle = branch_node->repack(
            tree.node_allocator_.get(), tree.node_allocator_.get() ); 

    auto packed_branch= tree.node_allocator_->get_tree_node<packed_node>(
            packed_handle );

    REQUIRE( read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            packed_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (unsigned*) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(unsigned);

    // Entry 1
    tree_node_handle child_handle = * (tree_node_handle *)
        (packed_branch->buffer_ + offset);
    REQUIRE( child_handle == leaf_handle ); 
    offset += sizeof( tree_node_handle );
    bool is_compressed =
        child_handle.get_associated_poly_is_compressed();
    REQUIRE( is_compressed == true );
    IsotheticPolygon decoded_poly =
        decompress_polygon( (packed_branch->buffer_ + offset) );
    REQUIRE( decoded_poly.basicRectangles.size() == 30 );
    for( unsigned i = 0; i < decoded_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decoded_poly.basicRectangles.at(i) == Rectangle( i, i,
                    i+1, i+1) );
    }
    unlink("nirdiskbacked.txt");
}

TEST_CASE("NIRTreeDisk: pack in a small out of band polygon") {
    unlink( "nirdiskbacked.txt" );
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    for( unsigned i = 0; i < 5; i++ ) {
        tree.insert( Point(i,i) );
    }

    auto alloc_branch_data =
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>();
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>();
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            alloc_branch_data.second, alloc_leaf_data.second, 0 );

    auto branch_node = alloc_branch_data.first;
    auto leaf_node = alloc_leaf_data.first;
    auto leaf_handle = alloc_leaf_data.second;

    // This is small enough to fit
    IsotheticPolygon polygon( Rectangle(0.0, 0.0, 1.0, 1.0) );
    for( double i = 1.0; i < 20.0; i += 1.0 ) {
        polygon.basicRectangles.push_back( Rectangle( i, i, i+1.0,
                    i+1.0) );
    }
    polygon.recomputeBoundingBox();

    auto alloc_poly_data = tree.node_allocator_->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
            compute_sizeof_inline_unbounded_polygon(
                polygon.basicRectangles.size()
                 ), NodeHandleType(BIG_POLYGON));
    new (&(*alloc_poly_data.first)) InlineUnboundedIsotheticPolygon(
            tree.node_allocator_.get(), polygon.basicRectangles.size() );
    alloc_poly_data.first->push_polygon_to_disk( polygon );

    nirtreedisk::Branch b = createBranchEntry( alloc_poly_data.second, leaf_handle );
    branch_node->addBranchToNode( b );

    // too big, should be out of line
    IsotheticPolygon polygon2( Rectangle(0.0, 0.0, 1.0, 1.0) );
    for( double i = 1.0; i < 30.0; i += 1.0 ) {
        polygon2.basicRectangles.push_back( Rectangle( i, i, i+1.0,
                    i+1.0) );
    }
    polygon2.recomputeBoundingBox();

    alloc_poly_data = tree.node_allocator_->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
            compute_sizeof_inline_unbounded_polygon(
                polygon2.basicRectangles.size()
                 ), NodeHandleType(BIG_POLYGON));
    new (&(*alloc_poly_data.first)) InlineUnboundedIsotheticPolygon(
            tree.node_allocator_.get(), polygon2.basicRectangles.size() );
    alloc_poly_data.first->push_polygon_to_disk( polygon2 );

    b = createBranchEntry( alloc_poly_data.second, leaf_handle );
    branch_node->addBranchToNode( b );

    tree_node_handle packed_handle = branch_node->repack(
            tree.node_allocator_.get(), tree.node_allocator_.get() ); 

    auto packed_branch= tree.node_allocator_->get_tree_node<packed_node>(
            packed_handle );

    REQUIRE( read_pointer_from_buffer<DefaulTreeType>(
                packed_branch->buffer_ ) == &tree );
    size_t offset = 8;
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            packed_handle );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (tree_node_handle *) (packed_branch->buffer_ + offset) ==
            branch_node->parent );
    offset += sizeof(tree_node_handle);
    REQUIRE( * (unsigned*) (packed_branch->buffer_ + offset) ==
            branch_node->cur_offset_ );
    offset += sizeof(unsigned);

    // Entry 1
    int new_offset = 0;
    tree_node_handle child_handle = * (tree_node_handle *)
        (packed_branch->buffer_ + offset);
    REQUIRE( child_handle == leaf_handle ); 
    REQUIRE( child_handle.get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    IsotheticPolygon child_poly = decompress_polygon(
            packed_branch->buffer_ + offset, &new_offset );
    REQUIRE( child_poly.basicRectangles.size() == 20 );
    offset += new_offset;

    child_handle = * (tree_node_handle *)
        (packed_branch->buffer_ + offset);
    REQUIRE( child_handle == leaf_handle ); 
    REQUIRE( child_handle.get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    child_poly = decompress_polygon( packed_branch->buffer_ + offset );
    REQUIRE( child_poly.basicRectangles.size() == 30 );
    unlink("nirdiskbacked.txt");
}



TEST_CASE("NIRTreeDisk: Search packed leaf from branch." ) {

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
            sizeof(unsigned) + sizeof(Point) * 5 );

    tree_node_handle repacked_handle = root_leaf_node->repack(
            tree.node_allocator_.get() );
    repacked_handle.set_type(
            NodeHandleType(REPACKED_LEAF_NODE) );

    REQUIRE( repacked_handle != nullptr );
    auto packed_leaf =
        tree.node_allocator_->get_tree_node<packed_node>(
                repacked_handle );

    auto alloc_branch_data =
        tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
                NodeHandleType(BRANCH_NODE));
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );

    auto branch_node = alloc_branch_data.first;

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(root_leaf_node->boundingBox()),
            tree.root );
    branch_node->addBranchToNode( b );

    for( int i = 0; i < 7; i++ ) {
        Point p( i, i );
        auto vec = point_search( branch_node->self_handle_, p, &tree );
        if( i < 5 ) {
            REQUIRE( vec.size() == 1 );
        } else {
            REQUIRE( vec.size() == 0 );
        }
    }
    unlink( "nirdiskbacked.txt");
}

TEST_CASE("NIRTreeDisk: Search packed leaf direct." ) {

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
            sizeof(unsigned) + sizeof(Point) * 5 );

    tree_node_handle repacked_handle = root_leaf_node->repack(
            tree.node_allocator_.get() );

    REQUIRE( repacked_handle != nullptr );
    auto packed_leaf =
        tree.node_allocator_->get_tree_node<packed_node>(
                repacked_handle );

    for( int i = 0; i < 7; i++ ) {
        Point p( i, i );
        auto vec = point_search(repacked_handle, p, &tree );
        if( i < 5 ) {
            REQUIRE( vec.size() == 1 );
        } else {
            REQUIRE( vec.size() == 0 );
        }
    }
    unlink( "nirdiskbacked.txt");
}

TEST_CASE( "NIRTreeDisk: Point search packed branch" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node1 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node2 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node3 = alloc_leaf_data.first;

    leaf_node1->addPoint( Point(1,1) );
    leaf_node1->addPoint( Point(2,2) );
    leaf_node1->addPoint( Point(3,3) );
    leaf_node1->addPoint( Point(4,4) );
    leaf_node1->addPoint( Point(5,5) );
    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node1->boundingBox()),
            leaf_node1->self_handle_ );
    branch_node->addBranchToNode( b );

    leaf_node2->addPoint( Point(6,6) );
    leaf_node2->addPoint( Point(7,7) );
    leaf_node2->addPoint( Point(8,8) );
    leaf_node2->addPoint( Point(9,9) );
    leaf_node2->addPoint( Point(10,10) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node2->boundingBox()),
            leaf_node2->self_handle_ );
    branch_node->addBranchToNode( b );

    leaf_node3->addPoint( Point(11,11) );
    leaf_node3->addPoint( Point(12,12) );
    leaf_node3->addPoint( Point(13,13) );
    leaf_node3->addPoint( Point(14,14) );
    leaf_node3->addPoint( Point(15,15) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node3->boundingBox()),
            leaf_node3->self_handle_ );
    branch_node->addBranchToNode( b );

    auto packed_branch_handle = branch_node->repack(
            tree.node_allocator_.get(), tree.node_allocator_.get() );

    for( unsigned i = 1; i < 20; i++ ) {
        Point p(i,i);
        if( i < 16 ) {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 1 );
        } else {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 0 );
        }
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: Point search packed branch multi-rect poly" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node1 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node2 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node3 = alloc_leaf_data.first;

    leaf_node1->addPoint( Point(1,1) );
    leaf_node1->addPoint( Point(2,2) );
    leaf_node1->addPoint( Point(3,3) );
    leaf_node1->addPoint( Point(4,4) );
    leaf_node1->addPoint( Point(5,5) );

    Rectangle rect1(1,1,nextafter(1,DBL_MAX),nextafter(5, DBL_MAX));
    Rectangle rect2(7,1,nextafter(20, DBL_MAX),nextafter(1,DBL_MAX));
    Rectangle rect3(-100,-100, -20, -20);
    Rectangle rect4(1,1,nextafter(5,DBL_MAX),nextafter(5,DBL_MAX));
    IsotheticPolygon polygon( rect1 );
    polygon.basicRectangles.push_back( rect2 );
    polygon.basicRectangles.push_back( rect3 );
    polygon.basicRectangles.push_back( rect4 );
    polygon.recomputeBoundingBox();

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node1->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( polygon );
    branch_node->addBranchToNode( b );

    leaf_node2->addPoint( Point(6,6) );
    leaf_node2->addPoint( Point(7,7) );
    leaf_node2->addPoint( Point(8,8) );
    leaf_node2->addPoint( Point(9,9) );
    leaf_node2->addPoint( Point(10,10) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node2->boundingBox()),
            leaf_node2->self_handle_ );
    branch_node->addBranchToNode( b );

    leaf_node3->addPoint( Point(11,11) );
    leaf_node3->addPoint( Point(12,12) );
    leaf_node3->addPoint( Point(13,13) );
    leaf_node3->addPoint( Point(14,14) );
    leaf_node3->addPoint( Point(15,15) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node3->boundingBox()),
            leaf_node3->self_handle_ );
    branch_node->addBranchToNode( b );

    auto packed_branch_handle = branch_node->repack(
            tree.node_allocator_.get(), tree.node_allocator_.get() );

    for( unsigned i = 1; i < 20; i++ ) {
        Point p(i,i);
        if( i < 16 ) {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 1 );
        } else {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 0 );
        }
    }

    unlink( "nirdiskbacked.txt" );
}


TEST_CASE( "NIRTreeDisk: Point search packed branch out of band poly" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node1 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node2 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node3 = alloc_leaf_data.first;

    leaf_node1->addPoint( Point(1,1) );
    leaf_node1->addPoint( Point(2,2) );
    leaf_node1->addPoint( Point(3,3) );
    leaf_node1->addPoint( Point(4,4) );
    leaf_node1->addPoint( Point(5,5) );


    IsotheticPolygon polygon( Rectangle(-100,-100,-50,-50) );
    for( int i = -50; i < 2; i++ ) {
        polygon.basicRectangles.push_back( Rectangle(i, i, i+6, i+6) );
    }
    polygon.recomputeBoundingBox();


    auto alloc_poly_data =
        tree.node_allocator_->create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                compute_sizeof_inline_unbounded_polygon(
                    polygon.basicRectangles.size() ),
                NodeHandleType(BIG_POLYGON) );
    new (&(*alloc_poly_data.first)) InlineUnboundedIsotheticPolygon(
            tree.node_allocator_.get(), polygon.basicRectangles.size() );
    alloc_poly_data.first->push_polygon_to_disk( polygon );
    

    nirtreedisk::Branch b = createBranchEntry(
            alloc_poly_data.second,
            leaf_node1->self_handle_ );
    branch_node->addBranchToNode( b );

    leaf_node2->addPoint( Point(6,6) );
    leaf_node2->addPoint( Point(7,7) );
    leaf_node2->addPoint( Point(8,8) );
    leaf_node2->addPoint( Point(9,9) );
    leaf_node2->addPoint( Point(10,10) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node2->boundingBox()),
            leaf_node2->self_handle_ );
    branch_node->addBranchToNode( b );

    leaf_node3->addPoint( Point(11,11) );
    leaf_node3->addPoint( Point(12,12) );
    leaf_node3->addPoint( Point(13,13) );
    leaf_node3->addPoint( Point(14,14) );
    leaf_node3->addPoint( Point(15,15) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node3->boundingBox()),
            leaf_node3->self_handle_ );
    branch_node->addBranchToNode( b );

    auto packed_branch_handle = branch_node->repack(
            tree.node_allocator_.get(), tree.node_allocator_.get() );

    /*
    for( unsigned i = 1; i < 20; i++ ) {
        Point p(i,i);
        if( i < 16 ) {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 1 );
        } else {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 0 );
        }
    }
    */

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: Repack subtree" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node1 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node2 = alloc_leaf_data.first;

    alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node3 = alloc_leaf_data.first;

    leaf_node1->addPoint( Point(1,1) );
    leaf_node1->addPoint( Point(2,2) );
    leaf_node1->addPoint( Point(3,3) );
    leaf_node1->addPoint( Point(4,4) );
    leaf_node1->addPoint( Point(5,5) );

    Rectangle rect1(1,1,nextafter(1,DBL_MAX),nextafter(5, DBL_MAX));
    Rectangle rect2(7,1,nextafter(20, DBL_MAX),nextafter(1,DBL_MAX));
    Rectangle rect3(-100,-100, -20, -20);
    Rectangle rect4(1,1,nextafter(5,DBL_MAX),nextafter(5,DBL_MAX));
    IsotheticPolygon polygon( rect1 );
    polygon.basicRectangles.push_back( rect2 );
    polygon.basicRectangles.push_back( rect3 );
    polygon.basicRectangles.push_back( rect4 );
    polygon.recomputeBoundingBox();

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node1->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( polygon );
    branch_node->addBranchToNode( b );

    leaf_node2->addPoint( Point(6,6) );
    leaf_node2->addPoint( Point(7,7) );
    leaf_node2->addPoint( Point(8,8) );
    leaf_node2->addPoint( Point(9,9) );
    leaf_node2->addPoint( Point(10,10) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node2->boundingBox()),
            leaf_node2->self_handle_ );
    branch_node->addBranchToNode( b );

    leaf_node3->addPoint( Point(11,11) );
    leaf_node3->addPoint( Point(12,12) );
    leaf_node3->addPoint( Point(13,13) );
    leaf_node3->addPoint( Point(14,14) );
    leaf_node3->addPoint( Point(15,15) );
    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(leaf_node3->boundingBox()),
            leaf_node3->self_handle_ );
    branch_node->addBranchToNode( b );

    auto packed_branch_handle =
        nirtreedisk::repack_subtree<3,7,nirtreedisk::LineMinimizeDownsplits>( branch_handle,
            tree.node_allocator_.get(), tree.node_allocator_.get() );

    for( unsigned i = 1; i < 20; i++ ) {
        Point p(i,i);
        if( i < 16 ) {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 1 );
        } else {
            REQUIRE( point_search( packed_branch_handle, p, &tree ).size() == 0 );
        }
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: Compress Simple Branch" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node = alloc_leaf_data.first;

    // Create highly compressible polygon
    IsotheticPolygon branch_polygon;
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon );
    branch_node->addBranchToNode( b );

    tree_node_allocator *allocator = tree.node_allocator_.get();

    auto compression_data = b.compute_compression_data( allocator );
    REQUIRE( compression_data.has_value() );
    REQUIRE( compression_data.value().second == 20 );

    IsotheticPolygon decomp_poly = decompress_polygon(
            compression_data.value().first );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon.basicRectangles.at(i) );
    }

    unlink( "nirdiskbacked.txt" );

}

TEST_CASE( "NIRTreeDisk: Compress/Repack Single Branch" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node = alloc_leaf_data.first;

    // Create highly compressible polygon
    IsotheticPolygon branch_polygon;
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon );
    branch_node->addBranchToNode( b );

    tree_node_allocator *allocator = tree.node_allocator_.get();
    tree_node_handle compressed_handle = branch_node->repack( allocator, allocator );

    auto compressed_branch = allocator->get_tree_node<packed_node>(
            compressed_handle );
    char *buffer = compressed_branch->buffer_;
    unsigned offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    unsigned count = * (unsigned *) (buffer +offset); \
    offset += sizeof( unsigned );

    REQUIRE( count == 1 );
    tree_node_handle *child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    IsotheticPolygon decomp_poly = decompress_polygon( buffer + offset );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon.basicRectangles.at(i) );
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: Compress/Repack Multiple-Branch" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node = alloc_leaf_data.first;

    // Create highly compressible polygon
    IsotheticPolygon branch_polygon;
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 2, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 2, 1, 3, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 3, 1, 4, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 4, 1, 1, 1 ) );

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon );
    branch_node->addBranchToNode( b );

    IsotheticPolygon branch_polygon2;
    branch_polygon2.basicRectangles.push_back( Rectangle( 5, 1, 1, 2 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle( 5, 2, 1, 3 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle( 5, 3, 1, 4 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle( 5, 4, 1, 1 ) );

    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon2 );
    branch_node->addBranchToNode( b );


    tree_node_allocator *allocator = tree.node_allocator_.get();
    tree_node_handle compressed_handle = branch_node->repack( allocator, allocator );

    auto compressed_branch = allocator->get_tree_node<packed_node>(
            compressed_handle );
    char *buffer = compressed_branch->buffer_;
    unsigned offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    unsigned count = * (unsigned *) (buffer +offset); \
    offset += sizeof( unsigned );

    REQUIRE( count == 2 );
    tree_node_handle *child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    int new_offset;
    IsotheticPolygon decomp_poly = decompress_polygon( buffer + offset,
            &new_offset );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon.basicRectangles.at(i) );
    }
    offset += new_offset;
    child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    decomp_poly = decompress_polygon( buffer + offset,
            &new_offset );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon2.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon2.basicRectangles.at(i) );
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: Some branches compressed" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node = alloc_leaf_data.first;

    // Create highly compressible polygon
    IsotheticPolygon branch_polygon;
    branch_polygon.basicRectangles.push_back( Rectangle( 5, 1, 1, 2 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 5, 2, 1, 3 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 5, 3, 1, 4 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 5, 4, 1, 1 ) );

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon );

    branch_node->addBranchToNode( b );

    IsotheticPolygon branch_polygon2;
    branch_polygon2.basicRectangles.push_back( Rectangle( 1.324234525342,
                -12.34925289, -354.95892761, 12.592089053 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle( 23.8954980323,
                2093.729, 98.6442, 43.942222 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle(
                17.293432908571, -31.2890808942, 1.980809808,
                4.2345982582 ) );

    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon2 );
    branch_node->addBranchToNode( b );

    tree_node_allocator *allocator = tree.node_allocator_.get();
    tree_node_handle compressed_handle = branch_node->repack( allocator, allocator );

    auto compressed_branch = allocator->get_tree_node<packed_node>(
            compressed_handle );
    char *buffer = compressed_branch->buffer_;
    unsigned offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    unsigned count = * (unsigned *) (buffer +offset); \
    offset += sizeof( unsigned );

    REQUIRE( count == 2 );
    tree_node_handle *child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    int new_offset;
    IsotheticPolygon decomp_poly = decompress_polygon( buffer + offset,
            &new_offset );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon.basicRectangles.at(i) );
    }

    offset += new_offset;
    child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == false );
    offset += sizeof( tree_node_handle );

    unsigned rect_count = * (unsigned *) (buffer+offset);
    REQUIRE( rect_count == branch_polygon2.basicRectangles.size() );
    offset += sizeof( unsigned );

    for( unsigned i = 0; i < rect_count; i++ ) {
        Rectangle *rect = (Rectangle *) (buffer + offset);
        REQUIRE( *rect == branch_polygon2.basicRectangles.at(i));
        offset += sizeof( Rectangle );
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: Compress/Repack weird alignment" ) {

    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );
    auto alloc_branch_data = tree.node_allocator_->create_new_tree_node<DefaultBranchNodeType>(
            NodeHandleType(BRANCH_NODE));
    auto branch_handle = alloc_branch_data.second;
    new (&(*alloc_branch_data.first)) DefaultBranchNodeType( &tree,
            tree_node_handle(nullptr), alloc_branch_data.second, 1 );
    auto branch_node = alloc_branch_data.first;

    auto alloc_leaf_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
            NodeHandleType(LEAF_NODE));
    new (&(*alloc_leaf_data.first)) DefaultLeafNodeType( &tree,
            branch_handle, alloc_leaf_data.second, 0 );
    auto leaf_node = alloc_leaf_data.first;

    // Create highly compressible polygon
    IsotheticPolygon branch_polygon;
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );
    branch_polygon.basicRectangles.push_back( Rectangle( 1, 1, 1, 1 ) );

    nirtreedisk::Branch b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon );
    branch_node->addBranchToNode( b );

    IsotheticPolygon branch_polygon2;
    branch_polygon2.basicRectangles.push_back( Rectangle( 2, 2, 2, 2 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle( 2, 2, 2, 2 ) );
    branch_polygon2.basicRectangles.push_back( Rectangle( 2, 2, 2, 2 ) );

    b = createBranchEntry(
            InlineBoundedIsotheticPolygon(),
            leaf_node->self_handle_ );
    std::get<InlineBoundedIsotheticPolygon>( b.boundingPoly
            ).push_polygon_to_disk( branch_polygon2 );
    branch_node->addBranchToNode( b );

    tree_node_allocator *allocator = tree.node_allocator_.get();
    tree_node_handle compressed_handle = branch_node->repack( allocator, allocator );

    auto compressed_branch = allocator->get_tree_node<packed_node>(
            compressed_handle );
    char *buffer = compressed_branch->buffer_;
    unsigned offset = sizeof(void *) + 2 * sizeof(tree_node_handle); \
    unsigned count = * (unsigned *) (buffer +offset); \
    offset += sizeof( unsigned );

    REQUIRE( count == 2 );
    tree_node_handle *child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == true );
    offset += sizeof( tree_node_handle );

    int new_offset;
    IsotheticPolygon decomp_poly = decompress_polygon( buffer + offset,
            &new_offset );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon.basicRectangles.at(i) );
    }

    offset += new_offset;
    child = (tree_node_handle *) (buffer + offset );
    REQUIRE( child->get_associated_poly_is_compressed() == true );
    offset += sizeof(tree_node_handle);
    decomp_poly = decompress_polygon( buffer + offset,
            &new_offset );
    REQUIRE( decomp_poly.basicRectangles.size() ==
            branch_polygon2.basicRectangles.size() );
    for( unsigned i = 0; i < decomp_poly.basicRectangles.size(); i++ ) {
        REQUIRE( decomp_poly.basicRectangles.at(i) ==
                branch_polygon2.basicRectangles.at(i) );
    }

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle NoIntersect" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    tree_node_handle a_handle = nullptr;
    tree_node_handle b_handle = nullptr;
    Rectangle a_rect( 1.0, 1.0, 3.0, 3.0 );
    Rectangle b_rect( 3.1, 3.1, 5.0, 5.0 );
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );
    REQUIRE( res.first.size() == 1 );
    REQUIRE( res.second.size() == 1 );
    REQUIRE( res.first.at(0) == a_rect );
    REQUIRE( res.second.at(0) == b_rect );

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle NoOwnerIntersect A Yields" ) {
    unlink( "nirdiskbacked.txt" );

    // Two rectanglesthat overlap, but there are no points in the
    // overlapping region
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,1) );
    a_leaf_node->addPoint( Point(2,3) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,0) );
    b_leaf_node->addPoint( Point(3,2) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );
    REQUIRE( res.first.size() == 2 );
    REQUIRE( res.second.size() == 1 );
    Rectangle decomp_a_first =
        Rectangle(0,1,1,nextafter(3.0, DBL_MAX));
    Rectangle decomp_a_second =
        Rectangle(1,nextafter(2,DBL_MAX),
                nextafter(2, DBL_MAX), nextafter(3.0, DBL_MAX));
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_first ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_second ) != res.first.end() );
    REQUIRE( res.second.at(0) == b_rect );

    IsotheticPolygon poly;
    poly.basicRectangles = res.first;
    poly.recomputeBoundingBox();
    REQUIRE( not poly.intersectsRectangle( res.second.at(0) ) );

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle NoOwnerIntersect B Yields" ) {
    unlink( "nirdiskbacked.txt" );

    // Two rectanglesthat overlap, but there are no points in the
    // overlapping region
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,1) );
    a_leaf_node->addPoint( Point(2,3) );
    a_leaf_node->addPoint( Point(1.5,1.5) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,0) );
    b_leaf_node->addPoint( Point(3,2) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );
    REQUIRE( res.first.size() == 1 );
    REQUIRE( res.second.size() == 2 );
    REQUIRE( res.first.at(0) == a_rect );

    Rectangle decomp_b_first =
        Rectangle(1,0,nextafter(2,DBL_MAX),1);
    Rectangle decomp_b_second =
        Rectangle(nextafter(2,DBL_MAX),0,nextafter(3,DBL_MAX),nextafter(2,DBL_MAX));

    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_first ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second ) != res.second.end() );

    IsotheticPolygon poly;
    poly.basicRectangles = res.second;
    poly.recomputeBoundingBox();
    REQUIRE( not poly.intersectsRectangle( res.first.at(0) ) );

    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle OwnershipIntersect but boxable" ) {
    unlink( "nirdiskbacked.txt" );

    // Two rectanglesthat overlap, but there are no points in the
    // overlapping region
    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,1) );
    a_leaf_node->addPoint( Point(2,3) );
    a_leaf_node->addPoint( Point(1.5,1.5) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,0) );
    b_leaf_node->addPoint( Point(1.7,1.7) );
    b_leaf_node->addPoint( Point(3,2) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );

    Rectangle decomp_a_first =
        Rectangle(0,1,1,nextafter(3.0, DBL_MAX));
    Rectangle decomp_a_second =
        Rectangle(1,nextafter(2,DBL_MAX),
                nextafter(2, DBL_MAX), nextafter(3.0, DBL_MAX));
    Rectangle decomp_a_third =
        Rectangle(1.5, 1.5, nextafter(1.5,DBL_MAX), nextafter(1.5,
                    DBL_MAX) );
    REQUIRE( res.first.size() == 3 );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_first ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_second ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_third ) != res.first.end() );

    REQUIRE( res.second.size() == 3 );
    Rectangle decomp_b_first =
        Rectangle(1,0,nextafter(2,DBL_MAX),1);
    Rectangle decomp_b_second =
        Rectangle(nextafter(2,DBL_MAX),0,nextafter(3,DBL_MAX),nextafter(2,DBL_MAX));
    Rectangle decomp_b_third=
        Rectangle(1.7, 1.7, nextafter(1.7, DBL_MAX), nextafter(1.7,
                    DBL_MAX));

    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_first ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_third ) != res.second.end() );
    IsotheticPolygon poly;
    poly.basicRectangles = res.first;
    poly.recomputeBoundingBox();

    IsotheticPolygon poly2;
    poly2.basicRectangles = res.second;
    poly2.recomputeBoundingBox();

    REQUIRE( poly.disjoint( poly2 ) );


    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle OwnershipIntersect space slice required" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,1) );
    a_leaf_node->addPoint( Point(2,3) );
    a_leaf_node->addPoint( Point(1.5,1.5) );
    a_leaf_node->addPoint( Point(1.7,1.7) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,0) );
    b_leaf_node->addPoint( Point(1.6,1.6) );
    b_leaf_node->addPoint( Point(3,2) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );

    Rectangle decomp_a_first =
        Rectangle(0,1,1,nextafter(3.0, DBL_MAX));
    Rectangle decomp_a_second =
        Rectangle(1,nextafter(2,DBL_MAX),
                nextafter(2, DBL_MAX), nextafter(3.0, DBL_MAX));

    Rectangle decomp_a_third =
        Rectangle(1.5, 1, nextafter(1.5,DBL_MAX), nextafter(2,DBL_MAX) );

    Rectangle decomp_a_fourth =
        Rectangle(1.7, 1, nextafter(1.7,DBL_MAX), nextafter(2,DBL_MAX) );

    REQUIRE( res.first.size() == 4 );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_first ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_second ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_third ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_fourth ) != res.first.end() );


    REQUIRE( res.second.size() == 3 );
    Rectangle decomp_b_first =
        Rectangle(1,0,nextafter(2,DBL_MAX),1);
    Rectangle decomp_b_second =
        Rectangle(nextafter(2,DBL_MAX),0,nextafter(3,DBL_MAX),nextafter(2,DBL_MAX));
    Rectangle decomp_b_third=
        Rectangle(1.6, 1, nextafter(1.6, DBL_MAX), nextafter(2,
                    DBL_MAX));

    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_first ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_third ) != res.second.end() );
    IsotheticPolygon poly;
    poly.basicRectangles = res.first;
    poly.recomputeBoundingBox();

    IsotheticPolygon poly2;
    poly2.basicRectangles = res.second;
    poly2.recomputeBoundingBox();

    REQUIRE( poly.disjoint( poly2 ) );


    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: MergeCmd Simple Tests" ) {
    std::vector<std::pair<Point,uint8_t>> points_with_ownership =
    { {Point(0,0), 0}, {Point(1,0), 0}, {Point(2,0), 1}, {Point(3,0),0}
        };

    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 1, 0, 0 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 2, 0, 0 ) ==
            nirtreedisk::STOP );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 3, 0, 1 ) ==
            nirtreedisk::STOP );

    points_with_ownership =
    { {Point(0,0), 0}, {Point(1,0), 1}, {Point(2,0), 1}, {Point(3,0),1} };
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 1, 0, 0 ) ==
            nirtreedisk::STOP );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 2, 0, 1 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 3, 0, 1 ) ==
            nirtreedisk::ADD );
}

TEST_CASE( "NIRTreeDisk: MergeCmd Vertical Tests" ) {
    std::vector<std::pair<Point,uint8_t>> points_with_ownership =
    { {Point(0,0), 0}, {Point(1,0), 0}, {Point(2,0), 1}, {Point(2,1),1},
        {Point(2,2),1}, {Point(3,0),0}, {Point(4,0),0} };

    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 1, 0, 0 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 2, 0, 0 ) ==
            nirtreedisk::STOP );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 3, 0, 1 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 4, 0, 1 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 5, 0, 1 ) ==
            nirtreedisk::STOP );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 6, 0, 0 ) ==
            nirtreedisk::ADD );

    points_with_ownership =
    { {Point(0,0), 0}, {Point(1,0), 0}, {Point(2,0), 1}, {Point(2,1),0},
        {Point(2,2),1}, {Point(3,0),0}, {Point(4,0),0} };

    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 1, 0, 0 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 2, 0, 0 ) ==
            nirtreedisk::CREATE_VERTICAL);
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 5, 0, 0 ) ==
            nirtreedisk::ADD );
    REQUIRE( nirtreedisk::get_merge_cmd( points_with_ownership, 5, 0, 0 ) ==
            nirtreedisk::ADD );

}

TEST_CASE( "NIRTreeDisk: DodgeRectangle OwnershipIntersect vertical slice required" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,1) );
    a_leaf_node->addPoint( Point(2,3) );
    a_leaf_node->addPoint( Point(1.5,1.5) );
    a_leaf_node->addPoint( Point(1.6,1.65) );
    a_leaf_node->addPoint( Point(1.7,1.7) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,0) );
    b_leaf_node->addPoint( Point(1.6,1.6) );
    b_leaf_node->addPoint( Point(1.6,1.7) );
    b_leaf_node->addPoint( Point(3,2) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );

    Rectangle decomp_a_first =
        Rectangle(0,1,1,nextafter(3.0, DBL_MAX));
    Rectangle decomp_a_second =
        Rectangle(1,nextafter(2,DBL_MAX),
                nextafter(2, DBL_MAX), nextafter(3.0, DBL_MAX));

    Rectangle decomp_a_third =
        Rectangle(1.5, 1, nextafter(1.5,DBL_MAX), nextafter(2,DBL_MAX) );

    Rectangle decomp_a_fourth =
        Rectangle(1.7, 1, nextafter(1.7,DBL_MAX), nextafter(2,DBL_MAX) );

    Rectangle decomp_a_fifth =
        Rectangle(1.6, 1.65, nextafter(1.6,DBL_MAX), nextafter(1.65,DBL_MAX) );

    REQUIRE( res.first.size() == 5 );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_first ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_second ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_third ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_fourth ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_fifth ) != res.first.end() );

    REQUIRE( res.second.size() == 4 );
    Rectangle decomp_b_first =
        Rectangle(1,0,nextafter(2,DBL_MAX),1);

    Rectangle decomp_b_second =
        Rectangle(nextafter(2,DBL_MAX),0,nextafter(3,DBL_MAX),nextafter(2,DBL_MAX));

    Rectangle decomp_b_third =
        Rectangle(1.6, 1.6, nextafter(1.6, DBL_MAX), nextafter(1.6,
                    DBL_MAX) );

    Rectangle decomp_b_fourth =
        Rectangle(1.6, 1.7, nextafter(1.6, DBL_MAX), nextafter(1.7,
                    DBL_MAX) );


    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_first ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_third ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_fourth ) != res.second.end() );

    IsotheticPolygon poly;
    poly.basicRectangles = res.first;
    poly.recomputeBoundingBox();

    IsotheticPolygon poly2;
    poly2.basicRectangles = res.second;
    poly2.recomputeBoundingBox();

    REQUIRE( poly.disjoint( poly2 ) );


    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle OwnershipIntersect vertical slice last el" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,0) );
    a_leaf_node->addPoint( Point(1.5,0.5) );
    a_leaf_node->addPoint( Point(2,1.8) );
    a_leaf_node->addPoint( Point(2,2) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,1) );
    b_leaf_node->addPoint( Point(1.6,1.6) );
    b_leaf_node->addPoint( Point(2,1.9) );
    b_leaf_node->addPoint( Point(3,1) );
    b_leaf_node->addPoint( Point(3,3) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );

    std::cout << "Rects." << std::endl;
    for( Rectangle r : res.first ) {
        std::cout << r << std::endl;
    }

    REQUIRE( res.first.size() == 4 );
    Rectangle decomp_a_first( 0.0, 0.0, 1.0, nextafter(2.0,DBL_MAX) );
    Rectangle decomp_a_second( 1.0, 0.0, nextafter(2.0,DBL_MAX), 1.0 );
    Rectangle decomp_a_third( 2.0, 2.0, nextafter(2.0,DBL_MAX),
            nextafter(2.0,DBL_MAX) );
    Rectangle decomp_a_fourth( 2.0, 1.8, nextafter(2.0,DBL_MAX),
            nextafter(1.8,DBL_MAX) );


    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_first ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_second ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_third ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_fourth ) != res.first.end() );

    REQUIRE( res.second.size() == 4 );
    Rectangle decomp_b_first( 1.0, nextafter(2.0,DBL_MAX), nextafter(2.0, DBL_MAX), nextafter(3.0,DBL_MAX) );
    Rectangle decomp_b_second( nextafter(2.0, DBL_MAX), 1, nextafter(3.0, DBL_MAX), nextafter(3.0,DBL_MAX) );

    Rectangle decomp_b_third( 1.0, 1.0, nextafter(1.6, DBL_MAX),
            nextafter(2.0,DBL_MAX) );
    Rectangle decomp_b_fourth( 2.0, 1.9, nextafter(2.0, DBL_MAX), nextafter(1.9,DBL_MAX) );

    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_third ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_fourth ) != res.second.end() );

    IsotheticPolygon poly;
    poly.basicRectangles = res.first;
    poly.recomputeBoundingBox();

    IsotheticPolygon poly2;
    poly2.basicRectangles = res.second;
    poly2.recomputeBoundingBox();

    REQUIRE( poly.disjoint( poly2 ) );


    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk: DodgeRectangle Bug1" ) {
    unlink( "nirdiskbacked.txt" );

    DefaulTreeType tree( 4096*5, "nirdiskbacked.txt" );

    auto alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle a_handle = alloc_data.second;
    auto a_leaf_node = alloc_data.first;
    new (&(*a_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            a_handle, 0 );
    a_leaf_node->addPoint( Point(0,0) );
    a_leaf_node->addPoint( Point(1.5,0.5) );
    a_leaf_node->addPoint( Point(2,1.8) );
    a_leaf_node->addPoint( Point(2,2) );
    alloc_data =
        tree.node_allocator_->create_new_tree_node<DefaultLeafNodeType>(
                NodeHandleType( LEAF_NODE ) );
    tree_node_handle b_handle = alloc_data.second;
    auto b_leaf_node = alloc_data.first;
    new (&(*b_leaf_node)) DefaultLeafNodeType( &tree, tree_node_handle(nullptr),
            b_handle, 0 );
    b_leaf_node->addPoint( Point(1,1) );
    b_leaf_node->addPoint( Point(1.6,1.6) );
    b_leaf_node->addPoint( Point(2,1.9) );
    b_leaf_node->addPoint( Point(3,1) );
    b_leaf_node->addPoint( Point(3,3) );

    Rectangle a_rect = a_leaf_node->boundingBox();
    Rectangle b_rect = b_leaf_node->boundingBox();
    auto res = nirtreedisk::make_rectangles_disjoint_accounting_for_region_ownership(
        &tree, a_rect, a_handle, b_rect, b_handle );

    REQUIRE( res.first.size() == 4 );
    Rectangle decomp_a_first( 0.0, 0.0, 1.0, nextafter(2.0,DBL_MAX) );
    Rectangle decomp_a_second( 1.0, 0.0, nextafter(2.0,DBL_MAX), 1.0 );
    Rectangle decomp_a_third( 2.0, 2.0, nextafter(2.0,DBL_MAX),
            nextafter(2.0,DBL_MAX) );
    Rectangle decomp_a_fourth( 2.0, 1.8, nextafter(2.0,DBL_MAX),
            nextafter(1.8,DBL_MAX) );


    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_first ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_second ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_third ) != res.first.end() );
    REQUIRE( std::find( res.first.begin(), res.first.end(),
                decomp_a_fourth ) != res.first.end() );

    REQUIRE( res.second.size() == 4 );
    Rectangle decomp_b_first( 1.0, nextafter(2.0,DBL_MAX), nextafter(2.0, DBL_MAX), nextafter(3.0,DBL_MAX) );
    Rectangle decomp_b_second( nextafter(2.0, DBL_MAX), 1, nextafter(3.0, DBL_MAX), nextafter(3.0,DBL_MAX) );

    Rectangle decomp_b_third( 1.0, 1.0, nextafter(1.6, DBL_MAX),
            nextafter(2.0,DBL_MAX) );
    Rectangle decomp_b_fourth( 2.0, 1.9, nextafter(2.0, DBL_MAX), nextafter(1.9,DBL_MAX) );

    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_second ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_third ) != res.second.end() );
    REQUIRE( std::find( res.second.begin(), res.second.end(),
                decomp_b_fourth ) != res.second.end() );

    IsotheticPolygon poly;
    poly.basicRectangles = res.first;
    poly.recomputeBoundingBox();

    IsotheticPolygon poly2;
    poly2.basicRectangles = res.second;
    poly2.recomputeBoundingBox();

    REQUIRE( poly.disjoint( poly2 ) );


    unlink( "nirdiskbacked.txt" );
}

TEST_CASE( "NIRTreeDisk:: Turbo Bug" ) {
    std::vector<Point> a_owned_points;
    a_owned_points.push_back( Point(-122.50269699999999773,
                37.535702499999999304) );
    a_owned_points.push_back( Point(-122.4957380000000029,
                37.535789000000001181) );
    a_owned_points.push_back( Point(-122.5146259999999927,
                37.53582500000000266) );
    a_owned_points.push_back( Point(-122.50148799999999483,
                37.535888999999997395) );
    a_owned_points.push_back( Point(-122.50228799999999296,
                37.535938999999999055) );
    a_owned_points.push_back( Point(-122.50336350000000607,
                37.535968999999994367) );
    a_owned_points.push_back( Point(-122.50013799999999264,
                37.535989000000000715) );
    a_owned_points.push_back( Point(-122.49648799999999937,
                37.536039000000002375) );
    a_owned_points.push_back( Point(-122.49933799999999451,
                37.536189000000000249) );
    a_owned_points.push_back( Point(-122.51568900000000895,
                37.536338999999998123) );
    a_owned_points.push_back( Point(-122.49783800000000156,
                37.536489000000003102) );
    a_owned_points.push_back( Point(-122.50098800000000665,
                37.536639000000000976) );
    a_owned_points.push_back( Point(-122.50213800000000219,
                37.536688999999995531) );
    a_owned_points.push_back( Point(-122.5000880000000052,
                37.536738999999997191) );
    a_owned_points.push_back( Point(-122.51538899999999899,
                37.53678899999999885) );
    a_owned_points.push_back( Point(-122.51693900000000781,
                37.53688900000000217) );
    a_owned_points.push_back( Point(-122.49868800000000135,
                37.53698900000000549) );
    a_owned_points.push_back( Point(-122.50288799999999867,
                37.537089000000001704) );
    a_owned_points.push_back( Point(-122.50108799999999576,
                37.537138999999996258) );
    a_owned_points.push_back( Point(-122.51708899999999858,
                37.537238999999999578) );
    a_owned_points.push_back( Point(-122.5136889999999994,
                37.537289000000001238) );
    a_owned_points.push_back( Point(-122.50408799999999587,
                37.537339000000002898) );
    a_owned_points.push_back( Point(-122.51488900000001081,
                37.537388999999997452) );
    a_owned_points.push_back( Point(-122.49178799999999967,
                37.533388999999999669) );
    a_owned_points.push_back( Point(-122.49328800000000683,
                37.533788999999998737) );
    a_owned_points.push_back( Point(-122.50643850000000157,
                37.533788999999998737) );
    a_owned_points.push_back( Point(-122.50672649999999919,
                37.533952999999996791) );
    a_owned_points.push_back( Point(-122.49443800000000238,
                37.534088999999994485) );
    a_owned_points.push_back( Point(-122.49528799999998796,
                37.534289000000001124) );
    a_owned_points.push_back( Point(-122.49608800000000031,
                37.534489000000000658) );
    a_owned_points.push_back( Point(-122.50141949999999724,
                37.534641000000000588) );
    a_owned_points.push_back( Point(-122.50038800000000094,
                37.534689000000000192) );
    a_owned_points.push_back( Point(-122.49688799999999844,
                37.534689000000000192) );
    a_owned_points.push_back( Point(-122.50533799999999474,
                37.534739000000001852) );
    a_owned_points.push_back( Point(-122.49333799999999428,
                37.534789000000003512) );
    a_owned_points.push_back( Point(-122.4977879999999999,
                37.534838999999998066) );
    a_owned_points.push_back( Point(-122.50600299999999265,
                37.535061499999997636) );
    a_owned_points.push_back( Point(-122.49423799999999574,
                37.535189000000002579) );
    a_owned_points.push_back( Point(-122.5008694999999932,
                37.53524099999999919) );
    a_owned_points.push_back( Point(-122.50426200000001131,
                37.535263000000000488) );
    a_owned_points.push_back( Point(-122.503421000000003,
                37.535476500000001465) );
    a_owned_points.push_back( Point(-122.49498800000000642,
                37.535488999999998327) );
    a_owned_points.push_back( Point(-122.50048799999999005,
                37.535589000000001647) );
    a_owned_points.push_back( Point(-122.50516500000000519,
                37.535597499999994398) );
    a_owned_points.push_back( Point(-122.5000880000000052,
                37.535639000000003307) );
    a_owned_points.push_back( Point(-122.50983899999999949,
                37.515590000000003101) );
    a_owned_points.push_back( Point(-122.5091329999999914,
                37.515923000000000798) );
    a_owned_points.push_back( Point(-122.51118900000000167,
                37.516089999999998383) );
    a_owned_points.push_back( Point(-122.51168899999998985,
                37.516189999999994598) );
    a_owned_points.push_back( Point(-122.5105389999999943,
                37.516189999999994598) );
    a_owned_points.push_back( Point(-122.49843849999999179,
                37.516490000000004557) );
    a_owned_points.push_back( Point(-122.50998899999999026,
                37.516589999999993665) );
    a_owned_points.push_back( Point(-122.5107890000000026,
                37.516840000000001965) );
    a_owned_points.push_back( Point(-122.51173900000000572,
                37.516989999999999839) );
    a_owned_points.push_back( Point(-122.51113900000000001,
                37.517189999999999372) );
    a_owned_points.push_back( Point(-122.51188899999999649,
                37.517240000000001032) );
    a_owned_points.push_back( Point(-122.51138900000000831,
                37.5176400000000001) );
    a_owned_points.push_back( Point(-122.51203900000000147,
                37.518140000000002487) );
    a_owned_points.push_back( Point(-122.51138900000000831,
                37.518540000000001555) );
    a_owned_points.push_back( Point(-122.51273399999999469,
                37.518591499999999428) );
    a_owned_points.push_back( Point(-122.51238900000001308,
                37.518789999999995644) );
    a_owned_points.push_back( Point(-122.51188899999999649,
                37.519040000000003943) );
    a_owned_points.push_back( Point(-122.51266599999999585,
                37.519065499999996405) );
    a_owned_points.push_back( Point(-122.51296600000000581,
                37.519215500000001384) );
    a_owned_points.push_back( Point(-122.51003900000000613,
                37.519339999999999691) );
    a_owned_points.push_back( Point(-122.51123900000000333,
                37.51944000000000301) );
    a_owned_points.push_back( Point(-122.51233899999999721,
                37.51944000000000301) );
    a_owned_points.push_back( Point(-122.51183900000000904,
                37.519689999999997099) );
    a_owned_points.push_back( Point(-122.51298900000000458,
                37.519739999999998759) );
    a_owned_points.push_back( Point(-122.51143899999999576,
                37.519890000000003738) );
    a_owned_points.push_back( Point(-122.50943900000000042,
                37.513640000000002317) );
    a_owned_points.push_back( Point(-122.51093900000000758,
                37.513689999999996871) );
    a_owned_points.push_back( Point(-122.51028900000000021,
                37.513689999999996871) );
    a_owned_points.push_back( Point(-122.50883899999999471,
                37.51394000000000517) );
    a_owned_points.push_back( Point(-122.50988900000000115,
                37.514039999999994279) );
    a_owned_points.push_back( Point(-122.51048900000000685,
                37.514039999999994279) );
    a_owned_points.push_back( Point(-122.51098899999999503,
                37.514139999999997599) );
    a_owned_points.push_back( Point(-122.50933900000001131,
                37.514189999999999259) );
    a_owned_points.push_back( Point(-122.50878900000000726,
                37.514290000000002578) );
    a_owned_points.push_back( Point(-122.51123900000000333,
                37.514340000000004238) );
    a_owned_points.push_back( Point(-122.50944150000000832,
                37.514465999999998758) );
    a_owned_points.push_back( Point(-122.51058900000001017,
                37.514489999999995007) );
    a_owned_points.push_back( Point(-122.50868900000000394,
                37.514740000000003306) );
    a_owned_points.push_back( Point(-122.5090914999999967,
                37.514766000000001611) );
    a_owned_points.push_back( Point(-122.51068899999999928,
                37.51483999999999952) );
    a_owned_points.push_back( Point(-122.50828899999999066,
                37.51483999999999952) );
    a_owned_points.push_back( Point(-122.51113900000000001,
                37.51489000000000118) );
    a_owned_points.push_back( Point(-122.50788900000000581,
                37.514989999999997394) );
    a_owned_points.push_back( Point(-122.50993900000000281,
                37.514989999999997394) );
    a_owned_points.push_back( Point(-122.50918899999999212,
                37.515140000000002374) );
    a_owned_points.push_back( Point(-122.51138900000000831,
                37.515190000000004034) );
    a_owned_points.push_back( Point(-122.50513899999999978,
                37.515290000000000248) );
    a_owned_points.push_back( Point(-122.51058899999999596,
                37.515439999999998122) );
    a_owned_points.push_back( Point(-122.5085329999999999,
                37.51547300000000007) );
    a_owned_points.push_back( Point(-122.50923299999999472,
                37.51557300000000339) );
    a_owned_points.push_back( Point(-122.50003050000000826,
                37.507034000000004426) );
    a_owned_points.push_back( Point(-122.4999390000000119,
                37.507541000000003351) );
    a_owned_points.push_back( Point(-122.49833899999998721,
                37.507590499999999167) );
    a_owned_points.push_back( Point(-122.4990889999999979,
                37.507690499999995382) );
    a_owned_points.push_back( Point(-122.50028900000000931,
                37.508091000000000292) );
    a_owned_points.push_back( Point(-122.49828899999999976,
                37.508240000000000691) );
    a_owned_points.push_back( Point(-122.49713900000000422,
                37.508440000000000225) );
    a_owned_points.push_back( Point(-122.49838900000000308,
                37.508440000000000225) );
    a_owned_points.push_back( Point(-122.5005390000000034,
                37.508441000000004806) );
    a_owned_points.push_back( Point(-122.49948900000001117,
                37.508540499999995177) );
    a_owned_points.push_back( Point(-122.49808899999999312,
                37.508890000000000953) );
    a_owned_points.push_back( Point(-122.49853899999999385,
                37.508989999999997167) );
    a_owned_points.push_back( Point(-122.49973900000000526,
                37.508990499999995905) );
    a_owned_points.push_back( Point(-122.50023899999999344,
                37.509140500000000884) );
    a_owned_points.push_back( Point(-122.49893900000000713,
                37.509439999999997895) );
    a_owned_points.push_back( Point(-122.4994389999999953,
                37.509689999999999088) );
    a_owned_points.push_back( Point(-122.4996890000000036,
                37.510590000000000543) );
    a_owned_points.push_back( Point(-122.49123799999999562,
                37.511790000000004852) );
    a_owned_points.push_back( Point(-122.51038900000000353,
                37.512689999999999202) );
    a_owned_points.push_back( Point(-122.50998899999999026,
                37.512740000000000862) );
    a_owned_points.push_back( Point(-122.50993900000000281,
                37.513090000000005375) );
    a_owned_points.push_back( Point(-122.50963899999999285,
                37.513090000000005375) );
    a_owned_points.push_back( Point(-122.51038900000000353,
                37.513190000000001589) );
    a_owned_points.push_back( Point(-122.51008899999999358,
                37.513440000000002783) );
    a_owned_points.push_back( Point(-122.5105389999999943,
                37.513540000000006103) );
    a_owned_points.push_back( Point(-122.47888800000001197,
                37.50929000000000002) );
    a_owned_points.push_back( Point(-122.48108799999999974,
                37.509339999999994575) );
    a_owned_points.push_back( Point(-122.47383800000000065,
                37.50934000000000168) );
    a_owned_points.push_back( Point(-122.47508799999999951,
                37.50934000000000168) );
    a_owned_points.push_back( Point(-122.48008799999999496,
                37.509439999999997895) );
    a_owned_points.push_back( Point(-122.48568800000001033,
                37.509540000000001214) );
    a_owned_points.push_back( Point(-122.4864379999999926,
                37.509540000000001214) );
    a_owned_points.push_back( Point(-122.48313799999999674,
                37.509590000000002874) );
    a_owned_points.push_back( Point(-122.48498800000000131,
                37.509639999999997428) );
    a_owned_points.push_back( Point(-122.48713799999998741,
                37.509639999999997428) );
    a_owned_points.push_back( Point(-122.48758799999998814,
                37.509639999999997428) );
    a_owned_points.push_back( Point(-122.48243800000000192,
                37.509739999999993643) );
    a_owned_points.push_back( Point(-122.47743800000000647,
                37.509740000000000748) );
    a_owned_points.push_back( Point(-122.47643800000000169,
                37.509740000000000748) );
    a_owned_points.push_back( Point(-122.48818799999999385,
                37.509789999999995302) );
    a_owned_points.push_back( Point(-122.47878249999999412,
                37.509793000000001939) );
    a_owned_points.push_back( Point(-122.47483800000000542,
                37.509839999999996962) );
    a_owned_points.push_back( Point(-122.47438800000000469,
                37.509940000000000282) );
    a_owned_points.push_back( Point(-122.47593799999999931,
                37.510090000000005261) );
    a_owned_points.push_back( Point(-122.48238800000000026,
                37.51018999999999437) );
    a_owned_points.push_back( Point(-122.48508799999999042,
                37.51018999999999437) );
    a_owned_points.push_back( Point(-122.47453799999999546,
                37.510190000000001476) );
    a_owned_points.push_back( Point(-122.48373800000000244,
                37.51023999999999603) );
    a_owned_points.push_back( Point(-122.47953800000000513,
                37.51023999999999603) );
    a_owned_points.push_back( Point(-122.48153800000000047,
                37.51028999999999769) );
    a_owned_points.push_back( Point(-122.47453799999999546,
                37.508240000000000691) );
    a_owned_points.push_back( Point(-122.48468800000000556,
                37.508240000000000691) );
    a_owned_points.push_back( Point(-122.47343799999998737,
                37.508290000000002351) );
    a_owned_points.push_back( Point(-122.47053750000000605,
                37.508440000000000225) );
    a_owned_points.push_back( Point(-122.48108799999999974,
                37.508440000000000225) );
    a_owned_points.push_back( Point(-122.48158800000000213,
                37.508440000000000225) );
    a_owned_points.push_back( Point(-122.48383799999999155,
                37.508490000000001885) );
    a_owned_points.push_back( Point(-122.47198799999999608,
                37.508539999999996439) );
    a_owned_points.push_back( Point(-122.47773799999998801,
                37.508539999999996439) );
    a_owned_points.push_back( Point(-122.47518800000000283,
                37.508589999999998099) );
    a_owned_points.push_back( Point(-122.48263800000000856,
                37.508589999999998099) );
    a_owned_points.push_back( Point(-122.47393800000000397,
                37.508690000000001419) );
    a_owned_points.push_back( Point(-122.47118799999999794,
                37.508740000000003079) );
    a_owned_points.push_back( Point(-122.4866880000000009,
                37.508789999999997633) );
    a_owned_points.push_back( Point(-122.48528799999999706,
                37.508789999999997633) );
    a_owned_points.push_back( Point(-122.48398799999999653,
                37.508890000000000953) );
    a_owned_points.push_back( Point(-122.47583799999999599,
                37.508890000000000953) );
    a_owned_points.push_back( Point(-122.48333800000000338,
                37.508940000000002613) );
    a_owned_points.push_back( Point(-122.47248799999999846,
                37.508940000000002613) );
    a_owned_points.push_back( Point(-122.47688800000000242,
                37.508989999999997167) );
    a_owned_points.push_back( Point(-122.47443799999999214,
                37.508989999999997167) );
    a_owned_points.push_back( Point(-122.47968800000001011,
                37.509140000000002146) );
    a_owned_points.push_back( Point(-122.48158800000000213,
                37.509140000000002146) );
    a_owned_points.push_back( Point(-122.48208800000000451,
                37.509140000000002146) );
    a_owned_points.push_back( Point(-122.47543799999999692,
                37.509240000000005466) );
    a_owned_points.push_back( Point(-122.47653800000000501,
                37.507390000000000896) );
    a_owned_points.push_back( Point(-122.47043750000000273,
                37.507390000000000896) );
    a_owned_points.push_back( Point(-122.4776379999999989,
                37.507440000000002556) );
    a_owned_points.push_back( Point(-122.47998800000000585,
                37.50753999999999877) );
    a_owned_points.push_back( Point(-122.4693369999999959,
                37.507540000000005875) );
    a_owned_points.push_back( Point(-122.48513800000000629,
                37.50764000000000209) );
    a_owned_points.push_back( Point(-122.48398799999999653,
                37.507689999999996644) );
    a_owned_points.push_back( Point(-122.47108749999999588,
                37.507689999999996644) );
    a_owned_points.push_back( Point(-122.48568800000001033,
                37.507689999999996644) );
    a_owned_points.push_back( Point(-122.47453799999999546,
                37.507739999999998304) );
    a_owned_points.push_back( Point(-122.47343799999998737,
                37.507789999999999964) );
    a_owned_points.push_back( Point(-122.48108799999999974,
                37.507789999999999964) );
    a_owned_points.push_back( Point(-122.47538799999999526,
                37.507840000000001623) );
    a_owned_points.push_back( Point(-122.48313799999999674,
                37.507940000000004943) );
    a_owned_points.push_back( Point(-122.4878879999999981,
                37.507940000000004943) );
    a_owned_points.push_back( Point(-122.48208800000000451,
                37.507989999999999498) );
    a_owned_points.push_back( Point(-122.47993800000000419,
                37.507989999999999498) );
    a_owned_points.push_back( Point(-122.47158799999999701,
                37.508040000000001157) );
    a_owned_points.push_back( Point(-122.47003699999999071,
                37.508089999999995712) );
    a_owned_points.push_back( Point(-122.47813800000000128,
                37.508139999999997372) );
    a_owned_points.push_back( Point(-122.47913800000000606,
                37.508139999999997372) );
    a_owned_points.push_back( Point(-122.47603799999998841,
                37.508139999999997372) );
    a_owned_points.push_back( Point(-122.46923699999999258,
                37.508139999999997372) );
    a_owned_points.push_back( Point(-122.48053799999999569,
                37.508189999999999031) );
    a_owned_points.push_back( Point(-122.47713799999999651,
                37.508189999999999031) );
    a_owned_points.push_back( Point(-122.48099299999999801,
                37.50643800000000283) );
    a_owned_points.push_back( Point(-122.48203800000000285,
                37.506489999999999441) );
    a_owned_points.push_back( Point(-122.47420250000000408,
                37.506509499999999946) );
    a_owned_points.push_back( Point(-122.48433800000000815,
                37.5065400000000011) );
    a_owned_points.push_back( Point(-122.4831879999999984,
                37.50659000000000276) );
    a_owned_points.push_back( Point(-122.47908799999999019,
                37.50664000000000442) );
    a_owned_points.push_back( Point(-122.47380250000000501,
                37.50665949999999782) );
    a_owned_points.push_back( Point(-122.47803799999999796,
                37.506689999999998975) );
    a_owned_points.push_back( Point(-122.47703799999999319,
                37.506740000000000634) );
    a_owned_points.push_back( Point(-122.47578799999999433,
                37.506740000000000634) );
    a_owned_points.push_back( Point(-122.47466299999999251,
                37.506920999999998401) );
    a_owned_points.push_back( Point(-122.47313800000000583,
                37.506940000000000168) );
    a_owned_points.push_back( Point(-122.47113799999999628,
                37.506940000000000168) );
    a_owned_points.push_back( Point(-122.47963799999999424,
                37.506940000000000168) );
    a_owned_points.push_back( Point(-122.47993800000000419,
                37.507089999999998042) );
    a_owned_points.push_back( Point(-122.47383800000000065,
                37.507090000000005148) );
    a_owned_points.push_back( Point(-122.47968800000001011,
                37.507190000000001362) );
    a_owned_points.push_back( Point(-122.47178800000000365,
                37.507239999999995916) );
    a_owned_points.push_back( Point(-122.48423800000000483,
                37.507289999999997576) );
    a_owned_points.push_back( Point(-122.4723380000000077,
                37.507339999999999236) );
    a_owned_points.push_back( Point(-122.48158800000000213,
                37.507339999999999236) );
    a_owned_points.push_back( Point(-122.48053799999999569,
                37.507339999999999236) );
    a_owned_points.push_back( Point(-122.48258799999999269,
                37.507339999999999236) );
    a_owned_points.push_back( Point(-122.47528800000000615,
                37.507390000000000896) );
    a_owned_points.push_back( Point(-122.47868800000000533,
                37.507390000000000896) );
    a_owned_points.push_back( Point(-122.46708699999999226,
                37.50478999999999985) );
    a_owned_points.push_back( Point(-122.46508149999999659,
                37.504815499999999417) );
    a_owned_points.push_back( Point(-122.46853699999999776,
                37.505040000000001044) );
    a_owned_points.push_back( Point(-122.46473399999999288,
                37.505063499999998555) );
    a_owned_points.push_back( Point(-122.46573399999999765,
                37.505063499999998555) );
    a_owned_points.push_back( Point(-122.46408700000000636,
                37.505189999999998918) );
    a_owned_points.push_back( Point(-122.46673699999999485,
                37.505389999999998452) );
    a_owned_points.push_back( Point(-122.46838700000000699,
                37.505589999999997985) );
    a_owned_points.push_back( Point(-122.46768699999999797,
                37.505690000000001305) );
    a_owned_points.push_back( Point(-122.46653700000000242,
                37.505740000000002965) );
    a_owned_points.push_back( Point(-122.4640370000000047,
                37.506039999999998713) );
    a_owned_points.push_back( Point(-122.46758700000000886,
                37.506289999999999907) );
    a_owned_points.push_back( Point(-122.46853699999999776,
                37.506340000000001567) );
    a_owned_points.push_back( Point(-122.46738700000000222,
                37.50664000000000442) );
    a_owned_points.push_back( Point(-122.46603700000000003,
                37.506790000000002294) );
    a_owned_points.push_back( Point(-122.4646370000000104,
                37.506839999999996849) );
    a_owned_points.push_back( Point(-122.4681370000000129,
                37.506889999999998508) );
    a_owned_points.push_back( Point(-122.46703700000000481,
                37.506990000000001828) );
    a_owned_points.push_back( Point(-122.46843699999999444,
                37.507140000000006808) );
    a_owned_points.push_back( Point(-122.46563700000000097,
                37.507240000000003022) );
    a_owned_points.push_back( Point(-122.4687370000000044,
                37.507339999999999236) );
    a_owned_points.push_back( Point(-122.46663699999999153,
                37.507440000000002556) );
    a_owned_points.push_back( Point(-122.46878699999999185,
                37.507689999999996644) );
    a_owned_points.push_back( Point(-122.46753699999999299,
                37.508040000000001157) );
    a_owned_points.push_back( Point(-122.4664369999999991,
                37.508240000000000691) );
    a_owned_points.push_back( Point(-122.47018800000000738,
                37.505340000000003897) );
    a_owned_points.push_back( Point(-122.48083800000000565,
                37.505340000000003897) );
    a_owned_points.push_back( Point(-122.48683800000000588,
                37.505340000000003897) );
    a_owned_points.push_back( Point(-122.48089299999999469,
                37.505487999999999715) );
    a_owned_points.push_back( Point(-122.47138799999999037,
                37.505489999999994666) );
    a_owned_points.push_back( Point(-122.47263799999998923,
                37.505490000000001771) );
    a_owned_points.push_back( Point(-122.48143799999999715,
                37.505539999999996326) );
    a_owned_points.push_back( Point(-122.47378799999999899,
                37.505589999999997985) );
    a_owned_points.push_back( Point(-122.4732879999999966,
                37.505589999999997985) );
    a_owned_points.push_back( Point(-122.4693875000000105,
                37.505639999999999645) );
    a_owned_points.push_back( Point(-122.47063800000000811,
                37.505740000000002965) );
    a_owned_points.push_back( Point(-122.48263800000000856,
                37.505740000000002965) );
    a_owned_points.push_back( Point(-122.47978799999999922,
                37.505740000000002965) );
    a_owned_points.push_back( Point(-122.47203799999999774,
                37.505790000000004625) );
    a_owned_points.push_back( Point(-122.47858800000000201,
                37.505890000000000839) );
    a_owned_points.push_back( Point(-122.475489500000009,
                37.5059180000000012) );
    a_owned_points.push_back( Point(-122.47653800000000501,
                37.505940000000002499) );
    a_owned_points.push_back( Point(-122.47758799999999724,
                37.505940000000002499) );
    a_owned_points.push_back( Point(-122.47404299999999466,
                37.505957000000002211) );
    a_owned_points.push_back( Point(-122.48038800000000492,
                37.506140000000002033) );
    a_owned_points.push_back( Point(-122.4803929999999923,
                37.506187999999994531) );
    a_owned_points.push_back( Point(-122.47323800000000915,
                37.506240000000005352) );
    a_owned_points.push_back( Point(-122.47038799999999981,
                37.506289999999999907) );
    a_owned_points.push_back( Point(-122.47453950000000589,
                37.506367999999994822) );
    a_owned_points.push_back( Point(-122.47093799999998964,
                37.506389999999996121) );
    a_owned_points.push_back( Point(-122.4776379999999989,
                37.504590000000000316) );
    a_owned_points.push_back( Point(-122.47658799999999246,
                37.504590000000000316) );
    a_owned_points.push_back( Point(-122.47558799999998769,
                37.504590000000000316) );
    a_owned_points.push_back( Point(-122.47138799999999037,
                37.504590000000000316) );
    a_owned_points.push_back( Point(-122.48103799999999808,
                37.504590000000000316) );
    a_owned_points.push_back( Point(-122.47438800000000469,
                37.504640000000001976) );
    a_owned_points.push_back( Point(-122.47958800000000679,
                37.504640000000001976) );
    a_owned_points.push_back( Point(-122.48798800000000142,
                37.50468999999999653) );
    a_owned_points.push_back( Point(-122.47838799999999537,
                37.50468999999999653) );
    a_owned_points.push_back( Point(-122.46933749999999463,
                37.50473999999999819) );
    a_owned_points.push_back( Point(-122.47313800000000583,
                37.50473999999999819) );
    a_owned_points.push_back( Point(-122.47178800000000365,
                37.50478999999999985) );
    a_owned_points.push_back( Point(-122.48908850000000825,
                37.50484000000000151) );
    a_owned_points.push_back( Point(-122.47218799999998851,
                37.50484000000000151) );
    a_owned_points.push_back( Point(-122.48108799999999974,
                37.504989999999999384) );
    a_owned_points.push_back( Point(-122.48053799999999569,
                37.505040000000001044) );
    a_owned_points.push_back( Point(-122.47093799999998964,
                37.505089999999995598) );
    a_owned_points.push_back( Point(-122.47933799999999849,
                37.505189999999998918) );
    a_owned_points.push_back( Point(-122.48943850000000566,
                37.505189999999998918) );
    a_owned_points.push_back( Point(-122.47708799999999485,
                37.505240000000000578) );
    a_owned_points.push_back( Point(-122.47603799999998841,
                37.505240000000000578) );
    a_owned_points.push_back( Point(-122.47813800000000128,
                37.505240000000000578) );
    a_owned_points.push_back( Point(-122.47353799999999069,
                37.505240000000000578) );
    a_owned_points.push_back( Point(-122.4823379999999986,
                37.505290000000002237) );
    a_owned_points.push_back( Point(-122.47483800000000542,
                37.505340000000003897) );
    a_owned_points.push_back( Point(-122.49240349999999466,
                37.501709500000004027) );
    a_owned_points.push_back( Point(-122.49298899999999435,
                37.502091000000000065) );
    a_owned_points.push_back( Point(-122.49193900000000212,
                37.502290999999999599) );
    a_owned_points.push_back( Point(-122.49358900000000006,
                37.502490999999999133) );
    a_owned_points.push_back( Point(-122.49223899999999787,
                37.502640999999997007) );
    a_owned_points.push_back( Point(-122.49023900000000253,
                37.502841000000003646) );
    a_owned_points.push_back( Point(-122.49118899999999144,
                37.502841000000003646) );
    a_owned_points.push_back( Point(-122.49137849999999617,
                37.503197499999998854) );
    a_owned_points.push_back( Point(-122.49368899999998916,
                37.503241000000002714) );
    a_owned_points.push_back( Point(-122.4903889999999933,
                37.503241000000002714) );
    a_owned_points.push_back( Point(-122.49283900000000358,
                37.503241000000002714) );
    a_owned_points.push_back( Point(-122.48953900000000772,
                37.503741000000005101) );
    a_owned_points.push_back( Point(-122.49063899999998739,
                37.50389099999999587) );
    a_owned_points.push_back( Point(-122.49193900000000212,
                37.50399099999999919) );
    a_owned_points.push_back( Point(-122.49278899999998771,
                37.50399099999999919) );
    a_owned_points.push_back( Point(-122.49313899999999933,
                37.504290999999994938) );
    a_owned_points.push_back( Point(-122.48968899999999849,
                37.50439049999999952) );
    a_owned_points.push_back( Point(-122.49098899999999901,
                37.504490500000002839) );
    a_owned_points.push_back( Point(-122.49253899999999362,
                37.504541000000003237) );
    a_owned_points.push_back( Point(-122.49168900000000804,
                37.505140499999995995) );
    a_owned_points.push_back( Point(-122.49906550000000038,
                37.505546999999999969) );
    a_owned_points.push_back( Point(-122.49913899999999956,
                37.506191000000001168) );
    a_owned_points.push_back( Point(-122.49463900000000649,
                37.506340500000000304) );
    a_owned_points.push_back( Point(-122.49883900000000381,
                37.506641000000001895) );
    a_owned_points.push_back( Point(-122.49938900000000785,
                37.506841000000001429) );
    a_owned_points.push_back( Point(-122.47498799999999619,
                37.503439999999997667) );
    a_owned_points.push_back( Point(-122.47373799999999733,
                37.503540000000000987) );
    a_owned_points.push_back( Point(-122.47578799999999433,
                37.503540000000000987) );
    a_owned_points.push_back( Point(-122.4872880000000066,
                37.503540499999999724) );
    a_owned_points.push_back( Point(-122.48843850000000089,
                37.503640500000003044) );
    a_owned_points.push_back( Point(-122.47438800000000469,
                37.503740000000000521) );
    a_owned_points.push_back( Point(-122.4788379999999961,
                37.503740000000000521) );
    a_owned_points.push_back( Point(-122.47683800000000076,
                37.503839999999996735) );
    a_owned_points.push_back( Point(-122.47508799999999951,
                37.503889999999998395) );
    a_owned_points.push_back( Point(-122.47263799999998923,
                37.503940000000000055) );
    a_owned_points.push_back( Point(-122.47588799999999765,
                37.503990000000001714) );
    a_owned_points.push_back( Point(-122.48743799999999737,
                37.504040000000003374) );
    a_owned_points.push_back( Point(-122.47078799999999887,
                37.504040000000003374) );
    a_owned_points.push_back( Point(-122.47733800000000315,
                37.504090000000005034) );
    a_owned_points.push_back( Point(-122.47808799999999962,
                37.504190000000001248) );
    a_owned_points.push_back( Point(-122.47343800000000158,
                37.504190000000001248) );
    a_owned_points.push_back( Point(-122.46983800000000997,
                37.504239999999995803) );
    a_owned_points.push_back( Point(-122.48858849999999165,
                37.504240500000001646) );
    a_owned_points.push_back( Point(-122.48133799999999383,
                37.504339999999999122) );
    a_owned_points.push_back( Point(-122.48753800000000069,
                37.504339999999999122) );
    a_owned_points.push_back( Point(-122.48423800000000483,
                37.504390000000000782) );
    a_owned_points.push_back( Point(-122.48723799999999073,
                37.504390000000000782) );
    a_owned_points.push_back( Point(-122.4724379999999968,
                37.504440000000002442) );
    a_owned_points.push_back( Point(-122.48168799999999123,
                37.504490000000004102) );
    a_owned_points.push_back( Point(-122.48673800000000256,
                37.504539999999998656) );
    a_owned_points.push_back( Point(-122.46608699999998748,
                37.501390000000000668) );
    a_owned_points.push_back( Point(-122.46148700000000531,
                37.501440000000002328) );
    a_owned_points.push_back( Point(-122.46778799999999876,
                37.501640000000001862) );
    a_owned_points.push_back( Point(-122.46508700000001113,
                37.501689999999996417) );
    a_owned_points.push_back( Point(-122.4641369999999938,
                37.501940000000004716) );
    a_owned_points.push_back( Point(-122.46858800000001111,
                37.501940000000004716) );
    a_owned_points.push_back( Point(-122.4658369999999934,
                37.50198999999999927) );
    a_owned_points.push_back( Point(-122.46343699999999899,
                37.502189999999998804) );
    a_owned_points.push_back( Point(-122.46683699999999817,
                37.502290000000002124) );
    a_owned_points.push_back( Point(-122.46488700000000449,
                37.502439999999999998) );
    a_owned_points.push_back( Point(-122.46783750000000168,
                37.502539999999996212) );
    a_owned_points.push_back( Point(-122.4688380000000052,
                37.502589999999997872) );
    a_owned_points.push_back( Point(-122.46608700000000169,
                37.502839999999999065) );
    a_owned_points.push_back( Point(-122.46398700000000304,
                37.502839999999999065) );
    a_owned_points.push_back( Point(-122.46708699999999226,
                37.503140000000001919) );
    a_owned_points.push_back( Point(-122.46813750000001164,
                37.503289999999999793) );
    a_owned_points.push_back( Point(-122.4652370000000019,
                37.503340000000001453) );
    a_owned_points.push_back( Point(-122.46432749999999601,
                37.503624500000000808) );
    a_owned_points.push_back( Point(-122.4664369999999991,
                37.503740000000000521) );
    a_owned_points.push_back( Point(-122.46758700000000886,
                37.503940000000000055) );
    a_owned_points.push_back( Point(-122.46863749999999982,
                37.504090000000005034) );
    a_owned_points.push_back( Point(-122.46578700000000595,
                37.504289999999997463) );
    a_owned_points.push_back( Point(-122.46527749999999912,
                37.504374499999997283) );
    a_owned_points.push_back( Point(-122.44812199999999791,
                37.504716500000000678) );
    a_owned_points.push_back( Point(-122.4658369999999934,
                37.50473999999999819) );
    a_owned_points.push_back( Point(-122.47243800000001102,
                37.501390000000000668) );
    a_owned_points.push_back( Point(-122.47013799999999151,
                37.501840000000001396) );
    a_owned_points.push_back( Point(-122.48593600000000947,
                37.501928500000005329) );
    a_owned_points.push_back( Point(-122.46938800000000924,
                37.50198999999999927) );
    a_owned_points.push_back( Point(-122.4718379999999911,
                37.502089999999995484) );
    a_owned_points.push_back( Point(-122.48598150000000828,
                37.502190499999997542) );
    a_owned_points.push_back( Point(-122.48674099999999498,
                37.502398499999998194) );
    a_owned_points.push_back( Point(-122.47043800000000147,
                37.502539999999996212) );
    a_owned_points.push_back( Point(-122.46963800000000333,
                37.502639999999999532) );
    a_owned_points.push_back( Point(-122.48599649999999883,
                37.502639999999999532) );
    a_owned_points.push_back( Point(-122.47143800000000624,
                37.502740000000002851) );
    a_owned_points.push_back( Point(-122.47258800000000178,
                37.502790000000004511) );
    a_owned_points.push_back( Point(-122.48677200000000198,
                37.502887000000001194) );
    a_owned_points.push_back( Point(-122.47338799999999992,
                37.50298999999999694) );
    a_owned_points.push_back( Point(-122.47088800000000219,
                37.503039999999998599) );
    a_owned_points.push_back( Point(-122.47203799999999774,
                37.503039999999998599) );
    a_owned_points.push_back( Point(-122.4823159999999973,
                37.503086499999994885) );
    a_owned_points.push_back( Point(-122.47423799999999972,
                37.503090000000000259) );
    a_owned_points.push_back( Point(-122.47008800000000406,
                37.503140000000001919) );
    a_owned_points.push_back( Point(-122.48828850000001012,
                37.503140999999999394) );
    a_owned_points.push_back( Point(-122.4894390000000044,
                37.503241000000002714) );
    a_owned_points.push_back( Point(-122.47293799999999919,
                37.503289999999999793) );
    a_owned_points.push_back( Point(-122.47148799999999369,
                37.503340000000001453) );
    a_owned_points.push_back( Point(-122.46913800000000094,
                37.503340000000001453) );
    a_owned_points.push_back( Point(-122.47228799999999183,
                37.503389999999996007) );
    a_owned_points.push_back( Point(-122.46643749999999784,
                37.500039999999998486) );
    a_owned_points.push_back( Point(-122.4602869999999939,
                37.500039999999998486) );
    a_owned_points.push_back( Point(-122.46578700000000595,
                37.500039999999998486) );
    a_owned_points.push_back( Point(-122.45923700000000167,
                37.500140000000001805) );
    a_owned_points.push_back( Point(-122.46653749999998695,
                37.500340000000001339) );
    a_owned_points.push_back( Point(-122.46478700000000117,
                37.500439999999997553) );
    a_owned_points.push_back( Point(-122.45983699999999317,
                37.500439999999997553) );
    a_owned_points.push_back( Point(-122.46388699999999972,
                37.500489999999999213) );
    a_owned_points.push_back( Point(-122.45928699999998912,
                37.500540000000000873) );
    a_owned_points.push_back( Point(-122.46283700000000749,
                37.500540000000000873) );
    a_owned_points.push_back( Point(-122.46763799999999378,
                37.500590000000002533) );
    a_owned_points.push_back( Point(-122.4608869999999996,
                37.500689999999998747) );
    a_owned_points.push_back( Point(-122.46588699999999506,
                37.500690000000005853) );
    a_owned_points.push_back( Point(-122.46723799999999471,
                37.500690000000005853) );
    a_owned_points.push_back( Point(-122.46188699999999017,
                37.500740000000000407) );
    a_owned_points.push_back( Point(-122.46878799999998932,
                37.500839999999996621) );
    a_owned_points.push_back( Point(-122.46768799999999544,
                37.500939999999999941) );
    a_owned_points.push_back( Point(-122.46658750000000282,
                37.501040000000003261) );
    a_owned_points.push_back( Point(-122.46043699999999887,
                37.501089999999997815) );
    a_owned_points.push_back( Point(-122.46533700000000522,
                37.50109000000000492) );
    a_owned_points.push_back( Point(-122.46433700000000044,
                37.501190000000001135) );
    a_owned_points.push_back( Point(-122.46308700000000158,
                37.501239999999995689) );
    a_owned_points.push_back( Point(-122.46848800000000779,
                37.501239999999995689) );
    a_owned_points.push_back( Point(-122.46358699999998976,
                37.501289999999997349) );
    a_owned_points.push_back( Point(-122.46243699999999421,
                37.501339999999999009) );
    a_owned_points.push_back( Point(-122.45928700000000333,
                37.49649000000000143) );
    a_owned_points.push_back( Point(-122.45554899999999066,
                37.496514500000003522) );
    a_owned_points.push_back( Point(-122.45192299999999364,
                37.496531500000003234) );
    a_owned_points.push_back( Point(-122.45073700000000372,
                37.496539999999995985) );
    a_owned_points.push_back( Point(-122.45983699999999317,
                37.496589999999997644) );
    a_owned_points.push_back( Point(-122.45903699999999503,
                37.496690000000000964) );
    a_owned_points.push_back( Point(-122.45586299999999369,
                37.49676700000000551) );
    a_owned_points.push_back( Point(-122.45656299999998851,
                37.49676700000000551) );
    a_owned_points.push_back( Point(-122.46158699999999442,
                37.496790000000004284) );
    a_owned_points.push_back( Point(-122.46073699999999462,
                37.496940000000002158) );
    a_owned_points.push_back( Point(-122.45758699999998953,
                37.496989999999996712) );
    a_owned_points.push_back( Point(-122.45853700000000686,
                37.497140000000001692) );
    a_owned_points.push_back( Point(-122.46548749999999472,
                37.49733999999999412) );
    a_owned_points.push_back( Point(-122.45671299999999349,
                37.497467000000000326) );
    a_owned_points.push_back( Point(-122.4579234999999926,
                37.497586499999997045) );
    a_owned_points.push_back( Point(-122.45747349999999187,
                37.497836499999998239) );
    a_owned_points.push_back( Point(-122.46433749999999918,
                37.498339999999998895) );
    a_owned_points.push_back( Point(-122.46778799999999876,
                37.499439999999999884) );
    a_owned_points.push_back( Point(-122.46768800000000965,
                37.499639999999999418) );
    a_owned_points.push_back( Point(-122.4652370000000019,
                37.499740000000002738) );
    a_owned_points.push_back( Point(-122.46433700000000044,
                37.499740000000002738) );
    a_owned_points.push_back( Point(-122.46333699999999567,
                37.499840000000006057) );
    a_owned_points.push_back( Point(-122.46228700000000345,
                37.499940000000002271) );
    a_owned_points.push_back( Point(-122.46733799999999803,
                37.499940000000002271) );
    a_owned_points.push_back( Point(-122.46128699999999867,
                37.499989999999996826) );
    a_owned_points.push_back( Point(-122.4500370000000089,
                37.495639999999994529) );
    a_owned_points.push_back( Point(-122.44878700000001004,
                37.495639999999994529) );
    a_owned_points.push_back( Point(-122.45753700000000208,
                37.495689999999996189) );
    a_owned_points.push_back( Point(-122.46108699999999203,
                37.495689999999996189) );
    a_owned_points.push_back( Point(-122.45673700000000395,
                37.495739999999997849) );
    a_owned_points.push_back( Point(-122.45813699999999358,
                37.495739999999997849) );
    a_owned_points.push_back( Point(-122.45253700000000663,
                37.495840000000001169) );
    a_owned_points.push_back( Point(-122.45073700000000372,
                37.495840000000001169) );
    a_owned_points.push_back( Point(-122.45788699999999949,
                37.495940000000004488) );
    a_owned_points.push_back( Point(-122.4540554999999955,
                37.495962000000005787) );
    a_owned_points.push_back( Point(-122.46263700000000085,
                37.495989999999999043) );
    a_owned_points.push_back( Point(-122.44983700000000226,
                37.496039999999993597) );
    a_owned_points.push_back( Point(-122.44923700000001077,
                37.496089999999995257) );
    a_owned_points.push_back( Point(-122.45868699999999762,
                37.496139999999996917) );
    a_owned_points.push_back( Point(-122.46168699999999774,
                37.496189999999998577) );
    a_owned_points.push_back( Point(-122.45028700000000299,
                37.496189999999998577) );
    a_owned_points.push_back( Point(-122.45118700000000445,
                37.496189999999998577) );
    a_owned_points.push_back( Point(-122.45594900000000393,
                37.496214499999993564) );
    a_owned_points.push_back( Point(-122.45668700000000229,
                37.496240000000000236) );
    a_owned_points.push_back( Point(-122.4573869999999971,
                37.496290000000001896) );
    a_owned_points.push_back( Point(-122.45525449999999523,
                37.496318000000002257) );
    a_owned_points.push_back( Point(-122.45272299999999177,
                37.496331499999996595) );
    a_owned_points.push_back( Point(-122.45329150000000595,
                37.496353499999997894) );
    a_owned_points.push_back( Point(-122.45833700000000022,
                37.49643999999999977) );
    a_owned_points.push_back( Point(-122.45372599999998897,
                37.496443999999996777) );
    a_owned_points.push_back( Point(-122.45543700000000342,
                37.494489999999998986) );
    a_owned_points.push_back( Point(-122.45428699999999367,
                37.49474000000000018) );
    a_owned_points.push_back( Point(-122.45843700000000354,
                37.494789999999994734) );
    a_owned_points.push_back( Point(-122.45603700000000913,
                37.494790000000001839) );
    a_owned_points.push_back( Point(-122.46228700000000345,
                37.494790000000001839) );
    a_owned_points.push_back( Point(-122.46158700000000863,
                37.494790000000001839) );
    a_owned_points.push_back( Point(-122.45538700000000176,
                37.494790000000001839) );
    a_owned_points.push_back( Point(-122.45928700000000333,
                37.494839999999996394) );
    a_owned_points.push_back( Point(-122.45163700000000517,
                37.494839999999996394) );
    a_owned_points.push_back( Point(-122.45243700000000331,
                37.494990000000001373) );
    a_owned_points.push_back( Point(-122.46023700000000645,
                37.494990000000001373) );
    a_owned_points.push_back( Point(-122.45343699999999387,
                37.495090000000004693) );
    a_owned_points.push_back( Point(-122.45605850000001169,
                37.495111500000000149) );
    a_owned_points.push_back( Point(-122.46110749999999712,
                37.495183500000003107) );
    a_owned_points.push_back( Point(-122.45043699999999376,
                37.495190000000000907) );
    a_owned_points.push_back( Point(-122.45768699999999285,
                37.495240000000002567) );
    a_owned_points.push_back( Point(-122.45428699999999367,
                37.495240000000002567) );
    a_owned_points.push_back( Point(-122.46298699999999826,
                37.495289999999997121) );
    a_owned_points.push_back( Point(-122.4550869999999918,
                37.495339999999998781) );
    a_owned_points.push_back( Point(-122.45665850000000319,
                37.495361500000001342) );
    a_owned_points.push_back( Point(-122.45113699999998857,
                37.495390000000000441) );
    a_owned_points.push_back( Point(-122.45248699999999076,
                37.495390000000000441) );
    a_owned_points.push_back( Point(-122.4585869999999943,
                37.495390000000000441) );
    a_owned_points.push_back( Point(-122.44948700000000485,
                37.495440000000002101) );
    a_owned_points.push_back( Point(-122.4518369999999976,
                37.495589999999999975) );
    a_owned_points.push_back( Point(-122.45948699999999576,
                37.493040000000000589) );
    a_owned_points.push_back( Point(-122.4588965000000087,
                37.49305599999999572) );
    a_owned_points.push_back( Point(-122.45829650000000299,
                37.49310599999999738) );
    a_owned_points.push_back( Point(-122.45999100000000226,
                37.493117999999995504) );
    a_owned_points.push_back( Point(-122.46018349999999941,
                37.493262000000001422) );
    a_owned_points.push_back( Point(-122.45368049999999016,
                37.493268999999997959) );
    a_owned_points.push_back( Point(-122.45758699999998953,
                37.493290000000001783) );
    a_owned_points.push_back( Point(-122.45703699999999969,
                37.493439999999999657) );
    a_owned_points.push_back( Point(-122.45830250000000206,
                37.493462999999998431) );
    a_owned_points.push_back( Point(-122.46047949999999105,
                37.493483999999995149) );
    a_owned_points.push_back( Point(-122.45613700000001245,
                37.493589999999997531) );
    a_owned_points.push_back( Point(-122.45878799999999842,
                37.493840500000004567) );
    a_owned_points.push_back( Point(-122.46038699999999722,
                37.493939999999994939) );
    a_owned_points.push_back( Point(-122.45488699999999938,
                37.493940000000002044) );
    a_owned_points.push_back( Point(-122.45894800000000657,
                37.493965000000002874) );
    a_owned_points.push_back( Point(-122.45673700000000395,
                37.493989999999996598) );
    a_owned_points.push_back( Point(-122.45973700000000406,
                37.493989999999996598) );
    a_owned_points.push_back( Point(-122.46128699999999867,
                37.494089999999999918) );
    a_owned_points.push_back( Point(-122.4541370000000029,
                37.494190000000003238) );
    a_owned_points.push_back( Point(-122.45936249999999745,
                37.494287499999998658) );
    a_owned_points.push_back( Point(-122.45448700000000031,
                37.494289999999999452) );
    a_owned_points.push_back( Point(-122.45988700000000904,
                37.494389999999995666) );
    a_owned_points.push_back( Point(-122.45313699999999812,
                37.494439999999997326) );
    a_owned_points.push_back( Point(-122.4541370000000029,
                37.494439999999997326) );
    a_owned_points.push_back( Point(-122.45478700000001027,
                37.494489999999998986) );
    a_owned_points.push_back( Point(-122.45278700000000072,
                37.490540000000002863) );
    a_owned_points.push_back( Point(-122.45328699999998889,
                37.490540000000002863) );
    a_owned_points.push_back( Point(-122.45558599999999672,
                37.490639000000001602) );
    a_owned_points.push_back( Point(-122.45773700000000872,
                37.490690000000000737) );
    a_owned_points.push_back( Point(-122.45191499999999962,
                37.490871999999995978) );
    a_owned_points.push_back( Point(-122.45223699999999667,
                37.490890000000000271) );
    a_owned_points.push_back( Point(-122.45263700000000995,
                37.49094000000000193) );
    a_owned_points.push_back( Point(-122.45540850000000432,
                37.491185500000000275) );
    a_owned_points.push_back( Point(-122.45613600000000076,
                37.491188999999998543) );
    a_owned_points.push_back( Point(-122.45688699999999471,
                37.491190000000003124) );
    a_owned_points.push_back( Point(-122.4576370000000054,
                37.491289999999999338) );
    a_owned_points.push_back( Point(-122.45808700000000613,
                37.491540000000000532) );
    a_owned_points.push_back( Point(-122.45439650000000142,
                37.491710999999995124) );
    a_owned_points.push_back( Point(-122.45253049999999462,
                37.491719000000003348) );
    a_owned_points.push_back( Point(-122.45668700000000229,
                37.49178999999999462) );
    a_owned_points.push_back( Point(-122.45743699999999876,
                37.49188999999999794) );
    a_owned_points.push_back( Point(-122.45810699999999827,
                37.492040000000002919) );
    a_owned_points.push_back( Point(-122.45346750000000213,
                37.492193499999999062) );
    a_owned_points.push_back( Point(-122.45858099999999524,
                37.492290499999995745) );
    a_owned_points.push_back( Point(-122.45588700000000415,
                37.492490000000003647) );
    a_owned_points.push_back( Point(-122.45730749999999887,
                37.492496000000002709) );
    a_owned_points.push_back( Point(-122.45797749999999837,
                37.492646000000000583) );
    a_owned_points.push_back( Point(-122.45851650000000177,
                37.492755999999999972) );
    a_owned_points.push_back( Point(-122.45468700000000695,
                37.492890000000002715) );
    a_owned_points.push_back( Point(-122.4577574999999996,
                37.492995999999997991) );
    a_owned_points.push_back( Point(-122.44783699999999271,
                37.485489999999998645) );
    a_owned_points.push_back( Point(-122.45133699999999521,
                37.485889999999997713) );
    a_owned_points.push_back( Point(-122.45433700000000954,
                37.488289999999999225) );
    a_owned_points.push_back( Point(-122.45043699999999376,
                37.488590000000002078) );
    a_owned_points.push_back( Point(-122.45433700000000954,
                37.488590000000002078) );
    a_owned_points.push_back( Point(-122.45368700000000217,
                37.488739999999999952) );
    a_owned_points.push_back( Point(-122.45478700000001027,
                37.488789999999994507) );
    a_owned_points.push_back( Point(-122.45403699999999958,
                37.488839999999996166) );
    a_owned_points.push_back( Point(-122.45358699999999885,
                37.48913999999999902) );
    a_owned_points.push_back( Point(-122.4530369999999948,
                37.48913999999999902) );
    a_owned_points.push_back( Point(-122.45518699999999512,
                37.489239999999995234) );
    a_owned_points.push_back( Point(-122.45418699999999035,
                37.489239999999995234) );
    a_owned_points.push_back( Point(-122.45478700000001027,
                37.489339999999998554) );
    a_owned_points.push_back( Point(-122.45228700000001254,
                37.489589999999999748) );
    a_owned_points.push_back( Point(-122.45288700000000404,
                37.489589999999999748) );
    a_owned_points.push_back( Point(-122.4535370000000114,
                37.489639999999994302) );
    a_owned_points.push_back( Point(-122.4541370000000029,
                37.489739999999997622) );
    a_owned_points.push_back( Point(-122.45488699999999938,
                37.489940000000004261) );
    a_owned_points.push_back( Point(-122.45278700000000072,
                37.490090000000002135) );
    a_owned_points.push_back( Point(-122.4520870000000059,
                37.490090000000002135) );
    a_owned_points.push_back( Point(-122.45568699999999751,
                37.490090000000002135) );
    a_owned_points.push_back( Point(-122.45398700000001213,
                37.490139999999996689) );
    a_owned_points.push_back( Point(-122.45343700000000808,
                37.490139999999996689) );
    a_owned_points.push_back( Point(-122.45176499999999464,
                37.49052199999999857) );

    std::vector<Point> b_owned_points;
    b_owned_points.push_back( Point(-122.46823700000000201,
                37.508290000000002351) );
    b_owned_points.push_back( Point(-122.46743700000000388,
                37.509090000000000487) );
    b_owned_points.push_back( Point(-122.46688699999999983,
                37.50929000000000002) );
    b_owned_points.push_back( Point(-122.46823700000000201,
                37.509439999999997895) );
    b_owned_points.push_back( Point(-122.46393700000000138,
                37.509439999999997895) );
    b_owned_points.push_back( Point(-122.46628699999999412,
                37.509889999999998622) );
    b_owned_points.push_back( Point(-122.46763699999999631,
                37.510040000000003602) );
    b_owned_points.push_back( Point(-122.46568700000000263,
                37.511089999999995825) );
    b_owned_points.push_back( Point(-122.46468699999999785,
                37.511839999999999407) );
    b_owned_points.push_back( Point(-122.46228700000000345,
                37.513239999999996144) );
    b_owned_points.push_back( Point(-122.46263700000000085,
                37.513490000000004443) );
    b_owned_points.push_back( Point(-122.45975350000000503,
                37.51768549999999891) );
    b_owned_points.push_back( Point(-122.46619749999999271,
                37.51992750000000143) );
    b_owned_points.push_back( Point(-122.47848799999999869,
                37.51028999999999769) );
    b_owned_points.push_back( Point(-122.48068800000000067,
                37.51033999999999935) );
    b_owned_points.push_back( Point(-122.47503800000001206,
                37.51039000000000101) );
    b_owned_points.push_back( Point(-122.47563800000000356,
                37.510490000000004329) );
    b_owned_points.push_back( Point(-122.47993800000000419,
                37.510540000000005989) );
    b_owned_points.push_back( Point(-122.47953800000000513,
                37.510640000000002203) );
    b_owned_points.push_back( Point(-122.48243800000000192,
                37.510790000000000077) );
    b_owned_points.push_back( Point(-122.47733800000000315,
                37.510840000000001737) );
    b_owned_points.push_back( Point(-122.48153800000000047,
                37.510939999999997951) );
    b_owned_points.push_back( Point(-122.48068800000000067,
                37.510989999999999611) );
    b_owned_points.push_back( Point(-122.47683800000000076,
                37.511189999999999145) );
    b_owned_points.push_back( Point(-122.48628800000000183,
                37.511189999999999145) );
    b_owned_points.push_back( Point(-122.48703799999999831,
                37.511189999999999145) );
    b_owned_points.push_back( Point(-122.48093800000000897,
                37.511489999999994893) );
    b_owned_points.push_back( Point(-122.48568799999999612,
                37.511639999999999873) );
    b_owned_points.push_back( Point(-122.48753800000000069,
                37.512940000000000396) );
    b_owned_points.push_back( Point(-122.48623800000000017,
                37.512940000000000396) );
    b_owned_points.push_back( Point(-122.48823799999999551,
                37.51313999999999993) );
    b_owned_points.push_back( Point(-122.48798800000000142,
                37.513689999999996871) );
    b_owned_points.push_back( Point(-122.48843800000000215,
                37.513890000000003511) );
    b_owned_points.push_back( Point(-122.48818800000000806,
                37.514490000000002112) );

    // No dupes
    for( Point p : a_owned_points ) {
        REQUIRE( std::find( b_owned_points.begin(), b_owned_points.end(),
                    p ) == b_owned_points.end() );
    }


    Rectangle a_rect(-122.51773900000000594, 37.471648999999999319, -122.4473365000000058, 37.537389000000004557);
    Rectangle b_rect(-122.5171890000000019, 37.485489999999998645, -122.4473365000000058, 37.605969000000008862);
    REQUIRE( a_rect.intersectsRectangle( b_rect ) );
    std::cout << "A: " << a_owned_points.size() << std::endl;
    std::cout << "B: " << b_owned_points.size() << std::endl;

    unlink( "big_tree.txt");
    nirtreedisk::NIRTreeDisk<12,25, nirtreedisk::LineMinimizeDownsplits>
        big_tree( 40960*100, "big_tree.txt");

    std::vector<tree_node_handle> a_handles;
    tree_node_allocator *allocator = big_tree.node_allocator_.get();
    auto alloc_data =
        allocator->create_new_tree_node<nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>>(
                NodeHandleType( LEAF_NODE ) );
    new (&(*alloc_data.first))
        nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>( &big_tree,
            nullptr, alloc_data.second, 0 );
    auto leaf_node = alloc_data.first;

    for( Point p : a_owned_points ) {
        if( leaf_node->cur_offset_ >= 25 ) {
            assert( leaf_node->cur_offset_ == 25 );
            a_handles.push_back( alloc_data.second );
            alloc_data = allocator->create_new_tree_node<nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>>(
                NodeHandleType( LEAF_NODE ) );
            new (&(*alloc_data.first)) nirtreedisk::LeafNode<12,25,
                nirtreedisk::LineMinimizeDownsplits>( &big_tree,
                    nullptr, alloc_data.second, 0 );
            leaf_node = alloc_data.first;
        }
        leaf_node->addPoint( p );
    }
    REQUIRE( a_handles.size() <= 25 );
    auto alloc_branch_data =
        allocator->create_new_tree_node<nirtreedisk::BranchNode<12,25,nirtreedisk::LineMinimizeDownsplits>>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_branch_data.first))
        nirtreedisk::BranchNode<12,25,nirtreedisk::LineMinimizeDownsplits>(
                &big_tree, nullptr, alloc_branch_data.second, 1 );
    auto a_branch_node = alloc_branch_data.first;
    auto a_branch_handle = alloc_branch_data.second;
    for( tree_node_handle child_handle : a_handles ) {
        auto child_node = allocator->get_tree_node<nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>>( child_handle );
        child_node->parent = a_branch_handle;
        Rectangle r = child_node->boundingBox();
        nirtreedisk::Branch b( r, child_handle );
        a_branch_node->addBranchToNode( b );
    }

    std::vector<tree_node_handle> b_handles;
    alloc_data =
        allocator->create_new_tree_node<nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>>( NodeHandleType( LEAF_NODE ) );
    new (&(*alloc_data.first))
        nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>(
                &big_tree, nullptr, alloc_data.second, 0 );
    leaf_node = alloc_data.first;
    for( Point p : b_owned_points ) {
        if( leaf_node->cur_offset_ >= 25 ) {
            assert( leaf_node->cur_offset_ == 25 );
            b_handles.push_back( alloc_data.second );
            alloc_data = allocator->create_new_tree_node<nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>>(
                NodeHandleType( LEAF_NODE ) );
            new (&(*alloc_data.first)) nirtreedisk::LeafNode<12,25,
                nirtreedisk::LineMinimizeDownsplits>( &big_tree,
                    nullptr, alloc_data.second, 0 );
            leaf_node = alloc_data.first;
        }
        leaf_node->addPoint( p );
    }

    alloc_branch_data = allocator->create_new_tree_node<nirtreedisk::BranchNode<12,25,nirtreedisk::LineMinimizeDownsplits>>(
                NodeHandleType( BRANCH_NODE ) );
    new (&(*alloc_branch_data.first))
        nirtreedisk::BranchNode<12,25,nirtreedisk::LineMinimizeDownsplits>(
                &big_tree, nullptr, alloc_branch_data.second, 1 );
    auto b_branch_node = alloc_branch_data.first;
    auto b_branch_handle = alloc_branch_data.second;
    for( tree_node_handle child_handle : b_handles ) {
        auto child_node = allocator->get_tree_node<nirtreedisk::LeafNode<12,25,nirtreedisk::LineMinimizeDownsplits>>( child_handle );
        child_node->parent = b_branch_handle;
        Rectangle r = child_node->boundingBox();
        nirtreedisk::Branch b( r, child_handle );
        b_branch_node->addBranchToNode( b );
    }

    auto res = make_rectangles_disjoint_accounting_for_region_ownership( &big_tree,
            a_rect, a_branch_handle, b_rect, b_branch_handle );

    IsotheticPolygon poly1;
    poly1.basicRectangles = res.first;
    poly1.recomputeBoundingBox();

    IsotheticPolygon poly2;
    poly2.basicRectangles = res.second;
    poly2.recomputeBoundingBox();

    REQUIRE( poly1.disjoint( poly2 ) );

            

    unlink( "big_tree.txt");
}
