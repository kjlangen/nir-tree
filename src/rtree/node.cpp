#include <rtree/node.h>
#include <rtree/rtree.h>

namespace rtree
{
	Node::Node(RTree &treeRef) :
		treeRef(treeRef)
	{
		minBranchFactor = 3;
		maxBranchFactor = 5;
		parent = nullptr;
		boundingBoxes.resize(0);
		children.resize(0);
		data.resize(0);
	}

	Node::Node(RTree &treeRef, unsigned minBranchFactor, unsigned maxBranchFactor, Node *p) :
		treeRef(treeRef)
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
			boundingBox = Rectangle(data[0], data[0]);
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

	void Node::exhaustiveSearch(Point &requestedPoint, std::vector<Point> &accumulator)
	{
		if (children.size() == 0)
		{
			// We are a leaf so add our data points when they are the search point
			for (unsigned i = 0; i < data.size(); ++i)
			{
				if (requestedPoint == data[i])
				{
					accumulator.push_back(data[i]);
					break;
				}
			}
		}
		else
		{
			// Determine which branches we need to follow
			assert(children.size() == boundingBoxes.size());
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				children[i]->exhaustiveSearch(requestedPoint, accumulator);
			}
		}
	}

	std::vector<Point> Node::search(Point &requestedPoint)
	{
		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
			{
				// We are a leaf so add our data points when they are the search point
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (requestedPoint == currentContext->data[i])
					{
						matchingPoints.push_back(currentContext->data[i]);
					}
				}
#ifdef STAT
				treeRef.stats.markLeafSearched();
#endif
			}
			else
			{
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->boundingBoxes[i].containsPoint(requestedPoint))
					{
						// Add to the nodes we will check
						context.push(currentContext->children[i]);
					}
				}
#ifdef STAT
				treeRef.stats.markNonLeafNodeSearched();
#endif
			}
		}

#ifdef STAT
		treeRef.stats.resetSearchTracker( false );
#endif

		return matchingPoints;
	}

	std::vector<Point> Node::search(Rectangle &requestedRectangle)
	{
		std::vector<Point> matchingPoints;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
			{
				// We are a leaf so add our data points when they are within the search rectangle
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (requestedRectangle.containsPoint(currentContext->data[i]))
					{
						matchingPoints.push_back(currentContext->data[i]);
					}
				}
#ifdef STAT
				treeRef.stats.markLeafSearched();
#endif
			}
			else
			{
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->boundingBoxes[i].intersectsRectangle(requestedRectangle))
					{
						// Add to the nodes we will check
						context.push(currentContext->children[i]);
					}
				}
#ifdef STAT
				treeRef.stats.markNonLeafNodeSearched();
#endif
			}
		}

#ifdef STAT
		treeRef.stats.resetSearchTracker( true );
