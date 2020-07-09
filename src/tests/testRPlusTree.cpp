#include <catch2/catch.hpp>
#include <rplustree/rPlusTree.h>
#include <util/geometry.h>

TEST_CASE("R+Tree: testExistence")
{
	RPlusTree tree(2, 3);

	Point p1(0.0f, 0.0f);
	tree.insert(p1);
	REQUIRE(tree.exists(p1));

	Point p2(15.0f, 15.0f);
	REQUIRE(!tree.exists(p2));
}

TEST_CASE("R+Tree: testBoundingBox")
{
	RPlusTree tree(2, 3);

	Point p1(0.0f, 0.0f);
	tree.insert(p1);
	Point p2(5.0f, 5.0f);
	tree.insert(p2);
	REQUIRE(tree.getRoot()->boundingBox == Rectangle(0.0f, 0.0f, 5.0f, 5.0f));

	Point p3(10.0f, 0.0f);
	tree.insert(p3);
	REQUIRE(tree.getRoot()->boundingBox == Rectangle(0.0f, 0.0f, 10.0f, 5.0f));
}

TEST_CASE("R+Tree: testSweepData")
{
	RPlusTree tree(2, 3);

	std::vector<Point> points;
	points.emplace_back(0.0f, 0.0f);
	points.emplace_back(4.0f, 0.0f);
	points.emplace_back(5.0f, 0.0f);
	points.emplace_back(5.0f, 5.0f);
	points.emplace_back(9.0f, 0.0f);

	// sweep along x axis
	Cost costX = RPlusTree::sweepData(points, RPlusTree::ALONG_X_AXIS);
	REQUIRE(costX.first == 0.0f);
	REQUIRE(costX.second == 4.5f);

	// sweep along y axis
	Cost costY = RPlusTree::sweepData(points, RPlusTree::ALONG_Y_AXIS);
	REQUIRE(costY.first == 0.0f);
	REQUIRE(costY.second == 2.5f);
}

TEST_CASE("R+Tree: testSweepCommon")
{
	RPlusTree tree(2, 3);

	std::vector<RPlusTreeNode*> nodes;
	nodes.push_back(new RPlusTreeNode());
	nodes.push_back(new RPlusTreeNode());
	nodes.push_back(new RPlusTreeNode());
	nodes.push_back(new RPlusTreeNode());

	nodes.at(0)->boundingBox = Rectangle(0.0f, 0.0f, 4.0f, 8.0f);
	nodes.at(1)->boundingBox = Rectangle(0.0f, 9.0f, 4.0f, 12.0f);
	nodes.at(2)->boundingBox = Rectangle(5.0f, 0.0f, 9.0f, 3.0f);
	nodes.at(3)->boundingBox = Rectangle(5.0f, 4.0f, 9.0f, 12.0f);

	// sweep along x axis
	Cost costX = RPlusTree::sweepNodes(nodes, RPlusTree::ALONG_X_AXIS);
	REQUIRE(costX.first == 0.0f);
	REQUIRE(costX.second == 5.0f);

	// sweep along y axis
	Cost costY = RPlusTree::sweepNodes(nodes, RPlusTree::ALONG_Y_AXIS);
	REQUIRE(costY.first == 1.0f);
	REQUIRE(costY.second == 4.0f);
}

TEST_CASE("R+Tree: testSweepEdge")
{
	RPlusTree tree(2, 3);

	std::vector<RPlusTreeNode*> nodes;
	nodes.push_back(new RPlusTreeNode());
	nodes.push_back(new RPlusTreeNode());
	nodes.push_back(new RPlusTreeNode());

	nodes.at(0)->boundingBox = Rectangle(0.0f, 0.0f, 2.0f, 8.0f);
	nodes.at(1)->boundingBox = Rectangle(3.0f, 0.0f, 5.0f, 4.0f);
	nodes.at(2)->boundingBox = Rectangle(6.0f, 0.0f, 8.0f, 2.0f);

	// sweep along x axis
	Cost costX = RPlusTree::sweepNodes(nodes, RPlusTree::ALONG_X_AXIS);
	REQUIRE(costX.first == 0.0f);
	REQUIRE(costX.second == 3.0f);

	// sweep along y axis
	Cost costY = RPlusTree::sweepNodes(nodes, RPlusTree::ALONG_Y_AXIS);
	REQUIRE(costY.first == 3.0f);
	REQUIRE(costY.second == 0.0f);
}

