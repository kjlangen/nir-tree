#include <rtree/rtree.h>

RTree::RTree()
{
}

RTree::RTree(Node *root)
{
	this->root = root;
}

std::vector<Point> RTree::search(Rectangle requestedRectangle)
{
	return root->search(requestedRectangle);
}

void RTree::insert(Point givenPoint)
{
	root = root->insert(givenPoint);
}

void RTree::remove(Point givenPoint)
{
	std::map<unsigned, std::list<std::pair<Rectangle, Node *>>> q;

	root = root->remove(givenPoint);
}

// void testSimpleSearch()
// {
// 	// NOTE: This test does not describe a tightest-fitting-boxes R-Tree but is a good simple test

// 	// The node describing rectangles 1 and 2
// 	Rectangle rect1 = Rectangle(2, 2, 4, 5);
// 	Rectangle rect2 = Rectangle(10, 18, 12, 20);
// 	Node *node1 = new Node(1);
// 	node1->addPolygon(rect1);
// 	node1->addPolygon(rect2);

// 	// The node describing rectangle 3
// 	Rectangle rect3 = Rectangle(17, 5, 18, 18);
// 	Node *node2 = new Node(2);
// 	node2->addPolygon(rect3);

// 	// The linking (root) node describing node1 and node2
// 	Rectangle rect4 = Rectangle(0, 0, 13, 21);
// 	Rectangle rect5 = Rectangle(15, 4, 21, 21);
// 	Node *root = new Node(3);
// 	root->addPolygon(rect4);
// 	root->addPolygon(rect5);
// 	root->addChild(node1);
// 	root->addChild(node2);

// 	// The first query rectangle
// 	Rectangle query1 = Rectangle(3, 3, 19, 19);
// 	// Test that the first query gives us back all three leaves
// 	std::vector<Rectangle> queryResult1 = root->searchRectangle(query1);
// 	assert(queryResult1.size() == 3);

// 	// The second query rectangle
// 	Rectangle query2 = Rectangle(16, 4, 19, 21);
// 	// Test that the second query gives us back only rectangle 3
// 	std::vector<Rectangle> queryResult2 = root->searchRectangle(query2);
// 	assert(queryResult2.size() == 1);

// 	delete node1;
// 	delete node2;
// 	delete root;
// }

// void testSimpleInsert()
// {
// 	// NOTE: This test does not describe a tightest-fitting-boxes R-Tree but is a good simple test

// 	// The node describing rectangles 1 and 2
// 	Rectangle rect1 = Rectangle(2, 2, 4, 5);
// 	Rectangle rect2 = Rectangle(10, 18, 12, 20);
// 	Node *node1 = new Node(1);
// 	node1->addPolygon(rect1);
// 	node1->addPolygon(rect2);

// 	// The node describing rectangle 3
// 	Rectangle rect3 = Rectangle(17, 5, 18, 18);
// 	Node *node2 = new Node(2);
// 	node2->addPolygon(rect3);

// 	// The linking (root) node describing node1 and node2
// 	Rectangle rect4 = Rectangle(0, 0, 13, 21);
// 	Rectangle rect5 = Rectangle(15, 4, 21, 21);
// 	Node *root = new Node(3);
// 	root->addPolygon(rect4);
// 	root->addPolygon(rect5);
// 	root->addChild(node1);
// 	root->addChild(node2);

// 	// Rectangles to be inserted, they all fall under rect4 and should all be inserted under node1
// 	Rectangle insert1 = Rectangle(6, 3, 8, 4);
// 	Rectangle insert2 = Rectangle(1, 13, 2, 14);
// 	Rectangle insert3 = Rectangle(11, 16, 15, 18);
// 	Rectangle insert4 = Rectangle(0, 19, 2, 21);

