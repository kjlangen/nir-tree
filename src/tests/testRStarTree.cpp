#include <catch2/catch.hpp>
#include <rstartree/rstartree.h>
#include <util/geometry.h>

static rstartree::Node::NodeEntry createBranchEntry( const Rectangle &boundingBox, rstartree::Node *child ) {
    rstartree::Node::Branch b( boundingBox, child );
    return b;
}

static rstartree::Node *createFullLeafNode( const rstartree::RStarTree &treeRef, Point p=Point(0.0,0.0) ) {
	rstartree::Node *node = new rstartree::Node( treeRef );
	std::vector<bool> reInsertedAtLevel = { false };

	for(unsigned i = 0; i < treeRef.maxBranchFactor; i++)
	{
		node = node->insert(p, reInsertedAtLevel);
		REQUIRE(reInsertedAtLevel[0] == false);
	}
	return node;
}

TEST_CASE("R*Tree: testBoundingBox")
{
	// Test set one
    rstartree::RStarTree tree(3,5);
	rstartree::Node *testNode = tree.root;

    rstartree::Node *child0 = new rstartree::Node(tree);
	testNode->entries.push_back(createBranchEntry( Rectangle(8.0, 1.0, 12.0, 5.0), child0));
    rstartree::Node *child1 = new rstartree::Node(tree);
	testNode->entries.push_back(createBranchEntry( Rectangle(12.0, -4.0, 16.0, -2.0), child1));
    rstartree::Node *child2 = new rstartree::Node(tree);
	testNode->entries.push_back(createBranchEntry( Rectangle(8.0, -6.0, 10.0, -4.0), child2));

	REQUIRE(testNode->boundingBox() == Rectangle(8.0, -6.0, 16.0, 5.0));

	// Test set two
    rstartree::RStarTree tree2(3,5);
	rstartree::Node *testNode2 = tree2.root;
    child0 = new rstartree::Node(tree);
	testNode2->entries.push_back(createBranchEntry(Rectangle(8.0, 12.0, 10.0, 14.0), child0));
    child1 = new rstartree::Node(tree);
	testNode2->entries.push_back(createBranchEntry(Rectangle(10.0, 12.0, 12.0, 14.0), child1));
    child2 = new rstartree::Node(tree);
	testNode2->entries.push_back(createBranchEntry(Rectangle(12.0, 12.0, 14.0, 14.0), child2));

	REQUIRE(testNode2->boundingBox() == Rectangle(8.0, 12.0, 14.0, 14.0));

}

TEST_CASE("R*Tree: testUpdateBoundingBox")
{
    rstartree::RStarTree tree(3,5);
	rstartree::Node *parentNode = tree.root;

	rstartree::Node *child0 = new rstartree::Node(tree);
	child0->parent = parentNode;
    parentNode->entries.push_back(createBranchEntry(Rectangle(8.0, -6.0, 10.0, -4.0), child0));

	rstartree::Node *child1 = new rstartree::Node(tree);
	child1->parent = parentNode;
	parentNode->entries.push_back(createBranchEntry(Rectangle(12.0, -4.0, 16.0, -2.0), child1));

	rstartree::Node *child2 = new rstartree::Node(tree);
	child2->parent = parentNode;
    parentNode->entries.push_back(createBranchEntry(Rectangle(10.0, 12.0, 12.0, 14.0), child2));

	rstartree::Node *child3 = new rstartree::Node(tree);
	child3->parent = parentNode;
    parentNode->entries.push_back(createBranchEntry(Rectangle(12.0, 12.0, 14.0, 14.0), child3));

	// Test the bounding box update
	parentNode->updateBoundingBox(child3, Rectangle(3.0, 3.0, 5.0, 5.0));
    const rstartree::Node::Branch &b = std::get<rstartree::Node::Branch>( parentNode->entries[3] );
	REQUIRE(b.boundingBox == Rectangle(3.0, 3.0, 5.0, 5.0));
}

TEST_CASE("R*Tree: testRemoveChild")
{
	// Setup a rtree::Node with some children
    rstartree::RStarTree tree(3,5);
	rstartree::Node *parentNode = tree.root;

	rstartree::Node *child0 = new rstartree::Node(tree);
	child0->parent = parentNode;
    parentNode->entries.push_back(createBranchEntry(Rectangle(8.0, -6.0, 10.0, -4.0), child0));

	rstartree::Node *child1 = new rstartree::Node(tree);
	child1->parent = parentNode;
	parentNode->entries.push_back(createBranchEntry(Rectangle(12.0, -4.0, 16.0, -2.0), child1));

	rstartree::Node *child2 = new rstartree::Node(tree);
	child2->parent = parentNode;
    parentNode->entries.push_back(createBranchEntry(Rectangle(10.0, 12.0, 12.0, 14.0), child2));

	rstartree::Node *child3 = new rstartree::Node(tree);
	child3->parent = parentNode;
    parentNode->entries.push_back(createBranchEntry(Rectangle(12.0, 12.0, 14.0, 14.0), child3));

	// Remove one of the children
	parentNode->removeChild(child3);
	REQUIRE(parentNode->entries.size() == 3);

    delete child3;

}

