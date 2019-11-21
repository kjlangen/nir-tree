#include <rtree/node.h>

Node::Node()
{
	id = 0;
	parent = nullptr;
}

Node::Node(unsigned id, Node *p)
{
	this->id = id;
	this->parent = p;
}

Rectangle Node::boundingBox()
{
	assert(boundingBoxes.size() > 0);

	Rectangle boundingBox = boundingBoxes[0];
	for (int i = 1; i < boundingBoxes.size(); ++i)
	{
		boundingBox.expand(boundingBoxes[i]);
	}

	return boundingBox;
}

// TODO: Optimize maybe
void Node::updateBoundingBox(Node *child, Rectangle updatedBoundingBox)
{
	for (int i = 0; i < children.size(); ++i)
	{
		if (children[i] == child)
		{
			boundingBoxes[i] = updatedBoundingBox;
			break;
		}
	}
}

// TODO: Optimize maybe
void Node::removeChild(Node *child)
{
	for (int i = 0; i < children.size(); ++i)
	{
		if (children[i] == child)
		{
			boundingBoxes.erase(boundingBoxes.begin() + i);
			children.erase(children.begin() + i);
			break;
		}
	}
}

// TODO: Optimize maybe
void Node::removeData(Point givenPoint)
{
	for (int i = 0; i < data.size(); ++i)
	{
		if (data[i] == givenPoint)
		{
			boundingBoxes.erase(boundingBoxes.begin() + i);
			data.erase(data.begin() + i);
		}
	}
}

std::vector<Point> Node::search(Rectangle requestedRectangle)
{
	// All the nodes in the R-Tree that match
	std::vector<Point> matchingPoints = std::vector<Point>();

	if (children.size() == 0)
	{
		// We are a leaf so add our data points when they are within the search rectangle
		for (unsigned i = 0; i < data.size(); ++i)
		{
			if (requestedRectangle.containsPoint(data[i]))
			{
				matchingPoints.push_back(data[i]);
			}
		}
	}
	else
	{
		// Determine which branches we need to follow
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			if (boundingBoxes[i].intersectsRectangle(requestedRectangle))
			{
				// Recurse
				std::vector<Point> extra = children[i]->search(requestedRectangle);

				// Add the result to our set
				for (unsigned j = 0; j < extra.size(); ++j)
				{
					matchingPoints.push_back(extra[j]);
				}
			}
		}
	}
}

// Always called on root, this = root
// TODO: Write the analogous chooseLeaf(Rectangle searchRectangle)
Node *Node::chooseLeaf(Point givenPoint)
{
	assert(parent == nullptr);

	// CL1 [Initialize]
	Node *node = this;

	for (;;)
	{
		// CL2 [Leaf check]
		if (node->children.size() == 0)
		{
			return node;
		}
		else
		{
			// CL3 [Choose subtree]
			// Find the bounding box with least required expansion? Find the bounding box with least overlap?
			// TODO: Break ties by using smallest area
			unsigned smallestExpansionIndex = 0;
			double smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(givenPoint);
			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
			{
				double testExpansionArea = node->boundingBoxes[i].computeExpansionArea(givenPoint);
				if (smallestExpansionArea > testExpansionArea)
				{
					smallestExpansionIndex = i;
					smallestExpansionArea = testExpansionArea;
				}
			}

			// CL4 [Descend until a leaf is reached]
			node = node->children[smallestExpansionIndex];
		}
	}
}

// TODO: Write the analogous findLeaf(Rectangle searchRectangle)
Node *Node::findLeaf(Point givenPoint)
{
	Node *finalAnswer = nullptr;

	if (children.size() == 0)
	{
		// FL2 [Search leaf node for record]
		// Check each entry to see if it matches E
		for (int i = 0; i < data.size(); ++i)
		{
			if (data[i] == givenPoint)
			{
				finalAnswer = this;
				break;
			}
		}
	}
	else
	{
		// FL1 [Search subtrees]
		// Determine which branches we need to follow
		for (unsigned i = 0; i < boundingBoxes.size() && finalAnswer == nullptr; ++i)
		{
			if (boundingBoxes[i].containsPoint(givenPoint))
			{
				// Recurse
				finalAnswer = children[i]->findLeaf(givenPoint);
			}
		}
	}

	return finalAnswer;
}

