#include <catch2/catch.hpp>
#include <rplustreedisk/rplustreedisk.h>
#include <util/geometry.h>

TEST_CASE("R+TreeDisk: testBoundingBox")
{
    unlink( "rplustreedisk.txt" );

	rplustreedisk::RPlusTreeDisk<2,3> tree(4096*10,
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

/*
TEST_CASE("R+Tree: testPartition")
{
	rplustree::RPlusTree tree(2, 3);

	tree.root->data.push_back(Point(0.0, 0.0));
	tree.root->data.push_back(Point(4.0, 0.0));
	tree.root->data.push_back(Point(5.0, 0.0));
	tree.root->data.push_back(Point(5.0, 5.0));
	tree.root->data.push_back(Point(9.0, 0.0));

	// Partition
	auto part = tree.root->partitionNode();

	// Test partition
	REQUIRE(part.dimension == 0);
	REQUIRE(part.location == 4.0);
}

TEST_CASE("R+Tree: testPartition2")
{
	rplustree::RPlusTree tree(2, 3);

	tree.root->branches.push_back({new rplustree::Node(tree, 2, 3, tree.root), Rectangle(0.0, 0.0, 2.0, 8.0)});
	tree.root->branches.push_back({new rplustree::Node(tree, 2, 3, tree.root), Rectangle(3.0, 0.0, 5.0, 4.0)});
	tree.root->branches.push_back({new rplustree::Node(tree, 2, 3, tree.root), Rectangle(6.0, 0.0, 8.0, 2.0)});

	// Partition
	auto part = tree.root->partitionNode();

	// Test partition
	REQUIRE(part.dimension == 0);
	REQUIRE(part.location == 2.0);
}

TEST_CASE("R+Tree: testSplitNode")
{
	rplustree::RPlusTree tree(2, 3);
	auto *root = new rplustree::Node(tree, 2, 3, nullptr);

	auto *n0 = new rplustree::Node(tree, 2, 3, root);
	auto *n1 = new rplustree::Node(tree, 2, 3, root);
	auto *n2 = new rplustree::Node(tree, 2, 3, root);
	auto *n3 = new rplustree::Node(tree, 2, 3, root);
	n3->data.push_back(Point(5.0, 4.0));
	n3->data.push_back(Point(9.0, 12.0));

	root->branches.push_back({n0, Rectangle(0.0, 0.0, 4.0, 8.0)});
	root->branches.push_back({n1, Rectangle(0.0, 9.0, 4.0, 12.0)});
	root->branches.push_back({n2, Rectangle(5.0, 0.0, 9.0, 3.0)});
	root->branches.push_back({n3, Rectangle(5.0, 4.0, 9.0, 12.0)});

	auto result = root->splitNode({1, 8.0});

	REQUIRE(result.leftBranch.child->branches.size() == 3);
	REQUIRE(result.leftBranch.boundingBox == Rectangle(0.0, 0.0, 9.0, 8.0));
	REQUIRE(result.rightBranch.child->branches.size() == 2);
	REQUIRE(result.rightBranch.boundingBox == Rectangle(0.0, 9.0, 9.0, 12.0));
}

TEST_CASE("R+Tree: testNewChildNode")
{
	rplustree::RPlusTree tree(2, 3);
	tree.insert(Point(0.0, 0.0));
	tree.insert(Point(4.0, 5.0));
	tree.insert(Point(5.0, 0.0));
	tree.insert(Point(9.0, 5.0));

	REQUIRE(tree.root->boundingBox() == Rectangle(0.0, 0.0,
                nextafter(9.0, DBL_MAX), nextafter(5.0, DBL_MAX)));
	REQUIRE(tree.root->data.size() == 0);
	REQUIRE(tree.root->branches.size() == 2);

	// test left child
	REQUIRE(tree.root->branches[0].boundingBox == Rectangle(0.0, 0.0,
                nextafter(4.0, DBL_MAX), nextafter(5.0, DBL_MAX)));
	REQUIRE(tree.root->branches[0].child->branches.size() == 0);
	REQUIRE(tree.root->branches[0].child->data.size() == 2);
	REQUIRE(tree.root->branches[0].child->parent == tree.root);

	// test right child
	REQUIRE(tree.root->branches[1].boundingBox == Rectangle(5.0, 0.0,
                nextafter(9.0, DBL_MAX), nextafter(5.0, DBL_MAX)));
	REQUIRE(tree.root->branches[1].child->branches.size() == 0);
	REQUIRE(tree.root->branches[1].child->data.size() == 2);
	REQUIRE(tree.root->branches[1].child->parent == tree.root);
}

TEST_CASE("R+Tree: testInsert")
{
	rplustree::RPlusTree tree(2, 3);

	auto *cluster1 = new rplustree::Node(tree, 2, 3, tree.root);
	cluster1->data.emplace_back(0.0, 0.0);
	cluster1->data.emplace_back(4.0, 4.0);
	tree.root->branches.push_back({cluster1, cluster1->boundingBox()});

	auto *cluster2 = new rplustree::Node(tree, 2, 3, tree.root);
	cluster2->data.emplace_back(5.0, 0.0);
	cluster2->data.emplace_back(9.0, 4.0);
	tree.root->branches.push_back({cluster2, cluster2->boundingBox()});

	auto *cluster3 = new rplustree::Node(tree, 2, 3, tree.root);
	cluster3->data.emplace_back(0.0, 5.0);
	cluster3->data.emplace_back(4.0, 9.0);
	cluster3->data.emplace_back(9.0, 9.0);
	tree.root->branches.push_back({cluster3, cluster3->boundingBox()});

	// Insert new point, causing node to overflow
	tree.insert(Point(5.0, 5.0));

	REQUIRE(tree.root->boundingBox() == Rectangle(0.0, 0.0,
                nextafter(9.0, DBL_MAX), nextafter(9.0, DBL_MAX) ));
	REQUIRE(tree.root->data.size() == 0);
	REQUIRE(tree.root->branches.size() == 2);

	REQUIRE(tree.root->branches[0].child->branches.size() == 2);
	REQUIRE(tree.root->branches[0].child->branches[0].child->data.size() == 2);
	REQUIRE(tree.root->branches[0].child->branches[1].child->data.size() == 2);
	REQUIRE(tree.root->branches[1].child->branches.size() == 2);
	REQUIRE(tree.root->branches[1].child->branches[0].child->data.size() == 2);
	REQUIRE(tree.root->branches[1].child->branches[1].child->data.size() == 2);
}

TEST_CASE("R+Tree: testSimpleRemove")
{
	rplustree::RPlusTree tree(2, 3);

	auto *cluster1a = new rplustree::Node(tree, 2, 3, tree.root);
	cluster1a->data.emplace_back(0.0, 0.0);
	cluster1a->data.emplace_back(4.0, 4.0);

	auto *cluster1b = new rplustree::Node(tree, 2, 3, tree.root);
	cluster1b->data.emplace_back(0.0, 5.0);
	cluster1b->data.emplace_back(4.0, 9.0);

	auto *cluster1c = new rplustree::Node(tree, 2, 3, tree.root);
	cluster1c->data.emplace_back(5.0, 0.0);
	cluster1c->data.emplace_back(7.0, 9.0);

	tree.root->branches.push_back({cluster1a, cluster1a->boundingBox()});
	tree.root->branches.push_back({cluster1b, cluster1b->boundingBox()});
	tree.root->branches.push_back({cluster1c, cluster1c->boundingBox()});

	// end of setup
	tree.remove(Point(7.0, 9.0));
	REQUIRE(tree.root->branches.size() == 3);
	REQUIRE(cluster1c->data.size() == 1);
}

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