TEST_CASE("R*Tree: testRemoveData")
{
	// Setup a rtree::Node with some data
    rstartree::RStarTree tree(3,5);
	rstartree::Node *parentNode = tree.root;

	parentNode->entries.push_back(Point(9.0, -5.0));
	parentNode->entries.push_back(Point(14.0, -3.0));
	parentNode->entries.push_back(Point(11.0, 13.0));
	parentNode->entries.push_back(Point(13.0, 13.0));

	// Remove some of the data
	parentNode->removeData(Point(13.0, 13.0));

	// Test the removal
	REQUIRE(parentNode->entries.size() == 3);
}

TEST_CASE("R*Tree: testChooseLeaf")
{
	// Create rtree::Nodes
    rstartree::RStarTree tree(3,5);
	rstartree::Node *root= tree.root;
    rstartree::Node *left = new rstartree::Node(tree);
    rstartree::Node *right = new rstartree::Node(tree);
    rstartree::Node *leftChild0 = createFullLeafNode(tree);
    rstartree::Node *leftChild1 = createFullLeafNode(tree);
    rstartree::Node *leftChild2 = createFullLeafNode(tree);
    rstartree::Node *rightChild0 = createFullLeafNode(tree);
    rstartree::Node *rightChild1 = createFullLeafNode(tree);
    rstartree::Node *rightChild2 = createFullLeafNode(tree);

	// Setup rtree::Nodes
    // NB: All of these bounding rectangles are wrong, but that's fine for the purposes of this test.
	leftChild0->parent = left;
	leftChild0->level = 2;
	left->entries.push_back(createBranchEntry(Rectangle(8.0, 12.0, 10.0, 14.0), leftChild0));

	leftChild1->parent = left;
	leftChild1->level = 2;
	left->entries.push_back(createBranchEntry(Rectangle(10.0, 12.0, 12.0, 14.0), leftChild1));

	leftChild2->parent = left;
	leftChild2->level = 2;
	left->entries.push_back(createBranchEntry(Rectangle(12.0, 12.0, 14.0, 14.0), leftChild2));

	rightChild0->parent = right;
	rightChild0->level = 2;
	right->entries.push_back(createBranchEntry(Rectangle(8.0, 1.0, 12.0, 5.0), rightChild0));

	rightChild1->parent = right;
	rightChild1->level = 2;
	right->entries.push_back(createBranchEntry(Rectangle(12.0, -4.0, 16.0, -2.0), rightChild1));

	rightChild2->parent = right;
	rightChild2->level = 2;
	right->entries.push_back(createBranchEntry(Rectangle(8.0, -6.0, 10.0, -4.0), rightChild2));

	left->parent = root;
	left->level = 1;
	root->entries.push_back(createBranchEntry(Rectangle(8.0, 12.0, 14.0, 14.0), left));

	right->parent = root;
	right->level = 1;
	root->entries.push_back(createBranchEntry(Rectangle(8.0, -6.0, 16.0, 5.0), right));

    REQUIRE( !root->entries.empty() );
    REQUIRE( !left->entries.empty() );
    REQUIRE( !right->entries.empty() );
    REQUIRE( !leftChild0->entries.empty() );
    REQUIRE( !leftChild1->entries.empty() );
    REQUIRE( !leftChild2->entries.empty() );
    REQUIRE( !rightChild0->entries.empty() );
    REQUIRE( !rightChild1->entries.empty() );
    REQUIRE( !rightChild2->entries.empty() );


	// Test that we get the correct child for the given point
	REQUIRE(rightChild1 == root->chooseSubtree(Point(13.0, -3.0)));
	REQUIRE(leftChild0 == root->chooseSubtree(Point(8.5, 12.5)));
	REQUIRE(leftChild2 == root->chooseSubtree(Point(13.5, 13.5)));
	REQUIRE(rightChild0 == root->chooseSubtree(Point(7.0, 3.0)));
	REQUIRE(leftChild1 == root->chooseSubtree(Point(11.0, 15.0)));
	REQUIRE(leftChild0 == root->chooseSubtree(Point(4.0, 8.0)));
}