#endif

		return matchingPoints;
	}

	// Always called on root, this = root
	// TODO: Write the analogous chooseLeaf(Rectangle searchRectangle)
	Node *Node::chooseLeaf(Point givenPoint)
	{
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

	// Always called on root, this = root
	Node *Node::chooseNode(ReinsertionEntry e)
	{
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

	Node *Node::findLeaf(Point givenPoint)
	{
		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			if (currentContext->children.size() == 0)
			{
				// FL2 [Search leaf node for record]
				// Check each entry to see if it matches E
				for (unsigned i = 0; i < currentContext->data.size(); ++i)
				{
					if (currentContext->data[i] == givenPoint)
					{
						return currentContext;
					}
				}
			}
			else
			{
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->boundingBoxes[i].containsPoint(givenPoint))
					{
						// Add the child to the nodes we will consider
						context.push(currentContext->children[i]);
					}
				}
			}
		}

		return nullptr;
	}

	void Node::moveData(unsigned fromIndex, std::vector<Point> &toData)
	{
		toData.push_back(data[fromIndex]);
		data[fromIndex] = data.back();
		data.pop_back();
	}

	void Node::moveChild(unsigned fromIndex, std::vector<Rectangle> &toRectangles, std::vector<Node *> &toChildren)
	{
		toRectangles.push_back(boundingBoxes[fromIndex]);
		toChildren.push_back(children[fromIndex]);
		boundingBoxes[fromIndex] = boundingBoxes.back();
		boundingBoxes.pop_back();
		children[fromIndex] = children.back();
		children.pop_back();
	}

	Node *Node::splitNode(Node *newChild)
	{
		// Consider newChild when splitting
		boundingBoxes.push_back(newChild->boundingBox());
		children.push_back(newChild);
		newChild->parent = this;
		unsigned boundingBoxesSize = boundingBoxes.size();

		// Setup the two groups which will be the entries in the two new nodes
		unsigned seedA = 0;
		unsigned seedB = boundingBoxesSize - 1;

		// Compute the first entry in each group based on PS1 & PS2
		double maxWasted = 0;
		Rectangle iBox, jBox;
		for (unsigned i = 0; i < boundingBoxesSize; ++i)
		{
			iBox = boundingBoxes[i];
			for (unsigned j = 0; j < boundingBoxesSize; ++j)
			{
				jBox = boundingBoxes[j];

				// Calculate the wasted space
				Rectangle temp = iBox;
				temp.expand(jBox);

				double wasted = temp.area() - iBox.area() - jBox.area() + iBox.computeIntersectionArea(jBox);

				if (maxWasted < wasted)
				{
					maxWasted = wasted;

					seedA = i;
					seedB = j;
				}
			}
		}

		// Setup the two groups which will be the entries in the two new nodes
		std::vector<Rectangle> groupABoundingBoxes;
		std::vector<Node *> groupAChildren;
		std::vector<Rectangle> groupBBoundingBoxes;
		std::vector<Node *> groupBChildren;

		// Set the bounding rectangles
		Rectangle boundingBoxA = boundingBoxes[seedA];
		Rectangle boundingBoxB = boundingBoxes[seedB];

		// seedA and seedB have both already been allocated so put them into the appropriate group
		// and remove them from our boundingBoxes being careful to delete the one which will not
		// affect the index of the other first
		groupABoundingBoxes.push_back(boundingBoxes[seedA]);
		groupAChildren.push_back(children[seedA]);
		groupBBoundingBoxes.push_back(boundingBoxes[seedB]);
		groupBChildren.push_back(children[seedB]);
		if (seedA > seedB)
		{
			boundingBoxes.erase(boundingBoxes.begin() + seedA);
			children.erase(children.begin() + seedA);
			boundingBoxes.erase(boundingBoxes.begin() + seedB);
			children.erase(children.begin() + seedB);
		}
		else
		{
			boundingBoxes.erase(boundingBoxes.begin() + seedB);
			children.erase(children.begin() + seedB);
			boundingBoxes.erase(boundingBoxes.begin() + seedA);
			children.erase(children.begin() + seedA);
		}

		// Go through the remaining entries and add them to groupA or groupB
		double groupAAffinity, groupBAffinity;
		// QS2 [Check if done]
		for (;!boundingBoxes.empty() && (groupABoundingBoxes.size() + boundingBoxes.size() > minBranchFactor) && (groupBBoundingBoxes.size() + boundingBoxes.size() > minBranchFactor);)
		{
			// PN1 [Determine the cost of putting each entry in each group]
			unsigned groupAIndex = 0;
			double groupAMin = std::numeric_limits<double>::infinity();
			unsigned groupBIndex = 0;
			double groupBMin = std::numeric_limits<double>::infinity();

			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				groupAAffinity = boundingBoxA.computeExpansionArea(boundingBoxes[i]);
				groupBAffinity = boundingBoxB.computeExpansionArea(boundingBoxes[i]);
				// PN2 [Find entry with greatest preference for one group]
				if (groupAAffinity < groupAMin)
				{
					groupAMin = groupAAffinity;
					groupAIndex = i;
				}

				if (groupBAffinity < groupBMin)
				{
					groupBMin = groupBAffinity;
					groupBIndex = i;
				}
			}

			// QS3 [Select where to assign entry]
			if (groupAMin == groupBMin)
			{
				// Tie so use smaller area
				if (boundingBoxA.area() < boundingBoxB.area())
				{
					boundingBoxA.expand(boundingBoxes[groupAIndex]);
					moveChild(groupAIndex, groupABoundingBoxes, groupAChildren);
				}
				else
				{
					// Better area or in the worst case an arbitrary choice
					boundingBoxB.expand(boundingBoxes[groupBIndex]);
					moveChild(groupBIndex, groupBBoundingBoxes, groupBChildren);
				}
			}
			else if (groupAMin < groupBMin)
			{
				// Higher affinity for groupA
				boundingBoxA.expand(boundingBoxes[groupAIndex]);
				moveChild(groupAIndex, groupABoundingBoxes, groupAChildren);
			}
			else
			{
				// Higher affinity for groupB
				boundingBoxB.expand(boundingBoxes[groupBIndex]);
				moveChild(groupBIndex, groupBBoundingBoxes, groupBChildren);
			}
		}

		// If we stopped because half the entries were assigned then great put the others in the
		// opposite group
		if (groupABoundingBoxes.size() + boundingBoxes.size() == minBranchFactor)
		{
			groupABoundingBoxes.insert(groupABoundingBoxes.end(), boundingBoxes.begin(), boundingBoxes.end());
			groupAChildren.insert(groupAChildren.end(), children.begin(), children.end());
		}
		else if (groupBBoundingBoxes.size() + boundingBoxes.size() == minBranchFactor)
		{
			groupBBoundingBoxes.insert(groupBBoundingBoxes.end(), boundingBoxes.begin(), boundingBoxes.end());
			groupBChildren.insert(groupBChildren.end(), children.begin(), children.end());
		}
		else
		{
			// We really shouldn't be here so panic!
			assert(false);
		}

		// Create the new node and fill it
		Node *newSibling = new Node(treeRef, minBranchFactor, maxBranchFactor, parent);

		// Fill us with groupA and the new node with groupB
		boundingBoxes = std::move(groupABoundingBoxes);
		children = std::move(groupAChildren);
