#include "rtree/rtree.h"

RTree::RTree()
{
}

RTree::RTree(Node *root)
{
	this->root = root;
}

std::vector<Rectangle> RTree::searchRectangle(Rectangle requestedRectangle)
{
	return root->searchRectangle(requestedRectangle);
}

void RTree::insert(Rectangle newRectangle)
{
	root->insert(newRectangle);
}

Node::Node()
{
}

Node::Node(unsigned id)
{
	this->id = id;
}

void Node::setParent(Node *parent)
{
	this->parent = parent;
}

void Node::addPolygon(Rectangle newRectangle)
{
	boundingPolygons.push_back(newRectangle);
}

void Node::addChild(Node *newChild)
{
	children.push_back(newChild);
}

std::vector<Rectangle> Node::searchRectangle(Rectangle requestedRectangle)
{
	// All the nodes in the R-Tree that match
	std::vector<Rectangle> matchingRectangles = std::vector<Rectangle>();

	// if (children.size() == 0)
	// {
	// 	// We are a leaf so check our rectangles
	// 	for (unsigned i = 0; i < boundingPolygons.size(); ++i)
	// 	{
	// 		// println!("[searchRectangle] Checking rectangle {}.", i);
	// 		bool intersecting = boundingPolygons[i].intersectsRectangle(requestedRectangle);
	// 		// println!("[searchRectangle] {:?}", intersecting);
	// 		if (intersecting)
	// 		{
	// 			//println!("[searchRectangle] Leaf {} adding rectangle {}", self.id, i);
	// 			matchingRectangles.push_back(boundingPolygons[i]);
	// 		}
	// 	}
	// }
	// else
	// {
	// 	// We are the ones testing and recursively asking the nodes below us for matching nodes.
	// 	assert(children.size() == boundingPolygons.size());
	// 	for (unsigned i = 0; i < boundingPolygons.size(); ++i)
	// 	{
	// 		//println!("[searchRectangle] Checking a child with id {}.", self.children[i].getId());
	// 		bool intersecting = boundingPolygons[i].intersectsRectangle(requestedRectangle);
	// 		//println!("[searchRectangle] {:?}", intersecting);
	// 		if (intersecting)
	// 		{
	// 			//println!("[searchRectangle] Child matched. Recursing to add all valid leaves.");
	// 			std::vector<Rectangle> lowerRectangles = children[i]->searchRectangle(requestedRectangle);
	// 			for (unsigned j = 0; j < lowerRectangles.size(); j++)
	// 			{
	// 				//println!("[searchRectangle] Adding to results at node {}", self.id);
	// 				matchingRectangles.push_back(lowerRectangles[j]);
	// 			}
	// 		}
	// 	}
	// }

	return matchingRectangles;
}

bool Node::searchPoint(Point requestedPoint)
{
	// if (children.size() == 0)
	// {
	// 	// We are a leaf so we have only to check our bounding boxes which represent the objects
	// 	for (unsigned i = 0; i < boundingPolygons.size(); ++i)
	// 	{
	// 		if (boundingPolygons[i].containsPoint(requestedPoint))
	// 		{
	// 			return true;
	// 		}
	// 	}
	// }
	// else
	// {
	// 	// Check each child's bounding box to see if we should go to that subtree
	// 	assert(children.size() == boundingPolygons.size());
	// 	for (unsigned i = 0; i < children.size(); ++i)
	// 	{
	// 		// println!("Checking a child.");
	// 		if (boundingPolygons[i].containsPoint(requestedPoint))
	// 		{
	// 			// println!("Bounding polygon contained point. Recursing.");
	// 			return children[i]->searchPoint(requestedPoint);
	// 		}
	// 	}
	// }

	return false;
}

Rectangle Node::computeBoundary()
{
	// assert(boundingPolygons.size() > 0);
	// unsigned minX = boundingPolygons[0].lowerLeft.x;
	// unsigned minY = boundingPolygons[0].lowerLeft.y;

	// unsigned maxX = boundingPolygons[0].upperRight.x;
	// unsigned maxY = boundingPolygons[0].upperRight.y;

	// // Run over all the bounding polygons and find the max/min extents to create a tight bound
	// for (int i = 0; i < boundingPolygons.size(); ++i)
	// {
	// 	minX = boundingPolygons[i].lowerLeft.x < minX ? boundingPolygons[i].lowerLeft.x: minX;
	// 	minY = boundingPolygons[i].lowerLeft.y < minY ? boundingPolygons[i].lowerLeft.y: minY;
	// 	maxX = boundingPolygons[i].upperRight.x > maxX ? boundingPolygons[i].upperRight.x: maxX;
	// 	maxY = boundingPolygons[i].upperRight.y > maxY ? boundingPolygons[i].upperRight.y: maxY;
	// }

	return Rectangle(0, 0, 0, 0);
}

Node *Node::splitNode()
{
	// Make the new node
	Node *babyNode = new Node();

	// // Split the bounding polygons
	// unsigned polygonsToMove = boundingPolygons.size() / 2;
	// for (unsigned i = 0; i < polygonsToMove; ++i)
	// {
	// 	std::cout << "Moving polygon " << i << std::endl;
	// 	babyNode->addPolygon(boundingPolygons.back());
	// 	boundingPolygons.pop_back();
	// }

	// // Split the children
	// for (unsigned i = 0; i < (children.size() / 2); ++i)
	// {
	// 	babyNode->addChild(children.back());
	// 	children.pop_back();
	// }

	return babyNode;
}