Node *Node::splitNode(Node *newChild)
{
	// Setup the two groups which will be the entries in the two new nodes
	std::vector<unsigned> groupA;
	groupA.push_back(0);
	Rectangle boundingBoxA = boundingBoxes[0];

	std::vector<unsigned> groupB;
	groupB.push_back(0);
	Rectangle boundingBoxB = boundingBoxes[0];

	// Compute the first entry in each group based on PS1 & PS2
	double maxWasted = 0;
	for (unsigned i = 0; i < boundingBoxes.size(); ++i)
	{
		for (unsigned j = 0; j < boundingBoxes.size(); ++j)
		{
			double widthX = fabs(boundingBoxes[i].centre.x - boundingBoxes[j].centre.x) + boundingBoxes[i].radiusX + boundingBoxes[j].radiusX;
			double widthY = fabs(boundingBoxes[i].centre.y - boundingBoxes[j].centre.y) + boundingBoxes[i].radiusY + boundingBoxes[j].radiusY;
			double wasted = (widthX * widthY) - boundingBoxes[i].area() - boundingBoxes[j].area();

			if (wasted > maxWasted)
			{
				wasted = maxWasted;

				groupA[0] = i;
				boundingBoxA = boundingBoxes[i];

				groupB[0] = j;
				boundingBoxB = boundingBoxes[j];
			}
		}
	}

	// Go through the remaining entries and add them to groupA or groupB
	for (unsigned i = 0; i < boundingBoxes.size(); ++i)
	{
		// Skip the original two rectangles we chose
		if (i == groupA[0] || i == groupB[0])
		{
			continue;
		}

		// Choose the group which will need to expand the least
		double expansionAreaA = boundingBoxA.computeExpansionArea(boundingBoxes[i]);
		double expansionAreaB = boundingBoxB.computeExpansionArea(boundingBoxes[i]);

		if (expansionAreaB > expansionAreaA)
		{
			groupA.push_back(i);
			boundingBoxA.expand(boundingBoxes[i]);
		}
		else
		{
			groupB.push_back(i);
			boundingBoxB.expand(boundingBoxes[i]);
		}
	}

	// Create the new node and fill it with groupB entries
	Node *newSibling = new Node();
	for (int i = 0; i < groupB.size(); ++i)
	{
		newSibling->boundingBoxes.push_back(boundingBoxes[groupB[i]]);
		newSibling->children.push_back(children[groupB[i]]);
		boundingBoxes.erase(boundingBoxes.begin() + groupB[i]);
		children.erase(children.begin() + groupB[i]);
	}

	// Add newChild which caused this split in the first place
	Rectangle newBox = newChild->boundingBox();
	// Choose the group which will need to expand the least
	double expansionAreaA = boundingBoxA.computeExpansionArea(newBox);
	double expansionAreaB = boundingBoxB.computeExpansionArea(newBox);

	if (expansionAreaB > expansionAreaA)
	{
		boundingBoxes.push_back(newBox);
		children.push_back(newChild);
	}
	else
	{
		newSibling->boundingBoxes.push_back(newBox);
		newSibling->children.push_back(newChild);
	}

	// Return our newly minted sibling
	return newSibling;
}