TEST_CASE("R*Tree: testFindLeaf")
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two rtree::Nodes
    rstartree::RStarTree tree(3,5);
	rstartree::Node *root = tree.root;
	rstartree::Node *cluster4a = new rstartree::Node(tree);
	cluster4a->entries.push_back(Point(-10.0, -2.0));
	cluster4a->entries.push_back(Point(-12.0, -3.0));
	cluster4a->entries.push_back(Point(-11.0, -3.0));
	cluster4a->entries.push_back(Point(-10.0, -3.0));

	rstartree::Node *cluster4b = new rstartree::Node(tree);
	cluster4b->entries.push_back(Point(-9.0, -3.0));
	cluster4b->entries.push_back(Point(-7.0, -3.0));
	cluster4b->entries.push_back(Point(-10.0, -5.0));

	rstartree::Node *cluster4 = new rstartree::Node(tree);
	cluster4a->parent = cluster4;
	cluster4->entries.push_back(createBranchEntry(cluster4a->boundingBox(), cluster4a));
	cluster4b->parent = cluster4;
	cluster4->entries.push_back(createBranchEntry(cluster4b->boundingBox(), cluster4b));

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)
	// Organized into four rstartree::Nodes
	rstartree::Node *cluster5a = new rstartree::Node(tree);
	cluster5a->entries.push_back(Point(-14.5, -13.0));
	cluster5a->entries.push_back(Point(-14.0, -13.0));
	cluster5a->entries.push_back(Point(-13.5, -13.5));
	cluster5a->entries.push_back(Point(-15.0, -14.0));

	rstartree::Node *cluster5b = new rstartree::Node(tree);
	cluster5b->entries.push_back(Point(-14.0, -14.0));
	cluster5b->entries.push_back(Point(-13.0, -14.0));
	cluster5b->entries.push_back(Point(-12.0, -14.0));
	cluster5b->entries.push_back(Point(-13.5, -16.0));

	rstartree::Node *cluster5c = new rstartree::Node(tree);
	cluster5c->entries.push_back(Point(-15.0, -14.5));
	cluster5c->entries.push_back(Point(-14.0, -14.5));
	cluster5c->entries.push_back(Point(-12.5, -14.5));
	cluster5c->entries.push_back(Point(-13.5, -15.5));

	rstartree::Node *cluster5d = new rstartree::Node(tree);
	cluster5d->entries.push_back(Point(-15.0, -15.0));
	cluster5d->entries.push_back(Point(-14.0, -15.0));
	cluster5d->entries.push_back(Point(-13.0, -15.0));
	cluster5d->entries.push_back(Point(-12.0, -15.0));

	rstartree::Node *cluster5 = new rstartree::Node(tree);
	cluster5d->entries.push_back(Point(-15.0, -15.0));
	cluster5a->parent = cluster5;
	cluster5->entries.push_back(createBranchEntry(cluster5a->boundingBox(),cluster5a));
	cluster5b->parent = cluster5;
	cluster5->entries.push_back(createBranchEntry(cluster5b->boundingBox(), cluster5b));
	cluster5c->parent = cluster5;
	cluster5->entries.push_back(createBranchEntry(cluster5c->boundingBox(), cluster5c));
	cluster5d->parent = cluster5;
	cluster5->entries.push_back(createBranchEntry(cluster5d->boundingBox(), cluster5d));

	// Root
	cluster4->parent = root;
	root->entries.push_back(createBranchEntry(cluster4->boundingBox(), cluster4));
	cluster5->parent = root;
	root->entries.push_back(createBranchEntry(cluster5->boundingBox(), cluster5));

	// Test finding leaves
	REQUIRE(root->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	REQUIRE(root->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	REQUIRE(root->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	REQUIRE(root->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	REQUIRE(root->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	REQUIRE(root->findLeaf(Point(-13.0, -15.0)) == cluster5d);
}

TEST_CASE("R*Tree: testSimpleSplitAxis" )
{
    // There is only one permutation grouping during chooseSplitAxis with minBranchFactor=3,
    // maxBranchFactor = 5. Hence, "simple"
    
    // Test split with X
    rstartree::RStarTree tree(3,5);
	rstartree::Node *cluster6X = tree.root;
	cluster6X->entries.push_back(Point(-3.0, -11.0));
	cluster6X->entries.push_back(Point(-2.0, -9.0));
	cluster6X->entries.push_back(Point(2.0, -10.0));
	cluster6X->entries.push_back(Point(3.0, -11.0));
	cluster6X->entries.push_back(Point(1.0, -9.0));
	cluster6X->entries.push_back(Point(-3.0, -10.0));

	// Split the rstartree::Node in two
	unsigned int axis = cluster6X->chooseSplitAxis();

	REQUIRE(axis == 0);
	REQUIRE(std::get<Point>(cluster6X->entries[0]) == Point(-3.0, -11.0));
	REQUIRE(std::get<Point>(cluster6X->entries[1]) == Point(-3.0, -10.0));
	REQUIRE(std::get<Point>(cluster6X->entries[2]) == Point(-2.0, -9.0));
	REQUIRE(std::get<Point>(cluster6X->entries[3]) == Point(1.0, -9.0));
	REQUIRE(std::get<Point>(cluster6X->entries[4]) == Point(2.0, -10.0));
	REQUIRE(std::get<Point>(cluster6X->entries[5]) == Point(3.0, -11.0));

	// Test split with Y
    rstartree::RStarTree tree2(3,5);
	rstartree::Node *cluster6Y = tree2.root;
	cluster6Y->entries.push_back(Point(-11.0, -3.0));
	cluster6Y->entries.push_back(Point(-9.0, -2.0));
	cluster6Y->entries.push_back(Point(-10.0, 2.0));
	cluster6Y->entries.push_back(Point(-11.0, 3.0));
	cluster6Y->entries.push_back(Point(-9.0, 1.0));
	cluster6Y->entries.push_back(Point(-10.0, -3.0));

	axis = cluster6Y->chooseSplitAxis();

	REQUIRE(axis == 1);
	REQUIRE(std::get<Point>(cluster6Y->entries[0]) == Point(-11.0, -3.0));
	REQUIRE(std::get<Point>(cluster6Y->entries[1]) == Point(-10.0, -3.0));
	REQUIRE(std::get<Point>(cluster6Y->entries[2]) == Point(-9.0, -2.0));
	REQUIRE(std::get<Point>(cluster6Y->entries[3]) == Point(-9.0, 1.0));
	REQUIRE(std::get<Point>(cluster6Y->entries[4]) == Point(-10.0, 2.0));
	REQUIRE(std::get<Point>(cluster6Y->entries[5]) == Point(-11.0, 3.0));
}

TEST_CASE("R*Tree: testComplexComputeMargin" )
{
    rstartree::RStarTree tree(3,7);
	rstartree::Node *cluster = tree.root;
	cluster->entries.push_back(Point(-3.0, -11.0));
	cluster->entries.push_back(Point(-2.0, -9.0));
	cluster->entries.push_back(Point(2.0, -10.0));
	cluster->entries.push_back(Point(3.0, -11.0));
	cluster->entries.push_back(Point(1.0, -9.0));
	cluster->entries.push_back(Point(-3.0, -10.0));
	cluster->entries.push_back(Point(3.0, -11.0));
	cluster->entries.push_back(Point(3.0, -9.0));

    // Check that produce the right margin under X order
    std::sort(cluster->entries.begin(), cluster->entries.end(), rstartree::sortByXRectangleFirst());
    REQUIRE( cluster->computeTotalMarginSum() == 14 + 18 + 18 );

    // Check that produce the right margin under Y order
    std::sort(cluster->entries.begin(), cluster->entries.end(), rstartree::sortByYRectangleFirst());
    REQUIRE( cluster->computeTotalMarginSum() == 26 + 26 + 24 );
}

TEST_CASE("R*Tree: testComplexSplitAxis" )
{
    // Now m=3, M = 7.
    // 2 <= 3 <= 7/2 so we are good.
    
    // Test split with X
    rstartree::RStarTree tree(3,7);
	rstartree::Node *cluster = tree.root;
	cluster->entries.push_back(Point(-3.0, -11.0));
	cluster->entries.push_back(Point(-2.0, -9.0));
	cluster->entries.push_back(Point(2.0, -10.0));
	cluster->entries.push_back(Point(3.0, -11.0));
	cluster->entries.push_back(Point(1.0, -9.0));
	cluster->entries.push_back(Point(-3.0, -10.0));
	cluster->entries.push_back(Point(3.0, -11.0));
	cluster->entries.push_back(Point(3.0, -9.0));

    // See above test for margin scores for X and Y.
    // whichever one is lower (here X) is the split axis

	// Split the rstartree::Node in two
	unsigned int axis = cluster->chooseSplitAxis();
	REQUIRE(axis == 0);

	// Test split with Y
    rstartree::RStarTree tree2(3,7);
	cluster = tree2.root;
	cluster->entries.push_back(Point(-11.0, -3.0));
	cluster->entries.push_back(Point(-9.0, -2.0));
	cluster->entries.push_back(Point(-10.0, 2.0));
	cluster->entries.push_back(Point(-11.0, 3.0));
	cluster->entries.push_back(Point(-9.0, 1.0));
	cluster->entries.push_back(Point(-10.0, -3.0));
	cluster->entries.push_back(Point(-11.0, 3.0));
	cluster->entries.push_back(Point(-9.0, 3.0));

	axis = cluster->chooseSplitAxis();
	REQUIRE(axis == 1);
}

TEST_CASE("R*Tree: testSplitNode")
{
	// Test set one
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
    
    rstartree::RStarTree tree(3,5);
	rstartree::Node *cluster6 = tree.root;
	cluster6->entries.push_back(Point(-2.0, -6.0));
	cluster6->entries.push_back(Point(2.0, -6.0));
	cluster6->entries.push_back(Point(-1.0, -7.0));
	cluster6->entries.push_back(Point(1.0, -7.0));
	cluster6->entries.push_back(Point(3.0, -8.0));
	cluster6->entries.push_back(Point(-2.0, -9.0));

	// Split the rstartree::Node in two
    cluster6->entries.push_back(Point(-3.0, -11.0));
	rstartree::Node *cluster6p = cluster6->splitNode();

	// Test the split
	REQUIRE(cluster6->entries.size() == 3);
	REQUIRE(std::get<Point>(cluster6->entries[0]) == Point(-3.0, -11.0));
	REQUIRE(std::get<Point>(cluster6->entries[1]) == Point(-2.0, -9.0));
	REQUIRE(std::get<Point>(cluster6->entries[2]) == Point(-2.0, -6.0));
	REQUIRE(cluster6p->entries.size() == 4);
	REQUIRE(std::get<Point>(cluster6p->entries[0]) == Point(-1.0, -7.0));
	REQUIRE(std::get<Point>(cluster6p->entries[1]) == Point(1.0, -7.0));
	REQUIRE(std::get<Point>(cluster6p->entries[2]) == Point(2.0, -6.0));
	REQUIRE(std::get<Point>(cluster6p->entries[3]) == Point(3.0, -8.0));

	// Test set two
	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
    rstartree::RStarTree tree2(3,7);
	rstartree::Node *cluster2 = tree2.root;
	cluster2->entries.push_back(Point(-14.0, 8.0));
	cluster2->entries.push_back(Point(-10.0, 8.0));
	cluster2->entries.push_back(Point(-9.0, 10.0));
	cluster2->entries.push_back(Point(-9.0, 9.0));
	cluster2->entries.push_back(Point(-8.0, 10.0));
	cluster2->entries.push_back(Point(-9.0, 7.0));
	cluster2->entries.push_back(Point(-8.0, 8.0));
    cluster2->entries.push_back(Point(-8.0, 9.0));

	// Split the rstartree::Node in two
	rstartree::Node *cluster2p = cluster2->splitNode();

	// Test the split
	REQUIRE(cluster2->entries.size() == 3);
	REQUIRE(std::get<Point>(cluster2->entries[0]) == Point(-9.0, 7.0));
	REQUIRE(std::get<Point>(cluster2->entries[1]) == Point(-14.0, 8.0));
	REQUIRE(std::get<Point>(cluster2->entries[2]) == Point(-10.0, 8.0));
	REQUIRE(cluster2p->entries.size() == 5);
	REQUIRE(std::get<Point>(cluster2p->entries[0]) == Point(-8.0, 8.0));
	REQUIRE(std::get<Point>(cluster2p->entries[1]) == Point(-9.0, 9.0));
	REQUIRE(std::get<Point>(cluster2p->entries[2]) == Point(-8.0, 9.0));
	REQUIRE(std::get<Point>(cluster2p->entries[3]) == Point(-9.0, 10.0));
	REQUIRE(std::get<Point>(cluster2p->entries[4]) == Point(-8.0, 10.0));

	// Test set three
	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// {(-5, 4), 1, 1}, {(-2, 4), 1, 1}, {(-1, 3), 1, 1}, {(-1, 1), 1, 1}, {(-3, 0), 1, 1},
	// {(-6, 2), 1, 1}
    rstartree::RStarTree tree3(3,5);
	rstartree::Node *cluster3 = tree3.root;
	rstartree::Node *dummys[6] = {new rstartree::Node(tree3), new rstartree::Node(tree3), new rstartree::Node(tree3), new rstartree::Node(tree3), new rstartree::Node(tree3), new rstartree::Node(tree3)};
	dummys[0]->parent = cluster3;
	cluster3->entries.push_back(createBranchEntry(Rectangle(-6.0, 3.0, -4.0, 5.0), dummys[0]));
	dummys[1]->parent = cluster3;
	cluster3->entries.push_back(createBranchEntry(Rectangle(-3.0, 3.0, -1.0, 5.0), dummys[1]));
	dummys[2]->parent = cluster3;
	cluster3->entries.push_back(createBranchEntry(Rectangle(-2.0, 2.0, 0.0, 4.0), dummys[2]));
	dummys[3]->parent = cluster3;
	cluster3->entries.push_back(createBranchEntry(Rectangle(-2.0, 0.0, 0.0, 2.0), dummys[3]));
	dummys[4]->parent = cluster3;
	cluster3->entries.push_back(createBranchEntry(Rectangle(-4.0, -1.0, -2.0, 1.0), dummys[4]));
	dummys[5]->parent = cluster3;
	cluster3->entries.push_back(createBranchEntry(Rectangle(-7.0, 1.0, -5.0, 3.0), dummys[5]));

	// Extra rstartree::Node causing the split
	rstartree::Node *cluster3extra = new rstartree::Node(tree3);
	cluster3extra->entries.push_back(Point(1.0, 1.0));
	cluster3extra->entries.push_back(Point(2.0, 2.0));

	// Test the split
    cluster3->entries.push_back(createBranchEntry(cluster3extra->boundingBox(), cluster3extra ) );
	rstartree::Node *cluster3p = cluster3->splitNode();

	REQUIRE(cluster3->entries.size() == 4);
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3->entries[0]).boundingBox == Rectangle(-7.0, 1.0, -5.0, 3.0));
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3->entries[1]).boundingBox == Rectangle(-6.0, 3.0, -4.0, 5.0));
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3->entries[2]).boundingBox == Rectangle(-4.0, -1.0, -2.0, 1.0));
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3->entries[3]).boundingBox == Rectangle(-3.0, 3.0, -1.0, 5.0));

	REQUIRE(cluster3p->entries.size() == 3);
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3p->entries[0]).boundingBox == Rectangle(-2.0, 0.0, 0.0, 2.0));
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3p->entries[1]).boundingBox == Rectangle(-2.0, 2.0, 0.0, 4.0));
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3p->entries[2]).boundingBox == Rectangle(1.0, 1.0, 2.0, 2.0));
	REQUIRE(std::get<rstartree::Node::Branch>(cluster3p->entries[2]).child== cluster3extra);
	
}

