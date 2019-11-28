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
	assert(data.size() > 0 || boundingBoxes.size() > 0);

	Rectangle boundingBox;
	if (boundingBoxes.size() > 0)
	{
		boundingBox = boundingBoxes[0];
		for (int i = 1; i < boundingBoxes.size(); ++i)
		{
			boundingBox.expand(boundingBoxes[i]);
		}
	}
	else
	{
		boundingBox = Rectangle(data[0], 0.0, 0.0);
		for (int i = 0; i < data.size(); ++i)
		{
			boundingBox.expand(data[i]);
		}
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

	return matchingPoints;
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
			// Find the bounding box with least required expansion/overlap?
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

// Always called odn root, this = root
Node *Node::chooseNode(ReinsertionEntry e)
{
	assert(parent == nullptr);

	// CL1 [Initialize]
	Node *node = this;

	for (;;)
	{
		// CL2 [Leaf check]
		if (node->children.size() == 0)
		{
			for (unsigned i = 0; i < e.level; ++i)
			{
				node = node->parent;
			}

			return node;
		}
		else
		{
			// CL3 [Choose subtree]
			// Find the bounding box with least required expansion/overlap?
			// TODO: Break ties by using smallest area
			unsigned smallestExpansionIndex = 0;
			double smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(e.boundingBox);
			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
			{
				double testExpansionArea = node->boundingBoxes[i].computeExpansionArea(e.boundingBox);
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
	Rectangle boundingBoxA = boundingBoxes[0];

	std::vector<unsigned> groupB;
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

			if (maxWasted < wasted)
			{
				maxWasted = wasted;

				boundingBoxA = boundingBoxes[i];
				boundingBoxB = boundingBoxes[j];
			}
		}
	}

	// Go through the remaining entries and add them to groupA or groupB
	for (unsigned i = 0; i < boundingBoxes.size(); ++i)
	{
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

	// Create the new node and fill it with groupB entries by doing complicated stuff
	Node *newSibling = new Node();
	unsigned groupALastIndex = groupA.size() - 1;
	for (unsigned i = 0; i < groupB.size(); ++i)
	{
		newSibling->boundingBoxes.push_back(boundingBoxes[groupB[i]]);
		newSibling->children.push_back(children[groupB[i]]);

		boundingBoxes[groupB[i]] = boundingBoxes[groupA[groupALastIndex]];
		children[groupB[i]] = children[groupA[groupALastIndex]];

		groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
	}
	boundingBoxes.resize(groupA.size());
	children.resize(groupA.size());

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
	double seedA, seedB;
	double maxWasted = 0;
	for (unsigned i = 0; i < data.size(); ++i)
	{
		for (unsigned j = 0; j < data.size(); ++j)
		{
			double wasted = fabs(data[i].x - data[j].x) * fabs(data[i].y - data[j].y);

			if (maxWasted < wasted)
			{
				maxWasted = wasted;

				seedA = i;
				seedB = j;
			}
		}
	}

	// Set the bounding rectangles
	Rectangle boundingBoxA = Rectangle(data[seedA].x, data[seedA].y, 0, 0);
	Rectangle boundingBoxB = Rectangle(data[seedB].x, data[seedB].y, 0, 0);

	// Go through the remaining entries and add them to groupA or groupB
	for (unsigned i = 0; i < data.size(); ++i)
	{
		// TODO: Is there an edge case where when considering one of the seeds, it is placed in the
		// incorrect group? We rely on the groups sorted in ascending order so that's why we
		// consider them here instead of adding them in the beginning

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

	// Create the new node and fill it with groupB entries by doing really complicated stuff
	Node *newSibling = new Node();
	unsigned groupALastIndex = groupA.size() - 1;
	for (unsigned i = 0; i < groupB.size(); ++i)
	{
		newSibling->data.push_back(data[groupB[i]]);
		data[groupB[i]] = data[groupA[groupALastIndex]];
		groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
	}
	data.resize(groupA.size());

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

Node *Node::adjustTree(Node *sibling)
{
	// AT1 [Initialize]
	Node *node = this;
	Node *siblingNode = sibling;

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
			else
			{
				// AT5 [Move up to next level]
				node = node->parent;
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
Node *Node::insert(ReinsertionEntry e)
{
	assert(parent == nullptr);

	if (e.level == 0)
	{
		return insert(e.data);
	}

	// I1 [Find position for new record]
	Node *node = chooseNode(e);
	Node *siblingNode = nullptr;

	// I2 [Add record to node]
	if (node->children.size() < MAXBRANCHFACTOR)
	{
		node->boundingBoxes.push_back(e.boundingBox);
		node->children.push_back(e.child);
	}
	else
	{
		siblingNode = node->splitNode(e.child);
	}

	// I3 [Propogate changes upward]
	siblingNode = node->adjustTree(siblingNode);

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

// To be called on a leaf
Node *Node::condenseTree()
{
	assert(children.size() == 0);

	// CT1 [Initialize]
	Node *node = this;
	unsigned level = 0;

	std::vector<ReinsertionEntry> Q;

	// CT2 [Find parent entry]
	while (node->parent != nullptr)
	{
		Node *parent = node->parent;

		// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
		if (node->boundingBoxes.size() >= MINBRANCHFACTOR || node->data.size() >= MINBRANCHFACTOR)
		{
			parent->updateBoundingBox(node, node->boundingBox());
		}
		else
		{
			// Remove ourselves from circulation
			parent->removeChild(node);

			// Add a reinsertion entry for each data point or branch of this node
			for (int i = 0; i < node->data.size(); ++i)
			{
				ReinsertionEntry e = {};
				e.data = node->data[i];
				e.level = 0;
				Q.push_back(e);
			}
			for (int i = 0; i < node->boundingBoxes.size(); ++i)
			{
				ReinsertionEntry e = {};
				e.boundingBox = node->boundingBoxes[i];
				e.child = node->children[i];
				e.level = level;
				Q.push_back(e);
			}
		}

		// CT5 [Move up one level in the tree]
		node = parent;
		level++;
	}

	// CT6 [Re-insert oprhaned entries]
	for (int i = 0; i < Q.size(); ++i)
	{
		node = node->insert(Q[i]);
	}

	return node;
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
	Node *root = leaf->condenseTree();
	delete leaf;

	// D4 [Shorten tree]
	if (root->children.size() == 1)
	{
		root->children[0]->parent = nullptr;
		return root->children[0];
	}
	else
	{
		return root;
	}
}

void Node::print()
{
	std::cout << "Node " << (void *)this << " {" << std::endl;
	std::cout << "    Parent: " << (void *)parent << std::endl;
	std::cout << "    Bounding Boxes: ";
	for (int i = 0; i < boundingBoxes.size(); ++i)
	{
		boundingBoxes[i].print();
	}
	std::cout << std::endl << "    Children: ";
	for (int i = 0; i < children.size(); ++i)
	{
		std::cout << (void *)children[i] << ' ';
	}
	std::cout << std::endl << "    Data: ";
	for (int i = 0; i < data.size(); ++i)
	{
		data[i].print();
	}
	std::cout << std::endl << "}" << std::endl;
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


	// Test the bounding box update
	parentNode.updateBoundingBox(child3, Rectangle(4.0, 4.0, 1.0, 1.0));
	assert(parentNode.boundingBoxes[3] == Rectangle(4.0, 4.0, 1.0, 1.0));


	// Cleanup
	delete child0;
	delete child1;
	delete child2;
	delete child3;
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


	// Test that we get the correct child for the given point
	assert(rightChild1 == root->chooseLeaf(Point(13.0, -3.0)));
	assert(leftChild0 == root->chooseLeaf(Point(8.5, 12.5)));
	assert(leftChild2 == root->chooseLeaf(Point(13.5, 13.5)));
	assert(rightChild0 == root->chooseLeaf(Point(7.0, 3.0)));
	assert(leftChild1 == root->chooseLeaf(Point(11.0, 15.0)));
	assert(leftChild0 == root->chooseLeaf(Point(4.0, 8.0)));


	// Cleanup
	delete root;
	delete left;
	delete right;
	delete leftChild0;
	delete leftChild1;
	delete leftChild2;
	delete rightChild0;
	delete rightChild1;
	delete rightChild2;
}

void testFindLeaf()
{
	// Setup the tree

	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two nodes
	Node *cluster4a = new Node();
	cluster4a->data.push_back(Point(-10.0, -2.0));
	cluster4a->data.push_back(Point(-12.0, -3.0));
	cluster4a->data.push_back(Point(-11.0, -3.0));
	cluster4a->data.push_back(Point(-10.0, -3.0));

	Node *cluster4b = new Node();
	cluster4b->data.push_back(Point(-9.0, -3.0));
	cluster4b->data.push_back(Point(-7.0, -3.0));
	cluster4b->data.push_back(Point(-10.0, -5.0));

	Node *cluster4 = new Node();
	cluster4->boundingBoxes.push_back(cluster4a->boundingBox());
	cluster4->children.push_back(cluster4a);
	cluster4->boundingBoxes.push_back(cluster4b->boundingBox());
	cluster4->children.push_back(cluster4b);

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)
	// Organized into four nodes
	Node *cluster5a = new Node();
	cluster5a->data.push_back(Point(-14.5, -13.0));
	cluster5a->data.push_back(Point(-14.0, -13.0));
	cluster5a->data.push_back(Point(-13.5, -13.5));
	cluster5a->data.push_back(Point(-15.0, -14.0));

	Node *cluster5b = new Node();
	cluster5b->data.push_back(Point(-14.0, -14.0));
	cluster5b->data.push_back(Point(-13.0, -14.0));
	cluster5b->data.push_back(Point(-12.0, -14.0));
	cluster5b->data.push_back(Point(-13.5, -16.0));

	Node *cluster5c = new Node();
	cluster5c->data.push_back(Point(-15.0, -14.5));
	cluster5c->data.push_back(Point(-14.0, -14.5));
	cluster5c->data.push_back(Point(-12.5, -14.5));
	cluster5c->data.push_back(Point(-13.5, -15.5));

	Node *cluster5d = new Node();
	cluster5d->data.push_back(Point(-15.0, -15.0));
	cluster5d->data.push_back(Point(-14.0, -15.0));
	cluster5d->data.push_back(Point(-13.0, -15.0));
	cluster5d->data.push_back(Point(-12.0, -15.0));

	Node *cluster5 = new Node();
	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	cluster5->children.push_back(cluster5a);
	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	cluster5->children.push_back(cluster5b);
	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	cluster5->children.push_back(cluster5c);
	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	cluster5->children.push_back(cluster5d);

	// Root
	Node *root = new Node();
	root->boundingBoxes.push_back(cluster4->boundingBox());
	root->children.push_back(cluster4);
	root->boundingBoxes.push_back(cluster5->boundingBox());
	root->children.push_back(cluster5);


	// Test finding leaves
	assert(root->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	assert(root->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	assert(root->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	assert(root->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	assert(root->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	assert(root->findLeaf(Point(-13.0, -15.0)) == cluster5d);


	// Cleanup
	delete cluster4a;
	delete cluster4b;
	delete cluster4;
	delete cluster5a;
	delete cluster5b;
	delete cluster5c;
	delete cluster5d;
	delete cluster5;
	delete root;
}

void testSplitNode()
{
	// Test set one
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	Node *cluster6 = new Node();
	cluster6->data.push_back(Point(-2.0, -6.0));
	cluster6->data.push_back(Point(2.0, -6.0));
	cluster6->data.push_back(Point(-1.0, -7.0));
	cluster6->data.push_back(Point(1.0, -7.0));
	cluster6->data.push_back(Point(3.0, -8.0));
	cluster6->data.push_back(Point(-2.0, -9.0));

	// Split the node in two
	Node *cluster6p = cluster6->splitNode(Point(-3.0, -11.0));

	// Test the split
	assert(cluster6->data.size() == 3);
	assert(cluster6->data[0] == Point(3.0, -8.0));
	assert(cluster6->data[1] == Point(2.0, -6.0));
	assert(cluster6->data[2] == Point(1.0, -7.0));
	assert(cluster6p->data.size() == 4);
	assert(cluster6p->data[0] == Point(-2.0, -6.0));
	assert(cluster6p->data[1] == Point(-1.0, -7.0));
	assert(cluster6p->data[2] == Point(-2.0, -9.0));
	assert(cluster6p->data[3] == Point(-3.0, -11.0));

	// Test set two
	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	Node *cluster2 = new Node();
	cluster2->data.push_back(Point(-14.0, 8.0));
	cluster2->data.push_back(Point(-10.0, 8.0));
	cluster2->data.push_back(Point(-9.0, 10.0));
	cluster2->data.push_back(Point(-9.0, 9.0));
	cluster2->data.push_back(Point(-8.0, 10.0));
	cluster2->data.push_back(Point(-9.0, 7.0));
	cluster2->data.push_back(Point(-8.0, 8.0));

	// Split the node in two
	Node *cluster2p = cluster2->splitNode(Point(-8.0, 9.0));

	// Test the split
	assert(cluster2->data.size() == 2);
	assert(cluster2->data[0] == Point(-14.0, 8.0));
	assert(cluster2->data[1] == Point(-10.0, 8.0));
	assert(cluster2p->data.size() == 6);
	assert(cluster2p->data[0] == Point(-9.0, 10.0));
	assert(cluster2p->data[1] == Point(-9.0, 9.0));
	assert(cluster2p->data[2] == Point(-8.0, 10.0));
	assert(cluster2p->data[3] == Point(-9.0, 7.0));
	assert(cluster2p->data[4] == Point(-8.0, 8.0));
	assert(cluster2p->data[5] == Point(-8.0, 9.0));

	// Test set three
	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// {(-5, 4), 1, 1}, {(-2, 4), 1, 1}, {(-1, 3), 1, 1}, {(-1, 1), 1, 1}, {(-3, 0), 1, 1},
	// {(-6, 2), 1, 1}
	Node *cluster5 = new Node();
	cluster5->boundingBoxes.push_back(Rectangle(-5.0, 4.0, 1.0, 1.0));
	cluster5->children.push_back((Node *)0x0);
	cluster5->boundingBoxes.push_back(Rectangle(-2.0, 4.0, 1.0, 1.0));
	cluster5->children.push_back((Node *)0x1);
	cluster5->boundingBoxes.push_back(Rectangle(-1.0, 3.0, 1.0, 1.0));
	cluster5->children.push_back((Node *)0x2);
	cluster5->boundingBoxes.push_back(Rectangle(-1.0, 1.0, 1.0, 1.0));
	cluster5->children.push_back((Node *)0x3);
	cluster5->boundingBoxes.push_back(Rectangle(-3.0, 0.0, 1.0, 1.0));
	cluster5->children.push_back((Node *)0x4);
	cluster5->boundingBoxes.push_back(Rectangle(-6.0, 2.0, 1.0, 1.0));
	cluster5->children.push_back((Node *)0x5);

	// Extra node causing the split
	Node *cluster5extra = new Node();
	cluster5extra->data.push_back(Point(1.0, 1.0));
	cluster5extra->data.push_back(Point(2.0, 2.0));


	// Test the split
	Node *cluster5p = cluster5->splitNode(cluster5extra);

	assert(cluster5->children.size() == 3);
	assert(cluster5->children[0] == (Node *)0x0);
	assert(cluster5->children[1] == (Node *)0x1);
	assert(cluster5->children[2] == (Node *)0x5);

	assert(cluster5->boundingBoxes.size() == 3);
	assert(cluster5->boundingBoxes[0] == Rectangle(-5.0, 4.0, 1.0, 1.0));
	assert(cluster5->boundingBoxes[1] == Rectangle(-2.0, 4.0, 1.0, 1.0));
	assert(cluster5->boundingBoxes[2] == Rectangle(-6.0, 2.0, 1.0, 1.0));

	assert(cluster5p->children.size() == 4);
	assert(cluster5p->children[0] == (Node *)0x2);
	assert(cluster5p->children[1] == (Node *)0x3);
	assert(cluster5p->children[2] == (Node *)0x4);
	assert(cluster5p->children[3] == cluster5extra);

	assert(cluster5p->boundingBoxes.size() == 4);
	assert(cluster5p->boundingBoxes[0] == Rectangle(-1.0, 3.0, 1.0, 1.0));
	assert(cluster5p->boundingBoxes[1] == Rectangle(-1.0, 1.0, 1.0, 1.0));
	assert(cluster5p->boundingBoxes[2] == Rectangle(-3.0, 0.0, 1.0, 1.0));
	assert(cluster5p->boundingBoxes[3] == Rectangle(1.5, 1.5, 0.5, 0.5));


	// Cleanup
	delete cluster2;
	delete cluster5;
	delete cluster5extra;
	delete cluster6;
}

void testAdjustTree()
{
	// Nodes
	Node *cluster4a;
	Node *cluster4b;
	Node *middle;
	Node *root;

	// Leaf Node and new sibling leaf
	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	cluster4a = new Node();
	cluster4a->data.push_back(Point(-10.0, -2.0));
	cluster4a->data.push_back(Point(-12.0, -3.0));
	cluster4a->data.push_back(Point(-11.0, -3.0));
	cluster4a->data.push_back(Point(-10.0, -3.0));

	cluster4b = new Node();
	cluster4b->data.push_back(Point(-9.0, -3.0));
	cluster4b->data.push_back(Point(-7.0, -3.0));
	cluster4b->data.push_back(Point(-10.0, -5.0));

	// Middle Node
	middle = new Node();
	middle->boundingBoxes.push_back(Rectangle(-9.0, 3.0, 1.0, 1.0));
	middle->children.push_back(nullptr);
	middle->boundingBoxes.push_back(Rectangle(-11.0, -9.0, 1.0, 1.0));
	middle->children.push_back(nullptr);
	middle->boundingBoxes.push_back(Rectangle(-9.5, -3.5, 2.5, 1.5));
	middle->children.push_back(cluster4a);
	middle->boundingBoxes.push_back(Rectangle(7.0, 7.0, 1.0, 1.0));
	middle->children.push_back(nullptr);
	middle->boundingBoxes.push_back(Rectangle(16.0, -16.0, 1.0, 1.0));
	middle->children.push_back(nullptr);

	// Root Node
	root = new Node();
	root->boundingBoxes.push_back(middle->boundingBox());
	root->children.push_back(middle);

	// Chain parent pointers together
	cluster4a->parent = middle;
	middle->parent = root;


	// Adjust the tree
	Node *result = cluster4a->adjustTree(cluster4b);


	// Test the adjustment
	assert(result == nullptr);
	assert(root->children.size() == 2);
	assert(root->boundingBoxes[0] == Rectangle(-9.5, -3.0, 2.5, 7.0));
	assert(root->boundingBoxes[1] == Rectangle(11.5, -4.5, 5.5, 12.5));
	assert(middle->children.size() == 4);
	assert(middle->boundingBoxes[2] == Rectangle(-11.0, -2.5, 1.0, 0.5));
	assert(middle->boundingBoxes[3] == Rectangle(-8.5, -4.0, 1.5, 1.0));


	// Cleanup
	delete cluster4a;
	delete cluster4b;
	delete middle;
	delete root;
}

void testCondenseTree()
{
	// Test where the leaf is the root
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	Node *cluster6 = new Node();
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
	assert(cluster6->parent == nullptr);
	assert(cluster6->boundingBoxes.size() == 0);
	assert(cluster6->children.size() == 0);
	assert(cluster6->data.size() == 7);


	// Cleanup
	delete cluster6;


	// Test where condensing is confined to a leaf != root
	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	cluster6 = new Node();
	cluster6->data.push_back(Point(-2.0, -6.0));
	cluster6->data.push_back(Point(2.0, -6.0));
	cluster6->data.push_back(Point(-1.0, -7.0));
	cluster6->data.push_back(Point(1.0, -7.0));
	cluster6->data.push_back(Point(3.0, -8.0));
	cluster6->data.push_back(Point(-2.0, -9.0));
	// cluster6->data.push_back(Point(-3.0, -11.0)); left out so the bounding box should change

	Node *root = new Node();
	root->boundingBoxes.push_back(Rectangle(0.0, -8.0, 3.0, 2.0));
	root->children.push_back(cluster6);
	cluster6->parent = root;


	// Condense the tree
	cluster6->condenseTree();


	// Test the condensing
	assert(root->parent == nullptr);
	assert(root->boundingBoxes.size() == 1);
	assert(root->children.size() == 1);
	assert(root->boundingBoxes[0] == Rectangle(0.5, -7.5, 2.5, 1.5));
	assert(root->children[0] == cluster6);
	assert(cluster6->parent == root);
	assert(cluster6->boundingBoxes.size() == 0);
	assert(cluster6->children.size() == 0);
	assert(cluster6->data.size() == 6);


	// Cleanup
	delete cluster6;
	delete root;


	// Test where condensing is unconfined to a leaf != root
	// Cluster 4, n = 7
	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// Organized into two nodes
	Node *cluster4a = new Node();
	cluster4a->data.push_back(Point(-10.0, -2.0));
	cluster4a->data.push_back(Point(-12.0, -3.0));
	cluster4a->data.push_back(Point(-11.0, -3.0));
	cluster4a->data.push_back(Point(-10.0, -3.0));

	Node *cluster4b = new Node();
	cluster4b->data.push_back(Point(-9.0, -3.0));
	cluster4b->data.push_back(Point(-7.0, -3.0));
	// cluster4b->data.push_back(Point(-10.0, -5.0)); left out to precipitate condensing

	Node *cluster4 = new Node();
	cluster4->boundingBoxes.push_back(cluster4a->boundingBox());
	cluster4->children.push_back(cluster4a);
	cluster4->boundingBoxes.push_back(cluster4b->boundingBox());
	cluster4->children.push_back(cluster4b);

	// Cluster 5, n = 16
	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// (-14, -15), (-13, -15), (-12, -15)
	// Organized into four nodes
	Node *cluster5a = new Node();
	cluster5a->data.push_back(Point(-14.5, -13.0));
	cluster5a->data.push_back(Point(-14.0, -13.0));
	cluster5a->data.push_back(Point(-13.5, -13.5));
	cluster5a->data.push_back(Point(-15.0, -14.0));

	Node *cluster5b = new Node();
	cluster5b->data.push_back(Point(-14.0, -14.0));
	cluster5b->data.push_back(Point(-13.0, -14.0));
	cluster5b->data.push_back(Point(-12.0, -14.0));
	cluster5b->data.push_back(Point(-13.5, -16.0));

	Node *cluster5c = new Node();
	cluster5c->data.push_back(Point(-15.0, -14.5));
	cluster5c->data.push_back(Point(-14.0, -14.5));
	cluster5c->data.push_back(Point(-12.5, -14.5));
	cluster5c->data.push_back(Point(-13.5, -15.5));

	Node *cluster5d = new Node();
	cluster5d->data.push_back(Point(-15.0, -15.0));
	cluster5d->data.push_back(Point(-14.0, -15.0));
	cluster5d->data.push_back(Point(-13.0, -15.0));
	cluster5d->data.push_back(Point(-12.0, -15.0));

	Node *cluster5 = new Node();
	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	cluster5->children.push_back(cluster5a);
	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	cluster5->children.push_back(cluster5b);
	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	cluster5->children.push_back(cluster5c);
	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	cluster5->children.push_back(cluster5d);

	// Root
	root = new Node();
	root->boundingBoxes.push_back(cluster4->boundingBox());
	root->children.push_back(cluster4);
	root->boundingBoxes.push_back(cluster5->boundingBox());
	root->children.push_back(cluster5);

	// Link the tree together
	cluster4->parent = root;
	cluster4a->parent = cluster4;
	cluster4b->parent = cluster4;
	cluster5->parent = root;
	cluster5a->parent = cluster5;
	cluster5b->parent = cluster5;
	cluster5c->parent = cluster5;
	cluster5d->parent = cluster5;


	// Condense the tree
	Node *newRoot = cluster4b->condenseTree();


	// Test the condensing
	assert(newRoot == root);
	assert(root->boundingBoxes.size() == 2);
	assert(root->children.size() == 2);
	assert(root->children[0]->children.size() == 4);
	assert(root->children[1]->children.size() == 2);
	assert(root->children[1]->children[0]->data.size() == 2);
	assert(root->children[1]->children[0]->data[0] == Point(-9.0, -3.0));
	assert(root->children[1]->children[0]->data[1] == Point(-7.0, -3.0));


	// Cleanup
	delete cluster4a;
	delete cluster4b;
	delete cluster4;
	delete cluster5a;
	delete cluster5b;
	delete cluster5c;
	delete cluster5d;
	delete cluster5;
	delete root;
}

void testSearch()
{
	// Build the tree directly

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
	Node *cluster1a = new Node();
	cluster1a->data.push_back(Point(-3.0, 16.0));
	cluster1a->data.push_back(Point(-3.0, 15.0));
	cluster1a->data.push_back(Point(-4.0, 13.0));

	Node *cluster1b = new Node();
	cluster1b->data.push_back(Point(-5.0, 12.0));
	cluster1b->data.push_back(Point(-5.0, 15.0));
	cluster1b->data.push_back(Point(-6.0, 14.0));
	cluster1b->data.push_back(Point(-8.0, 16.0));

	// Cluster 2, n = 8
	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	Node *cluster2a = new Node();
	cluster2a->data.push_back(Point(-8.0, 10.0));
	cluster2a->data.push_back(Point(-9.0, 10.0));
	cluster2a->data.push_back(Point(-8.0, 9.0));
	cluster2a->data.push_back(Point(-9.0, 9.0));
	cluster2a->data.push_back(Point(-8.0, 8.0));

	Node *cluster2b = new Node();
	cluster2b->data.push_back(Point(-14.0, 8.0));
	cluster2b->data.push_back(Point(-10.0, 8.0));
	cluster2b->data.push_back(Point(-9.0, 7.0));

	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	Node *cluster3a = new Node();
	cluster3a->data.push_back(Point(-3.0, 4.0));
	cluster3a->data.push_back(Point(-3.0, 0.0));
	cluster3a->data.push_back(Point(-2.0, 4.0));
	cluster3a->data.push_back(Point(-1.0, 3.0));
	cluster3a->data.push_back(Point(-1.0, 1.0));

	Node *cluster3b = new Node();
	cluster3b->data.push_back(Point(-5.0, 4.0));
	cluster3b->data.push_back(Point(-4.0, 3.0));
	cluster3b->data.push_back(Point(-4.0, 1.0));
	cluster3b->data.push_back(Point(-6.0, 2.0));

	// High level nodes
	Node *left = new Node();
	left->boundingBoxes.push_back(cluster1a->boundingBox());
	left->children.push_back(cluster1a);
	left->boundingBoxes.push_back(cluster1b->boundingBox());
	left->children.push_back(cluster1b);
	left->boundingBoxes.push_back(cluster2a->boundingBox());
	left->children.push_back(cluster2a);
	left->boundingBoxes.push_back(cluster2b->boundingBox());
	left->children.push_back(cluster2b);

	Node *right = new Node();
	right->boundingBoxes.push_back(cluster3a->boundingBox());
	right->children.push_back(cluster3a);
	right->boundingBoxes.push_back(cluster3b->boundingBox());
	right->children.push_back(cluster3b);

	Node *root = new Node();
	root->boundingBoxes.push_back(left->boundingBox());
	root->children.push_back(left);
	root->boundingBoxes.push_back(right->boundingBox());
	root->children.push_back(right);


	// Test search

	// Test set one
	std::vector<Point> v1 = root->search(Rectangle(-7.0, 11.0, 2.0, 1.5));
	assert(v1.size() == 3);
	assert(v1[0] == Point(-5.0, 12.0));
	assert(v1[1] == Point(-8.0, 10.0));
	assert(v1[2] == Point(-9.0, 10.0));

	// Test set two
	std::vector<Point> v2 = root->search(Rectangle(-6.5, 6.0, 1.5, 2.0));
	assert(v2.size() == 2);
	assert(v2[0] == Point(-8.0, 8.0));
	assert(v2[1] == Point(-5.0, 4.0));

	// Test set three
	std::vector<Point> v3 = root->search(Rectangle(-6.0, 8.0, 2.0, 8.0));
	assert(v3.size() == 12);
	assert(v3[0] == Point(-4.0, 13.0));
	assert(v3[1] == Point(-5.0, 12.0));
	assert(v3[2] == Point(-5.0, 15.0));
	assert(v3[3] == Point(-6.0, 14.0));
	assert(v3[4] == Point(-8.0, 16.0));
	assert(v3[5] == Point(-8.0, 10.0));
	assert(v3[6] == Point(-8.0, 9.0));
	assert(v3[7] == Point(-8.0, 8.0));
	assert(v3[8] == Point(-5.0, 4.0));
	assert(v3[9] == Point(-4.0, 3.0));
	assert(v3[10] == Point(-4.0, 1.0));
	assert(v3[11] == Point(-6.0, 2.0));

	// Test set four
	std::vector<Point> v4 = root->search(Rectangle(3.0, -3.0, 1.0, 1.0));
	assert(v4.size() == 0);

	// Test set five
	std::vector<Point> v5 = root->search(Rectangle(-2.5, 2.0, 1.0, 1.0));
	assert(v5.size() == 0);


	// Cleanup
	delete cluster1a;
	delete cluster1b;
	delete cluster2a;
	delete cluster2b;
	delete cluster3a;
	delete cluster3b;
	delete left;
	delete right;
	delete root;
}

void testInsert()
{
	return;
}

void testRemove()
{
	// std::cout << "----------------------------" << std::endl;
	// node->print();
	// for (unsigned i = 0; i < node->children.size(); ++i)
	// {
	// 	std::cout << "----------------------------" << std::endl;
	// 	node->children[i]->print();
	// 	std::cout << "----------------------------" << std::endl;
	// 	for (int j = 0; j < node->children[i]->children.size(); ++j)
	// 	{
	// 		node->children[i]->children[j]->print();
	// 	}
	// 	std::cout << "----------------------------" << std::endl;
	// }
}