Node *Node::splitNode(Point newData)
{
	// Setup the two groups which will be the entries in the two new nodes
	std::vector<unsigned> groupA;
	std::vector<unsigned> groupB;

	// Compute the first entry in each group based on PS1 & PS2
	double maxWasted = 0;
	for (unsigned i = 0; i < data.size(); ++i)
	{
		for (unsigned j = 0; j < data.size(); ++j)
		{
			double wasted = fabs(data[i].x - data[j].x) * fabs(data[i].y - data[j].y);

			if (wasted > maxWasted)
			{
				wasted = maxWasted;

				groupA[0] = i;
				groupB[0] = j;
			}
		}
	}

	// Set the bounding rectangles
	Rectangle boundingBoxA = Rectangle(data[groupA[0]].x, data[groupA[0]].y, 0, 0);
	Rectangle boundingBoxB = Rectangle(data[groupB[0]].x, data[groupB[0]].y, 0, 0);

	// Go through the remaining entries and add them to groupA or groupB
	for (unsigned i = 0; i < data.size(); ++i)
	{
		// Skip the original two rectangles we chose
		if (i == groupA[0] || i == groupB[0])
		{
			continue;
		}

		// Choose the group which will need to expand the least
		double expansionAreaA = boundingBoxA.computeExpansionArea(data[i]);
		double expansionAreaB = boundingBoxB.computeExpansionArea(data[i]);

		if (expansionAreaB > expansionAreaA)
		{
			groupA.push_back(i);
			boundingBoxA.expand(data[i]);
		}
		else
		{
			groupB.push_back(i);
			boundingBoxB.expand(data[i]);
		}
	}

	// Create the new node and fill it with groupB entries
	Node *newSibling = new Node();
	for (int i = 0; i < groupB.size(); ++i)
	{
		newSibling->data.push_back(data[groupB[i]]);
		data.erase(data.begin() + groupB[i]);
	}

	// Add newData which caused this split in the first place
	// Choose the group which will need to expand the least
	double expansionAreaA = boundingBoxA.computeExpansionArea(newData);
	double expansionAreaB = boundingBoxB.computeExpansionArea(newData);

	if (expansionAreaB > expansionAreaA)
	{
		data.push_back(newData);
	}
	else
	{
		newSibling->data.push_back(newData);
	}

	// Return our newly minted sibling
	return newSibling;
}

// To be called on the leaf l
Node *Node::adjustTree(Node *siblingLeaf)
{
	// AT1 [Initialize]
	Node *node = this;
	Node *siblingNode = siblingLeaf;

	for (;;)
	{
		// AT2 [If node is the root, stop]
		if (node->parent == nullptr)
		{
			break;
		}
		else
		{
			// AT3 [Adjust covering rectangle in parent entry]
			Node *parent = node->parent;
			parent->updateBoundingBox(node, node->boundingBox());

			// Did we have a split on the previous level
			if (siblingNode != nullptr)
			{
				// AT4 [Propogate the node split upwards]
				if (parent->children.size() == MAXBRANCHFACTOR)
				{
					Node *siblingParent = parent->splitNode(siblingNode);
					// AT5 [Move up to next level]
					node = parent;
					siblingNode = siblingParent;
				}
				else
				{
					// Create entry in p for nn
					parent->boundingBoxes.push_back(siblingNode->boundingBox());
					parent->children.push_back(siblingNode);
					siblingNode->parent = parent;

					// AT5 [Move up to next level]
					node = parent;
					siblingNode = nullptr;
				}
			}
		}
	}

	return siblingNode;
}

// Always called on root, this = root
Node *Node::insert(Point givenPoint)
{
	assert(parent == nullptr);

	// I1 [Find position for new record]
	Node *leaf = chooseLeaf(givenPoint);
	Node *siblingLeaf = nullptr;

	// I2 [Add record to leaf node]
	if (leaf->data.size() < MAXBRANCHFACTOR)
	{
		leaf->data.push_back(givenPoint);
	}
	else
	{
		siblingLeaf = leaf->splitNode(givenPoint);
	}

	// I3 [Propogate changes upward]
	Node *siblingNode = leaf->adjustTree(siblingLeaf);

	// I4 [Grow tree taller]
	if (siblingNode != nullptr)
	{
		Node *newRoot = new Node();
		newRoot->boundingBoxes.push_back(this->boundingBox());
		newRoot->children.push_back(this);
		newRoot->boundingBoxes.push_back(siblingNode->boundingBox());
		newRoot->children.push_back(siblingNode);

		return newRoot;
	}
	else
	{
		return this;
	}
}

// Always called on root, this = root
Node *Node::insert(Node *orphan, unsigned level)
{
	assert(parent == nullptr);

	// I0 [Initialization]
	Rectangle box = orphan->boundingBox();

	// I1 [Find position for new record]
	Node *leaf = chooseLeaf(box.centre);
	Node *siblingLeaf = nullptr;

	// I1.5 [Move up to level]
	for (unsigned i = 0; i < level; ++i)
	{
		leaf = leaf->parent;
	}

	// I2 [Add record to leaf node]
	if (leaf->children.size() < MAXBRANCHFACTOR)
	{
		leaf->boundingBoxes.push_back(box);
		leaf->children.push_back(orphan);
	}
	else
	{
		siblingLeaf = leaf->splitNode(orphan);
	}

	// I3 [Propogate changes upward]
	Node *siblingNode = leaf->adjustTree(siblingLeaf);

	// I4 [Grow tree taller]
	if (siblingNode != nullptr)
	{
		Node *newRoot = new Node();
		newRoot->boundingBoxes.push_back(this->boundingBox());
		newRoot->children.push_back(this);
		newRoot->boundingBoxes.push_back(siblingNode->boundingBox());
		newRoot->children.push_back(siblingNode);

		return newRoot;
	}
	else
	{
		return this;
	}
}