TEST_CASE("R*Tree: testInsertOverflowReInsertAndSplit" )
{
	// From paper:
	// If level is not the root level and this is the first call of
	// overflowTreatment in the current level during a single rectangle insertion,
	// then reInsert. Else, split.

	// Leaf rtree::Node and new sibling leaf
	// Cluster 4, n = 5
	rstartree::RStarTree tree(3,7);
	rstartree::Node *root = tree.root;
	rstartree::Node *cluster4aAugment = new rstartree::Node(tree);
	cluster4aAugment->entries.push_back(Point(-30.0, -30.0));
	cluster4aAugment->entries.push_back(Point(30.0, 30.0));
	cluster4aAugment->entries.push_back(Point(-20.0, -20.0));
	cluster4aAugment->entries.push_back(Point(20.0, 20.0));
	cluster4aAugment->entries.push_back(Point(-10.0, -10.0));
	cluster4aAugment->entries.push_back(Point(10.0, 10.0));
	cluster4aAugment->entries.push_back(Point(0.0, 0.0));
	cluster4aAugment->level = 1;

	// Root rstartree::Node
	root->level = 0;
	root->entries.push_back(createBranchEntry(cluster4aAugment->boundingBox(), cluster4aAugment));
	cluster4aAugment->parent = root;

	Point point(0.0,0.0);

	REQUIRE(cluster4aAugment->entries.size() == 7);
	REQUIRE(cluster4aAugment->boundingBox().centerPoint() == Point(0.0,0.0));
	REQUIRE(root->checksum() == 0);

	// We are not the root and we haven't inserted anything yet. Should call reinsert
	// But then we will fill out the node again. Then we call split.
	std::vector<bool> reInsertedAtLevel = { false, false };
	root->insert(point, reInsertedAtLevel);

	// We tried to reinsert, but it won't work.
	REQUIRE(reInsertedAtLevel[0] == false);
	REQUIRE(reInsertedAtLevel[1] == true);

	// Should force split
	REQUIRE(root->entries.size() == 2);

	// We will have split along the x axis (y axis isomorphic so we prefer x).
	// Overlap is always zero between any cut along X. Cumulative area is minimized at 3,5 or 5,3 split.
	// We prefer 3,5.
    rstartree::Node::Branch bLeft = std::get<rstartree::Node::Branch>( root->entries[0] );
    rstartree::Node::Branch bRight = std::get<rstartree::Node::Branch>( root->entries[1] );
	REQUIRE(bLeft.child->entries.size() == 3);
	REQUIRE(bRight.child->entries.size() == 5);

	REQUIRE(std::get<Point>(bLeft.child->entries[0]) == Point(-30,-30));
	REQUIRE(std::get<Point>(bLeft.child->entries[1]) == Point(-20,-20));
	REQUIRE(std::get<Point>(bLeft.child->entries[2]) == Point(-10,-10));

	REQUIRE(std::get<Point>(bRight.child->entries[0]) == Point(0,0));
	REQUIRE(std::get<Point>(bRight.child->entries[1]) == Point(0,0));
	REQUIRE(std::get<Point>(bRight.child->entries[2]) == Point(10,10));
	REQUIRE(std::get<Point>(bRight.child->entries[3]) == Point(20,20));
	REQUIRE(std::get<Point>(bRight.child->entries[4]) == Point(30,30));
}

