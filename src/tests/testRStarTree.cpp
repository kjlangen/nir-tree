#include <catch2/catch.hpp>
#include <rtree/rtree.h>
#include <util/geometry.h>

TEST_CASE("R*Tree: testBoundingBox")
{
	// Test set one
	rtree::Node testNode = rtree::Node();
	testNode.boundingBoxes.push_back(Rectangle(8.0, 1.0, 12.0, 5.0));
	testNode.boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
	testNode.boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));

	REQUIRE(testNode.boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0));

	// Test set two
	rtree::Node testNode2 = rtree::Node();
	testNode2.boundingBoxes.push_back(Rectangle(8.0, 12.0, 10.0, 14.0));
	testNode2.boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
	testNode2.boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));

	REQUIRE(testNode2.boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));
}

TEST_CASE("R*Tree: testUpdateBoundingBox")
{
	rtree::Node parentNode = rtree::Node();

	rtree::Node *child0 = new rtree::Node();
	child0->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));
	parentNode.children.push_back(child0);

	rtree::Node *child1 = new rtree::Node();
	child1->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
	parentNode.children.push_back(child1);

	rtree::Node *child2 = new rtree::Node();
	child2->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
	parentNode.children.push_back(child2);

	rtree::Node *child3 = new rtree::Node();
	child3->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));
	parentNode.children.push_back(child3);

	// Test the bounding box update
	parentNode.updateBoundingBox(child3, Rectangle(3.0, 3.0, 5.0, 5.0));
	REQUIRE(parentNode.boundingBoxes[3] == Rectangle(3.0, 3.0, 5.0, 5.0));

	// Cleanup
	delete child0;
	delete child1;
	delete child2;
	delete child3;
}

TEST_CASE("R*Tree: testRemoveChild")
{
	// Setup a rtree::Node with some children
	rtree::Node parentNode = rtree::Node();

	rtree::Node *child0 = new rtree::Node();
	child0->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));
	parentNode.children.push_back(child0);

	rtree::Node *child1 = new rtree::Node();
	child1->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
	parentNode.children.push_back(child1);

	rtree::Node *child2 = new rtree::Node();
	child2->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
	parentNode.children.push_back(child2);

	rtree::Node *child3 = new rtree::Node();
	child3->parent = &parentNode;
	parentNode.boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));
	parentNode.children.push_back(child3);

	// Remove one of the children
	parentNode.removeChild(child3);
	REQUIRE(parentNode.boundingBoxes.size() == 3);
	REQUIRE(parentNode.children.size() == 3);

	// Cleanup
	delete child0;
	delete child1;
	delete child2;
	delete child3;
}

TEST_CASE("R*Tree: testRemoveData")
{
	// Setup a rtree::Node with some data
	rtree::Node parentNode = rtree::Node();

	parentNode.boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));
	parentNode.data.push_back(Point(9.0, -5.0));

	parentNode.boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
	parentNode.data.push_back(Point(14.0, -3.0));

	parentNode.boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
	parentNode.data.push_back(Point(11.0, 13.0));

	parentNode.boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));
	parentNode.data.push_back(Point(13.0, 13.0));

	// Remove some of the data
	parentNode.removeData(Point(13.0, 13.0));

	// Test the removal
	REQUIRE(parentNode.data.size() == 3);
}

TEST_CASE("R*Tree: testChooseLeaf")
{
	// Create rtree::Nodes
	 rtree::Node *root = new rtree::Node();
	 rtree::Node *left = new rtree::Node();
	 rtree::Node *right = new rtree::Node();
	 rtree::Node *leftChild0 = new rtree::Node();
	 rtree::Node *leftChild1 = new rtree::Node();
	 rtree::Node *leftChild2 = new rtree::Node();
	 rtree::Node *rightChild0 = new rtree::Node();
	 rtree::Node *rightChild1 = new rtree::Node();
	 rtree::Node *rightChild2 = new rtree::Node();

	// Setup rtree::Nodes
	leftChild0->parent = left;
	left->boundingBoxes.push_back(Rectangle(8.0, 12.0, 10.0, 14.0));
	left->children.push_back(leftChild0);

	leftChild1->parent = left;
	left->boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
	left->children.push_back(leftChild1);

	leftChild2->parent = left;
	left->boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));
	left->children.push_back(leftChild2);

	rightChild0->parent = right;
	right->boundingBoxes.push_back(Rectangle(8.0, 1.0, 12.0, 5.0));
	right->children.push_back(rightChild0);

	rightChild1->parent = right;
	right->boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
	right->children.push_back(rightChild1);

	rightChild2->parent = right;
	right->boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));
	right->children.push_back(rightChild2);

	left->parent = root;
	root->boundingBoxes.push_back(Rectangle(8.0, 12.0, 14.0, 14.0));
	root->children.push_back(left);

	right->parent = root;
	root->boundingBoxes.push_back(Rectangle(8.0, -6.0, 16.0, 5.0));
	root->children.push_back(right);

	// Test that we get the correct child for the given point
	REQUIRE(rightChild1 == root->chooseLeaf(Point(13.0, -3.0)));
	REQUIRE(leftChild0 == root->chooseLeaf(Point(8.5, 12.5)));
	REQUIRE(leftChild2 == root->chooseLeaf(Point(13.5, 13.5)));
	REQUIRE(rightChild0 == root->chooseLeaf(Point(7.0, 3.0)));
	REQUIRE(leftChild1 == root->chooseLeaf(Point(11.0, 15.0)));
	REQUIRE(leftChild0 == root->chooseLeaf(Point(4.0, 8.0)));

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

