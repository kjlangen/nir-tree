#include <rtree/node.h>

Node::Node()
{
}

Node::Node(unsigned id, Node *p)
{
	this->id = id;
	this->parent = p;
}

Rectangle Node::boundingBox()
{
	assert(boundingBoxes.size() > 0);

	double maxX = 0;
	double minX = 0;
	double maxY = 0;
	double minY = 0;

	for (int i = 0; i < boundingBoxes.size(); ++i)
	{
		maxX = boundingBoxes[maxX].centre.x < boundingBoxes[i].centre.x ? i : maxX;
		minX = boundingBoxes[minX].centre.x > boundingBoxes[i].centre.x ? i : minX;

		maxY = boundingBoxes[maxY].centre.y < boundingBoxes[i].centre.y ? i : maxY;
		minY = boundingBoxes[minY].centre.y > boundingBoxes[i].centre.y ? i : minY;
	}

	double radiusX = boundingBoxes[maxX].radiusX > boundingBoxes[minX].radiusX ? boundingBoxes[maxX].radiusX : boundingBoxes[minX].radiusX;
	radiusX += boundingBoxes[maxX].centre.x - boundingBoxes[minX].centre.x;
	double radiusY = boundingBoxes[maxY].radiusY > boundingBoxes[minY].radiusY ? boundingBoxes[maxY].radiusY : boundingBoxes[minY].radiusY;
	radiusY += boundingBoxes[maxY].centre.y - boundingBoxes[minY].centre.y;

	return Rectangle((boundingBoxes[maxX].centre.x + boundingBoxes[minX].centre.x) / 2, (boundingBoxes[maxY].centre.y + boundingBoxes[minY].centre.y) / 2, radiusX, radiusY);
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

// 	return matchingPoints;
// }

// Node *Node::insert(Point givenPoint)
// {
// 	// Leaf case and general case
// 	if (children.size() == 0)
// 	{
// 		// Check if we have room for this new data
// 		if ((data.size() + 1) * sizeof(Point) < PAGESIZE)
// 		{
// 			// We do have room, just insert
// 			data.push_back(givenPoint);

// 			// Nothing else to do
// 			return nullptr;
// 		}
// 		else
// 		{
// 			// We don't have room, so build a new node
// 			Node *newLeaf = new Node();

// 			// Split the data in half between the new and current node
// 			// TODO: Splitting the data is non-trivial b/c it has spatial implications
// 			for (int i = 0; i < data.size() / 2; ++i)
// 			{
// 				Point p = data[data.size() - i - 1];
// 				newLeaf->data.push_back(p);
// 				data.pop_back();
// 			}

// 			// Hand back the node that needs to be inserted in our parent
// 			return newLeaf;
// 		}
// 	}
// 	else
// 	{
// 		// Find the bounding box with least required expansion? Find the bounding box with least overlap?
// 		unsigned leastOverlapRequriedIndex = 0;
// 		double leastOverlapArea = boundingBoxes[0].computeExpansionArea(givenPoint);
// 		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
// 		{
// 			double testOverlapArea = boundingBoxes[leastOverlapRequriedIndex].computeExpansionArea(givenPoint);
// 			if (leastOverlapArea < testOverlapArea)
// 			{
// 				leastOverlapRequriedIndex = i;
// 				leastOverlapArea = testOverlapArea;
// 			}
// 		}

// 		// Recurse
// 		Node *newChild = children[leastOverlapRequriedIndex]->insert(givenPoint);

// 		// Do we need to deal with a new child or not
// 		if (newChild != nullptr)
// 		{
// 			// Check if we have room for this new child
// 			if (children.size() + 1 <= MAXBRANCHFACTOR)
// 			{
// 				// We do have room, just insert
// 				children.push_back(newChild);

// 				// Update bounding box b/c which nodes have which points has shifted
// 				// TODO: HTF do we do this without accessing pointers???

// 				// Nothing else to do
// 				return nullptr;
// 			}
// 			else
// 			{
// 				// We don't have room, so build a new node
// 				Node *newSibling = new Node();

// 				// Split the children in half between the new and current node
// 				// TODO: Splitting the children is non-trivial b/c it has spatial implications
// 				for (int i = 0; i < children.size() / 2; ++i)
// 				{
// 					Node *child = children[children.size() - i - 1];
// 					newSibling->children.push_back(child);
// 					children.pop_back();
// 				}

// 				// Update bounding box b/c there are now two nodes
// 				// TODO: HTF do we do this without accessing pointers???

// 				// Hand back the node that needs to be inserted in our parent
// 				return newSibling;
// 			}
// 		}
// 		else
// 		{
// 			// Update boundingBox
// 			boundingBoxes[leastOverlapRequriedIndex].expand(givenPoint);

// 			return nullptr;
// 		}
// 	}
// }

// // Failsafe, if the givenPoint is not in the tree we do nothing
// Node *Node::remove(Point givenPoint, std::map<unsigned, std::list<std::pair<Rectangle, Node *>>> &q)
// {
// 	// Leaf case and general case
// 	if (children.size() == 0)
// 	{
// 		// Erase the data
// 		for (int i = 0; i < data.size(); ++i)
// 		{
// 			if (data[i] == givenPoint)
// 			{
// 				data.erase(data.begin() + i);
// 			}
// 		}

// 		// Check ourselves before we wreck ourselves
// 		if (data.size() > 0)
// 		{
// 			return nullptr;
// 		}
// 		else
// 		{
// 			return this;
// 		}
// 	}
// 	else
// 	{
// 		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
// 		{
// 			if (boundingBoxes[i].containsPoint(givenPoint))
// 			{
// 				// Recurse
// 				Node *emptyChild = children[i]->remove(givenPoint, q);

// 				// Do we need to delete this child or not?
// 				if (emptyChild != nullptr)
// 				{
// 					// Delete the object
// 					delete emptyChild;

// 					// Remove the child from our list
// 					children.erase(children.begin() + i);
// 				}
// 				else
// 				{
// 					// Update bounding box somehow
// 				}
// 			}
// 		}

// 		// Check ourselves before we wreck ourselves
// 		if (data.size() > 0)
// 		{
// 			return nullptr;
// 		}
// 		else
// 		{
// 			return this;
// 		}
// 	}
// }

// Always called on root, this = root
// TODO: Write the analogous chooseLeaf(Rectangle searchRectangle)
Node *Node::chooseLeaf(Point givenPoint)
{
	assert(parent == nullptr);

	Node *node = this;

	for (;;)
	{
		if (node->children.size() == 0)
		{
			return node;
		}
		else
		{
			// Find the bounding box with least required expansion? Find the bounding box with least overlap?
			// TODO: Break ties by using smallest area
			unsigned leastOverlapRequriedIndex = 0;
			double leastOverlapArea = boundingBoxes[0].computeExpansionArea(givenPoint);
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				double testOverlapArea = boundingBoxes[leastOverlapRequriedIndex].computeExpansionArea(givenPoint);
				if (leastOverlapArea < testOverlapArea)
				{
					leastOverlapRequriedIndex = i;
					leastOverlapArea = testOverlapArea;
				}
			}

			node = children[leastOverlapRequriedIndex];
		}
	}
}

