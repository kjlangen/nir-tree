#include <rtree/node.h>

namespace rtree
{
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
			}
		}

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
			}
		}

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

	// TODO: Optimize
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

	Node *Node::splitNode(Node *newChild)
	{
		unsigned boundingBoxesSize = boundingBoxes.size();

		// Setup the two groups which will be the entries in the two new nodes
		unsigned seedA = 0;
		std::vector<unsigned> groupA;

		unsigned seedB = boundingBoxesSize - 1;
		std::vector<unsigned> groupB;

		// Compute the first entry in each group based on PS1 & PS2
		double maxWasted = 0;
		Rectangle iBox, jBox;
		for (unsigned i = 0; i < boundingBoxesSize; ++i)
		{
			iBox = boundingBoxes[i];
			for (unsigned j = 0; j < boundingBoxesSize; ++j)
			{
				jBox = boundingBoxes[j];
				double dist, distPrime;
				double wasted = 0;

				for (unsigned d = 0; d < dimensions; ++d)
				{
					dist = iBox.lowerLeft[d] - jBox.lowerLeft[d];
					distPrime = iBox.upperRight[d] - jBox.upperRight[d];
					wasted += dist * dist + distPrime * distPrime;
				}

				wasted /= (double)dimensions;

				if (maxWasted < wasted)
				{
					maxWasted = wasted;

					seedA = i;
					seedB = j;
				}
			}
		}

		Rectangle boundingBoxA = boundingBoxes[seedA];
		Rectangle boundingBoxB = boundingBoxes[seedB];

		// Go through the remaining entries and add them to groupA or groupB
		for (unsigned i = 0; i < boundingBoxesSize; ++i)
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
			if (boundingBoxB.computeExpansionArea(boundingBoxes[i]) > boundingBoxA.computeExpansionArea(boundingBoxes[i]))
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
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		unsigned groupASize = groupA.size();
		unsigned groupALastIndex = groupASize - 1;
		unsigned iGroupB;
		for (unsigned i = 0; i < groupB.size(); ++i)
		{
			iGroupB = groupB[i];
			children[iGroupB]->parent = newSibling;
			newSibling->boundingBoxes.push_back(boundingBoxes[iGroupB]);
			newSibling->children.push_back(children[iGroupB]);

			boundingBoxes[iGroupB] = boundingBoxes[groupA[groupALastIndex]];
			children[iGroupB] = children[groupA[groupALastIndex]];

			groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
		}
		boundingBoxes.resize(groupASize);
		children.resize(groupASize);

		// Add newChild which caused this split in the first place
		Rectangle newBox = newChild->boundingBox();

		// Choose the group which will need to expand the least
		if (boundingBoxB.computeExpansionArea(newBox) > boundingBoxA.computeExpansionArea(newBox))
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

		// Return our newly minted sibling
		return newSibling;
	}

	// TODO: Because we're using vectors and didn't exactly implement the original R-Tree rewriting this
	// with sets is necessary and that will necessitate rewriting the entire R-Tree with sets.
	Node *Node::splitNode(Point newData)
	{
		double dataSize = data.size();

		// Setup the two groups which will be the entries in the two new nodes
		std::vector<unsigned> groupA;
		std::vector<unsigned> groupB;

		// Compute the first entry in each group based on PS1 & PS2
		unsigned seedA = 0;
		unsigned seedB = dataSize - 1;

		double dist;
		double maxWasted = 0;
		for (unsigned d = 0; d < dimensions; ++d)
		{
			dist = data[seedA][d] - data[seedB][d];
			maxWasted += dist * dist;
		}

		Point iData, jData;
		for (unsigned i = 0; i < dataSize; ++i)
		{
			iData = data[i];
			for (unsigned j = 0; j < dataSize; ++j)
			{
				jData = data[j];

				double wasted = 0;
				for (unsigned d = 0; d < dimensions; ++d)
				{
					dist = iData[d] - jData[d];
					wasted += dist * dist;
				}

				if (maxWasted < wasted)
				{
					maxWasted = wasted;

					seedA = i;
					seedB = j;
				}
			}
		}

		// Set the bounding rectangles
		Rectangle boundingBoxA = Rectangle(data[seedA], data[seedA]);
		Rectangle boundingBoxB = Rectangle(data[seedB], data[seedB]);

		// Go through the remaining entries and add them to groupA or groupB
		for (unsigned i = 0; i < dataSize; ++i)
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
			if (boundingBoxB.computeExpansionArea(data[i]) > boundingBoxA.computeExpansionArea(data[i]))
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
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		unsigned groupALastIndex = groupA.size() - 1;
		unsigned iGroupB;
		for (unsigned i = 0; i < groupB.size(); ++i)
		{
			iGroupB = groupB[i];
			newSibling->data.push_back(data[iGroupB]);
			data[iGroupB] = data[groupA[groupALastIndex]];
			groupALastIndex = groupALastIndex == 0 ? 0 : groupALastIndex - 1;
		}
		data.resize(groupA.size());

		// Add newData which caused this split in the first place
		// Choose the group which will need to expand the least
		if (boundingBoxB.computeExpansionArea(newData) > boundingBoxA.computeExpansionArea(newData))
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
				node->parent->updateBoundingBox(node, node->boundingBox());

				// If we have a split then deal with it otherwise move up the tree
				if (siblingNode != nullptr)
				{
					// AT4 [Propogate the node split upwards]
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
		unsigned long childrenSize;
		unsigned long dataSize;
		size_t memoryFootprint = 0;
		unsigned long totalNodes = 1;
		unsigned long singularBranches = 0;
		unsigned long totalLeaves = 0;

		std::vector<unsigned long> histogramFanout;
		histogramFanout.resize(maxBranchFactor + 10, 0);


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
			++histogramFanout[fanout];

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
		STATFANHIST();
		for (unsigned i = 0; i < histogramFanout.size(); ++i)
		{
			if (histogramFanout[i] > 0)
			{
				STATHIST(i, histogramFanout[i]);
			}
		}
	}
}