TEST_CASE("R+Tree: testNewChildNode")
{
	RPlusTree tree(2, 3);
	tree.insert(Point(0.0f, 0.0f));
	tree.insert(Point(4.0f, 5.0f));
	tree.insert(Point(5.0f, 0.0f));
	tree.insert(Point(9.0f, 5.0f));

	auto * root = tree.getRoot();
	REQUIRE(root->boundingBox == Rectangle(0.0f, 0.0f, 9.0f, 5.0f));
	REQUIRE(root->numDataEntries() == 0);
	REQUIRE(root->numChildren() == 2);

	// test left child
	REQUIRE(root->children.at(0)->boundingBox == Rectangle(0.0f, 0.0f, 4.0f, 5.0f));
	REQUIRE(root->children.at(0)->numChildren() == 0);
	REQUIRE(root->children.at(0)->numDataEntries() == 2);
	REQUIRE(root->children.at(0)->parent == root);

	// test right child
	REQUIRE(root->children.at(1)->boundingBox == Rectangle(5.0f, 0.0f, 9.0f, 5.0f));
	REQUIRE(root->children.at(1)->numChildren() == 0);
	REQUIRE(root->children.at(1)->numDataEntries() == 2);
	REQUIRE(root->children.at(1)->parent == root);
}

TEST_CASE("R+Tree: testInsert")
{
	RPlusTree tree(2, 3);
	auto * root = tree.getRoot();

	auto * cluster1 = new RPlusTreeNode();
	cluster1->data.emplace_back(0.0f, 0.0f);
	cluster1->data.emplace_back(4.0f, 4.0f);
	RPlusTree::tighten(cluster1);
	root->children.push_back(cluster1);
	cluster1->parent = root;

	auto * cluster2 = new RPlusTreeNode();
	cluster2->data.emplace_back(5.0f, 0.0f);
	cluster2->data.emplace_back(9.0f, 4.0f);
	RPlusTree::tighten(cluster2);
	root->children.push_back(cluster2);
	cluster2->parent = root;

	auto * cluster3 = new RPlusTreeNode();
	cluster3->data.emplace_back(0.0f, 5.0f);
	cluster3->data.emplace_back(4.0f, 9.0f);
	cluster3->data.emplace_back(9.0f, 9.0f);
	RPlusTree::tighten(cluster3);
	root->children.push_back(cluster3);
	cluster3->parent = root;

	// tighten root's bounding box
	RPlusTree::tighten(root);

	// insert new point, causing node to overflow
	tree.insert(Point(5.0f, 5.0f));

	root = tree.getRoot();  // refresh value
	REQUIRE(root->boundingBox == Rectangle(0.0f, 0.0f, 9.0f, 9.0f));
	REQUIRE(root->numDataEntries() == 0);
	REQUIRE(root->numChildren() == 2);

	REQUIRE(root->children.at(0)->numChildren() == 2);
	REQUIRE(root->children.at(0)->children.at(0)->numDataEntries() == 2);
	REQUIRE(root->children.at(0)->children.at(1)->numDataEntries() == 2);
	REQUIRE(root->children.at(1)->numChildren() == 2);
	REQUIRE(root->children.at(1)->children.at(0)->numDataEntries() == 2);
	REQUIRE(root->children.at(1)->children.at(1)->numDataEntries() == 2);
}

TEST_CASE("R+Tree: testSimpleRemove")
{
	RPlusTree tree(2, 3);

	auto * cluster1a = new RPlusTreeNode();
	cluster1a->data.emplace_back(0.0f, 0.0f);
	cluster1a->data.emplace_back(4.0f, 4.0f);
	RPlusTree::tighten(cluster1a);

	auto * cluster1b = new RPlusTreeNode();
	cluster1b->data.emplace_back(0.0f, 5.0f);
	cluster1b->data.emplace_back(4.0f, 9.0f);
	RPlusTree::tighten(cluster1b);

	auto * cluster1 = new RPlusTreeNode();
	cluster1->children.push_back(cluster1a);
	cluster1->children.push_back(cluster1b);
	RPlusTree::tighten(cluster1);

	cluster1a->parent = cluster1;
	cluster1b->parent = cluster1;

	auto * cluster2 = new RPlusTreeNode();
	cluster2->data.emplace_back(5.0f, 0.0f);
	cluster2->data.emplace_back(7.0f, 9.0f);
	RPlusTree::tighten(cluster2);

	auto * root = tree.getRoot();
	root->children.push_back(cluster1);
	root->children.push_back(cluster2);
	RPlusTree::tighten(root);

	cluster1->parent = root;
	cluster2->parent = root;

	// end of setup
	REQUIRE(tree.numDataElements() == 6);
	tree.remove(Point(7.0f, 9.0f));
	REQUIRE(tree.numDataElements() == 5);
	REQUIRE(tree.getRoot()->numChildren() == 2);
}

TEST_CASE("R+Tree: testChooseLeaf")
{
	Point p = Point(165.0f, 181.0f);
	RPlusTree tree(2, 3);

	auto * child1 = new RPlusTreeNode();
	child1->boundingBox = Rectangle(123.0f, 151.0f, 146.0f, 186.0f);
	auto * child2 = new RPlusTreeNode();
	child2->boundingBox = Rectangle(150.0f, 183.0f, 152.0f, 309.0f);

	auto * root = tree.getRoot();
	root->children.push_back(child1);
	root->children.push_back(child2);
	RPlusTree::tighten(root);

	auto * leaf = tree.chooseLeaf(root, p);
	REQUIRE(leaf == child2);
}