TEST_CASE("R*Tree: testInsertGrowTreeHeight")
{
	std::vector<bool> reInsertedAtLevel = { false };
	unsigned maxBranchFactor = 7;
    rstartree::RStarTree tree(3,7);
	rstartree::Node *root = tree.root;
	root->level = 0;

	for( unsigned i = 0; i < maxBranchFactor+1; i++ )
	{
		root = root->insert(Point(0.0,0.0), reInsertedAtLevel);
	}

	REQUIRE(root->entries.size() == 2);
    rstartree::Node::Branch bLeft = std::get<rstartree::Node::Branch>( root->entries[0] );
    rstartree::Node::Branch bRight = std::get<rstartree::Node::Branch>( root->entries[1] );

	REQUIRE(bLeft.child->entries.size() == 3);
	REQUIRE(bLeft.child->level == 1);
	REQUIRE(bRight.child->entries.size() == 5);
	REQUIRE(bRight.child->level == 1);
}


TEST_CASE("R*Tree: testSplitNonLeafNode")
{
	unsigned maxBranchFactor = 7;
    rstartree::RStarTree tree(3,7);
	rstartree::Node *root = tree.root;
	root->level = 0;

	for(unsigned i = 0; i < maxBranchFactor; i++)
	{
		rstartree::Node *child = createFullLeafNode(tree);
		child->level = 1;
		child->parent = root;
        rstartree::Node::Branch b( child->boundingBox(), child );
		root->entries.emplace_back( std::move( b ) );
	}

    unsigned height = root->height();
    tree.hasReinsertedOnLevel = std::vector( height, false );

    // Should be 49 points
    std::vector<Point> accumulator = tree.search( Point(0.0,0.0) );
    REQUIRE( accumulator.size() == maxBranchFactor * maxBranchFactor );

	REQUIRE(root->entries.size() == maxBranchFactor);
	tree.insert(Point(0.0, 0.0));
    rstartree::Node *newRoot = tree.root;
    REQUIRE( newRoot != root );

    // Confirm tree structure
	REQUIRE( newRoot->entries.size() == 2 );
    const rstartree::Node::Branch &bLeft = std::get<rstartree::Node::Branch>( newRoot->entries[0] );
    const rstartree::Node::Branch &bRight = std::get<rstartree::Node::Branch>( newRoot->entries[1] );
    REQUIRE( bLeft.child->entries.size() == 3 );
    REQUIRE( bRight.child->entries.size() == 5 );

    for( const auto &entry : bLeft.child->entries ) {
        rstartree::Node *child = std::get<rstartree::Node::Branch>( entry ).child;
        // These are all leaves
        REQUIRE( std::holds_alternative<Point>( child->entries[0] ) );
    }

    for( const auto &entry : bRight.child->entries ) {
        rstartree::Node *child = std::get<rstartree::Node::Branch>( entry ).child;
        // These are all leaves
        REQUIRE( std::holds_alternative<Point>( child->entries[0] ) );
    }

    // Count
    accumulator = newRoot->search( Point(0.0,0.0) );
    REQUIRE( accumulator.size() == maxBranchFactor * maxBranchFactor + 1 );
}