// To be called on the leaf l
void Node::condenseTree()
{
	assert(children.size() == 0);

	// CT1 [Initialize]
	Node *node = this;
	unsigned level = 0;

	std::vector<Node *> Q;
	std::vector<unsigned> Qlvl;

	// CT2 [Find parent entry]
	while (node->parent != nullptr)
	{
		Node *parent = node->parent;

		// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
		if (node->boundingBoxes.size() < MINBRANCHFACTOR)
		{
			parent->removeChild(node);
			Q.push_back(node);
			Qlvl.push_back(level);
		}
		else
		{
			parent->updateBoundingBox(node, node->boundingBox());
		}

		// CT5 [Move up one level in the tree]
		node = parent;
		level++;
	}

	// CT6 [Re-insert oprhaned entries]
	for (int i = 0; i < Q.size(); ++i)
	{
		node->insert(Q[i], Qlvl[i]);
	}
}

// Always called on root, this = root
Node *Node::remove(Point givenPoint)
{
	assert(parent == nullptr);

	// D1 [Find node containing record]
	Node *leaf = findLeaf(givenPoint);

	if (leaf == nullptr)
	{
		return nullptr;
	}

	// D2 [Delete record]
	leaf->removeData(givenPoint);

	// D3 [Propagate changes]
	leaf->condenseTree();
	delete leaf;

	// D4 [Shorten tree]
	if (children.size() == 1)
	{
		children[0]->parent = nullptr;
		return children[0];
	}
	else
	{
		return this;
	}
}

void Node::print()
{
	std::cout << "Node " << id << " {" << std::endl;
	for (int i = 0; i < boundingBoxes.size(); ++i)
	{
		boundingBoxes[i].print();
	}
	for (int i = 0; i < children.size(); ++i)
	{
		std::cout << "Child " << children[i]->id << std::endl;
	}
	for (int i = 0; i < data.size(); ++i)
	{
		data[i].print();
	}
	std::cout << "}" << std::endl;
}

void testBoundingBox()
{
	// Test set one
	Node testNode = Node();
	testNode.boundingBoxes.push_back(Rectangle(10.0, 3.0, 2.0, 2.0));
	testNode.boundingBoxes.push_back(Rectangle(14.0, -3.0, 2.0, 1.0));
	testNode.boundingBoxes.push_back(Rectangle(9.0, -5.0, 1.0, 1.0));

	assert(testNode.boundingBox() == Rectangle(12.0, -0.5, 4.0, 5.5));

	// Test set two
	Node testNode2 = Node();
	testNode2.boundingBoxes.push_back(Rectangle(9.0, 13.0, 1.0, 1.0));
	testNode2.boundingBoxes.push_back(Rectangle(11.0, 13.0, 1.0, 1.0));
	testNode2.boundingBoxes.push_back(Rectangle(13.0, 13.0, 1.0, 1.0));

	assert(testNode2.boundingBox() == Rectangle(11.0, 13.0, 3.0, 1.0));
}

void testUpdateBoundingBox()
{
	Node parentNode = Node();

	Node *child0 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(9.0, -5.0, 1.0, 1.0));
	parentNode.children.push_back(child0);

	Node *child1 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(14.0, -3.0, 2.0, 1.0));
	parentNode.children.push_back(child1);

	Node *child2 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(11.0, 13.0, 1.0, 1.0));
	parentNode.children.push_back(child2);

	Node *child3 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(13.0, 13.0, 1.0, 1.0));
	parentNode.children.push_back(child3);

	parentNode.updateBoundingBox(child3, Rectangle(4.0, 4.0, 1.0, 1.0));
	assert(parentNode.boundingBoxes[3] == Rectangle(4.0, 4.0, 1.0, 1.0));
}