// 	// Insert our rectangles
// 	root->print();
// 	root->children[0]->print();
// 	root->children[1]->print();
// 	std::cout << "==============================" << std::endl;
// 	root->insert(insert1);
// 	std::cout << "Insert 1" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	root->children[0]->print();
// 	root->children[1]->print();
// 	std::cout << "==============================" << std::endl;
// 	root->insert(insert2);
// 	std::cout << "Insert 2" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	root->children[0]->print();
// 	root->children[1]->print();
// 	std::cout << "==============================" << std::endl;
// 	root->insert(insert3);
// 	std::cout << "Insert 3" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	root->children[0]->print();
// 	root->children[1]->print();
// 	std::cout << "==============================" << std::endl;
// 	root->insert(insert4);
// 	std::cout << "Insert 4" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	root->children[0]->print();
// 	root->children[1]->print();
// 	root->children[2]->print();
// 	/*
// 	// Test that our rectangles were inserted into the expected node
// 	assert!(root.children[0].boundingPolygons.len() == 5);
// 	assert!(root.children[0].boundingPolygons[2].area == 2);
// 	assert!(root.children[0].boundingPolygons[3].area == 1);
// 	assert!(root.children[0].boundingPolygons[4].area == 8);
// 	// Test that the previous rectangles are still there
// 	assert!(root.children[0].boundingPolygons[0].area == 6);
// 	assert!(root.children[0].boundingPolygons[1].area == 4);
// 	*/
// }

// void expandRootTest()
// {
// 	// The node describing rectangles 1 and 2
// 	Rectangle rect1 = Rectangle(20, 20, 21, 21);
// 	Rectangle rect2 = Rectangle(0, 0, 1, 1);
// 	Node *node1 = new Node(1);
// 	node1->addPolygon(rect1);
// 	node1->addPolygon(rect2);

// 	// The node describing rectangle 3
// 	Rectangle rect3 = Rectangle(17, 17, 18, 18);
// 	Node *node2 = new Node(2);
// 	node2->addPolygon(rect3);

// 	// The linking (root) node describing node1 and node2
// 	Rectangle rect4 = Rectangle(0, 0, 13, 21);
// 	Rectangle rect5 = Rectangle(15, 4, 21, 21);
// 	Node *root = new Node(3);
// 	root->addPolygon(rect4);
// 	root->addPolygon(rect5);
// 	root->addChild(node1);
// 	root->addChild(node2);

// 	// Rectangles to be inserted, they all fall under rect4 and should all be inserted under node1
// 	// Expand node 1 once
// 	Rectangle insert1 = Rectangle(1, 1, 2, 2);
// 	Rectangle insert2 = Rectangle(2, 2, 3, 3);
// 	Rectangle insert3 = Rectangle(3, 3, 4, 4);
// 	root->insert(insert1);
// 	root->insert(insert2);
// 	root->insert(insert3);

// 	std::cout << "After Insert Group 1" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	for (int i = 0; i < root->children.size(); ++i)
// 	{
// 		root->children[i]->print();
// 	}

// 	// Expand node 2 once
// 	Rectangle insert4 = Rectangle(15, 4, 16, 5);
// 	Rectangle insert5 = Rectangle(16, 5, 17, 6);
// 	Rectangle insert6 = Rectangle(17, 6, 18, 7);
// 	Rectangle insert7 = Rectangle(18, 7, 19, 8);
// 	root->insert(insert4);
// 	root->insert(insert5);
// 	root->insert(insert6);
// 	root->insert(insert7);

// 	std::cout << "After Insert Group 2" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	for (int i = 0; i < root->children.size(); ++i)
// 	{
// 		root->children[i]->print();
// 	}

// 	// Expand node 1 again, causing the root to split
// 	Rectangle insert8 = Rectangle(5, 5, 6, 6);
// 	Rectangle insert9 = Rectangle(7, 7, 8, 8);
// 	Rectangle insert10 = Rectangle(9, 9, 10, 10);
// 	Node *result8 = root->insert(insert8);

// 	std::cout << "After Insert 8" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	for (int i = 0; i < root->children.size(); ++i)
// 	{
// 		root->children[i]->print();
// 	}

// 	Node *result9 = root->insert(insert9);
// 	std::cout << "After Insert 9" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	for (int i = 0; i < root->children.size(); ++i)
// 	{
// 		root->children[i]->print();
// 	}
// 	std::cout << "----------------------" << std::endl;
// 	result9->print();
// 	for (int i = 0; i < result9->children.size(); ++i)
// 	{
// 		result9->children[i]->print();
// 	}


// 	Node *result10 = root->insert(insert10);
// 	std::cout << "After Insert 10" << std::endl;
// 	std::cout << "==============================" << std::endl;
// 	root->print();
// 	result10->print();
// }