TEST_CASE("R*Tree: RemoveLeafNode", "[broken]") {
	unsigned maxBranchFactor = 7;
    unsigned minBranchFactor = 3;
    rstartree::RStarTree tree(minBranchFactor,maxBranchFactor);

	for(unsigned i = 0; i < maxBranchFactor*maxBranchFactor+1; i++) {
        tree.insert( Point(i,i) );
	}

    for( unsigned i = 0; i < maxBranchFactor*maxBranchFactor+1; i++ ) {
        Point p(i,i);
        REQUIRE( tree.search( p ).size() == 1 );
    }

    //Find a leaf
    rstartree::Node *node = tree.root;
    while( std::holds_alternative<rstartree::Node::Branch>( node->entries[0] ) ) {
        const rstartree::Node::Branch &b = std::get<rstartree::Node::Branch>( node->entries[0] );
        node = b.child;
    }
    REQUIRE( std::holds_alternative<Point>( node->entries[0] ) );
    size_t cnt = node->entries.size();
    std::vector<rstartree::Node::NodeEntry> nodesToRemove( node->entries.begin(), node->entries.begin() + (cnt-minBranchFactor+1) );
    for( const auto &entry : nodesToRemove ) {
        const Point &p = std::get<Point>( entry );
        tree.remove( p );
    }

    for( unsigned i = 0; i < maxBranchFactor*maxBranchFactor+1; i++ ) {
        Point p(i,i);
        rstartree::Node::NodeEntry ne = p;
        if( std::find(nodesToRemove.begin(), nodesToRemove.end(), ne) == nodesToRemove.end() ) {
            REQUIRE( tree.search( p ).size() == 1 );
        } else {
            REQUIRE( tree.search( p ).size() == 0 );
        }
    }
    
}