void testRemoveChild()
{
	// Setup a node with some children
	Node parentNode = Node();

	Node *child0 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(9.0, -5.0, 1.0, 1.0));
	parentNode.children.push_back(child0);

	Node *child1 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(14.0, -3.0, 2.0, 1.0));
	parentNode.children.push_back(child1);

	Node *child2 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(11.0, 13.0, 1.0, 1.0));
	parentNode.children.push_back(child2);

	Node *child3 = new Node();
	parentNode.boundingBoxes.push_back(Rectangle(13.0, 13.0, 1.0, 1.0));
	parentNode.children.push_back(child3);

	// Remove one of the children
	parentNode.removeChild(child3);
	assert(parentNode.boundingBoxes.size() == 3);
	assert(parentNode.children.size() == 3);

	// Cleanup
	delete child0;
	delete child1;
	delete child2;
	delete child3;
}

void testRemoveData()
{
	// Setup a node with some data
	Node parentNode = Node();

	parentNode.boundingBoxes.push_back(Rectangle(9.0, -5.0, 1.0, 1.0));
	parentNode.data.push_back(Point(9.0, -5.0));

	parentNode.boundingBoxes.push_back(Rectangle(14.0, -3.0, 2.0, 1.0));
	parentNode.data.push_back(Point(14.0, -3.0));

	parentNode.boundingBoxes.push_back(Rectangle(11.0, 13.0, 1.0, 1.0));
	parentNode.data.push_back(Point(11.0, 13.0));

	parentNode.boundingBoxes.push_back(Rectangle(13.0, 13.0, 1.0, 1.0));
	parentNode.data.push_back(Point(13.0, 13.0));

	// Remove some of the data
	parentNode.removeData(Point(13.0, 13.0));
	assert(parentNode.boundingBoxes.size() == 3);
	assert(parentNode.data.size() == 3);
}

void testChooseLeaf()
{
	// Create nodes
	Node *root = new Node();
	Node *left = new Node();
	Node *right = new Node();
	Node *leftChild0 = new Node();
	Node *leftChild1 = new Node();
	Node *leftChild2 = new Node();
	Node *rightChild0 = new Node();
	Node *rightChild1 = new Node();
	Node *rightChild2 = new Node();

	// Setup nodes
	left->boundingBoxes.push_back(Rectangle(9.0, 13.0, 1.0, 1.0));
	left->children.push_back(leftChild0);
	left->boundingBoxes.push_back(Rectangle(11.0, 13.0, 1.0, 1.0));
	left->children.push_back(leftChild1);
	left->boundingBoxes.push_back(Rectangle(13.0, 13.0, 1.0, 1.0));
	left->children.push_back(leftChild2);

	right->boundingBoxes.push_back(Rectangle(10.0, 3.0, 2.0, 2.0));
	right->children.push_back(rightChild0);
	right->boundingBoxes.push_back(Rectangle(14.0, -3.0, 2.0, 1.0));
	right->children.push_back(rightChild1);
	right->boundingBoxes.push_back(Rectangle(9.0, -5.0, 1.0, 1.0));
	right->children.push_back(rightChild2);

	root->boundingBoxes.push_back(Rectangle(11.0, 13.0, 3.0, 1.0));
	root->children.push_back(left);

	root->boundingBoxes.push_back(Rectangle(12.0, -0.5, 4.0, 5.5));
	root->children.push_back(right);

	// Test that we get the right child for the given point
	assert(rightChild1 == root->chooseLeaf(Point(13.0, -3.0)));
	assert(leftChild0 == root->chooseLeaf(Point(8.5, 12.5)));
	assert(leftChild2 == root->chooseLeaf(Point(13.5, 13.5)));
	assert(rightChild0 == root->chooseLeaf(Point(7.0, 3.0)));
	assert(leftChild1 == root->chooseLeaf(Point(11.0, 15.0)));
	assert(leftChild0 == root->chooseLeaf(Point(4.0, 8.0)));
}

void testFindLeaf()
{
	return;
}

void testSplitNode()
{
	return;
}

void testAdjustTree()
{
	return;
}

void testCondenseTree()
{
	return;
}

void testSearch()
{
	return;
}

void testInsert()
{
	return;
}

void testRemove()
{
	return;
}
