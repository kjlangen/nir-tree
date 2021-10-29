#include <catch2/catch.hpp>
#include <rplustreedisk/rplustreedisk.h>
#include <util/geometry.h>

#define TWO_THREE_TREE rplustreedisk::RPlusTreeDisk<2,3>
#define TWO_THREE_NODE rplustreedisk::Node<2,3>

TEST_CASE("R+TreeDisk: testBoundingBox")
{
    unlink( "rplustreedisk.txt" );

	TWO_THREE_TREE tree(4096*10,
            "rplustreedisk.txt");

	Point p1(0.0, 0.0);
	tree.insert(p1);
	Point p2(5.0, 5.0);
	tree.insert(p2);
    auto root_node = tree.get_node( tree.root_ );
	REQUIRE( root_node->boundingBox() == Rectangle(0.0, 0.0,
                nextafter(5.0, DBL_MAX), nextafter(5.0, DBL_MAX)));

	Point p3(10.0, 0.0);
	tree.insert(p3);
	REQUIRE(root_node->boundingBox() == Rectangle(0.0, 0.0,
                nextafter(10.0, DBL_MAX), nextafter(5.0, DBL_MAX)));

    unlink( "rplustreedisk.txt" );
}

TEST_CASE("R+TreeDisk: testPartition")
{
    unlink( "rplustreedisk.txt" );
	rplustreedisk::RPlusTreeDisk<2,5> tree( 4096*10, "rplustreedisk.txt" );

    auto root_node = tree.get_node( tree.root_ );
    root_node->entries.at( root_node->cur_offset_++ ) = Point(0.0, 0.0);
	root_node->entries.at( root_node->cur_offset_++ ) = Point(4.0, 0.0);
	root_node->entries.at( root_node->cur_offset_++ ) = Point(5.0, 0.0);
	root_node->entries.at( root_node->cur_offset_++ ) = Point(5.0, 5.0);
	root_node->entries.at( root_node->cur_offset_++ ) = Point(9.0, 0.0);

	// Partition
	auto part = root_node->partitionNode();

	// Test partition
	REQUIRE(part.dimension == 0);
	REQUIRE(part.location == 4.0);

    unlink( "rplustreedisk.txt" );
}

TEST_CASE("R+TreeDisk: testPartition2")
{
    unlink( "rplustreedisk.txt" );

	TWO_THREE_TREE tree( 4096*10, "rplustreedisk.txt" );

    auto alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto first_handle = alloc_data.second;

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto second_handle = alloc_data.second;

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto third_handle = alloc_data.second;


    auto root_node = tree.get_node( tree.root_ );
	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch(first_handle, Rectangle(0.0, 0.0, 2.0, 8.0));
	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch(second_handle, Rectangle(3.0, 0.0, 5.0, 4.0));
	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch(third_handle, Rectangle(6.0, 0.0, 8.0, 2.0));

	// Partition
	auto part = root_node->partitionNode();

	// Test partition
	REQUIRE(part.dimension == 0);
	REQUIRE(part.location == 2.0);
    
    unlink( "rplustreedisk.txt" );
}

TEST_CASE("R+TreeDisk: testSplitNode")
{
    unlink( "rplustreedisk.txt" );

    TWO_THREE_TREE tree( 4096*10, "rplustreedisk.txt" );
    auto root_node = tree.get_node( tree.root_ );

    tree_node_handle child_handles[4];

    auto alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
                alloc_data.second, tree.root_ );
    child_handles[0] = alloc_data.second;

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
                alloc_data.second, tree.root_ );
    child_handles[1] = alloc_data.second;

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
                alloc_data.second, tree.root_ );
    child_handles[2] = alloc_data.second;

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
                alloc_data.second, tree.root_ );
    child_handles[3] = alloc_data.second;


    auto child3 = tree.get_node( child_handles[3] );
	child3->entries.at( child3->cur_offset_++ ) = Point(5.0, 4.0);
	child3->entries.at( child3->cur_offset_++ ) = Point(9.0, 12.0);

	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch({child_handles[0], Rectangle(0.0, 0.0,
                    4.0, 8.0)});
	root_node->entries.at( root_node->cur_offset_++ ) =
    rplustreedisk::Branch({child_handles[1], Rectangle(0.0, 9.0, 4.0, 12.0)});
	root_node->entries.at( root_node->cur_offset_++ ) =
    rplustreedisk::Branch({child_handles[2], Rectangle(5.0, 0.0, 9.0, 3.0)});
	root_node->entries.at( root_node->cur_offset_++ ) =
    rplustreedisk::Branch({child_handles[3], Rectangle(5.0, 4.0, 9.0, 12.0)});

	auto result = root_node->splitNode( {1, 8.0} );

    auto left_child = tree.get_node( result.leftBranch.child );
    auto right_child = tree.get_node( result.rightBranch.child );

	REQUIRE(left_child->cur_offset_ == 3);
	REQUIRE(result.leftBranch.boundingBox == Rectangle(0.0, 0.0, 9.0, 8.0));
	REQUIRE(right_child->cur_offset_  == 2);
	REQUIRE(result.rightBranch.boundingBox == Rectangle(0.0, 9.0,
                nextafter(9.0, DBL_MAX), nextafter(12.0,DBL_MAX)));
    unlink( "rplustreedisk.txt" );
}