TEST_CASE("R*Tree: computeTotalMarginSum") {
	// Leaf rtree::Node and new sibling leaf
	// Cluster 4, n = 5
    rstartree::RStarTree tree(3,5);
	rstartree::Node *cluster4aAugment = tree.root;
	cluster4aAugment->entries.push_back(Point(-20.0, -20.0));
	cluster4aAugment->entries.push_back(Point(-10.0, -10.0));
	cluster4aAugment->entries.push_back(Point(0.0, 0.0));
	cluster4aAugment->entries.push_back(Point(0.0, 0.0));
	cluster4aAugment->entries.push_back(Point(10.0, 10.0));
	cluster4aAugment->entries.push_back(Point(20.0, 20.0));
    REQUIRE( cluster4aAugment->computeTotalMarginSum() == 160.0 );
}

TEST_CASE("R*Tree: testSearch")
{
	// Build the tree directly

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)

    rstartree::RStarTree tree(3,5);
	rstartree::Node *root = tree.root;
	rstartree::Node *cluster1a = new rstartree::Node(tree);
	cluster1a->entries.push_back(Point(-3.0, 16.0));
	cluster1a->entries.push_back(Point(-3.0, 15.0));
	cluster1a->entries.push_back(Point(-4.0, 13.0));

	rstartree::Node *cluster1b = new rstartree::Node(tree);
	cluster1b->entries.push_back(Point(-5.0, 12.0));
	cluster1b->entries.push_back(Point(-5.0, 15.0));
	cluster1b->entries.push_back(Point(-6.0, 14.0));
	cluster1b->entries.push_back(Point(-8.0, 16.0));

	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	rstartree::Node *cluster2a = new rstartree::Node(tree);
	cluster2a->entries.push_back(Point(-8.0, 10.0));
	cluster2a->entries.push_back(Point(-9.0, 10.0));
	cluster2a->entries.push_back(Point(-8.0, 9.0));
	cluster2a->entries.push_back(Point(-9.0, 9.0));
	cluster2a->entries.push_back(Point(-8.0, 8.0));

	rstartree::Node *cluster2b = new rstartree::Node(tree);
	cluster2b->entries.push_back(Point(-14.0, 8.0));
	cluster2b->entries.push_back(Point(-10.0, 8.0));
	cluster2b->entries.push_back(Point(-9.0, 7.0));

	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	rstartree::Node *cluster3a = new rstartree::Node(tree);
	cluster3a->entries.push_back(Point(-3.0, 4.0));
	cluster3a->entries.push_back(Point(-3.0, 0.0));
	cluster3a->entries.push_back(Point(-2.0, 4.0));
	cluster3a->entries.push_back(Point(-1.0, 3.0));
	cluster3a->entries.push_back(Point(-1.0, 1.0));

	rstartree::Node *cluster3b = new rstartree::Node(tree);
	cluster3b->entries.push_back(Point(-5.0, 4.0));
	cluster3b->entries.push_back(Point(-4.0, 3.0));
	cluster3b->entries.push_back(Point(-4.0, 1.0));
	cluster3b->entries.push_back(Point(-6.0, 2.0));

	// High level rstartree::Nodes
	rstartree::Node *left = new rstartree::Node(tree);
	cluster1a->parent = left;
	left->entries.push_back(createBranchEntry(cluster1a->boundingBox(), cluster1a));
	cluster1b->parent = left;
	left->entries.push_back(createBranchEntry(cluster1b->boundingBox(), cluster1b));
	cluster2a->parent = left;
	left->entries.push_back(createBranchEntry(cluster2a->boundingBox(), cluster2a));
	cluster2b->parent = left;
	left->entries.push_back(createBranchEntry(cluster2b->boundingBox(), cluster2b));

	rstartree::Node *right = new rstartree::Node(tree);
	cluster3a->parent = right;
	right->entries.push_back(createBranchEntry(cluster3a->boundingBox(), cluster3a));
	cluster3b->parent = right;
	right->entries.push_back(createBranchEntry(cluster3b->boundingBox(), cluster3b));

	left->parent = root;
	root->entries.push_back(createBranchEntry(left->boundingBox(), left));
	right->parent = root;
	root->entries.push_back(createBranchEntry(right->boundingBox(), right));

	// Test search

	// Test set one
	Rectangle sr1 = Rectangle(-9.0, 9.5, -5.0, 12.5);
	std::vector<Point> v1 = root->search(sr1);
	REQUIRE(v1.size() == 3);
	REQUIRE(std::find( v1.begin(), v1.end(), Point(-8.0, 10.0)) != v1.end());
	REQUIRE(std::find( v1.begin(), v1.end(), Point(-9.0, 10.0)) != v1.end());
	REQUIRE(std::find( v1.begin(), v1.end(), Point(-5.0, 12.0)) != v1.end());

	// Test set two
	Rectangle sr2 = Rectangle(-8.0, 4.0, -5.0, 8.0);
	std::vector<Point> v2 = root->search(sr2);
	REQUIRE(v2.size() == 2);
	REQUIRE(std::find( v2.begin(), v2.end(), Point(-5.0, 4.0)) != v2.end());
	REQUIRE(std::find( v2.begin(), v2.end(), Point(-8.0, 8.0)) != v2.end());

	// Test set three
	Rectangle sr3 = Rectangle(-8.0, 0.0, -4.0, 16.0);
	std::vector<Point> v3 = root->search(sr3);
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
	Rectangle sr4 = Rectangle(2.0, -4.0, 4.0, -2.0);
	std::vector<Point> v4 = root->search(sr4);
	REQUIRE(v4.size() == 0);

	// Test set five
	Rectangle sr5 = Rectangle(-3.5, 1.0, -1.5, 3.0);
	std::vector<Point> v5 = root->search(sr5);
	REQUIRE(v5.size() == 0);

}

