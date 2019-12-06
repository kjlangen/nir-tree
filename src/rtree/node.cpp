#include <rtree/node.h>

Node::Node()
{
	minBranchFactor = 3;
	maxBranchFactor = 5;
	parent = nullptr;
	boundingBoxes.resize(0);
	children.resize(0);
	data.resize(0);
}

Node::Node(unsigned minBranchFactor, unsigned maxBranchFactor, Node *p)
{
	this->minBranchFactor = minBranchFactor;
	this->maxBranchFactor = maxBranchFactor;
	this->parent = p;
	boundingBoxes.resize(0);
	children.resize(0);
	data.resize(0);
}

void Node::deleteSubtrees()
{
	if (children.size() == 0)
	{
		return;
	}
	else
	{
		for (unsigned i = 0; i < children.size(); ++i)
		{
			children[i]->deleteSubtrees();
			delete children[i];
		}
	}
}

Rectangle Node::boundingBox()
{
	assert(data.size() > 0 || boundingBoxes.size() > 0);

	Rectangle boundingBox;
	if (boundingBoxes.size() > 0)
	{
		boundingBox = boundingBoxes[0];
		for (unsigned i = 1; i < boundingBoxes.size(); ++i)
		{
			boundingBox.expand(boundingBoxes[i]);
		}
	}
	else
	{
		boundingBox = Rectangle(data[0], 0.0, 0.0);
		for (unsigned i = 0; i < data.size(); ++i)
		{
			boundingBox.expand(data[i]);
		}
	}

	return boundingBox;
}

// TODO: Optimize maybe
void Node::updateBoundingBox(Node *child, Rectangle updatedBoundingBox)
{
	assert(children.size() == boundingBoxes.size());
	for (unsigned i = 0; i < children.size(); ++i)
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
	for (unsigned i = 0; i < children.size(); ++i)
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
	for (unsigned i = 0; i < data.size(); ++i)
	{
		if (data[i] == givenPoint)
		{
			data.erase(data.begin() + i);
			break;
		}
	}
}