Node *Node::insert(Rectangle newRectangle)
{
	// if (children.size() == 0)
	// {
	// 	// Phase 2: We are a leaf, so we need to check if we need to split and then if we do, call
	// 	// adjust tree on our parent
	// 	if (boundingPolygons.size() >= 4)
	// 	{
	// 		// Split node
	// 		Node *newChild = splitNode();
	// 		// Insert new Rectangle
	// 		boundingPolygons.push_back(newRectangle);
	// 		// Send the new child
	// 		return newChild;
	// 	}
	// 	else
	// 	{
	// 		// Insert new Rectangle
	// 		addPolygon(newRectangle);
	// 	}

	// 	return this;
	// }
	// else
	// {
	// 	// Phase 1: We are not a leaf so descend the tree to the proper leaf
	// 	// Pick a new node based on least amount of expansion area
	// 	unsigned chosen = 0;
	// 	unsigned chosenMetric = boundingPolygons[chosen].computeExpansionArea(newRectangle);
	// 	for (int i = 0; i < children.size(); ++i)
	// 	{
	// 		unsigned competitorMetric = boundingPolygons[i].computeExpansionArea(newRectangle);
	// 		if (competitorMetric < chosenMetric)
	// 		{
	// 			chosen = i;
	// 			chosenMetric = competitorMetric;
	// 		}
	// 	}

	// 	// Descend through the tree
	// 	Node *newChildNode = children[chosen]->insert(newRectangle);

	// 	// Update the bounding box for the chosen child
	// 	boundingPolygons[chosen] = children[chosen]->computeBoundary();

	// 	// If there really is a new child we need to absorb it, maybe split, and keep propogating
	// 	if (newChildNode != children[chosen])
	// 	{
	// 		// There was a split, do we need to split now?
	// 		if (boundingPolygons.size() >= 4)
	// 		{
	// 			// Split node
	// 			Node *newNavNode = splitNode();
	// 			// Insert new Node
	// 			children.push_back(newChildNode);
	// 			boundingPolygons.push_back(newChildNode->computeBoundary());
	// 			// Send the new nav node that resulted from our split
	// 			return newNavNode;
	// 		}
	// 		else
	// 		{
	// 			// Insert new Node
	// 			children.push_back(newChildNode);
	// 			boundingPolygons.push_back(newChildNode->computeBoundary());
	// 		}
	// 	}
		
	// 	// There wasn't a split so return ourselves
	// 	return this;
	// }

	return this;
}

/*
// Insert
// 1. Find position for the new record. Call ChooseLeaf to select a leaf L in which to put E.
// 2. Add record to the leaf node. If L has room for E then add it, otherwise call SplitNode to get
// L and LL containing E and all the old entries of L.
// 3. Propogate changes upwards. Call AdjustTree on L, also passing LL if created by step 2.
// 4. Grow the tree taller. If the root split, create a new root with the results of AdjustTree.

// ChooseLeaf
// 1. Initialize. Set N to be the root node.
// 2. Leaf check. If N is a leaf, return N.
// 3. Choose subtree. If N is not a leaf, let F be the entry in N whose rectangle requires least
// enlargement to include E. Break ties by using smallest area.
// 4. Descend until a leaf is reached. Set N to be the chosen child node and repeat from step 2.

// AdjustTree
// 1. Initialize. Set N to be the leaf L. If L was split then also set NN to be LL.
// 2. Check if done. If N is the root then stop.
// 3. Adjust covering rectangle in parent. Let P be the parent of N. Adjust P's bounding box for N
// so that it tightly encloses all entry rectangles in N.
// 4. Propogate upwards. If N has a partner NN then create a new entry pointing to NN enclosing all
// rectangles within NN. Add this to P if there is room. Otherwise call SplitNode to make P and PP.
// 5. Move up to next level. Set N = P and NN = PP. Repeat from step 2.

// Deletion
// 1. Find node containing record E. Call FindLeaf to locate the leaf node L containing E. Stop if
// the record was not found.
// 2. Delete record. Remove E from L.
// 3. Propagate changes. Call CondenseTree, passing L.
// 4. Shorten tree. If the root node has only one child after the tree has been adjusted, make the
// child the new root.

// FindLeaf
// 1. Search subtrees. If T is not a leaf, check each entry F in T to determine overlaps. For each
// such entry call FindLeaf on the subtrees until E is found or all subtrees have been checked.
// 2. Search leaf node for record E. If T is a leaf, check each entry to see if it matches E. If E
// is found return T.

// CondenseTree
// 1. Initialize. Set N = L. Set Q, the set of eliminated nodes, to be empty.
// 2. Find parent entry. If N is the root, go to step 6. Otherwise let P be the parent of N, and let
// EN be N's entry in P.
// 3. Eliminate under-full node. If N has fewer than m entries, delete EN from P and add N to set Q
// 4. Adjust covering rectangle. If N has not been eliminated, adjust EN.I to tightly contain all
// entries in N.
// 5. Move up one level in tree. Set N = P and repeat from step 2.
// 6. Re-insert orphaned entries. Re-insert all entries of nodes in set Q. Entries from eliminated
// leaf nodes are re-inserted in tree leaves as described in Insert, but entries from higher-level
// nodes must be placed higher in the tree, so that leaves of their dependent subtrees will be on
// the same level as leaves of the main tree.
*/


void Node::print()
{
	std::cout << "Node " << id << " {" << std::endl;
	for (int i = 0; i < boundingPolygons.size(); ++i)
	{
		boundingPolygons[i].print();
	}
	for (int i = 0; i < children.size(); ++i)
	{
		std::cout << "Child " << children[i]->id << std::endl;
	}
	std::cout << "}" << std::endl;
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