#ifndef NDEBUG
		for (Node *child : children)
		{
			assert(child->parent == this);
		}
#endif

		newSibling->boundingBoxes = std::move(groupBBoundingBoxes);
		newSibling->children = std::move(groupBChildren);
		for (Node *child : newSibling->children)
		{
			child->parent = newSibling;
		}

		// Return our newly minted sibling
		return newSibling;
	}

	Node *Node::splitNode(Point newData)
	{
		// Include the new point in our split consideration
		data.push_back(newData);
		double dataSize = data.size();

		// Compute the first entry in each group based on PS1 & PS2
		unsigned seedA = 0;
		unsigned seedB = dataSize - 1;

		// This rectangle drank too much and represents how wasted iData and jData are
		double maxWasted = 0.0;

		// QS1 [Pick entry for each group]
		Point iData, jData;
		for (unsigned i = 0; i < dataSize; ++i)
		{
			iData = data[i];
			for (unsigned j = 0; j < dataSize; ++j)
			{
				jData = data[j];

				Rectangle temp = Rectangle(iData, iData);
				temp.expand(jData);

				double wasted = temp.area();

				if (maxWasted < wasted)
				{
					maxWasted = wasted;

					seedA = i;
					seedB = j;
				}
			}
		}

		// Setup the two groups which will be the entries in the two new nodes
		std::vector<Point> groupAData;
		std::vector<Point> groupBData;

		// Set the bounding rectangles
		Rectangle boundingBoxA = Rectangle(data[seedA], data[seedA]);
		Rectangle boundingBoxB = Rectangle(data[seedB], data[seedB]);

		// seedA and seedB have both already been allocated so put them into the appropriate group
		// and remove them from our data being careful to delete the one which will not affect the
		// index of the other first
		groupAData.push_back(data[seedA]);
		groupBData.push_back(data[seedB]);
		if (seedA > seedB)
		{
			data.erase(data.begin() + seedA);
			data.erase(data.begin() + seedB);
		}
		else
		{
			data.erase(data.begin() + seedB);
			data.erase(data.begin() + seedA);
		}

		// Go through the remaining entries and add them to groupA or groupB
		double groupAAffinity, groupBAffinity;
		// QS2 [Check if done]
		for (;!data.empty() && (groupAData.size() + data.size() > minBranchFactor) && (groupBData.size() + data.size() > minBranchFactor);)
		{
			// PN1 [Determine the cost of putting each entry in each group]
			unsigned groupAIndex = 0;
			double groupAMin = std::numeric_limits<double>::infinity();
			unsigned groupBIndex = 0;
			double groupBMin = std::numeric_limits<double>::infinity();

			for (unsigned i = 0; i < data.size(); ++i)
			{
				groupAAffinity = boundingBoxA.computeExpansionArea(data[i]);
				groupBAffinity = boundingBoxB.computeExpansionArea(data[i]);
				// PN2 [Find entry with greatest preference for one group]
				if (groupAAffinity < groupAMin)
				{
					groupAMin = groupAAffinity;
					groupAIndex = i;
				}

				if (groupBAffinity < groupBMin)
				{
					groupBMin = groupBAffinity;
					groupBIndex = i;
				}
			}

			// QS3 [Select where to assign entry]
			if (groupAMin == groupBMin)
			{
				// Tie so use smaller area
				if (boundingBoxA.area() < boundingBoxB.area())
				{
					boundingBoxA.expand(data[groupAIndex]);
					moveData(groupAIndex, groupAData);
				}
				else
				{
					// Better area or in the worst case an arbitrary choice
					boundingBoxB.expand(data[groupBIndex]);
					moveData(groupBIndex, groupBData);
				}
			}
			else if (groupAMin < groupBMin)
			{
				// Higher affinity for groupA
				boundingBoxA.expand(data[groupAIndex]);
				moveData(groupAIndex, groupAData);
			}
			else
			{
				// Higher affinity for groupB
				boundingBoxB.expand(data[groupBIndex]);
				moveData(groupBIndex, groupBData);
			}
		}

		// If we stopped because half the entries were assigned then great put the others in the
		// opposite group
		if (groupAData.size() + data.size() == minBranchFactor)
		{
			groupAData.insert(groupAData.end(), data.begin(), data.end());
		}
		else if (groupBData.size() + data.size() == minBranchFactor)
		{
			groupBData.insert(groupBData.end(), data.begin(), data.end());
		}
		else
		{
			// We really shouldn't be here so panic!
			assert(false);
		}

		// Create the new node and fill it
		Node *newSibling = new Node(treeRef, minBranchFactor, maxBranchFactor, parent);

		// Fill us with groupA and the new node with groupB
		data = std::move(groupAData);
		newSibling->data = std::move(groupBData);

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
				node->parent->updateBoundingBox(node, node->boundingBox());

				// If we have a split then deal with it otherwise move up the tree
				if (siblingNode != nullptr)
				{
					// AT4 [Propagate the node split upwards]
					if (node->parent->children.size() < node->parent->maxBranchFactor)
					{
						node->parent->boundingBoxes.push_back(siblingNode->boundingBox());
						node->parent->children.push_back(siblingNode);
						siblingNode->parent = node->parent;

						node = node->parent;
						siblingNode = nullptr;
					}
					else
					{
						Node *siblingParent = node->parent->splitNode(siblingNode);

						node = node->parent;
						siblingNode = siblingParent;
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
			siblingLeaf = leaf->splitNode(givenPoint);
		}

		// I3 [Propogate changes upward]
		Node *siblingNode = leaf->adjustTree(siblingLeaf);

		// I4 [Grow tree taller]
		if (siblingNode != nullptr)
		{
			Node *newRoot = new Node(treeRef, minBranchFactor, maxBranchFactor);

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
			e.child->parent = node;
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
			Node *newRoot = new Node(treeRef, minBranchFactor, maxBranchFactor);

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
		// CT1 [Initialize]
		Node *node = this;
		unsigned level = 0;

		std::vector<ReinsertionEntry> Q;

		// CT2 [Find parent entry]
		unsigned nodeBoundingBoxesSize, nodeDataSize;
		while (node->parent != nullptr)
		{
			nodeBoundingBoxesSize = node->boundingBoxes.size();
			nodeDataSize = node->data.size();
			// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
			if (nodeBoundingBoxesSize >= node->minBranchFactor || nodeDataSize >= node->minBranchFactor)
			{
				node->parent->updateBoundingBox(node, node->boundingBox());

				// CT5 [Move up one level in the tree]
				// Move up a level without deleting ourselves
				node = node->parent;
				level++;
			}
			else
			{
				// Remove ourselves from our parent
				node->parent->removeChild(node);

				// Add a reinsertion entry for each data point or branch of this node
				for (unsigned i = 0; i < nodeDataSize; ++i)
				{
					ReinsertionEntry e = {};
					e.child = nullptr;
					e.data = node->data[i];
					e.level = 0;
					Q.push_back(e);
				}
				for (unsigned i = 0; i < nodeBoundingBoxesSize; ++i)
				{
					ReinsertionEntry e = {};
					e.boundingBox = node->boundingBoxes[i];
					e.child = node->children[i];
					e.level = level;
					Q.push_back(e);
				}

				// Prepare for garbage collection
				Node *garbage = node;

				// CT5 [Move up one level in the tree]
				// Move up a level before deleting ourselves
				node = node->parent;
				level++;

				// Cleanup ourselves without deleting children b/c they will be reinserted
				delete garbage;
			}
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

	bool Node::validate(Node *expectedParent, unsigned index)
	{
		if (parent != expectedParent || boundingBoxes.size() > maxBranchFactor || data.size() > maxBranchFactor || boundingBoxes.size() != children.size())
		{
			std::cout << "node = " << (void *)this << std::endl;
			std::cout << "parent = " << (void *)parent << " expectedParent = " << (void *)expectedParent << std::endl;
			std::cout << "maxBranchFactor = " << maxBranchFactor << std::endl;
			std::cout << "boundingBoxes.size() = " << boundingBoxes.size() << std::endl;
			std::cout << "children.size() = " << children.size() << std::endl;
			std::cout << "data.size() = " << data.size() << std::endl;
			assert(parent == expectedParent);
			assert(boundingBoxes.size() == children.size());
		}

		if (expectedParent != nullptr)
		{
			for (Point &dataPoint : data)
			{
				if (!parent->boundingBoxes[index].containsPoint(dataPoint))
				{
					std::cout << parent->boundingBoxes[index] << " fails to contain " << dataPoint << std::endl;
					assert(parent->boundingBoxes[index].containsPoint(dataPoint));
				}
			}
		}

		bool valid = true;
		for (unsigned i = 0; i < children.size(); ++i)
		{
			valid = valid && children[i]->validate(this, i);
		}

		return valid;
	}

	void Node::print(unsigned n)
	{
		std::string indendtation(n * 4, ' ');
		std::cout << indendtation << "Node " << (void *)this << std::endl;
		std::cout << indendtation << "{" << std::endl;
		std::cout << indendtation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indendtation << "    Bounding Boxes: " << std::endl;
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			std::cout << indendtation << "		" << boundingBoxes[i] << std::endl;
		}
		std::cout << std::endl << indendtation << "    Children: ";
		for (unsigned i = 0; i < children.size(); ++i)
		{
			std::cout << (void *)children[i] << ' ';
		}
		std::cout << std::endl << indendtation << "    Data: ";
		for (unsigned i = 0; i < data.size(); ++i)
		{
			std::cout << data[i];
		}
		std::cout << std::endl << indendtation << "}" << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
		if (children.size() > 0)
		{
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				children[i]->printTree(n + 1);
			}
		}
	}

	unsigned Node::checksum()
	{
		unsigned sum = 0;

		if (children.size() == 0)
		{
			for (unsigned i = 0; i < data.size(); ++i)
			{
				for (unsigned d = 0; d < dimensions; ++d)
				{
					sum += (unsigned)data[i][d];
				}
			}
		}
		else
		{
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				sum += children[i]->checksum();
			}
		}

		return sum;
	}

	unsigned Node::height()
	{
		unsigned ret = 0;
		Node *node = this;

		for (;;)
		{
			ret++;
			if (node->children.size() == 0)
			{
				return ret;
			}
			else
			{
				node = node->children[0];
			}
		}
	}

	void Node::stat()
	{
#ifdef STAT
		unsigned long childrenSize;
		unsigned long dataSize;
		size_t memoryFootprint = 0;
		unsigned long totalNodes = 1;
		unsigned long singularBranches = 0;
		unsigned long totalLeaves = 0;

		std::vector<unsigned long> histogramFanout;
		histogramFanout.resize(maxBranchFactor + 10, 0);

		double coverage = 0.0;
		double overlap = 0.0;

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();

			childrenSize = currentContext->children.size();
			dataSize = currentContext->data.size();
			unsigned fanout = childrenSize == 0 ? dataSize : childrenSize;
			if (unlikely(fanout >= histogramFanout.size()))
			{
				histogramFanout.resize(2*fanout,0);
			}
			++histogramFanout[fanout];

			// Compute the overlap and coverage of our children
			for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
			{
				coverage += currentContext->boundingBoxes[i].area();

				for (unsigned j = 0; j < currentContext->boundingBoxes.size(); ++j)
				{
					if (i != j)
					{
						overlap += currentContext->boundingBoxes[i].computeIntersectionArea(currentContext->boundingBoxes[j]);
					}
				}
			}

			if (childrenSize == 0 && dataSize > 0)
			{
				++totalLeaves;
				memoryFootprint += sizeof(Node) + currentContext->data.size() * sizeof(Point);
			}
			else
			{
				totalNodes += childrenSize;
				memoryFootprint += sizeof(Node) + childrenSize * sizeof(Node *) + currentContext->boundingBoxes.size() * sizeof(Rectangle);
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					if (currentContext->children[i]->children.size() == 1 || currentContext->children[i]->data.size() == 1)
					{
						singularBranches++;
					}

					context.push(currentContext->children[i]);
				}
			}
		}

		// Print out statistics
		STATMEM(memoryFootprint);
		STATHEIGHT(height());
		STATSIZE(totalNodes);
		STATSINGULAR(singularBranches);
		STATLEAF(totalLeaves);
		STATBRANCH(totalNodes - 1);
		STATCOVER(coverage);
		STATOVERLAP(overlap);
		STATAVGCOVER(coverage / totalNodes);
		STATAVGOVERLAP(overlap /totalNodes);
		STATFANHIST();
		for (unsigned i = 0; i < histogramFanout.size(); ++i)
		{
			if (histogramFanout[i] > 0)
			{
				STATHIST(i, histogramFanout[i]);
			}
		}
		std::cout << treeRef.stats;

		STATEXEC(std::cout << "### ### ### ###" << std::endl);
#else
		(void) 0;
#endif

	}
}