// TODO: Write the analogous findLeaf(Rectangle searchRectangle)
Node *Node::findLeaf(Point givenPoint)
{
	Node *finalAnswer = nullptr;

	if (children.size() == 0)
	{
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
	// Splitting an interior node
	return nullptr;
}

Node *Node::splitNode(Point givenPoint)
{
	// Splitting a leaf
	return nullptr;
}

// To be called on the leaf l
Node *Node::adjustTree(Node *siblingLeaf)
{
	Node *node = this;
	Node *siblingNode = siblingLeaf;

	for (;;)
	{
		if (node->parent == nullptr)
		{
			break;
		}
		else
		{
			Node *parent = node->parent;
			parent->updateBoundingBox(node, node->boundingBox());
			if (siblingNode != nullptr)
			{
				if (parent->children.size() == MAXBRANCHFACTOR)
				{
					Node *siblingParent = parent->splitNode(siblingNode);
					node = parent;
					siblingNode = siblingParent;
				}
				else
				{
					// Create entry in p for nn
					parent->boundingBoxes.push_back(siblingNode->boundingBox());
					parent->children.push_back(siblingNode);
					siblingNode->parent = parent;
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

	// I1
	Node *leaf = chooseLeaf(givenPoint);
	Node *siblingLeaf = nullptr;

	// I2
	if (leaf->data.size() < MAXBRANCHFACTOR)
	{
		leaf->data.push_back(givenPoint);
	}
	else
	{
		siblingLeaf = leaf->splitNode(givenPoint);
	}

	// I3
	Node *siblingNode = leaf->adjustTree(siblingLeaf);

	// I4
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
// TODO: Make Q a thing
void Node::condenseTree()
{
	// CT1
	Node *node = this;
	std::vector<Point> Q;

	// CT2
	while (node->parent != nullptr)
	{
		Node *parent = node->parent;

		// CT3 & CT4
		if (node->boundingBoxes.size() < MINBRANCHFACTOR)
		{
			parent->removeChild(node);
			// Q.push_back(node);
		}
		else
		{
			parent->updateBoundingBox(node, node->boundingBox());
		}

		// CT5
		node = parent;
	}

	// CT6
	for (int i = 0; i < Q.size(); ++i)
	{
		node->insert(Q[i]);
	}
}

// Always called on root, this = root
Node *Node::remove(Point givenPoint)
{
	assert(this->parent == nullptr);

	// D1
	Node *leaf = findLeaf(givenPoint);

	if (leaf == nullptr)
	{
		return nullptr;
	}

	// D2
	leaf->removeData(givenPoint);

	// D3
	leaf->condenseTree();
	delete leaf;

	// D4
	if (children.size() == 1)
	{
		return children[0];
	}

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