TEST_CASE("R+TreeDisk: testNewChildNode")
{
    unlink("rplustreedisk.txt" );
    TWO_THREE_TREE tree( 4096 *10, "rplustreedisk.txt" );

	tree.insert(Point(0.0, 0.0));
	tree.insert(Point(4.0, 5.0));
	tree.insert(Point(5.0, 0.0));
	tree.insert(Point(9.0, 5.0));


    auto root_node = tree.get_node( tree.root_ );
	REQUIRE(root_node->boundingBox() == Rectangle(0.0, 0.0,
                nextafter(9.0, DBL_MAX), nextafter(5.0, DBL_MAX)));
    REQUIRE( not root_node->isLeaf() );
	REQUIRE( root_node->cur_offset_ == 2 );

	// test left child
    rplustreedisk::Branch &b1 = std::get<rplustreedisk::Branch>( root_node->entries.at(0) );
	REQUIRE(b1.boundingBox == Rectangle(0.0, 0.0,
                nextafter(4.0, DBL_MAX), nextafter(5.0, DBL_MAX)));

    auto child = tree.get_node( b1.child );
	REQUIRE( child->isLeaf() );
	REQUIRE( child->cur_offset_ == 2 );
    REQUIRE( child->parent_ == tree.root_ );

	// test right child
    rplustreedisk::Branch &b2 = std::get<rplustreedisk::Branch>( root_node->entries.at(1) );
    child = tree.get_node( b2.child );
	REQUIRE(b2.boundingBox == Rectangle(5.0, 0.0,
                nextafter(9.0, DBL_MAX), nextafter(5.0, DBL_MAX)));
	REQUIRE( child->isLeaf() );
	REQUIRE( child->cur_offset_ == 2 );
	REQUIRE( child->parent_ == tree.root_ );
    unlink( "rplustreedisk.txt" );
}

TEST_CASE("R+TreeDisk: testInsert")
{
    unlink( "rplustreedisk.txt" );
	TWO_THREE_TREE tree(4096*10, "rplustreedisk.txt" );
    auto root_node = tree.get_node( tree.root_ );

    auto alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto cluster1_handle = alloc_data.second;
    auto cluster1_node = alloc_data.first;

	cluster1_node->entries.at( cluster1_node->cur_offset_++ ) = Point( 0.0, 0.0 );
	cluster1_node->entries.at( cluster1_node->cur_offset_++ ) = Point( 4.0, 4.0 );
    root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch( {cluster1_handle, cluster1_node->boundingBox()});

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto cluster2_handle = alloc_data.second;
    auto cluster2_node = alloc_data.first;

	cluster2_node->entries.at( cluster2_node->cur_offset_++ ) = Point( 5.0, 0.0 );
	cluster2_node->entries.at( cluster2_node->cur_offset_++ ) = Point( 9.0, 4.0 );
    root_node->entries.at( root_node->cur_offset_++ ) = 
        rplustreedisk::Branch({cluster2_handle, cluster2_node->boundingBox()});

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto cluster3_handle = alloc_data.second;
    auto cluster3_node = alloc_data.first;

	cluster3_node->entries.at( cluster3_node->cur_offset_++ ) = Point( 0.0, 5.0 );
	cluster3_node->entries.at( cluster3_node->cur_offset_++ ) = Point( 4.0, 9.0 );
	cluster3_node->entries.at( cluster3_node->cur_offset_++ ) = Point( 9.0, 9.0 );
    root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch( {cluster3_handle, cluster3_node->boundingBox()});

	// Insert new point, causing node to overflow
	tree.insert(Point(5.0, 5.0));

    root_node = tree.get_node( tree.root_ );
	REQUIRE(root_node->boundingBox() == Rectangle(0.0, 0.0,
                nextafter(9.0, DBL_MAX), nextafter(9.0, DBL_MAX) ));
	REQUIRE( not root_node->isLeaf() );
	REQUIRE( root_node->cur_offset_ == 2 );


    rplustreedisk::Branch &b_0 = std::get<rplustreedisk::Branch>( root_node->entries.at(0) );
    rplustreedisk::Branch &b_1 = std::get<rplustreedisk::Branch>( root_node->entries.at(1) );
    auto child0 = tree.get_node( b_0.child );
    auto child1 = tree.get_node( b_1.child );

	REQUIRE( child0->cur_offset_ == 2 );
    rplustreedisk::Branch &b_0_0 = std::get<rplustreedisk::Branch>( child0->entries.at(0) );
    rplustreedisk::Branch &b_0_1 = std::get<rplustreedisk::Branch>( child0->entries.at(1) );
    auto child0_0 = tree.get_node( b_0_0.child );
    auto child0_1 = tree.get_node( b_0_1.child );
    REQUIRE( not child0->isLeaf() );
    REQUIRE( child0_0->isLeaf() );
    REQUIRE( child0_0->cur_offset_ == 2 );
    REQUIRE( child0_1->isLeaf() );
    REQUIRE( child0_1->cur_offset_ == 2 );

    REQUIRE( child1->cur_offset_ == 2 );
    rplustreedisk::Branch &b_1_0 = std::get<rplustreedisk::Branch>( child1->entries.at(0) );
    rplustreedisk::Branch &b_1_1 = std::get<rplustreedisk::Branch>( child1->entries.at(1) );
    auto child1_0 = tree.get_node( b_1_0.child );
    auto child1_1 = tree.get_node( b_1_1.child );

	REQUIRE( not child1->isLeaf() );
    REQUIRE( child1_0->isLeaf() );
    REQUIRE( child1_0->cur_offset_ == 2 );
    REQUIRE( child1_1->isLeaf() );
    REQUIRE( child1_1->cur_offset_ == 2 );
    unlink( "rplustreedisk.txt" );
}