void Node::search(Rectangle &requestedRectangle, std::vector<Point> &accumulator)
{
	if (children.size() == 0)
	{
		// We are a leaf so add our data points when they are within the search rectangle
		for (unsigned i = 0; i < data.size(); ++i)
		{
			if (requestedRectangle.containsPoint(data[i]))
			{
				accumulator.push_back(data[i]);
			}
		}
	}
	else
	{
		// Determine which branches we need to follow
		unsigned searchedBranches = 0;
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			if (boundingBoxes[i].intersectsRectangle(requestedRectangle))
			{
				// Recurse
				children[i]->search(requestedRectangle, accumulator);
				searchedBranches++;
			}
		}
		std::cout << "[search] searched " << searchedBranches << " branches of " << boundingBoxes.size() << " total branches." << std::endl;
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
			// Find the bounding box with least required expansion/overlap?
			// TODO: Break ties by using smallest area
			unsigned smallestExpansionIndex = 0;
			float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(givenPoint);
			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
			{
				float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(givenPoint);
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

// Always called on root, this = root
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
			float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(e.boundingBox);
			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
			{
				float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(e.boundingBox);
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
	if (children.size() == 0)
	{
		// FL2 [Search leaf node for record]
		// Check each entry to see if it matches E
		for (unsigned i = 0; i < data.size(); ++i)
		{
			if (data[i] == givenPoint)
			{
				// finalAnswer = this;
				return this;
			}
		}
	}
	else
	{
		// FL1 [Search subtrees]
		// Determine which branches we need to follow
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			if (boundingBoxes[i].containsPoint(givenPoint))
			{
				// Recurse
				Node *recurseResult = children[i]->findLeaf(givenPoint);
				if (recurseResult != nullptr)
				{
					return recurseResult;
				}
			}
		}
	}

	return nullptr;
}

Node *Node::splitNode(Node *newChild)
{
	// std::cout << "[splitNode] Begin" << std::endl;
	// Setup the two groups which will be the entries in the two new nodes
	// std::cout << "SN1" << std::endl;
	std::vector<unsigned> groupA;
	Rectangle boundingBoxA = Rectangle(0.0, 0.0, 0.0, 0.0); // boundingBoxes[0];

	std::vector<unsigned> groupB;
	Rectangle boundingBoxB = Rectangle(0.0, 0.0, 0.0, 0.0); // boundingBoxes[1];

	// Compute the first entry in each group based on PS1 & PS2
	// std::cout << "SN2" << std::endl;
	float seedA = 0;
	float seedB = 0;
	float maxWasted = 0;
	for (unsigned i = 0; i < boundingBoxes.size(); ++i)
	{
		for (unsigned j = 0; j < boundingBoxes.size(); ++j)
		{
			float xdist = fabs(boundingBoxes[i].centre.x - boundingBoxes[j].centre.x);
			float ydist = fabs(boundingBoxes[i].centre.y - boundingBoxes[j].centre.y);

			// float xRadiiSum = boundingBoxes[i].radiusX + boundingBoxes[j].radiusX;
			// float yRadiiSum = boundingBoxes[i].radiusY + boundingBoxes[j].radiusY;

			// float xoverlap = xdist < xRadiiSum ? xRadiiSum - xdist : 0;
			// float yoverlap = ydist < yRadiiSum ? yRadiiSum - ydist : 0;

			// float largeArea = (xdist + xRadiiSum) * (ydist + yRadiiSum);
			// float overlapArea = xoverlap * yoverlap;
			// float wasted = largeArea - boundingBoxes[i].area() - boundingBoxes[j].area() + overlapArea;

			float wasted = xdist * xdist + ydist * ydist;

			// std::cout << "[splitNode] largeArea: " << largeArea << std::endl;
			// std::cout << "[splitNode] boundingBoxes[i].area(): " << boundingBoxes[i].area() << std::endl;
			// std::cout << "[splitNode] boundingBoxes[j].area(): " << boundingBoxes[j].area() << std::endl;
			// std::cout << "[splitNode] overlapArea: " << overlapArea << std::endl;
			// std::cout << "[splitNode] wasted: " << wasted << std::endl;

			if (maxWasted < wasted)
			{
				maxWasted = wasted;

				// boundingBoxA = boundingBoxes[i];
				// boundingBoxB = boundingBoxes[j];

				seedA = i;
				seedB = j;
			}
		}
	}

	boundingBoxA = boundingBoxes[seedA];
	boundingBoxB = boundingBoxes[seedB];
	// boundingBoxA.print();
	// std::cout << std::endl;
	// boundingBoxB.print();
	// std::cout << std::endl;

	if (boundingBoxA == boundingBoxB)
	{	
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			boundingBoxes[i].print();
			std::cout << std::endl;
		}
		assert(!(boundingBoxA == boundingBoxB));
	}

	// Go through the remaining entries and add them to groupA or groupB
	// std::cout << "SN3" << std::endl;
	for (unsigned i = 0; i < boundingBoxes.size(); ++i)
	{
		if (i == seedA)
		{
			groupA.push_back(i);
			continue;
		}
		else if (i == seedB)
		{
			groupB.push_back(i);
			continue;
		}

		// Choose the group which will need to expand the least
		// std::cout << "[splitNode] Checking bounding box " << i << " "; boundingBoxes[i].print();
		float expansionAreaA = boundingBoxA.computeExpansionArea(boundingBoxes[i]);
		float expansionAreaB = boundingBoxB.computeExpansionArea(boundingBoxes[i]);
		// std::cout << std::endl << "[splitNode] expansionAreaA: " << expansionAreaA << std::endl;
		// std::cout << "[splitNode] expansionAreaB: " << expansionAreaB << std::endl;

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
	// std::cout << "SN4" << std::endl;
	Node *newSibling = new Node(minBranchFactor, maxBranchFactor);
	unsigned groupALastIndex = groupA.size() - 1;
	// std::cout << "[splitNode] groupA.size(): " << groupA.size() << std::endl;
	// std::cout << "[splitNode] groupB.size(): " << groupB.size() << std::endl;
	// std::cout << "[splitNode] groupALastIndex: " << groupALastIndex << std::endl;
	for (unsigned i = 0; i < groupB.size(); ++i)
	{
		newSibling->boundingBoxes.push_back(boundingBoxes[groupB[i]]);
		newSibling->children.push_back(children[groupB[i]]);

		boundingBoxes[groupB[i]] = boundingBoxes[groupA[groupALastIndex]];
		// std::cout << "SN4.1" << std::endl;
		children[groupB[i]] = children[groupA[groupALastIndex]];
		// std::cout << "SN4.2" << std::endl;

		groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
	}
	boundingBoxes.resize(groupA.size());
	children.resize(groupA.size());

	// Add newChild which caused this split in the first place
	// std::cout << "SN5" << std::endl;
	Rectangle newBox = newChild->boundingBox();
	// Choose the group which will need to expand the least
	// std::cout << "SN6" << std::endl;
	float expansionAreaA = boundingBoxA.computeExpansionArea(newBox);
	float expansionAreaB = boundingBoxB.computeExpansionArea(newBox);

	if (expansionAreaB > expansionAreaA)
	{
		newChild->parent = this;
		boundingBoxes.push_back(newBox);
		children.push_back(newChild);
	}
	else
	{
		newChild->parent = newSibling;
		newSibling->boundingBoxes.push_back(newBox);
		newSibling->children.push_back(newChild);
	}

	// std::cout << "[splitNode] End" << std::endl;

	// Return our newly minted sibling
	// std::cout << "SN7" << std::endl;
	return newSibling;
}

// TODO: Because we're using vectors and didn't exactly implement the original R-Tree rewriting this
// with sets is necessary and that with necessitate rewriting the entire R-Tree with sets.
Node *Node::splitNode(Point newData)
{
	// std::cout << "[splitNode] Begin" << std::endl;
	// Setup the two groups which will be the entries in the two new nodes
	std::vector<unsigned> groupA;
	std::vector<unsigned> groupB;

	// Compute the first entry in each group based on PS1 & PS2
	unsigned seedA = 0;
	unsigned seedB = data.size() - 1;

	float xdist = fabs(data[seedA].x - data[seedB].x);
	float ydist = fabs(data[seedA].y - data[seedB].y);

	float maxWasted = xdist * xdist + ydist * ydist;

	for (unsigned i = 0; i < data.size(); ++i)
	{
		for (unsigned j = 0; j < data.size(); ++j)
		{
			xdist = fabs(data[i].x - data[j].x);
			ydist = fabs(data[i].y - data[j].y);
			float wasted = xdist * xdist + ydist * ydist;

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
		if (i == seedA)
		{
			groupA.push_back(i);
			boundingBoxA.expand(data[i]);
			continue;
		}
		else if (i == seedB)
		{
			groupB.push_back(i);
			boundingBoxB.expand(data[i]);
			continue;
		}

		// Choose the group which will need to expand the least
		float expansionAreaA = boundingBoxA.computeExpansionArea(data[i]);
		float expansionAreaB = boundingBoxB.computeExpansionArea(data[i]);

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
	Node *newSibling = new Node(minBranchFactor, maxBranchFactor);
	unsigned groupALastIndex = groupA.size() - 1;
	// std::cout << "[splitNode] groupA.size(): " << groupA.size() << std::endl;
	// std::cout << "[splitNode] groupB.size(): " << groupB.size() << std::endl;
	// std::cout << "[splitNode] groupALastIndex: " << groupALastIndex << std::endl;
	for (unsigned i = 0; i < groupB.size(); ++i)
	{
		newSibling->data.push_back(data[groupB[i]]);
		data[groupB[i]] = data[groupA[groupALastIndex]];
		groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
	}
	data.resize(groupA.size());

	// Add newData which caused this split in the first place
	// Choose the group which will need to expand the least
	float expansionAreaA = boundingBoxA.computeExpansionArea(newData);
	float expansionAreaB = boundingBoxB.computeExpansionArea(newData);

	if (expansionAreaB > expansionAreaA)
	{
		data.push_back(newData);
	}
	else
	{
		newSibling->data.push_back(newData);
	}

	// std::cout << "[splitNode] End" << std::endl;

	// Return our newly minted sibling
	return newSibling;
}

Node *Node::adjustTree(Node *sibling)
{
	// AT1 [Initialize]
	// std::cout << "AT1" << std::endl;
	Node *node = this;
	Node *siblingNode = sibling;

	for (;;)
	{
		// AT2 [If node is the root, stop]
		// std::cout << "AT2" << std::endl;
		if (node->parent == nullptr)
		{
			break;
		}
		else
		{
			// AT3 [Adjust covering rectangle in parent entry]
			// std::cout << "AT3" << std::endl;
			Node *parent = node->parent;
			parent->updateBoundingBox(node, node->boundingBox());

			// Did we have a split on the previous level
			if (siblingNode != nullptr)
			{
				// AT4 [Propogate the node split upwards]
				// std::cout << "AT4" << std::endl;
				if (parent->children.size() == parent->maxBranchFactor)
				{
					Node *siblingParent = parent->splitNode(siblingNode);

					// AT5 [Move up to next level]
					// std::cout << "AT5" << std::endl;
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
					// std::cout << "AT5'" << std::endl;
					node = parent;
					siblingNode = nullptr;
				}
			}
			else
			{
				// AT5 [Move up to next level]
				// std::cout << "AT5''" << std::endl;
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
	if (leaf->data.size() < leaf->maxBranchFactor)
	{
		leaf->data.push_back(givenPoint);
	}
	else
	{
		assert(leaf != nullptr);
		siblingLeaf = leaf->splitNode(givenPoint);
	}

	// I3 [Propogate changes upward]
	Node *siblingNode = leaf->adjustTree(siblingLeaf);

	// I4 [Grow tree taller]
	if (siblingNode != nullptr)
	{
		Node *newRoot = new Node(minBranchFactor, maxBranchFactor);
		this->parent = newRoot;
		newRoot->boundingBoxes.push_back(this->boundingBox());
		newRoot->children.push_back(this);
		siblingNode->parent = newRoot;
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

	// If reinserting a leaf then use normal insert
	if (e.level == 0)
	{
		return insert(e.data);
	}

	// I1 [Find position for new record]
	Node *node = chooseNode(e);
	Node *siblingNode = nullptr;

	// I2 [Add record to node]
	if (node->children.size() < node->maxBranchFactor)
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
		Node *newRoot = new Node(minBranchFactor, maxBranchFactor);
		this->parent = newRoot;
		newRoot->boundingBoxes.push_back(this->boundingBox());
		newRoot->children.push_back(this);
		siblingNode->parent = newRoot;
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
		if (node->boundingBoxes.size() >= node->minBranchFactor || node->data.size() >= node->minBranchFactor)
		{
			parent->updateBoundingBox(node, node->boundingBox());
		}
		else
		{
			// Remove ourselves from circulation
			parent->removeChild(node);

			// Add a reinsertion entry for each data point or branch of this node
			for (unsigned i = 0; i < node->data.size(); ++i)
			{
				ReinsertionEntry e = {};
				e.data = node->data[i];
				e.level = 0;
				Q.push_back(e);
			}
			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
			{
				ReinsertionEntry e = {};
				e.boundingBox = node->boundingBoxes[i];
				e.child = node->children[i];
				e.level = level;
				Q.push_back(e);
			}

			// Cleanup ourselves without deleting children b/c they will be reinserted
			delete node;
		}

		// CT5 [Move up one level in the tree]
		node = parent;
		level++;
	}

	// CT6 [Re-insert oprhaned entries]
	for (unsigned i = 0; i < Q.size(); ++i)
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
	// std::cout << "D1" << std::endl;
	Node *leaf = findLeaf(givenPoint);

	if (leaf == nullptr)
	{
		// std::cout << "[remove] Returning null" << std::endl;
		return nullptr;
	}

	// D2 [Delete record]
	// std::cout << "D2" << std::endl;
	leaf->removeData(givenPoint);

	// D3 [Propagate changes]
	// std::cout << "D3" << std::endl;
	Node *root = leaf->condenseTree();

	// D4 [Shorten tree]
	// std::cout << "D4" << std::endl;
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

	// Test the removal
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
	assert(cluster6->data.size() == 2);
	assert(cluster6->data[0] == Point(-2.0, -6.0));
	assert(cluster6->data[1] == Point(2.0, -6.0));
	assert(cluster6p->data.size() == 5);
	assert(cluster6p->data[0] == Point(-1.0, -7.0));
	assert(cluster6p->data[1] == Point(1.0, -7.0));
	assert(cluster6p->data[2] == Point(3.0, -8.0));
	assert(cluster6p->data[3] == Point(-2.0, -9.0));
	assert(cluster6p->data[4] == Point(-3.0, -11.0));

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
	Node *cluster3 = new Node();
	cluster3->boundingBoxes.push_back(Rectangle(-5.0, 4.0, 1.0, 1.0));
	cluster3->children.push_back((Node *)0x0);
	cluster3->boundingBoxes.push_back(Rectangle(-2.0, 4.0, 1.0, 1.0));
	cluster3->children.push_back((Node *)0x1);
	cluster3->boundingBoxes.push_back(Rectangle(-1.0, 3.0, 1.0, 1.0));
	cluster3->children.push_back((Node *)0x2);
	cluster3->boundingBoxes.push_back(Rectangle(-1.0, 1.0, 1.0, 1.0));
	cluster3->children.push_back((Node *)0x3);
	cluster3->boundingBoxes.push_back(Rectangle(-3.0, 0.0, 1.0, 1.0));
	cluster3->children.push_back((Node *)0x4);
	cluster3->boundingBoxes.push_back(Rectangle(-6.0, 2.0, 1.0, 1.0));
	cluster3->children.push_back((Node *)0x5);

	// Extra node causing the split
	Node *cluster3extra = new Node();
	cluster3extra->data.push_back(Point(1.0, 1.0));
	cluster3extra->data.push_back(Point(2.0, 2.0));

	// Test the split
	Node *cluster3p = cluster3->splitNode(cluster3extra);

	assert(cluster3->children.size() == 3);
	assert(cluster3->children[0] == (Node *)0x0);
	assert(cluster3->children[1] == (Node *)0x1);
	assert(cluster3->children[2] == (Node *)0x5);

	assert(cluster3->boundingBoxes.size() == 3);
	assert(cluster3->boundingBoxes[0] == Rectangle(-5.0, 4.0, 1.0, 1.0));
	assert(cluster3->boundingBoxes[1] == Rectangle(-2.0, 4.0, 1.0, 1.0));
	assert(cluster3->boundingBoxes[2] == Rectangle(-6.0, 2.0, 1.0, 1.0));

	assert(cluster3p->children.size() == 4);
	assert(cluster3p->children[0] == (Node *)0x2);
	assert(cluster3p->children[1] == (Node *)0x3);
	assert(cluster3p->children[2] == (Node *)0x4);
	assert(cluster3p->children[3] == cluster3extra);

	assert(cluster3p->boundingBoxes.size() == 4);
	assert(cluster3p->boundingBoxes[0] == Rectangle(-1.0, 3.0, 1.0, 1.0));
	assert(cluster3p->boundingBoxes[1] == Rectangle(-1.0, 1.0, 1.0, 1.0));
	assert(cluster3p->boundingBoxes[2] == Rectangle(-3.0, 0.0, 1.0, 1.0));
	assert(cluster3p->boundingBoxes[3] == Rectangle(1.5, 1.5, 0.5, 0.5));

	// Cleanup
	delete cluster2;
	// delete cluster3; fake pointers will cause the destructor to seg fault
	delete cluster3extra;
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
	std::vector<Point> v1;
	Rectangle sr1 = Rectangle(-7.0, 11.0, 2.0, 1.5);
	root->search(sr1, v1);
	assert(v1.size() == 3);
	assert(v1[0] == Point(-5.0, 12.0));
	assert(v1[1] == Point(-8.0, 10.0));
	assert(v1[2] == Point(-9.0, 10.0));

	// Test set two
	std::vector<Point> v2;
	Rectangle sr2 = Rectangle(-6.5, 6.0, 1.5, 2.0);
	root->search(sr2, v2);
	assert(v2.size() == 2);
	assert(v2[0] == Point(-8.0, 8.0));
	assert(v2[1] == Point(-5.0, 4.0));

	// Test set three
	std::vector<Point> v3;
	Rectangle sr3 = Rectangle(-6.0, 8.0, 2.0, 8.0);
	root->search(sr3, v3);
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
	std::vector<Point> v4;
	Rectangle sr4 = Rectangle(3.0, -3.0, 1.0, 1.0);
	root->search(sr4, v4);
	assert(v4.size() == 0);

	// Test set five
	std::vector<Point> v5;
	Rectangle sr5 = Rectangle(-2.5, 2.0, 1.0, 1.0);
	root->search(sr5, v5);
	assert(v5.size() == 0);

	// Cleanup
	root->deleteSubtrees();
	delete root;
}

void testInsert()
{
	// Setup the tree
	Node *root = new Node();

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
	assert(root->boundingBoxes.size() == 2);
	assert(root->children.size() == 2);
	assert(root->children[0]->boundingBoxes.size() == 0);
	assert(root->children[0]->children.size() == 0);
	assert(root->children[0]->data.size() == 3);
	assert(root->children[1]->boundingBoxes.size() == 0);
	assert(root->children[1]->children.size() == 0);
	assert(root->children[1]->data.size() == 5);

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
	assert(root->boundingBoxes.size() == 2);
	assert(root->children.size() == 2);
	assert(root->children[0]->boundingBoxes.size() == 3);
	assert(root->children[0]->children.size() == 3);
	assert(root->children[0]->data.size() == 0);
	assert(root->children[1]->boundingBoxes.size() == 3);
	assert(root->children[1]->children.size() == 3);
	assert(root->children[1]->data.size() == 0);

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
	assert(root->boundingBoxes.size() == 2);
	assert(root->children.size() == 2);
	assert(root->children[0]->boundingBoxes.size() == 5);
	assert(root->children[0]->children.size() == 5);
	assert(root->children[0]->data.size() == 0);
	assert(root->children[1]->boundingBoxes.size() == 3);
	assert(root->children[1]->children.size() == 3);
	assert(root->children[1]->data.size() == 0);

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
	assert(root->boundingBoxes.size() == 3);
	assert(root->children.size() == 3);
	assert(root->children[0]->boundingBoxes.size() == 5);
	assert(root->children[0]->children.size() == 5);
	assert(root->children[0]->data.size() == 0);
	assert(root->children[1]->boundingBoxes.size() == 3);
	assert(root->children[1]->children.size() == 3);
	assert(root->children[1]->data.size() == 0);
	assert(root->children[2]->boundingBoxes.size() == 3);
	assert(root->children[2]->children.size() == 3);
	assert(root->children[2]->data.size() == 0);


	// Cleanup
	delete root;
}

void testRemove()
{
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

	// Cluster 3, n = 9
	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	Node *cluster3a = new Node();
	cluster3a->data.push_back(Point(-5.0, 4.0));
	cluster3a->data.push_back(Point(-3.0, 4.0));
	cluster3a->data.push_back(Point(-2.0, 4.0));

	Node *cluster3b = new Node();
	cluster3b->data.push_back(Point(-4.0, 1.0));
	cluster3b->data.push_back(Point(-3.0, 0.0));
	cluster3b->data.push_back(Point(-1.0, 1.0));

	Node *cluster3c = new Node();
	cluster3c->data.push_back(Point(-4.0, 3.0));
	cluster3c->data.push_back(Point(-1.0, 3.0));
	cluster3c->data.push_back(Point(-6.0, 2.0));

	Node *cluster3 = new Node();
	cluster3->boundingBoxes.push_back(cluster3a->boundingBox());
	cluster3->children.push_back(cluster3a);
	cluster3->boundingBoxes.push_back(cluster3b->boundingBox());
	cluster3->children.push_back(cluster3b);
	cluster3->boundingBoxes.push_back(cluster3c->boundingBox());
	cluster3->children.push_back(cluster3c);

	// Cluster 1, n = 7
	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12), + (-4.5, 15.5), (-2.0, 13.5)
	Node *cluster1a = new Node();
	cluster1a->data.push_back(Point(-3.0, 16.0));
	cluster1a->data.push_back(Point(-3.0, 15.0));
	cluster1a->data.push_back(Point(-4.0, 13.0));

	Node *cluster1b = new Node();
	cluster1b->data.push_back(Point(-5.0, 12.0));
	cluster1b->data.push_back(Point(-5.0, 15.0));
	cluster1b->data.push_back(Point(-6.0, 14.0));

	Node *cluster1c = new Node();
	cluster1c->data.push_back(Point(-8.0, 16.0));
	cluster1c->data.push_back(Point(-4.5, 15.5));
	cluster1c->data.push_back(Point(-2.0, 13.5));

	Node *cluster1 = new Node();
	cluster1->boundingBoxes.push_back(cluster1a->boundingBox());
	cluster1->children.push_back(cluster1a);
	cluster1->boundingBoxes.push_back(cluster1b->boundingBox());
	cluster1->children.push_back(cluster1b);
	cluster1->boundingBoxes.push_back(cluster1c->boundingBox());
	cluster1->children.push_back(cluster1c);

	// Root
	Node *root = new Node();
	root->boundingBoxes.push_back(cluster3->boundingBox());
	root->children.push_back(cluster3);
	root->boundingBoxes.push_back(cluster5->boundingBox());
	root->children.push_back(cluster5);
	root->boundingBoxes.push_back(cluster1->boundingBox());
	root->children.push_back(cluster1);
	assert(root->boundingBoxes.size() == 3);

	// Setup parentage
	cluster5a->parent = cluster5;
	cluster5b->parent = cluster5;
	cluster5c->parent = cluster5;
	cluster5d->parent = cluster5;

	cluster3a->parent = cluster3;
	cluster3b->parent = cluster3;
	cluster3c->parent = cluster3;

	cluster1a->parent = cluster1;
	cluster1b->parent = cluster1;
	cluster1c->parent = cluster1;

	cluster5->parent = root;
	cluster3->parent = root;
	cluster1->parent = root;

	// Remove an element, no other changes in tree
	root->remove(Point(-12.0, -15.0));

	// Test the removal
	assert(cluster5d->data.size() == 3);
	assert(cluster5d->data[0] == Point(-15.0, -15.0));
	assert(cluster5d->data[1] == Point(-14.0, -15.0));
	assert(cluster5d->data[2] == Point(-13.0, -15.0));
	assert(cluster5->boundingBoxes.size() == 4);
	assert(cluster5->children.size() == 4);
	assert(root->boundingBoxes.size() == 3);
	assert(root->children.size() == 3);

	// Remove an element, parent is altered
	root->remove(Point(-6.0, 2.0));

	// Test the removal
	assert(root->boundingBoxes.size() == 2);
	assert(root->children.size() == 2);
	assert(root->children[0]->boundingBoxes.size() == 4);
	assert(root->children[0]->children.size() == 4);
	assert(root->children[1]->boundingBoxes.size() == 5);
	assert(root->children[1]->children.size() == 5);

	// Cleanup
	root->deleteSubtrees();
	delete root;

	// Remove an element, tree shrinks

	// Cluster 6, n = 7
	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	Node *cluster6a = new Node();
	cluster6a->data.push_back(Point(-2.0, -6.0));
	cluster6a->data.push_back(Point(2.0, -6.0));
	cluster6a->data.push_back(Point(-1.0, -7.0));

	Node *cluster6b = new Node();
	cluster6b->data.push_back(Point(1.0, -7.0));
	cluster6b->data.push_back(Point(3.0, -8.0));
	cluster6b->data.push_back(Point(-2.0, -9.0));
	// cluster6->data.push_back(Point(-3.0, -11.0)); held out so we get a shrinking root

	// Root
	root = new Node();
	root->boundingBoxes.push_back(cluster6a->boundingBox());
	root->children.push_back(cluster6a);
	root->boundingBoxes.push_back(cluster6b->boundingBox());
	root->children.push_back(cluster6b);

	// Setup parentage
	cluster6a->parent = root;
	cluster6b->parent = root;

	// Remove an element, the tree should shrink and cluster6a should be the new root
	root = root->remove(Point(3.0, -8.0));

	// Test the removal
	assert(root == cluster6a);
	assert(root->boundingBoxes.size() == 0);
	assert(root->children.size() == 0);
	assert(root->data.size() == 5);

	// Cleanup
	root->deleteSubtrees();
	delete root;
}