TEST_CASE("R*Tree: testFindLeaf")
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
	rtree::Node *cluster4a = new rtree::Node();
	cluster4a->data.push_back(Point(-10.0, -2.0));
	cluster4a->data.push_back(Point(-12.0, -3.0));
	cluster4a->data.push_back(Point(-11.0, -3.0));
	cluster4a->data.push_back(Point(-10.0, -3.0));

	rtree::Node *cluster4b = new rtree::Node();
	cluster4b->data.push_back(Point(-9.0, -3.0));
	cluster4b->data.push_back(Point(-7.0, -3.0));
	cluster4b->data.push_back(Point(-10.0, -5.0));

	rtree::Node *cluster4 = new rtree::Node();
	cluster4a->parent = cluster4;
	cluster4->boundingBoxes.push_back(cluster4a->boundingBox());
	cluster4->children.push_back(cluster4a);
	cluster4b->parent = cluster4;
	cluster4->boundingBoxes.push_back(cluster4b->boundingBox());
	cluster4->children.push_back(cluster4b);

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)
	// Organized into four rtree::Nodes
	rtree::Node *cluster5a = new rtree::Node();
	cluster5a->data.push_back(Point(-14.5, -13.0));
	cluster5a->data.push_back(Point(-14.0, -13.0));
	cluster5a->data.push_back(Point(-13.5, -13.5));
	cluster5a->data.push_back(Point(-15.0, -14.0));

	rtree::Node *cluster5b = new rtree::Node();
	cluster5b->data.push_back(Point(-14.0, -14.0));
	cluster5b->data.push_back(Point(-13.0, -14.0));
	cluster5b->data.push_back(Point(-12.0, -14.0));
	cluster5b->data.push_back(Point(-13.5, -16.0));

	rtree::Node *cluster5c = new rtree::Node();
	cluster5c->data.push_back(Point(-15.0, -14.5));
	cluster5c->data.push_back(Point(-14.0, -14.5));
	cluster5c->data.push_back(Point(-12.5, -14.5));
	cluster5c->data.push_back(Point(-13.5, -15.5));

	rtree::Node *cluster5d = new rtree::Node();
	cluster5d->data.push_back(Point(-15.0, -15.0));
	cluster5d->data.push_back(Point(-14.0, -15.0));
	cluster5d->data.push_back(Point(-13.0, -15.0));
	cluster5d->data.push_back(Point(-12.0, -15.0));

	rtree::Node *cluster5 = new rtree::Node();
	cluster5a->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	cluster5->children.push_back(cluster5a);
	cluster5b->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	cluster5->children.push_back(cluster5b);
	cluster5c->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	cluster5->children.push_back(cluster5c);
	cluster5d->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	cluster5->children.push_back(cluster5d);

	// Root
	rtree::Node *root = new rtree::Node();
	cluster4->parent = root;
	root->boundingBoxes.push_back(cluster4->boundingBox());
	root->children.push_back(cluster4);
	cluster5->parent = root;
	root->boundingBoxes.push_back(cluster5->boundingBox());
	root->children.push_back(cluster5);

	// Test finding leaves
	REQUIRE(root->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(root->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(root->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(root->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(root->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(root->findLeaf(Point(-13.0, -15.0)) == cluster5d);

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

TEST_CASE("R*Tree: testSplitNode")
{
	// Test set one
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	rtree::Node *cluster6 = new rtree::Node();
	cluster6->data.push_back(Point(-2.0, -6.0));
	cluster6->data.push_back(Point(2.0, -6.0));
	cluster6->data.push_back(Point(-1.0, -7.0));
	cluster6->data.push_back(Point(1.0, -7.0));
	cluster6->data.push_back(Point(3.0, -8.0));
	cluster6->data.push_back(Point(-2.0, -9.0));

	// Split the rtree::Node in two
	rtree::Node *cluster6p = cluster6->splitNode(Point(-3.0, -11.0));

	// Test the split
	REQUIRE(cluster6->data.size() == 2);
	REQUIRE(cluster6->data[0] == Point(-2.0, -6.0));
	REQUIRE(cluster6->data[1] == Point(2.0, -6.0));
	REQUIRE(cluster6p->data.size() == 5);
	REQUIRE(cluster6p->data[0] == Point(-1.0, -7.0));
	REQUIRE(cluster6p->data[1] == Point(1.0, -7.0));
	REQUIRE(cluster6p->data[2] == Point(3.0, -8.0));
	REQUIRE(cluster6p->data[3] == Point(-2.0, -9.0));
	REQUIRE(cluster6p->data[4] == Point(-3.0, -11.0));

	// Cleanup
	delete cluster6;
	delete cluster6p;

	// Test set two
	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	rtree::Node *cluster2 = new rtree::Node();
	cluster2->data.push_back(Point(-14.0, 8.0));
	cluster2->data.push_back(Point(-10.0, 8.0));
	cluster2->data.push_back(Point(-9.0, 10.0));
	cluster2->data.push_back(Point(-9.0, 9.0));
	cluster2->data.push_back(Point(-8.0, 10.0));
	cluster2->data.push_back(Point(-9.0, 7.0));
	cluster2->data.push_back(Point(-8.0, 8.0));

	// Split the rtree::Node in two
	rtree::Node *cluster2p = cluster2->splitNode(Point(-8.0, 9.0));

	// Test the split
	REQUIRE(cluster2->data.size() == 2);
	REQUIRE(cluster2->data[0] == Point(-14.0, 8.0));
	REQUIRE(cluster2->data[1] == Point(-10.0, 8.0));
	REQUIRE(cluster2p->data.size() == 6);
	REQUIRE(cluster2p->data[0] == Point(-9.0, 10.0));
	REQUIRE(cluster2p->data[1] == Point(-9.0, 9.0));
	REQUIRE(cluster2p->data[2] == Point(-8.0, 10.0));
	REQUIRE(cluster2p->data[3] == Point(-9.0, 7.0));
	REQUIRE(cluster2p->data[4] == Point(-8.0, 8.0));
	REQUIRE(cluster2p->data[5] == Point(-8.0, 9.0));

	// Cleanup
	delete cluster2;
	delete cluster2p;

	// Test set three
	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// {(-5, 4), 1, 1}, {(-2, 4), 1, 1}, {(-1, 3), 1, 1}, {(-1, 1), 1, 1}, {(-3, 0), 1, 1},
	// {(-6, 2), 1, 1}
	rtree::Node *cluster3 = new rtree::Node();
	rtree::Node *dummys[6] = {new rtree::Node(), new rtree::Node(), new rtree::Node(), new rtree::Node(), new rtree::Node(), new rtree::Node()};
	dummys[0]->parent = cluster3;
	cluster3->boundingBoxes.push_back(Rectangle(-6.0, 3.0, -4.0, 5.0));
	cluster3->children.push_back(dummys[0]);
	dummys[1]->parent = cluster3;
	cluster3->boundingBoxes.push_back(Rectangle(-3.0, 3.0, -1.0, 5.0));
	cluster3->children.push_back(dummys[1]);
	dummys[2]->parent = cluster3;
	cluster3->boundingBoxes.push_back(Rectangle(-2.0, 2.0, 0.0, 4.0));
	cluster3->children.push_back(dummys[2]);
	dummys[3]->parent = cluster3;
	cluster3->boundingBoxes.push_back(Rectangle(-2.0, 0.0, 0.0, 2.0));
	cluster3->children.push_back(dummys[3]);
	dummys[4]->parent = cluster3;
	cluster3->boundingBoxes.push_back(Rectangle(-4.0, -1.0, -2.0, 1.0));
	cluster3->children.push_back(dummys[4]);
	dummys[5]->parent = cluster3;
	cluster3->boundingBoxes.push_back(Rectangle(-7.0, 1.0, -5.0, 3.0));
	cluster3->children.push_back(dummys[5]);

	// Extra rtree::Node causing the split
	rtree::Node *cluster3extra = new rtree::Node();
	cluster3extra->data.push_back(Point(1.0, 1.0));
	cluster3extra->data.push_back(Point(2.0, 2.0));

	// Test the split
	rtree::Node *cluster3p = cluster3->splitNode(cluster3extra);

	REQUIRE(cluster3->children.size() == 5);
	REQUIRE(cluster3->boundingBoxes[0] == Rectangle(-4.0, -1.0, -2.0, 1.0));
	REQUIRE(cluster3->boundingBoxes[1] == Rectangle(-3.0, 3.0, -1.0, 5.0));
	REQUIRE(cluster3->boundingBoxes[2] == Rectangle(-2.0, 2.0, 0.0, 4.0));
	REQUIRE(cluster3->boundingBoxes[3] == Rectangle(-2.0, 0.0, 0.0, 2.0));
	REQUIRE(cluster3->boundingBoxes[4] == Rectangle(1.0, 1.0, 2.0, 2.0));
	REQUIRE(cluster3->children[4] == cluster3extra);

	REQUIRE(cluster3p->children.size() == 2);
	REQUIRE(cluster3p->boundingBoxes[0] == Rectangle(-6.0, 3.0, -4.0, 5.0));
	REQUIRE(cluster3p->boundingBoxes[1] == Rectangle(-7.0, 1.0, -5.0, 3.0));

	// Cleanup
	delete cluster3;
	delete cluster3p;
	delete cluster3extra;
	delete dummys[0];
	delete dummys[1];
	delete dummys[2];
	delete dummys[3];
	delete dummys[4];
	delete dummys[5];
}

TEST_CASE("R*Tree: testAdjustTree")
{
	// Leaf rtree::Node and new sibling leaf
	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	rtree::Node *cluster4a = new rtree::Node();
	cluster4a->data.push_back(Point(-10.0, -2.0));
	cluster4a->data.push_back(Point(-12.0, -3.0));
	cluster4a->data.push_back(Point(-11.0, -3.0));
	cluster4a->data.push_back(Point(-10.0, -3.0));

	rtree::Node *cluster4b = new rtree::Node();
	cluster4b->data.push_back(Point(-9.0, -3.0));
	cluster4b->data.push_back(Point(-7.0, -3.0));
	cluster4b->data.push_back(Point(-10.0, -5.0));

	// Middle rtree::Node
	rtree::Node *middle = new rtree::Node();
	rtree::Node *dummys[4] = {new rtree::Node(), new rtree::Node(), new rtree::Node(), new rtree::Node()};
	dummys[0]->parent = middle;
	middle->boundingBoxes.push_back(Rectangle(-10.0, 2.0, -8.0, 4.0));
	middle->children.push_back(dummys[0]);
	dummys[1]->parent = middle;
	middle->boundingBoxes.push_back(Rectangle(-12.0, -10.0, -10.0, -8.0));
	middle->children.push_back(dummys[1]);
	cluster4a->parent = middle;
	middle->boundingBoxes.push_back(Rectangle(-12.0, -5.0, -7.0, -2.0));
	middle->children.push_back(cluster4a);
	dummys[2]->parent = middle;
	middle->boundingBoxes.push_back(Rectangle(6.0, 6.0, 8.0, 8.0));
	middle->children.push_back(dummys[2]);
	dummys[3]->parent = middle;
	middle->boundingBoxes.push_back(Rectangle(15.0, -17.0, 17.0, -15.0));
	middle->children.push_back(dummys[3]);

	// Root rtree::Node
	rtree::Node *root = new rtree::Node();
	middle->parent = root;
	root->boundingBoxes.push_back(middle->boundingBox());
	root->children.push_back(middle);

	// Adjust the tree
	rtree::Node *result = cluster4a->adjustTree(cluster4b);

	// Test the adjustment
	REQUIRE(result == nullptr);
	REQUIRE(root->children.size() == 2);
	REQUIRE(root->boundingBoxes[0] == Rectangle(-12.0, -10.0, -7.0, 4.0));
	REQUIRE(root->boundingBoxes[1] == Rectangle(6.0, -17.0, 17.0, 8.0));
	REQUIRE(middle->children.size() == 4);
	REQUIRE(middle->boundingBoxes[2] == Rectangle(-12.0, -3.0, -10.0, -2.0));
	REQUIRE(middle->boundingBoxes[3] == Rectangle(-10.0, -5.0, -7.0, -3.0));

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

TEST_CASE("R*Tree: testCondenseTree")
{
	// Test where the leaf is the root
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	rtree::Node *cluster6 = new rtree::Node();
	cluster6->data.push_back(Point(-2.0, -6.0));
	cluster6->data.push_back(Point(2.0, -6.0));
	cluster6->data.push_back(Point(-1.0, -7.0));
	cluster6->data.push_back(Point(1.0, -7.0));
	cluster6->data.push_back(Point(3.0, -8.0));
	cluster6->data.push_back(Point(-2.0, -9.0));
	cluster6->data.push_back(Point(-3.0, -11.0));

	// Condense the tree
	cluster6->condenseTree();

	// Test the condensing
	REQUIRE(cluster6->parent == nullptr);
	REQUIRE(cluster6->boundingBoxes.size() == 0);
	REQUIRE(cluster6->children.size() == 0);
	REQUIRE(cluster6->data.size() == 7);
	REQUIRE(cluster6->data[0] == Point(-2.0, -6.0));
	REQUIRE(cluster6->data[6] == Point(-3.0, -11.0));

	// Cleanup
	delete cluster6;

	// Test where condensing is confined to a leaf != root
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	cluster6 = new rtree::Node();
	cluster6->data.push_back(Point(-2.0, -6.0));
	cluster6->data.push_back(Point(2.0, -6.0));
	cluster6->data.push_back(Point(-1.0, -7.0));
	cluster6->data.push_back(Point(1.0, -7.0));
	cluster6->data.push_back(Point(3.0, -8.0));
	cluster6->data.push_back(Point(-2.0, -9.0));
	// (-3, -11) left out so the bounding box should change

	rtree::Node *root = new rtree::Node();
	cluster6->parent = root;
	root->boundingBoxes.push_back(Rectangle(-3.0, -10.0, 3.0, 6.0));
	root->children.push_back(cluster6);

	// Condense the tree
	cluster6->condenseTree();

	// Test the condensing
	REQUIRE(root->parent == nullptr);
	REQUIRE(root->boundingBoxes.size() == 1);
	REQUIRE(root->children.size() == 1);
	REQUIRE(root->boundingBoxes[0] == Rectangle(-2.0, -9.0, 3.0, -6.0));
	REQUIRE(root->children[0] == cluster6);
	REQUIRE(cluster6->parent == root);
	REQUIRE(cluster6->boundingBoxes.size() == 0);
	REQUIRE(cluster6->children.size() == 0);
	REQUIRE(cluster6->data.size() == 6);

	// Cleanup
	delete cluster6;
	delete root;

	// Test where condensing is unconfined to a leaf != root
	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
	rtree::Node *cluster4a = new rtree::Node();
	cluster4a->data.push_back(Point(-10.0, -2.0));
	cluster4a->data.push_back(Point(-12.0, -3.0));
	cluster4a->data.push_back(Point(-11.0, -3.0));
	cluster4a->data.push_back(Point(-10.0, -3.0));

	rtree::Node *cluster4b = new rtree::Node();
	cluster4b->data.push_back(Point(-9.0, -3.0));
	cluster4b->data.push_back(Point(-7.0, -3.0));
	// cluster4b->data.push_back(Point(-10.0, -5.0)); left out to precipitate condensing

	rtree::Node *cluster4 = new rtree::Node();
	cluster4a->parent = cluster4;
	cluster4->boundingBoxes.push_back(cluster4a->boundingBox());
	cluster4->children.push_back(cluster4a);
	cluster4b->parent = cluster4;
	cluster4->boundingBoxes.push_back(cluster4b->boundingBox());
	cluster4->children.push_back(cluster4b);

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)
	// Organized into four rtree::Nodes
	rtree::Node *cluster5a = new rtree::Node();
	cluster5a->data.push_back(Point(-14.5, -13.0));
	cluster5a->data.push_back(Point(-14.0, -13.0));
	cluster5a->data.push_back(Point(-13.5, -13.5));
	cluster5a->data.push_back(Point(-15.0, -14.0));
	rtree::Node *cluster5b = new rtree::Node();
	cluster5b->data.push_back(Point(-14.0, -14.0));
	cluster5b->data.push_back(Point(-13.0, -14.0));
	cluster5b->data.push_back(Point(-12.0, -14.0));
	cluster5b->data.push_back(Point(-13.5, -16.0));
	rtree::Node *cluster5c = new rtree::Node();
	cluster5c->data.push_back(Point(-15.0, -14.5));
	cluster5c->data.push_back(Point(-14.0, -14.5));
	cluster5c->data.push_back(Point(-12.5, -14.5));
	cluster5c->data.push_back(Point(-13.5, -15.5));
	rtree::Node *cluster5d = new rtree::Node();
	cluster5d->data.push_back(Point(-15.0, -15.0));
	cluster5d->data.push_back(Point(-14.0, -15.0));
	cluster5d->data.push_back(Point(-13.0, -15.0));
	cluster5d->data.push_back(Point(-12.0, -15.0));
	rtree::Node *cluster5 = new rtree::Node();
	cluster5a->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	cluster5->children.push_back(cluster5a);
	cluster5b->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	cluster5->children.push_back(cluster5b);
	cluster5c->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	cluster5->children.push_back(cluster5c);
	cluster5d->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	cluster5->children.push_back(cluster5d);

	// Root
	root = new rtree::Node();
	cluster4->parent = root;
	root->boundingBoxes.push_back(cluster4->boundingBox());
	root->children.push_back(cluster4);
	cluster5->parent = root;
	root->boundingBoxes.push_back(cluster5->boundingBox());
	root->children.push_back(cluster5);

	// Condense the tree
	rtree::Node *newRoot = cluster4b->condenseTree();

	// Test the condensing
	REQUIRE(newRoot == root);
	REQUIRE(root->boundingBoxes.size() == 2);
	REQUIRE(root->children.size() == 2);
	REQUIRE(root->children[0]->children.size() == 4);
	REQUIRE(root->children[1]->children.size() == 2);
	REQUIRE(root->children[1]->children[0]->data.size() == 2);
	REQUIRE(root->children[1]->children[0]->data[0] == Point(-9.0, -3.0));
	REQUIRE(root->children[1]->children[0]->data[1] == Point(-7.0, -3.0));

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

TEST_CASE("R*Tree: testSearch")
{
	// Build the tree directly

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
	rtree::Node *cluster1a = new rtree::Node();
	cluster1a->data.push_back(Point(-3.0, 16.0));
	cluster1a->data.push_back(Point(-3.0, 15.0));
	cluster1a->data.push_back(Point(-4.0, 13.0));

	rtree::Node *cluster1b = new rtree::Node();
	cluster1b->data.push_back(Point(-5.0, 12.0));
	cluster1b->data.push_back(Point(-5.0, 15.0));
	cluster1b->data.push_back(Point(-6.0, 14.0));
	cluster1b->data.push_back(Point(-8.0, 16.0));

	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	rtree::Node *cluster2a = new rtree::Node();
	cluster2a->data.push_back(Point(-8.0, 10.0));
	cluster2a->data.push_back(Point(-9.0, 10.0));
	cluster2a->data.push_back(Point(-8.0, 9.0));
	cluster2a->data.push_back(Point(-9.0, 9.0));
	cluster2a->data.push_back(Point(-8.0, 8.0));

	rtree::Node *cluster2b = new rtree::Node();
	cluster2b->data.push_back(Point(-14.0, 8.0));
	cluster2b->data.push_back(Point(-10.0, 8.0));
	cluster2b->data.push_back(Point(-9.0, 7.0));

	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	rtree::Node *cluster3a = new rtree::Node();
	cluster3a->data.push_back(Point(-3.0, 4.0));
	cluster3a->data.push_back(Point(-3.0, 0.0));
	cluster3a->data.push_back(Point(-2.0, 4.0));
	cluster3a->data.push_back(Point(-1.0, 3.0));
	cluster3a->data.push_back(Point(-1.0, 1.0));

	rtree::Node *cluster3b = new rtree::Node();
	cluster3b->data.push_back(Point(-5.0, 4.0));
	cluster3b->data.push_back(Point(-4.0, 3.0));
	cluster3b->data.push_back(Point(-4.0, 1.0));
	cluster3b->data.push_back(Point(-6.0, 2.0));

	// High level rtree::Nodes
	rtree::Node *left = new rtree::Node();
	cluster1a->parent = left;
	left->boundingBoxes.push_back(cluster1a->boundingBox());
	left->children.push_back(cluster1a);
	cluster1b->parent = left;
	left->boundingBoxes.push_back(cluster1b->boundingBox());
	left->children.push_back(cluster1b);
	cluster2a->parent = left;
	left->boundingBoxes.push_back(cluster2a->boundingBox());
	left->children.push_back(cluster2a);
	cluster2b->parent = left;
	left->boundingBoxes.push_back(cluster2b->boundingBox());
	left->children.push_back(cluster2b);

	rtree::Node *right = new rtree::Node();
	cluster3a->parent = right;
	right->boundingBoxes.push_back(cluster3a->boundingBox());
	right->children.push_back(cluster3a);
	cluster3b->parent = right;
	right->boundingBoxes.push_back(cluster3b->boundingBox());
	right->children.push_back(cluster3b);

	rtree::Node *root = new rtree::Node();
	left->parent = root;
	root->boundingBoxes.push_back(left->boundingBox());
	root->children.push_back(left);
	right->parent = root;
	root->boundingBoxes.push_back(right->boundingBox());
	root->children.push_back(right);

	// Test search

	// Test set one
	Rectangle sr1 = Rectangle(-9.0, 9.5, -5.0, 12.5);
	std::vector<Point> v1 = root->search(sr1);
	REQUIRE(v1.size() == 3);
	REQUIRE(v1[0] == Point(-8.0, 10.0));
	REQUIRE(v1[1] == Point(-9.0, 10.0));
	REQUIRE(v1[2] == Point(-5.0, 12.0));

	// Test set two
	Rectangle sr2 = Rectangle(-8.0, 4.0, -5.0, 8.0);
	std::vector<Point> v2 = root->search(sr2);
	REQUIRE(v2.size() == 2);
	REQUIRE(v2[0] == Point(-5.0, 4.0));
	REQUIRE(v2[1] == Point(-8.0, 8.0));

	// Test set three
	Rectangle sr3 = Rectangle(-8.0, 0.0, -4.0, 16.0);
	std::vector<Point> v3 = root->search(sr3);
	REQUIRE(v3.size() == 12);
	REQUIRE(v3[0] == Point(-5.0, 4.0));
	REQUIRE(v3[1] == Point(-4.0, 3.0));
	REQUIRE(v3[2] == Point(-4.0, 1.0));
	REQUIRE(v3[3] == Point(-6.0, 2.0));
	REQUIRE(v3[4] == Point(-8.0, 10.0));
	REQUIRE(v3[5] == Point(-8.0, 9.0));
	REQUIRE(v3[6] == Point(-8.0, 8.0));
	REQUIRE(v3[7] == Point(-5.0, 12.0));
	REQUIRE(v3[8] == Point(-5.0, 15.0));
	REQUIRE(v3[9] == Point(-6.0, 14.0));
	REQUIRE(v3[10] == Point(-8.0, 16.0));
	REQUIRE(v3[11] == Point(-4.0, 13.0));

	// Test set four
	Rectangle sr4 = Rectangle(2.0, -4.0, 4.0, -2.0);
	std::vector<Point> v4 = root->search(sr4);
	REQUIRE(v4.size() == 0);

	// Test set five
	Rectangle sr5 = Rectangle(-3.5, 1.0, -1.5, 3.0);
	std::vector<Point> v5 = root->search(sr5);
	REQUIRE(v5.size() == 0);

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

TEST_CASE("R*Tree: testInsert")
{
	// Setup the tree
	rtree::Node *root = new rtree::Node();

	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	root = root->insert(Point(-14.0, 8.0));
	root = root->insert(Point(-10.0, 8.0));
	root = root->insert(Point(-9.0, 10.0));
	root = root->insert(Point(-9.0, 9.0));
	root = root->insert(Point(-8.0, 10.0));
	root = root->insert(Point(-9.0, 7.0));
	root = root->insert(Point(-8.0, 8.0));
	root = root->insert(Point(-8.0, 9.0));

	// Test set one
	REQUIRE(root->boundingBoxes.size() == 2);
	REQUIRE(root->children.size() == 2);
	REQUIRE(root->children[0]->parent == root);
	REQUIRE(root->children[0]->boundingBoxes.size() == 0);
	REQUIRE(root->children[0]->children.size() == 0);
	REQUIRE(root->children[0]->data.size() == 3);
	REQUIRE(root->children[1]->parent == root);
	REQUIRE(root->children[1]->boundingBoxes.size() == 0);
	REQUIRE(root->children[1]->children.size() == 0);
	REQUIRE(root->children[1]->data.size() == 5);

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
	root = root->insert(Point(-8.0, 16.0));
	root = root->insert(Point(-3.0, 16.0));
	root = root->insert(Point(-5.0, 15.0));
	root = root->insert(Point(-3.0, 15.0));
	root = root->insert(Point(-6.0, 14.0));
	root = root->insert(Point(-4.0, 13.0));
	root = root->insert(Point(-5.0, 12.0));

	// Test set two
	REQUIRE(root->boundingBoxes.size() == 2);
	REQUIRE(root->children.size() == 2);
	REQUIRE(root->children[0]->parent == root);
	REQUIRE(root->children[0]->boundingBoxes.size() == 3);
	REQUIRE(root->children[0]->children.size() == 3);
	REQUIRE(root->children[0]->data.size() == 0);
	REQUIRE(root->children[1]->parent == root);
	REQUIRE(root->children[1]->boundingBoxes.size() == 3);
	REQUIRE(root->children[1]->children.size() == 3);
	REQUIRE(root->children[1]->data.size() == 0);

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	root = root->insert(Point(-10.0, -2.0));
	root = root->insert(Point(-12.0, -3.0));
	root = root->insert(Point(-11.0, -3.0));
	root = root->insert(Point(-10.0, -3.0));
	root = root->insert(Point(-10.0, -3.0));
	root = root->insert(Point(-9.0, -3.0));
	root = root->insert(Point(-7.0, -3.0));
	root = root->insert(Point(-10.0, -5.0));

	// Test set three
	REQUIRE(root->boundingBoxes.size() == 2);
	REQUIRE(root->children.size() == 2);
	REQUIRE(root->children[0]->parent == root);
	REQUIRE(root->children[0]->boundingBoxes.size() == 5);
	REQUIRE(root->children[0]->children.size() == 5);
	REQUIRE(root->children[0]->data.size() == 0);
	REQUIRE(root->children[1]->parent == root);
	REQUIRE(root->children[1]->boundingBoxes.size() == 3);
	REQUIRE(root->children[1]->children.size() == 3);
	REQUIRE(root->children[1]->data.size() == 0);

	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	root = root->insert(Point(-5.0, 4.0));
	root = root->insert(Point(-3.0, 4.0));
	root = root->insert(Point(-2.0, 4.0));
	root = root->insert(Point(-4.0, 3.0));
	root = root->insert(Point(-1.0, 3.0));
	root = root->insert(Point(-6.0, 2.0));
	root = root->insert(Point(-4.0, 1.0));
	root = root->insert(Point(-3.0, 0.0));
	root = root->insert(Point(-1.0, 1.0));

	// Test set four
	REQUIRE(root->boundingBoxes.size() == 3);
	REQUIRE(root->children.size() == 3);
	REQUIRE(root->children[0]->parent == root);
	REQUIRE(root->children[0]->boundingBoxes.size() == 5);
	REQUIRE(root->children[0]->children.size() == 5);
	REQUIRE(root->children[0]->data.size() == 0);
	REQUIRE(root->children[1]->parent == root);
	REQUIRE(root->children[1]->boundingBoxes.size() == 3);
	REQUIRE(root->children[1]->children.size() == 3);
	REQUIRE(root->children[1]->data.size() == 0);
	REQUIRE(root->children[2]->parent == root);
	REQUIRE(root->children[2]->boundingBoxes.size() == 3);
	REQUIRE(root->children[2]->children.size() == 3);
	REQUIRE(root->children[2]->data.size() == 0);

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

TEST_CASE("R*Tree: testRemove")
{
	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)
	// Organized into four rtree::Nodes
	rtree::Node *cluster5a = new rtree::Node();
	cluster5a->data.push_back(Point(-14.5, -13.0));
	cluster5a->data.push_back(Point(-14.0, -13.0));
	cluster5a->data.push_back(Point(-13.5, -13.5));
	cluster5a->data.push_back(Point(-15.0, -14.0));

	rtree::Node *cluster5b = new rtree::Node();
	cluster5b->data.push_back(Point(-14.0, -14.0));
	cluster5b->data.push_back(Point(-13.0, -14.0));
	cluster5b->data.push_back(Point(-12.0, -14.0));
	cluster5b->data.push_back(Point(-13.5, -16.0));

	rtree::Node *cluster5c = new rtree::Node();
	cluster5c->data.push_back(Point(-15.0, -14.5));
	cluster5c->data.push_back(Point(-14.0, -14.5));
	cluster5c->data.push_back(Point(-12.5, -14.5));
	cluster5c->data.push_back(Point(-13.5, -15.5));

	rtree::Node *cluster5d = new rtree::Node();
	cluster5d->data.push_back(Point(-15.0, -15.0));
	cluster5d->data.push_back(Point(-14.0, -15.0));
	cluster5d->data.push_back(Point(-13.0, -15.0));
	cluster5d->data.push_back(Point(-12.0, -15.0));

	rtree::Node *cluster5 = new rtree::Node();
	cluster5a->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	cluster5->children.push_back(cluster5a);
	cluster5b->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	cluster5->children.push_back(cluster5b);
	cluster5c->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	cluster5->children.push_back(cluster5c);
	cluster5c->parent = cluster5;
	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	cluster5->children.push_back(cluster5d);

	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	rtree::Node *cluster3a = new rtree::Node();
	cluster3a->data.push_back(Point(-5.0, 4.0));
	cluster3a->data.push_back(Point(-3.0, 4.0));
	cluster3a->data.push_back(Point(-2.0, 4.0));

	rtree::Node *cluster3b = new rtree::Node();
	cluster3b->data.push_back(Point(-4.0, 1.0));
	cluster3b->data.push_back(Point(-3.0, 0.0));
	cluster3b->data.push_back(Point(-1.0, 1.0));

	rtree::Node *cluster3c = new rtree::Node();
	cluster3c->data.push_back(Point(-4.0, 3.0));
	cluster3c->data.push_back(Point(-1.0, 3.0));
	cluster3c->data.push_back(Point(-6.0, 2.0));

	rtree::Node *cluster3 = new rtree::Node();
	cluster3a->parent = cluster3;
	cluster3->boundingBoxes.push_back(cluster3a->boundingBox());
	cluster3->children.push_back(cluster3a);
	cluster3b->parent = cluster3;
	cluster3->boundingBoxes.push_back(cluster3b->boundingBox());
	cluster3->children.push_back(cluster3b);
	cluster3c->parent = cluster3;
	cluster3->boundingBoxes.push_back(cluster3c->boundingBox());
	cluster3->children.push_back(cluster3c);

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12), + (-4.5, 15.5), (-2.0, 13.5)
	rtree::Node *cluster1a = new rtree::Node();
	cluster1a->data.push_back(Point(-3.0, 16.0));
	cluster1a->data.push_back(Point(-3.0, 15.0));
	cluster1a->data.push_back(Point(-4.0, 13.0));

	rtree::Node *cluster1b = new rtree::Node();
	cluster1b->data.push_back(Point(-5.0, 12.0));
	cluster1b->data.push_back(Point(-5.0, 15.0));
	cluster1b->data.push_back(Point(-6.0, 14.0));

	rtree::Node *cluster1c = new rtree::Node();
	cluster1c->data.push_back(Point(-8.0, 16.0));
	cluster1c->data.push_back(Point(-4.5, 15.5));
	cluster1c->data.push_back(Point(-2.0, 13.5));

	rtree::Node *cluster1 = new rtree::Node();
	cluster1a->parent = cluster1;
	cluster1->boundingBoxes.push_back(cluster1a->boundingBox());
	cluster1->children.push_back(cluster1a);
	cluster1b->parent = cluster1;
	cluster1->boundingBoxes.push_back(cluster1b->boundingBox());
	cluster1->children.push_back(cluster1b);
	cluster1c->parent = cluster1;
	cluster1->boundingBoxes.push_back(cluster1c->boundingBox());
	cluster1->children.push_back(cluster1c);

	// Root
	rtree::Node *root = new rtree::Node();
	cluster3->parent = root;
	root->boundingBoxes.push_back(cluster3->boundingBox());
	root->children.push_back(cluster3);
	cluster5->parent = root;
	root->boundingBoxes.push_back(cluster5->boundingBox());
	root->children.push_back(cluster5);
	cluster1->parent = root;
	root->boundingBoxes.push_back(cluster1->boundingBox());
	root->children.push_back(cluster1);

	REQUIRE(root->boundingBoxes.size() == 3);

	// Remove an element, no other changes in tree
	root->remove(Point(-12.0, -15.0));

	// Test the removal
	REQUIRE(cluster5d->data.size() == 3);
	REQUIRE(cluster5d->data[0] == Point(-15.0, -15.0));
	REQUIRE(cluster5d->data[1] == Point(-14.0, -15.0));
	REQUIRE(cluster5d->data[2] == Point(-13.0, -15.0));
	REQUIRE(cluster5->parent == root);
	REQUIRE(cluster5->boundingBoxes.size() == 4);
	REQUIRE(cluster5->children.size() == 4);
	REQUIRE(root->parent == nullptr);
	REQUIRE(root->boundingBoxes.size() == 3);
	REQUIRE(root->children.size() == 3);

	// Remove an element, parent is altered
	root->remove(Point(-6.0, 2.0));

	// Test the removal
	REQUIRE(root->boundingBoxes.size() == 2);
	REQUIRE(root->children.size() == 2);
	REQUIRE(root->children[0]->parent == root);
	REQUIRE(root->children[0]->boundingBoxes.size() == 4);
	REQUIRE(root->children[0]->children.size() == 4);
	REQUIRE(root->children[1]->parent == root);
	REQUIRE(root->children[1]->boundingBoxes.size() == 5);
	REQUIRE(root->children[1]->children.size() == 5);

	// Cleanup
	root->deleteSubtrees();
	delete root;

	// Remove an element, tree shrinks

	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	rtree::Node *cluster6a = new rtree::Node();
	cluster6a->data.push_back(Point(-2.0, -6.0));
	cluster6a->data.push_back(Point(2.0, -6.0));
	cluster6a->data.push_back(Point(-1.0, -7.0));

	rtree::Node *cluster6b = new rtree::Node();
	cluster6b->data.push_back(Point(1.0, -7.0));
	cluster6b->data.push_back(Point(3.0, -8.0));
	cluster6b->data.push_back(Point(-2.0, -9.0));
	// (-3.0, -11.0) held out so we get a shrinking root

	// Root
	root = new rtree::Node();
	cluster6a->parent = root;
	root->boundingBoxes.push_back(cluster6a->boundingBox());
	root->children.push_back(cluster6a);
	cluster6b->parent = root;
	root->boundingBoxes.push_back(cluster6b->boundingBox());
	root->children.push_back(cluster6b);

	// Remove an element, the tree should shrink and cluster6a should be the new root
	root = root->remove(Point(3.0, -8.0));

	// Test the removal
	REQUIRE(root == cluster6a);
	REQUIRE(root->boundingBoxes.size() == 0);
	REQUIRE(root->children.size() == 0);
	REQUIRE(root->data.size() == 5);

	// Cleanup
	root->deleteSubtrees();
	delete root;
}