TEST_CASE("R+TreeDisk: testSimpleRemove")
{
    TWO_THREE_TREE tree( 4096*10, "rplustreedisk.txt" );

    auto alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto cluster1a_handle = alloc_data.second;
    auto cluster1a_node = alloc_data.first;

	cluster1a_node->entries.at( cluster1a_node->cur_offset_++ ) = Point( 0.0, 0.0 );
	cluster1a_node->entries.at( cluster1a_node->cur_offset_++ ) = Point( 4.0, 4.0 );

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto cluster1b_handle = alloc_data.second;
    auto cluster1b_node = alloc_data.first;

	cluster1b_node->entries.at( cluster1b_node->cur_offset_++ ) = Point(0.0, 5.0);
	cluster1b_node->entries.at( cluster1b_node->cur_offset_++ ) = Point(4.0, 9.0);

    alloc_data = tree.node_allocator_.create_new_tree_node<TWO_THREE_NODE>();
    new (&(*(alloc_data.first))) TWO_THREE_NODE( &tree,
            alloc_data.second, tree.root_ );
    auto cluster1c_handle = alloc_data.second;
    auto cluster1c_node = alloc_data.first;

	cluster1c_node->entries.at( cluster1c_node->cur_offset_++ ) = Point( 5.0, 0.0 );
	cluster1c_node->entries.at( cluster1c_node->cur_offset_++ ) = Point( 7.0, 9.0 );

    auto root_node = tree.get_node( tree.root_ );
	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch({cluster1a_handle, cluster1a_node->boundingBox()});
	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch({cluster1b_handle, cluster1b_node->boundingBox()});
	root_node->entries.at( root_node->cur_offset_++ ) =
        rplustreedisk::Branch({cluster1c_handle, cluster1c_node->boundingBox()});

	// end of setup
	tree.remove( Point(7.0, 9.0) );
	REQUIRE( root_node->cur_offset_ == 3 );
	REQUIRE( cluster1c_node->cur_offset_ == 1 );
}

/*
TEST_CASE("R+Tree: testChooseNode")
{
	Point p = Point(165.0, 181.0);
	rplustree::RPlusTree tree(2, 3);

	auto *child1 = new rplustree::Node(tree, 2, 3, tree.root);
	auto *child2 = new rplustree::Node(tree, 2, 3, tree.root);

	tree.root->branches.push_back({child1, Rectangle(123.0, 151.0, 146.0, 186.0)});
	tree.root->branches.push_back({child2, Rectangle(150.0, 183.0, 152.0, 309.0)});

	auto *n = tree.root->chooseNode(p);
	REQUIRE(n == child1);
}
*/
