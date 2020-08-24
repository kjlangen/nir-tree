#include <nirtree/node.h>

namespace nirtree
{
	Node::Node()
	{
		minBranchFactor = 0;
		maxBranchFactor = 0;
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

	// TODO: Memory management here
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
			boundingBox = boundingBoxes[0].boundingBox();
			for (unsigned i = 1; i < boundingBoxes.size(); ++i)
			{
				boundingBox.expand(boundingBoxes[i].boundingBox());
			}
		}
		else
		{
			boundingBox = Rectangle(data[0], data[0]);
			for (unsigned i = 1; i < data.size(); ++i)
			{
				boundingBox.expand(data[i]);
			}
		}

		return boundingBox;
	}

	void Node::updateChild(Node *child, IsotheticPolygon &boundingBox)
	{
		// Locate the child
		unsigned childIndex;
		for (childIndex = 0; children[childIndex] != child && childIndex < children.size(); ++childIndex) {}

		// Update the child
		boundingBoxes[childIndex] = boundingBox;
		children[childIndex] = child;
	}

	void Node::removeChild(Node *child)
	{
		// Locate the child
		unsigned childIndex;
		for (childIndex = 0; children[childIndex] != child && childIndex < children.size(); ++childIndex) {}

		// Delete the child by overwriting it
		boundingBoxes[childIndex] = boundingBoxes.back();
		boundingBoxes.pop_back();
		children[childIndex] = children.back();
		children.pop_back();
	}

	void Node::removeData(Point givenPoint)
	{
		// Locate the point
		unsigned pointIndex;
		for (pointIndex = 0; data[pointIndex] != givenPoint && pointIndex < data.size(); ++pointIndex) {}

		// Delete the point by overwriting it
		data[pointIndex] = data.back();
		data.pop_back();
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
			DEXEC(validate(nullptr, 0));
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
			DEXEC(validate(nullptr, 0));
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
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and
	// choosing a particular leaf
	Node *Node::chooseLeaf(Point givenPoint)
	{
		// CL1 [Initialize]
		Node *node = this;
		unsigned prevSmallestExpansionIndex = 0;
		DEXEC(PencilPrinter p);

		for (;;)
		{
			// CL2 [Leaf check]
			if (node->children.size() == 0)
			{
				return node;
			}
			else
			{
				std::vector<unsigned> indexPriorityList;
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					indexPriorityList.push_back(i);
				}

				std::sort(indexPriorityList.begin(), indexPriorityList.end(), [node, givenPoint](unsigned a, unsigned b){return node->boundingBoxes[a].computeExpansionArea(givenPoint) < node->boundingBoxes[b].computeExpansionArea(givenPoint);});

				IsotheticPolygon backupPolygon;
				unsigned smallestExpansionIndex = 0;

				for (unsigned i = 0; i < indexPriorityList.size(); ++i)
				{
					DPRINT3("chose ", i, " for expansion");
					smallestExpansionIndex = indexPriorityList[i];
					backupPolygon = node->boundingBoxes[smallestExpansionIndex];

					// CL3.1 Expand the chosen bounding polygon to cover the new point and keep it
					// within the area defined by our bounding polygon. The root has no bounding polygon
					if (node->parent == nullptr)
					{
						node->boundingBoxes[smallestExpansionIndex].expand(givenPoint);
						DASSERT(node->boundingBoxes[smallestExpansionIndex].unique());
					}
					else
					{
						node->boundingBoxes[smallestExpansionIndex].expand(givenPoint, node->parent->boundingBoxes[prevSmallestExpansionIndex]);
						DASSERT(node->boundingBoxes[smallestExpansionIndex].unique());
					}

					DEXEC(p.printToPencil(node->boundingBoxes));
					DASSERT(node->boundingBoxes[smallestExpansionIndex].unique());
					DASSERT(node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint));
					DASSERT(node->boundingBoxes[smallestExpansionIndex].contiguous());
					DPRINT1("Expanded successfully");

					// CL3.2 Trim back the chosen bounding polygon so it plays nice with the other
					// children
					for (unsigned j = 0; j < node->boundingBoxes.size(); ++j)
					{
						if (j == smallestExpansionIndex)
						{
							continue;
						}

						DPRINT1("Pre chooseLeaf increaseRes");
						node->boundingBoxes[smallestExpansionIndex].increaseResolution(node->boundingBoxes[j]);
						DPRINT1("Post chooseLeaf increaseRes");
						DPRINT2("i = ", i);
						DPRINT1("node->boundingBoxes[i] = ");
						DEXEC(node->boundingBoxes[i].print());
						DPRINT1("node->boundingBoxes[smallestExpansionIndex] = ");
						DEXEC(node->boundingBoxes[smallestExpansionIndex].print());
						DASSERT(node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint));
					}

					// If our initial choice is contiguous then great we can move on, otherwise try
					// every other node in descending order of required expansion area
					if (node->boundingBoxes[smallestExpansionIndex].contiguous())
					{
						break;
					}
					else
					{
						node->boundingBoxes[smallestExpansionIndex] = backupPolygon;
					}
				}

				DPRINT1("Trimmed back to ");
				DPRINT4("C: node->boundingBoxes[", smallestExpansionIndex, "].size() = ", node->boundingBoxes[smallestExpansionIndex].basicRectangles.size());
				DEXEC(node->boundingBoxes[smallestExpansionIndex].print());
				DEXEC(p.printToPencil(node->boundingBoxes));
				DASSERT(node->boundingBoxes[smallestExpansionIndex].unique());
				DASSERT(node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint));
				DASSERT(node->boundingBoxes[smallestExpansionIndex].contiguous());
				DPRINT1("Trimmed successfully");

				// CL4 [Descend until a leaf is reached]
				DASSERT(node->boundingBoxes[smallestExpansionIndex].contiguous());
				node = node->children[smallestExpansionIndex];
				prevSmallestExpansionIndex = smallestExpansionIndex;
			}
		}
	}

	// Always called on root, this = root
	Node *Node::chooseNode(ReinsertionEntry e)
	{
		return this;
	}

	Node *Node::findLeaf(Point givenPoint)
	{
		DPRINT1("Given point = ");
		DEXEC(givenPoint.print());

		// Initialize our context stack
		std::stack<Node *> context;
		context.push(this);
		Node *currentContext;

		for (;!context.empty();)
		{
			currentContext = context.top();
			context.pop();
			DPRINT2("Current find leaf context = ", (void *)currentContext);

			if (currentContext->children.size() == 0)
			{
				DPRINT2("FL2 size = ", currentContext->boundingBoxes.size());
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
				DPRINT2("FL1 size = ", currentContext->boundingBoxes.size());
				// FL1 [Search subtrees]
				// Determine which branches we need to follow
				for (unsigned i = 0; i < currentContext->boundingBoxes.size(); ++i)
				{
					DPRINT2("Considering ", i);
					if (currentContext->boundingBoxes[i].containsPoint(givenPoint))
					{
						DPRINT2("Pushing ", (void *)currentContext->children[i]);
						// Add the child to the nodes we will consider
						context.push(currentContext->children[i]);
					}
				}
			}
		}

		return nullptr;
	}

	std::vector<Rectangle> Node::decomposeNode(IsotheticPolygon &boundingPolygon)
	{
		// Copy our bounding polygon to start
		IsotheticPolygon temp = boundingPolygon;

		// Decompose our bounding box using code for increasing a polygon's "resolution"
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			DPRINT1("Current polygon ");
			DEXEC(temp.print());
			DPRINT1("Cutting out ");
			DEXEC(boundingBoxes[i].print());
			DPRINT1("Pre decompose increaseRes");
			temp.increaseResolution(boundingBoxes[i]);
			DPRINT1("Post decompose increaseRes");
			DPRINT1("Result ");
			DEXEC(temp.print());
		}

		return temp.basicRectangles;
	}

	Node::SplitResult Node::splitNode(SplitResult &lowerSplit)
	{
		DPRINT1("Split starting");
		// Setup the split
		Node *left = new Node(minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(minBranchFactor, maxBranchFactor, parent);

		// Insert the lower split as if we had capacity
		DEXEC(lowerSplit.leftPolygon.print());
		DEXEC(lowerSplit.rightPolygon.print());
		boundingBoxes.push_back(lowerSplit.leftPolygon);
		children.push_back(lowerSplit.left);
		boundingBoxes.push_back(lowerSplit.rightPolygon);
		children.push_back(lowerSplit.right);

		const unsigned childrenSize = children.size();
		unsigned polygonIndex;
		IsotheticPolygon unsplitPolygon;

		DPRINT1("Checking edge case");
		if (parent == nullptr)
		{
			DPRINT1("CHKPT13");
			assert(boundingBoxes.size() != 0);
			Rectangle r = boundingBoxes[0].boundingBox();
			for (unsigned i = 0; i < children.size(); ++i)
			{
				r.expand(boundingBoxes[i].boundingBox());
			}
			unsplitPolygon = IsotheticPolygon(r);
			DPRINT1("unsplitPolygon: ");
			DEXEC(unsplitPolygon.print());
			DPRINT1("CHKPT15");
		}
		else
		{
			for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}

			unsplitPolygon = parent->boundingBoxes[polygonIndex];
		}

		// Decompose polygon
		DPRINT1("Decomposing node");
		std::vector<Rectangle> decomposed = decomposeNode(unsplitPolygon);

		const unsigned decomposedSize = decomposed.size();
		const unsigned totalSize = childrenSize + decomposedSize;
		const unsigned root = totalSize / 2;

		DEXEC(PencilPrinter p1);
		DEXEC(p1.printToPencil(decomposed));

		// Build graph
		DPRINT1("Building graph");
		DPRINT2("totalSize = ", totalSize);
		bool graph[totalSize][totalSize];
		std::memset(graph, false, totalSize * totalSize);

		for (unsigned i = 0; i < childrenSize; ++i)
		{
			// Children X Children
			DPRINT1("Children X Children");
			for (unsigned j = 0; j < i && j < childrenSize; ++j)
			{
				if (boundingBoxes[i].intersectsPolygon(boundingBoxes[j]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
			DPRINT1("Done");

			// Children X Decomposed Polygon
			DPRINT1("Children X Decomposed");
			for (unsigned j = childrenSize; j < totalSize; ++j)
			{
				if (boundingBoxes[i].intersectsRectangle(decomposed[j - childrenSize]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
			DPRINT1("Done");
		}
		DPRINT1("Decomposed polygon X Decomposed polygon");
		for (unsigned i = 0; i < decomposedSize; ++i)
		{
			// Decomposed Polygon X Decomposed Polygon
			for (unsigned j = 0; j < decomposedSize; ++j)
			{
				if (decomposed[i].intersectsRectangle(decomposed[j]))
				{
					graph[i + childrenSize][j + childrenSize] = true;
					graph[j + childrenSize][i + childrenSize] = true;
				}
			}
		}
		DPRINT1("Done");

		// Build tree
		DPRINT1("Building tree");
		bool tree[totalSize][totalSize];
		unsigned currentVertex;
		std::queue<unsigned> explorationQ;
		bool explored[totalSize]; // Exploration labels so we don't cycle during search
		bool connected[totalSize]; // Connection labels so we don't double connect leaves
		unsigned weights[totalSize];
		std::stack<unsigned> weightStack;
		std::stack<unsigned> parentStack;

		std::memset(tree, false, totalSize * totalSize);
		std::memset(explored, false, totalSize);
		std::memset(connected, false, totalSize);
		std::memset(weights, 0, totalSize * sizeof(unsigned));

		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();

			if (explored[currentVertex])
			{
				continue;
			}

			// Connect children of this node to the tree
			for (unsigned neighbouringVertex = 0; neighbouringVertex < totalSize; ++neighbouringVertex)
			{
				if (graph[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					if (!connected[neighbouringVertex])
					{
						tree[currentVertex][neighbouringVertex] = true;
						tree[neighbouringVertex][currentVertex] = true;
						connected[neighbouringVertex] = true;
						weightStack.push(neighbouringVertex);
						parentStack.push(currentVertex);
					}
					explorationQ.push(neighbouringVertex);
				}
			}

			// Weight the current node
			if (currentVertex < childrenSize)
			{
				// Give the node weight if and only if it is in this->children
				weights[currentVertex] = 1;
			}

			explored[currentVertex] = true;
		}

		// Push weights up the tree
		DPRINT1("Pushing weights up the tree");
		DPRINT2("parentStack.size(): ", parentStack.size());
		DPRINT2("WeightStack.size(): ", weightStack.size());
		DPRINT2("totalSize: ", totalSize);
		for (;parentStack.size() && weightStack.size();)
		{
			weights[parentStack.top()] += weights[weightStack.top()];
			weightStack.pop();
			parentStack.pop();
		}

		// Find a separator
		DPRINT1("Finding a separator");
		unsigned delta = std::numeric_limits<unsigned>::max();
		unsigned subtreeRoot = 0;
		unsigned subtreeParent = 0;

		std::memset(explored, false, totalSize);

		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();

			if (explored[currentVertex])
			{
				continue;
			}

			for (unsigned neighbouringVertex = 0; neighbouringVertex < totalSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					// |(weight of whole tree - weight of this subtree) - weight of this subtree| < delta?
					// If yes that means that the subtree rooted at neighbouring vertex is more
					// balanced than any of our previous splits.
					unsigned componentTwoWeight = weights[neighbouringVertex];
					unsigned componentOneWeight = weights[root] - componentTwoWeight;
					unsigned comparisonDelta = componentOneWeight > componentTwoWeight ? componentOneWeight - componentTwoWeight : componentTwoWeight - componentOneWeight;
					if (comparisonDelta < delta)
					{
						delta = comparisonDelta;
						subtreeRoot = neighbouringVertex;
						subtreeParent = currentVertex;
					}
					explorationQ.push(neighbouringVertex);
				}
			}
			explored[currentVertex] = true;
		}

		// Split along separator
		DPRINT1("Splitting along separator");
		bool switchboard[totalSize];
		tree[subtreeRoot][subtreeParent] = tree[subtreeParent][subtreeRoot] = false;
		DPRINT1("CHKPT15");

		std::memset(switchboard, false, totalSize);
		std::memset(explored, false, totalSize);
		DPRINT1("CHKPT16");

		switchboard[subtreeRoot] = true;
		DPRINT1("CHKPT17");

		explorationQ.push(subtreeRoot);
		DPRINT1("CHKPT18");
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();
			DPRINT1("CHKPT19");

			if (explored[currentVertex])
			{
				continue;
			}
			DPRINT1("CHKPT20");

			for (unsigned neighbouringVertex = 0; neighbouringVertex < totalSize; ++neighbouringVertex)
			{
				DPRINT1("CHKPT21");
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					DPRINT1("CHKPT22");
					switchboard[neighbouringVertex] = true;
					explorationQ.push(neighbouringVertex);
					DPRINT1("CHKPT23");
				}
				DPRINT1("CHKPT24");
			}
			explored[currentVertex] = true;
			DPRINT1("CHKPT25");
		}

		// Recompose into two polygons by building polygons representing what we have now and then
		// reducing in size their representation
		DPRINT1("Recomposing polygons");
		IsotheticPolygon leftPolygon;
		IsotheticPolygon rightPolygon;

		for (unsigned i = 0; i < totalSize; ++i)
		{
			if (switchboard[i])
			{
				if (i < childrenSize)
				{
					leftPolygon.basicRectangles.insert(leftPolygon.basicRectangles.end(), boundingBoxes[i].basicRectangles.begin(), boundingBoxes[i].basicRectangles.end());
				}
				else
				{
					leftPolygon.basicRectangles.push_back(decomposed[i - childrenSize]);
				}
			}
			else
			{
				if (i < childrenSize)
				{
					rightPolygon.basicRectangles.insert(rightPolygon.basicRectangles.end(), boundingBoxes[i].basicRectangles.begin(), boundingBoxes[i].basicRectangles.end());
				}
				else
				{
					rightPolygon.basicRectangles.push_back(decomposed[i - childrenSize]);
				}
			}
		}

		leftPolygon.refine();
		rightPolygon.refine();

		// Because we touch the polygon's rectangles directly we must leave them in a valid sorted state
		leftPolygon.sort(true);
		rightPolygon.sort(true);

		DPRINT1("L/R polygons post increase resolution");
		DEXEC(leftPolygon.print());
		DEXEC(rightPolygon.print());

		// Split children in two
		DPRINT1("Splitting children");
		left->boundingBoxes.clear();
		left->children.clear();
		right->boundingBoxes.clear();
		right->children.clear();

		for (unsigned i = 0; i < childrenSize; ++i)
		{
			if (switchboard[i])
			{
				left->boundingBoxes.push_back(boundingBoxes[i]);
				left->children.push_back(children[i]);
				left->children[left->children.size() - 1]->parent = left;
			}
			else
			{
				right->boundingBoxes.push_back(boundingBoxes[i]);
				right->children.push_back(children[i]);
				right->children[right->children.size() - 1]->parent = right;
			}
		}

		DPRINT1("Return");
		return {left, leftPolygon, right, rightPolygon};
	}

	Node::SplitResult Node::splitNodeSpecialCase(Point newData)
	{
		// Setup new nodes
		Node *left = new Node(minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(minBranchFactor, maxBranchFactor, parent);

		data.push_back(newData);

		const unsigned dataSize = data.size();
		const unsigned dataHalfSize = dataSize / 2;

		// Always split along the first dimension
		// TODO: To get really fancy we could do some variance analysis and split along the
		// dimension with highest variance
		std::sort(data.begin(), data.end(), [](Point a, Point b){return a.x < b.x;});

		// Fill left node and determine bounding box
		float leftMinY = data[0].y;
		float leftMaxY = data[0].y;
		for (unsigned i = 0; i < dataHalfSize; ++i)
		{
			left->data.push_back(data[i]);
			leftMinY = std::min(leftMinY, data[i].y);
			leftMaxY = std::max(leftMaxY, data[i].y);
		}

		// Fill right node and determine bounding box
		float rightMinY = data[dataHalfSize].y;
		float rightMaxY = data[dataHalfSize].y;
		for (unsigned i = dataHalfSize; i < dataSize; ++i)
		{
			right->data.push_back(data[i]);
			rightMinY = std::min(rightMinY, data[i].y);
			rightMaxY = std::max(rightMaxY, data[i].y);
		}

		return
		{
			left,
			IsotheticPolygon(Rectangle(left->data[0].x, leftMinY, left->data[left->data.size() - 1].x, leftMaxY)),
			right,
			IsotheticPolygon(Rectangle(right->data[0].x, rightMinY, right->data[right->data.size() - 1].x, rightMaxY))
		};
	}

	// TODO: START Make sure isothetic polygon constituent rectangles are always sorted forever and for always from here down
	Node::SplitResult Node::splitNode(Point newData)
	{
		// Special case where the leaf to be split is the root
		if (parent == nullptr)
		{
			DPRINT1("nullptr case");
			return splitNodeSpecialCase(newData);
		}

		unsigned polygonIndex;
		for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}
		DEXEC(parent->boundingBoxes[polygonIndex].print());
		std::vector<Rectangle> &basics = parent->boundingBoxes[polygonIndex].basicRectangles;
		const unsigned basicsSize = basics.size();

		// Special case where the leaf is bounded by a simple rectangle
		if (basicsSize == 1)
		{
			DPRINT1("basicsSize = 1 case");
			return splitNodeSpecialCase(newData);
		}

		Node *left = new Node(minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(minBranchFactor, maxBranchFactor, parent);
		data.push_back(newData);
		const unsigned dataSize = data.size();
		const unsigned root = basicsSize / 2;

		DEXEC(PencilPrinter p);
		DEXEC(p.printToPencil(basics));
		
		// Build graph
		DPRINT1("Building graph...");
		bool graph[basicsSize][basicsSize];
		std::memset(graph, false, basicsSize * basicsSize);
		for (unsigned i = 0; i < basicsSize; ++i)
		{
			for (unsigned j = 0; j < i; ++j)
			{
				if (basics[i].intersectsRectangle(basics[j]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
		}

		// Build tree
		DPRINT1("Building tree...");
		bool tree[basicsSize][basicsSize];
		unsigned currentVertex;
		std::queue<unsigned> explorationQ;
		bool explored[basicsSize]; // Exploration labels so we don't cycle during search
		bool connected[basicsSize]; // Connection labels so we don't double connect vertices
		unsigned weights[basicsSize];
		std::stack<unsigned> weightStack;
		std::stack<unsigned> parentStack;

		std::memset(tree, false, basicsSize * basicsSize);
		std::memset(explored, false, basicsSize);
		std::memset(connected, false, basicsSize);
		std::memset(weights, 0, basicsSize * sizeof(unsigned));

		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();

			if (explored[currentVertex])
			{
				continue;
			}

			// Connect children of this node to the tree
			for (unsigned neighbouringVertex = 0; neighbouringVertex < basicsSize; ++neighbouringVertex)
			{
				if (graph[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					if (!connected[neighbouringVertex])
					{
						tree[currentVertex][neighbouringVertex] = true;
						tree[neighbouringVertex][currentVertex] = true;
						connected[neighbouringVertex] = true;
						weightStack.push(neighbouringVertex);
						parentStack.push(currentVertex);
					}
					explorationQ.push(neighbouringVertex);
				}
			}

			// Weight this node
			for (unsigned j = 0; j < dataSize; ++j)
			{
				if (basics[currentVertex].containsPoint(data[j]))
				{
					++weights[currentVertex];
				}
			}

			// Done with this vertex
			explored[currentVertex] = true;
		}

		// Weight the tree so we can quickly find a separator
		DPRINT1("Combining weights...");
		DPRINT2("basicsSize = ", basicsSize);
		DPRINT2("parentStack.size() = ", parentStack.size());
		DASSERT(parentStack.size() == weightStack.size());
		DASSERT(basicsSize - 1 == parentStack.size());
		for (unsigned i = 0; i < basicsSize - 1; ++i)
		{
			weights[parentStack.top()] += weights[weightStack.top()];
			weightStack.pop();
			parentStack.pop();
		}

		// Find a separator
		DPRINT1("Finding a separator...");
		unsigned delta = std::numeric_limits<unsigned>::max();
		unsigned subtreeRoot = 0;
		unsigned subtreeParent = 0;

		std::memset(explored, false, basicsSize);

		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();

			if (explored[currentVertex])
			{
				continue;
			}

			for (unsigned neighbouringVertex = 0; neighbouringVertex < basicsSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					// |(weight of whole tree - weight of this subtree) - weight of this subtree| < delta?
					// If yes that means that the subtree rooted at neighbouring vertex is more
					// balanced than any of our previous splits.
					unsigned componentTwoWeight = weights[neighbouringVertex];
					unsigned componentOneWeight = weights[root] - componentTwoWeight;
					unsigned comparisonDelta = componentOneWeight > componentTwoWeight ? componentOneWeight - componentTwoWeight : componentTwoWeight - componentOneWeight;
					if (comparisonDelta < delta)
					{
						delta = comparisonDelta;
						subtreeRoot = neighbouringVertex;
						subtreeParent = currentVertex;
					}
					explorationQ.push(neighbouringVertex);
				}
			}
			explored[currentVertex] = true;
		}

		// Split along separator
		DPRINT1("Splitting along separator...");
		bool switchboard[basicsSize];
		tree[subtreeRoot][subtreeParent] = tree[subtreeParent][subtreeRoot] = false;

		std::memset(switchboard, false, basicsSize);
		std::memset(explored, false, basicsSize);

		switchboard[subtreeRoot] = true;

		explorationQ.push(subtreeRoot);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();

			if (explored[currentVertex])
			{
				continue;
			}

			for (unsigned neighbouringVertex = 0; neighbouringVertex < basicsSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					switchboard[neighbouringVertex] = true;
					explorationQ.push(neighbouringVertex);
				}
			}
			explored[currentVertex] = true;
		}

		// Break the bounding polygon in two
		IsotheticPolygon leftPolygon;
		IsotheticPolygon rightPolygon;

		for (unsigned i = 0; i < basicsSize; ++i)
		{
			if (switchboard[i])
			{
				leftPolygon.basicRectangles.push_back(basics[i]);
			}
			else
			{
				rightPolygon.basicRectangles.push_back(basics[i]);
			}
		}

		// Because we touch the polygon's rectangles directly we must leave them sorted as the polygon expects
		leftPolygon.sort(true);
		rightPolygon.sort(true);

		// Break the data in two
		left->data.clear();
		right->data.clear();

		for (unsigned i = 0; i < dataSize; ++i)
		{
			if (leftPolygon.containsPoint(data[i]))
			{
				left->data.push_back(data[i]);
			}
			else
			{
				right->data.push_back(data[i]);
			}
		}

		return {left, leftPolygon, right, rightPolygon};
	}

	// This bottom-to-top sweep is only for splitting bounding boxes as necessary
	Node::SplitResult Node::adjustTree(SplitResult &leafSplit)
	{
		// Initialize
		Node *node = this;
		SplitResult lowerSplit = leafSplit;

		for (;;)
		{
			DPRINT1("CHKPT5");
			if (node == nullptr)
			{
				DPRINT1("CHKPT6");
				// Stop if at the top of the tree
				return lowerSplit;
			}
			else if (node->children.size() + 2 <= node->maxBranchFactor)
			{
				DPRINT1("CHKPT7");
				// Add additional children gracefully
				node->boundingBoxes.push_back(lowerSplit.leftPolygon);
				node->children.push_back(lowerSplit.left);
				node->boundingBoxes.push_back(lowerSplit.rightPolygon);
				node->children.push_back(lowerSplit.right);
				DASSERT(node->boundingBoxes.size() <= node->maxBranchFactor);
				DASSERT(node->children.size() <= node->maxBranchFactor);
				DPRINT1("CHKPT8");

				// Stop there is no split on this level
				return {nullptr, IsotheticPolygon(), nullptr, IsotheticPolygon()};
			}
			else
			{
				DPRINT1("CHKPT9");
				// Split this node
				DPRINT1("Before routing split");
				DEXEC(node->printTree());
				lowerSplit = node->splitNode(lowerSplit);
				DPRINT1("After routing split");
				DASSERT(lowerSplit.left->boundingBoxes.size() <= node->maxBranchFactor);
				DASSERT(lowerSplit.left->children.size() <= node->maxBranchFactor);
				DASSERT(lowerSplit.right->boundingBoxes.size() <= node->maxBranchFactor);
				DASSERT(lowerSplit.right->children.size() <= node->maxBranchFactor);

				DPRINT1("CHKPT10");
				// Remove the single old over-capacity node from the tree
				if (node->parent != nullptr)
				{
					DPRINT1("node->parent->printTree");
					DEXEC(node->parent->printTree());
					DPRINT1("CHKPT11");
					node->parent->removeChild(node);
				}
				DPRINT1("Left ");
				DEXEC(lowerSplit.leftPolygon.print());
				DEXEC(lowerSplit.left->printTree());
				DPRINT1("Right ");
				DEXEC(lowerSplit.rightPolygon.print());
				DEXEC(lowerSplit.right->printTree());
				DPRINT1("CHKPT12");
				// Continue to move up the tree
				node = lowerSplit.left->parent;
			}
		}

		return lowerSplit;
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		DPRINT1("Inserting ");
		DEXEC(givenPoint.print());

		// I1 [Find position for new record]
		DPRINT1("Before the leaf choosing");
		Node *leaf = chooseLeaf(givenPoint);
		DPRINT1("After the leaf choosing");

		// I2 [Add record to leaf node]
		if (leaf->data.size() < leaf->maxBranchFactor)
		{
			DPRINT1("CHKPT0");
			leaf->data.push_back(givenPoint);
			DPRINT1("I2a complete");
		}
		else
		{
			DPRINT1("CHKPT1");
			SplitResult sr = leaf->splitNode(givenPoint);
			DPRINT1("I2b complete");

			if (sr.left->parent != nullptr)
			{
				DPRINT1("CHKPT2");
				sr.left->parent->removeChild(leaf);
				DPRINT1("CHKPT3");
				// I3 [Propogate changes upward]
				sr = sr.left->parent->adjustTree(sr);
				DPRINT1("I3 complete");
			}

			// I4 [Grow tree taller]
			if (sr.left != nullptr && sr.right != nullptr)
			{
				DPRINT1("CHKPT4");
				Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

				sr.left->parent = newRoot;
				newRoot->boundingBoxes.push_back(sr.leftPolygon);
				newRoot->children.push_back(sr.left);
				DPRINT1("CHKPT5");
				sr.right->parent = newRoot;
				newRoot->boundingBoxes.push_back(sr.rightPolygon);
				newRoot->children.push_back(sr.right);
				DPRINT1("I4a complete");

				return newRoot;
			}
		}

		DPRINT1("I4b complete");
		return this;
	}

	// Always called on root, this = root
	Node *Node::insert(ReinsertionEntry e)
	{
		return this;
	}

	bool Node::condenseLeaf(Point givenPoint)
	{
		// Find ourselves, much like a teenager :P
		unsigned polygonIndex;
		for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}
		DPRINT1("Found ourselves");

		auto &basics = parent->boundingBoxes[polygonIndex].basicRectangles;
		unsigned basicsSize = basics.size();

		std::vector<unsigned> S;

		for (unsigned i = 0; i < basicsSize; ++i)
		{
			if (basics[i].containsPoint(givenPoint))
			{
				S.push_back(i);
			}
		}

		// Build a graph
		Graph g(basics);

		// Decide how to shrink rectangles in S
		unsigned deletedCount = 0;
		for (unsigned i = 0; i < S.size(); ++i)
		{
			// Call the current rectangle R
			unsigned R = S[i];

			// Now record every point R covers
			std::vector<Point> pointSet;
			for (unsigned j = 0; j < data.size(); ++j)
			{
				if (basics[R].containsPoint(data[j]))
				{
					pointSet.push_back(data[j]);
				}
			}

			// Depending on whether or not R is a bridge we may be able to delete it
			if (pointSet.size() == 0 && g.contiguous(R))
			{
				// Delete R and then check if its neighbours can be deleted
				basics[R] = Rectangle();
				g.remove(R);
				++deletedCount;
				DPRINT2("Deleted R = ", R);
			}
			else
			{
				// Shrink R as much as possible under the restrictions of the pin set and the points
				// it still must cover
			}
		}

		// Cleanup
		parent->boundingBoxes[polygonIndex].sort(true);
		basics.resize(basics.size() - deletedCount);

		return deletedCount > 0;
	}

	bool Node::condenseNode(IsotheticPolygon &givenPolygon)
	{
		// Find ourselves, much like a teenager :P
		unsigned polygonIndex;
		for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}

		auto &basics = parent->boundingBoxes[polygonIndex].basicRectangles;
		unsigned basicsSize = basics.size();

		// Determine the set of rectangles which intersect the changed polygon
		std::vector<unsigned> S;

		for (unsigned i = 0; i < basicsSize; ++i)
		{
			if (givenPolygon.intersectsRectangle(basics[i]))
			{
				S.push_back(i);
			}
		}

		// Build a graph
		Graph g(basics);

		// Decide how to shrink rectangles in S
		unsigned deletedCount = 0;
		for (unsigned i = 0; i < S.size(); ++i)
		{
			// Call the current rectangle R
			unsigned R = S[i];

			// Now record every object R covers
			std::vector<Rectangle> coverSet;
			for (unsigned j = 0; j < children.size(); ++j)
			{
				if (boundingBoxes[j].intersectsRectangle(basics[R]))
				{
					coverSet.push_back(Rectangle());
				}
			}

			// Depending on whether or not R is a bridge we may be able to delete it
			if (coverSet.size() == 0 && g.contiguous(R))
			{
				// Delete R and then check if its neighbours can be deleted
				basics[R] = Rectangle();
				g.remove(R);
				++deletedCount;
				DPRINT2("Deleted R = ", R);
			}
			else
			{
				// Shrink R as much as possible under the restrictions of the pin set and the points
				// it still must cover
			}
		}

		// Cleanup
		parent->boundingBoxes[polygonIndex].sort(true);
		basics.resize(basics.size() - deletedCount);

		return deletedCount > 0;
	}

	// To be called on a leaf
	Node *Node::condenseTree(Point givenPoint)
	{
		DPRINT1("Condensing");
		// CT1 [Initialize]
		Node *node = this;
		IsotheticPolygon givenPolygon;
		bool continueCondensing = true;

		for (;node->parent != nullptr;)
		{
			if (data.size() > 0)
			{
				DPRINT1("Condensing leaf");
				continueCondensing = node->condenseLeaf(givenPoint);
			}
			else if (children.size() > 0)
			{
				DPRINT1("Condensing node");
				continueCondensing = node->condenseNode(givenPolygon);
			}

			// Finish early if no condensing happens at a level because this means no condensing is
			// needed at higher levels
			if (node->boundingBoxes.size() == 0 && node->data.size() == 0)
			{
				DPRINT1("Deleting empty and continuing");
				unsigned polygonIndex; for(polygonIndex = 0; node->parent->children[polygonIndex] != node; ++polygonIndex){}
				givenPolygon = node->parent->boundingBoxes[polygonIndex];

				// Remove ourselves from our parent
				node->parent->removeChild(node);

				// Prepare for garbage collection
				Node *garbage = node;

				// Move up a level before deleting ourselves
				node = node->parent;

				// Garbage collect current node
				delete garbage;
			}
			else if (continueCondensing)
			{
				DPRINT1("Continue condensing");
				unsigned polygonIndex; for(polygonIndex = 0; node->parent->children[polygonIndex] != node; ++polygonIndex){}
				givenPolygon = node->parent->boundingBoxes[polygonIndex];

				// Move up a level
				node = node->parent;
			}
			else
			{
				DPRINT1("Early exit");
				return node;
			}
		}

		DPRINT1("Regular exit");
		return node;
	}

	// Always called on root, this = root
	Node *Node::remove(Point givenPoint)
	{
		DEXEC(PencilPrinter pr);
		DEXEC(pr.printToPencil(this));
		DPRINT1("Removing ");
		DEXEC(givenPoint.print());
		DPRINT1("CHKPT0");
		// D1 [Find node containing record]
		Node *leaf = findLeaf(givenPoint);

		DPRINT1("CHKPT1");
		if (leaf == nullptr)
		{
			DEXEC(this->printTree());
			DPRINT1("Leaf was nullptr");
			return nullptr;
		}

		DPRINT1("CHKPT2");

		// D2 [Delete record]
		leaf->removeData(givenPoint);
		DPRINT1("CHKPT3");

		// D3 [Propagate changes]
		leaf->condenseTree(givenPoint);
		DPRINT1("CHKPT4");

		DEXEC(pr.printToPencil(this));

		// D4 [Shorten tree]
		Node *root = this;
		if (root->children.size() == 1)
		{
			DPRINT1("CHKPT5");
			Node *newRoot = root->children[0];
			delete root;
			newRoot->parent = nullptr;
			return newRoot;
		}
		else
		{
			DPRINT1("CHKPT6");
			return root;
		}
	}

	unsigned Node::checksum()
	{
		unsigned sum = 0;

		if (children.size() == 0)
		{
			for (unsigned i = 0; i < data.size(); ++i)
			{
				sum += (unsigned)data[i].x;
				sum += (unsigned)data[i].y;
			}
		}
		else
		{
			for (unsigned i = 0; i < children.size(); ++i)
			{
				// Recurse
				sum += children[i]->checksum();
			}
		}

		return sum;
	}

	bool Node::validate(Node *expectedParent, unsigned index)
	{
		if (parent != expectedParent || boundingBoxes.size() > maxBranchFactor || children.size() > maxBranchFactor)
		{
			std::cout << "parent = " << (void *)parent << " expectedParent = " << (void *)expectedParent << std::endl;
			std::cout << "maxBranchFactor = " << maxBranchFactor << std::endl;
			std::cout << "boundingBoxes.size() = " << boundingBoxes.size() << std::endl;
			std::cout << "children.size() = " << children.size() << std::endl;
			assert(parent == expectedParent);
			assert(boundingBoxes.size() <= maxBranchFactor);
			assert(children.size() <= maxBranchFactor);
		}

		if (expectedParent != nullptr)
		{
			for (unsigned i = 0; i < data.size(); ++i)
			{
				if (!parent->boundingBoxes[index].contiguous())
				{
					parent->boundingBoxes[index].print();
					std::cout << " not contiguous" << std::endl;
					assert(parent->boundingBoxes[index].contiguous());
				}
				if (!parent->boundingBoxes[index].containsPoint(data[i]))
				{
					parent->boundingBoxes[index].print();
					std::cout << " fails to contain ";
					data[i].print();
					std::cout << std::endl;
					assert(parent->boundingBoxes[index].containsPoint(data[i]));
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
		std::cout << indendtation << "(" << std::endl;
		std::cout << indendtation << "    Parent: " << (void *)parent << std::endl;
		std::cout << indendtation << "    Bounding Boxes: " << std::endl;
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			std::cout << indendtation << "		";
			boundingBoxes[i].print();
		}
		std::cout << std::endl << indendtation << "    Children: ";
		for (unsigned i = 0; i < children.size(); ++i)
		{
			std::cout << (void *)children[i] << ' ';
		}
		std::cout << std::endl << indendtation << "    Data: ";
		for (unsigned i = 0; i < data.size(); ++i)
		{
			data[i].print();
		}
		std::cout << std::endl << indendtation << ")" << std::endl;
	}

	void Node::printTree(unsigned n)
	{
		// Print this node first
		print(n);

		// Print any of our children with one more level of indentation
		std::string indendtation(n * 4, ' ');
		std::cout << std::endl << indendtation << "{" << std::endl;
		if (children.size() > 0)
		{
			for (unsigned i = 0; i < boundingBoxes.size(); ++i)
			{
				// Recurse
				children[i]->printTree(n + 1);
			}
		}
		std::cout << std::endl << indendtation << "}" << std::endl;
	}

	void testPlayground()
	{
	}

	void testRemoveData()
	{
		// Setup a node with some data
		Node n = Node();

		n.boundingBoxes.push_back(IsotheticPolygon(Rectangle(8.0, -6.0, 10.0, -4.0)));
		n.data.push_back(Point(9.0, -5.0));

		n.boundingBoxes.push_back(IsotheticPolygon(Rectangle(12.0, -4.0, 16.0, -2.0)));
		n.data.push_back(Point(14.0, -3.0));

		n.boundingBoxes.push_back(IsotheticPolygon(Rectangle(10.0, 12.0, 12.0, 14.0)));
		n.data.push_back(Point(11.0, 13.0));

		n.boundingBoxes.push_back(IsotheticPolygon(Rectangle(12.0, 12.0, 14.0, 14.0)));
		n.data.push_back(Point(13.0, 13.0));

		// Remove some of the data
		n.removeData(Point(13.0, 13.0));

		// Test the removal
		assert(n.data.size() == 3);
	}

	void testRemoveChild()
	{
		// Setup a node with some children
		Node parentNode = Node();

		Node *child0 = new Node();
		child0->parent = &parentNode;
		parentNode.boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));
		parentNode.children.push_back(child0);

		Node *child1 = new Node();
		child1->parent = &parentNode;
		parentNode.boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
		parentNode.children.push_back(child1);

		Node *child2 = new Node();
		child2->parent = &parentNode;
		parentNode.boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
		parentNode.children.push_back(child2);

		Node *child3 = new Node();
		child3->parent = &parentNode;
		parentNode.boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));
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
		assert(rightChild1 == root->chooseLeaf(Point(13.0, -3.0)));
		assert(leftChild0 == root->chooseLeaf(Point(8.5, 12.5)));
		assert(leftChild2 == root->chooseLeaf(Point(13.5, 13.5)));
		assert(rightChild0 == root->chooseLeaf(Point(7.0, 3.0)));
		assert(leftChild1 == root->chooseLeaf(Point(11.0, 15.0)));
		assert(leftChild0 == root->chooseLeaf(Point(4.0, 8.0)));

		// Cleanup
		root->deleteSubtrees();
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
		Node *root = new Node();
		cluster4->parent = root;
		root->boundingBoxes.push_back(cluster4->boundingBox());
		root->children.push_back(cluster4);
		cluster5->parent = root;
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
		root->deleteSubtrees();
		delete root;
	}

	void testSplitNodeLeaf()
	{
		// Test set one
		Node *root = new Node();
		Node *leaf = new Node(3, 3, root);

		// Organize into a tree
		root->children.push_back(leaf);
		IsotheticPolygon ip;
		ip.basicRectangles.push_back(Rectangle(0.0, 400.0, 200.0, 700.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 300.0, 700.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(400.0, 0.0, 600.0, 300.0));
		ip.basicRectangles.push_back(Rectangle(700.0, 400.0, 900.0, 700.0));
		root->boundingBoxes.push_back(ip);

		leaf->data.push_back(Point(100.0, 550.0));
		leaf->data.push_back(Point(500.0, 125.0));

		// Split the node in two
		Node::SplitResult sr = leaf->splitNode(Point(800.0, 550.0));

		// TODO: Test the split

		PencilPrinter pr;
		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;


		// Test set two
		root = new Node();
		leaf = new Node(3, 3, root);

		// Organize into a tree
		root->children.push_back(leaf);
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 400.0, 200.0, 700.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 300.0, 700.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(400.0, 0.0, 600.0, 300.0));
		ip.basicRectangles.push_back(Rectangle(700.0, 400.0, 900.0, 700.0));
		root->boundingBoxes.push_back(ip);

		leaf->data.push_back(Point(100.0, 550.0));
		leaf->data.push_back(Point(800.0, 550.0));
		leaf->data.push_back(Point(300.0, 350.0));
		leaf->data.push_back(Point(350.0, 450.0));
		leaf->data.push_back(Point(450.0, 200.0));
		leaf->data.push_back(Point(550.0, 200.0));
		leaf->data.push_back(Point(500.0, 550.0));
		leaf->data.push_back(Point(600.0, 400.0));
		leaf->data.push_back(Point(650.0, 450.0));

		// Split the node in two
		sr = leaf->splitNode(Point(450.0, 350.0));

		// TODO: Test the split

		pr.printToPencil(root);
		
		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;


		// Test set three
		root = new Node();
		leaf = new Node(3, 3, root);

		// Organize into a tree
		root->children.push_back(leaf);
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 300.0, 100.0, 600.0));
		ip.basicRectangles.push_back(Rectangle(0.0, 200.0, 300.0, 300.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 300.0, 300.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(300.0, 400.0, 400.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(400.0, 200.0, 500.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(500.0, 0.0, 600.0, 300.0));
		ip.basicRectangles.push_back(Rectangle(600.0, 100.0, 700.0, 200.0));
		ip.basicRectangles.push_back(Rectangle(600.0, 0.0, 900.0, 100.0));
		ip.basicRectangles.push_back(Rectangle(800.0, 100.0, 900.0, 200.0));
		root->boundingBoxes.push_back(ip);

		leaf->data.push_back(Point(425.0, 250.0));
		leaf->data.push_back(Point(425.0, 450.0));
		leaf->data.push_back(Point(475.0, 250.0));
		leaf->data.push_back(Point(475.0, 450.0));
		leaf->data.push_back(Point(310.0, 490.0));
		leaf->data.push_back(Point(390.0, 410.0));
		leaf->data.push_back(Point(280.0, 250.0));
		leaf->data.push_back(Point(100.0, 250.0));
		leaf->data.push_back(Point(50.0, 250.0));
		leaf->data.push_back(Point(250.0, 350.0));
		leaf->data.push_back(Point(250.0, 450.0));
		leaf->data.push_back(Point(20.0, 400.0));
		leaf->data.push_back(Point(50.0, 500.0));
		leaf->data.push_back(Point(80.0, 580.0));
		leaf->data.push_back(Point(880.0, 190.0));
		leaf->data.push_back(Point(650.0, 150.0));
		leaf->data.push_back(Point(550.0, 50.0));
		leaf->data.push_back(Point(550.0, 299.0));
		leaf->data.push_back(Point(625.0, 25.0));
		leaf->data.push_back(Point(675.0, 75.0));
		leaf->data.push_back(Point(725.0, 25.0));
		leaf->data.push_back(Point(775.0, 75.0));
		leaf->data.push_back(Point(825.0, 25.0));
		leaf->data.push_back(Point(875.0, 75.0));

		// Split the node in two
		sr = leaf->splitNode(Point(550.0, 150.0));

		// TODO: Test the split

		pr.printToPencil(root);
		
		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;


		// Test set four
		root = new Node();
		leaf = new Node(3, 3, root);

		// Organize into a tree
		root->children.push_back(leaf);
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 0.0, 100.0, 200.0));
		ip.basicRectangles.push_back(Rectangle(0.0, 200.0, 300.0, 400.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 0.0, 300.0, 200.0));
		ip.basicRectangles.push_back(Rectangle(300.0, 300.0, 600.0, 400.0));
		ip.basicRectangles.push_back(Rectangle(300.0, 400.0, 400.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(500.0, 400.0, 600.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(300.0, 500.0, 600.0, 600.0));
		root->boundingBoxes.push_back(ip);

		leaf->data.clear();
		leaf->data.push_back(Point(450.0, 310.0));
		leaf->data.push_back(Point(450.0, 390.0));
		leaf->data.push_back(Point(590.0, 350.0));
		leaf->data.push_back(Point(380.0, 480.0));
		leaf->data.push_back(Point(580.0, 410.0));
		leaf->data.push_back(Point(350.0, 600.0));
		leaf->data.push_back(Point(450.0, 600.0));
		leaf->data.push_back(Point(580.0, 550.0));
		leaf->data.push_back(Point(250.0, 75.0));
		leaf->data.push_back(Point(250.0, 125.0));
		leaf->data.push_back(Point(250.0, 250.0));
		leaf->data.push_back(Point(50.0, 250.0));
		leaf->data.push_back(Point(150.0, 300.0));
		leaf->data.push_back(Point(80.0, 150.0));
		leaf->data.push_back(Point(80.0, 50.0));

		// Split the node in two
		sr = leaf->splitNode(Point(200.0, 350.0));

		// TODO: Test the split

		std::cout << "Test Set 4" << std::endl;
		pr.printToPencil(root);
		
		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;


		// Test set five
		root = new Node();
		leaf = new Node(3, 3, root);

		// Organize into a tree
		root->children.push_back(leaf);
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 500.0, 200.0, 700.0));
		ip.basicRectangles.push_back(Rectangle(100.0, 200.0, 600.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(500.0, 500.0, 700.0, 700.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 0.0, 500.0, 200.0));
		root->boundingBoxes.push_back(ip);

		leaf->data.push_back(Point(50.0, 550.0));
		leaf->data.push_back(Point(100.0, 600.0));
		leaf->data.push_back(Point(150.0, 650.0));
		leaf->data.push_back(Point(310.0, 210.0));
		leaf->data.push_back(Point(350.0, 450.0));
		leaf->data.push_back(Point(590.0, 210.0));
		leaf->data.push_back(Point(250.0, 50.0));
		leaf->data.push_back(Point(350.0, 100.0));
		leaf->data.push_back(Point(400.0, 100.0));
		leaf->data.push_back(Point(450.0, 150.0));
		leaf->data.push_back(Point(550.0, 550.0));
		leaf->data.push_back(Point(550.0, 650.0));
		leaf->data.push_back(Point(650.0, 550.0));

		// Split the node in two
		sr = leaf->splitNode(Point(250.0, 350.0));

		// TODO: Test the split

		pr.printToPencil(root);
		
		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;


		// Test set six
		root = new Node();
		leaf = new Node(3, 3, root);

		// Organize into a tree
		root->children.push_back(leaf);
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 0.0, 800.0, 700.0));
		root->boundingBoxes.push_back(ip);

		leaf->data.push_back(Point(50.0, 50.0));
		leaf->data.push_back(Point(750.0, 350.0));
		leaf->data.push_back(Point(400.0, 350.0));
		leaf->data.push_back(Point(500.0, 300.0));
		leaf->data.push_back(Point(350.0, 250.0));
		leaf->data.push_back(Point(550.0, 400.0));
		leaf->data.push_back(Point(350.0, 450.0));
		leaf->data.push_back(Point(100.0, 200.0));
		leaf->data.push_back(Point(100.0, 300.0));
		leaf->data.push_back(Point(150.0, 650.0));
		leaf->data.push_back(Point(100.0, 600.0));
		leaf->data.push_back(Point(700.0, 600.0));

		// Split the node in two
		sr = leaf->splitNode(Point(700.0, 100.0));

		// TODO: Test the split

		pr.printToPencil(root);
		
		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;


		// Test set seven
		root = new Node();

		// Organize into a tree
		root->data.push_back(Point(50.0, 50.0));
		root->data.push_back(Point(750.0, 350.0));
		root->data.push_back(Point(400.0, 350.0));
		root->data.push_back(Point(500.0, 300.0));
		root->data.push_back(Point(350.0, 250.0));
		root->data.push_back(Point(550.0, 400.0));
		root->data.push_back(Point(350.0, 450.0));
		root->data.push_back(Point(100.0, 200.0));
		root->data.push_back(Point(100.0, 300.0));
		root->data.push_back(Point(150.0, 650.0));
		root->data.push_back(Point(100.0, 600.0));
		root->data.push_back(Point(700.0, 600.0));

		// Split the node in two
		sr = root->splitNode(Point(700.0, 100.0));

		std::vector<IsotheticPolygon> bb;
		bb.push_back(sr.leftPolygon);
		bb.push_back(sr.rightPolygon);

		// TODO: Test the split

		pr.printToPencil(bb);
		
		// Cleanup
		root->deleteSubtrees();
		delete root;
		delete sr.left;
		delete sr.right;
	}

	void testSplitNodeRoutingSimple()
	{
		// Test set one
		Node *root = new Node();
		Node *routing = new Node(3, 3, root);
		Node *child0 = new Node(3, 3, routing);
		Node *child1 = new Node(3, 3, routing);
		// Node *child2 = new Node(3, 3, routing);

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 500.0, 500.0)));
		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 200.0, 100.0, 300.0)));
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 200.0, 500.0, 300.0)));

		// Split the node in two
		IsotheticPolygon ip;
		ip.basicRectangles.push_back(Rectangle(200.0, 400.0, 500.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 100.0, 300.0, 400.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 0.0, 500.0, 100.0));
		// Node::SplitResult sr = routing->splitNode(child2, ip);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 1);
		// assert(routing->children.size() == 1);
		// assert(sr.first->boundingBoxes.size() == 2);
		// assert(sr.first->children.size() == 2);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set two
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		// child2 = new Node(3, 3, routing);

		// Organize the tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 500.0, 500.0)));
		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 200.0, 100.0, 300.0)));
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 200.0, 500.0, 300.0)));

		// Split the node in two
		// sr = routing->splitNode(child2, IsotheticPolygon(Rectangle(200.0, 0.0, 300.0, 500.0)));

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set three
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		// child2 = new Node(3, 3, routing);

		// Organize the tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 500.0, 500.0)));
		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 200.0, 100.0, 300.0)));
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 200.0, 500.0, 300.0)));

		// Split the node in two
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 400.0, 300.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 100.0, 300.0, 400.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 0.0, 500.0, 100.0));
		// sr = routing->splitNode(child2, ip);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set four
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		// child2 = new Node(3, 3, routing);

		// Organize the tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 500.0, 500.0)));
		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 200.0, 100.0, 300.0)));
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 200.0, 500.0, 300.0)));

		// Split the node in two
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(0.0, 400.0, 300.0, 500.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 0.0, 300.0, 400.0));
		// sr = routing->splitNode(child2, ip);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set five
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);

		// Organize the tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 800.0, 700.0)));
		IsotheticPolygon ip0;
		ip0.basicRectangles.push_back(Rectangle(0.0, 500.0, 400.0, 600.0));
		ip0.basicRectangles.push_back(Rectangle(200.0, 400.0, 500.0, 500.0));
		ip0.basicRectangles.push_back(Rectangle(200.0, 300.0, 600.0, 400.0));
		ip0.basicRectangles.push_back(Rectangle(100.0, 200.0, 700.0, 300.0));
		routing->boundingBoxes.push_back(ip0);
		routing->children.push_back(child0);

		// Split the node in two
		IsotheticPolygon ip1;
		ip1.basicRectangles.push_back(Rectangle(400.0, 600.0, 800.0, 700.0));
		ip1.basicRectangles.push_back(Rectangle(500.0, 500.0, 800.0, 600.0));
		ip1.basicRectangles.push_back(Rectangle(600.0, 400.0, 800.0, 500.0));
		ip1.basicRectangles.push_back(Rectangle(700.0, 300.0, 800.0, 400.0));
		// sr = routing->splitNode(child1, ip1);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 1);
		// assert(routing->children.size() == 1);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set six
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		// child2 = new Node(3, 3, routing);

		// Organize the tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 400.0, 700.0)));
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 600.0, 400.0, 700.0)));
		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 400.0, 400.0, 500.0)));
		routing->children.push_back(child1);

		// Split the node in two
		ip.basicRectangles.clear();
		ip.basicRectangles.push_back(Rectangle(100.0, 200.0, 400.0, 300.0));
		ip.basicRectangles.push_back(Rectangle(200.0, 100.0, 400.0, 200.0));
		ip.basicRectangles.push_back(Rectangle(300.0, 0.0, 400.0, 100.0));
		// sr = routing->splitNode(child2, ip);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 1);
		// assert(routing->children.size() == 1);
		// assert(sr.first->boundingBoxes.size() == 2);
		// assert(sr.first->children.size() == 2);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;
	}

	void testSplitNodeRoutingComplex()
	{
		// Test set one
		Node *root = new Node();
		Node *routing = new Node(3, 3, root);
		Node *child0 = new Node(3, 3, routing);
		Node *child1 = new Node(3, 3, routing);
		Node *child2 = new Node(3, 3, routing);
		Node *child3 = new Node(3, 3, routing);
		Node *child4 = new Node(3, 3, routing);

		IsotheticPolygon routingPolygon;
		routingPolygon.basicRectangles.push_back(Rectangle(100.0, 500.0, 500.0, 1100.0));
		routingPolygon.basicRectangles.push_back(Rectangle(500.0, 0.0, 900.0, 600.0));

		IsotheticPolygon child0Polygon;
		child0Polygon.basicRectangles.push_back(Rectangle(100.0, 1000.0, 200.0, 1100.0));

		IsotheticPolygon child1Polygon;
		child1Polygon.basicRectangles.push_back(Rectangle(400.0, 700.0, 500.0, 800.0));

		IsotheticPolygon child2Polygon;
		child2Polygon.basicRectangles.push_back(Rectangle(200.0, 900.0, 500.0, 1000.0));
		child2Polygon.basicRectangles.push_back(Rectangle(200.0, 600.0, 300.0, 900.0));
		child2Polygon.basicRectangles.push_back(Rectangle(200.0, 500.0, 800.0, 600.0));
		child2Polygon.basicRectangles.push_back(Rectangle(700.0, 200.0, 800.0, 500.0));
		child2Polygon.basicRectangles.push_back(Rectangle(500.0, 100.0, 800.0, 200.0));

		IsotheticPolygon child3Polygon;
		child3Polygon.basicRectangles.push_back(Rectangle(500.0, 300.0, 600.0, 400.0));

		IsotheticPolygon child4Polygon;
		child4Polygon.basicRectangles.push_back(Rectangle(800.0, 0.0, 900.0, 100.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);
		routing->children.push_back(child4);
		routing->boundingBoxes.push_back(child4Polygon);

		// Split the node in two
		// Node::SplitResult sr = routing->splitNode(child3, child3Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		PencilPrinter pr;
		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set two
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);
		child3 = new Node(3, 3, routing);
		child4 = new Node(3, 3, routing);
		Node *child5 = new Node(3, 3, routing);
		Node *child6 = new Node(3, 3, routing);
		Node *child7 = new Node(3, 3, routing);

		routingPolygon.basicRectangles.clear();
		routingPolygon.basicRectangles.push_back(Rectangle(100.0, 700.0, 400.0, 1100.0));
		routingPolygon.basicRectangles.push_back(Rectangle(100.0, 400.0, 700.0, 700.0));
		routingPolygon.basicRectangles.push_back(Rectangle(700.0, 100.0, 1000.0, 700.0));

		child0Polygon.basicRectangles.clear();
		child0Polygon.basicRectangles.push_back(Rectangle(300.0, 500.0, 400.0, 900.0));
		child0Polygon.basicRectangles.push_back(Rectangle(300.0, 400.0, 800.0, 500.0));
		child0Polygon.basicRectangles.push_back(Rectangle(700.0, 300.0, 800.0, 400.0));

		child1Polygon.basicRectangles.clear();
		child1Polygon.basicRectangles.push_back(Rectangle(100.0, 400.0, 200.0, 500.0));

		child2Polygon.basicRectangles.clear();
		child2Polygon.basicRectangles.push_back(Rectangle(100.0, 1000.0, 200.0, 1100.0));

		child3Polygon.basicRectangles.clear();
		child3Polygon.basicRectangles.push_back(Rectangle(300.0, 1000.0, 400.0, 1100.0));

		child4Polygon.basicRectangles.clear();
		child4Polygon.basicRectangles.push_back(Rectangle(500.0, 600.0, 600.0, 700.0));

		IsotheticPolygon child5Polygon;
		child5Polygon.basicRectangles.push_back(Rectangle(900.0, 100.0, 1000.0, 200.0));

		IsotheticPolygon child6Polygon;
		child6Polygon.basicRectangles.push_back(Rectangle(800.0, 600.0, 900.0, 700.0));

		IsotheticPolygon child7Polygon;
		child7Polygon.basicRectangles.push_back(Rectangle(700.0, 100.0, 800.0, 200.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);
		routing->children.push_back(child3);
		routing->boundingBoxes.push_back(child3Polygon);
		routing->children.push_back(child5);
		routing->boundingBoxes.push_back(child5Polygon);
		routing->children.push_back(child6);
		routing->boundingBoxes.push_back(child6Polygon);
		routing->children.push_back(child7);
		routing->boundingBoxes.push_back(child7Polygon);

		// Split the node in two
		// sr = routing->splitNode(child4, child4Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set three
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);
		child3 = new Node(3, 3, routing);
		child4 = new Node(3, 3, routing);
		child5 = new Node(3, 3, routing);
		child6 = new Node(3, 3, routing);

		routingPolygon.basicRectangles.clear();
		routingPolygon.basicRectangles.push_back(Rectangle(800.0, 0.0, 900.0, 300.0));
		routingPolygon.basicRectangles.push_back(Rectangle(900.0, 0.0, 1300.0, 700.0));
		routingPolygon.basicRectangles.push_back(Rectangle(300.0, 700.0, 1300.0, 900.0));
		routingPolygon.basicRectangles.push_back(Rectangle(0.0, 0.0, 500.0, 700.0));

		child0Polygon.basicRectangles.clear();
		child0Polygon.basicRectangles.push_back(Rectangle(200.0, 600.0, 400.0, 700.0));
		child0Polygon.basicRectangles.push_back(Rectangle(200.0, 400.0, 300.0, 600.0));

		child1Polygon.basicRectangles.clear();
		child1Polygon.basicRectangles.push_back(Rectangle(800.0, 0.0, 1300.0, 100.0));
		child1Polygon.basicRectangles.push_back(Rectangle(1200.0, 100.0, 1300.0, 500.0));

		child2Polygon.basicRectangles.clear();
		child2Polygon.basicRectangles.push_back(Rectangle(900.0, 600.0, 1000.0, 700.0));
		child2Polygon.basicRectangles.push_back(Rectangle(700.0, 700.0, 1000.0, 800.0));

		child3Polygon.basicRectangles.clear();
		child3Polygon.basicRectangles.push_back(Rectangle(1100.0, 600.0, 1300.0, 700.0));

		child4Polygon.basicRectangles.clear();
		child4Polygon.basicRectangles.push_back(Rectangle(900.0, 300.0, 1000.0, 400.0));

		child5Polygon.basicRectangles.clear();
		child5Polygon.basicRectangles.push_back(Rectangle(0.0, 200.0, 400.0, 300.0));

		child6Polygon.basicRectangles.clear();
		child6Polygon.basicRectangles.push_back(Rectangle(100.0, 0.0, 200.0, 100.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);
		routing->children.push_back(child3);
		routing->boundingBoxes.push_back(child3Polygon);
		routing->children.push_back(child5);
		routing->boundingBoxes.push_back(child5Polygon);
		routing->children.push_back(child6);
		routing->boundingBoxes.push_back(child6Polygon);

		// Split the node in two
		// sr = routing->splitNode(child4, child4Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set four
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);
		child3 = new Node(3, 3, routing);

		routingPolygon.basicRectangles.clear();
		routingPolygon.basicRectangles.push_back(Rectangle(0.0, 400.0, 800.0, 900.0));
		routingPolygon.basicRectangles.push_back(Rectangle(200.0, 0.0, 700.0, 400.0));

		child0Polygon.basicRectangles.clear();
		child0Polygon.basicRectangles.push_back(Rectangle(0.0, 800.0, 300.0, 900.0));
		child0Polygon.basicRectangles.push_back(Rectangle(100.0, 700.0, 200.0, 800.0));

		child1Polygon.basicRectangles.clear();
		child1Polygon.basicRectangles.push_back(Rectangle(300.0, 200.0, 400.0, 500.0));
		child1Polygon.basicRectangles.push_back(Rectangle(300.0, 500.0, 600.0, 600.0));
		child1Polygon.basicRectangles.push_back(Rectangle(500.0, 200.0, 600.0, 500.0));
		child1Polygon.basicRectangles.push_back(Rectangle(600.0, 200.0, 700.0, 400.0));

		child2Polygon.basicRectangles.clear();
		child2Polygon.basicRectangles.push_back(Rectangle(500.0, 800.0, 800.0, 900.0));
		child2Polygon.basicRectangles.push_back(Rectangle(600.0, 700.0, 700.0, 800.0));

		child3Polygon.basicRectangles.clear();
		child3Polygon.basicRectangles.push_back(Rectangle(200.0, 0.0, 300.0, 100.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);

		// Split the node in two
		// sr = routing->splitNode(child3, child3Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set five
		std::cout << "Test Set 5 Begin" << std::endl;
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);
		child3 = new Node(3, 3, routing);

		routingPolygon.basicRectangles.clear();
		routingPolygon.basicRectangles.push_back(Rectangle(0.0, 0.0, 700.0, 1000.0));
		routingPolygon.basicRectangles.push_back(Rectangle(700.0, 0.0, 1100.0, 1100.0));

		child0Polygon.basicRectangles.clear();
		child0Polygon.basicRectangles.push_back(Rectangle(900.0, 0.0, 1100.0, 200.0));

		child1Polygon.basicRectangles.clear();
		child1Polygon.basicRectangles.push_back(Rectangle(0.0, 800.0, 900.0, 900.0));
		child1Polygon.basicRectangles.push_back(Rectangle(600.0, 900.0, 800.0, 1000.0));
		child1Polygon.basicRectangles.push_back(Rectangle(0.0, 400.0, 200.0, 800.0));
		child1Polygon.basicRectangles.push_back(Rectangle(0.0, 300.0, 100.0, 400.0));

		child2Polygon.basicRectangles.clear();
		child2Polygon.basicRectangles.push_back(Rectangle(500.0, 300.0, 600.0, 400.0));

		child3Polygon.basicRectangles.clear();
		child3Polygon.basicRectangles.push_back(Rectangle(300.0, 500.0, 1100.0, 600.0));
		child3Polygon.basicRectangles.push_back(Rectangle(300.0, 200.0, 400.0, 500.0));
		child3Polygon.basicRectangles.push_back(Rectangle(300.0, 100.0, 700.0, 200.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);

		// Split the node in two
		// sr = routing->splitNode(child3, child3Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;
		std::cout << "Test Set 5 End" << std::endl;


		// Test set six
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);
		child3 = new Node(3, 3, routing);
		child4 = new Node(3, 3, routing);
		child5 = new Node(3, 3, routing);

		routingPolygon.basicRectangles.clear();
		routingPolygon.basicRectangles.push_back(Rectangle(500.0, 0.0, 1000.0, 600.0));
		routingPolygon.basicRectangles.push_back(Rectangle(0.0, 300.0, 500.0, 700.0));
		routingPolygon.basicRectangles.push_back(Rectangle(200.0, 700.0, 300.0, 800.0));
		routingPolygon.basicRectangles.push_back(Rectangle(200.0, 800.0, 600.0, 1000.0));

		child0Polygon.basicRectangles.clear();
		child0Polygon.basicRectangles.push_back(Rectangle(500.0, 900.0, 600.0, 1000.0));

		child1Polygon.basicRectangles.clear();
		child1Polygon.basicRectangles.push_back(Rectangle(200.0, 800.0, 500.0, 900.0));
		child1Polygon.basicRectangles.push_back(Rectangle(200.0, 400.0, 300.0, 800.0));
		child1Polygon.basicRectangles.push_back(Rectangle(200.0, 300.0, 700.0, 400.0));

		child2Polygon.basicRectangles.clear();
		child2Polygon.basicRectangles.push_back(Rectangle(400.0, 600.0, 500.0, 700.0));

		child3Polygon.basicRectangles.clear();
		child3Polygon.basicRectangles.push_back(Rectangle(700.0, 0.0, 800.0, 200.0));

		child4Polygon.basicRectangles.clear();
		child4Polygon.basicRectangles.push_back(Rectangle(800.0, 300.0, 900.0, 600.0));
		child4Polygon.basicRectangles.push_back(Rectangle(900.0, 400.0, 1000.0, 500.0));

		child5Polygon.basicRectangles.clear();
		child5Polygon.basicRectangles.push_back(Rectangle(0.0, 300.0, 100.0, 500.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);
		routing->children.push_back(child4);
		routing->boundingBoxes.push_back(child4Polygon);
		routing->children.push_back(child5);
		routing->boundingBoxes.push_back(child5Polygon);

		// Split the node in two
		// sr = routing->splitNode(child3, child3Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;


		// Test set seven
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);
		child3 = new Node(3, 3, routing);
		child4 = new Node(3, 3, routing);
		child5 = new Node(3, 3, routing);
		child6 = new Node(3, 3, routing);
		child7 = new Node(3, 3, routing);
		Node *child8 = new Node(3, 3, routing);
		Node *child9 = new Node(3, 3, routing);
		Node *child10 = new Node(3, 3, routing);
		Node *child11 = new Node(3, 3, routing);

		routingPolygon.basicRectangles.clear();
		routingPolygon.basicRectangles.push_back(Rectangle(700.0, 0.0, 1000.0, 600.0));
		routingPolygon.basicRectangles.push_back(Rectangle(0.0, 500.0, 300.0, 1000.0));
		routingPolygon.basicRectangles.push_back(Rectangle(300.0, 400.0, 700.0, 700.0));
		routingPolygon.basicRectangles.push_back(Rectangle(1000.0, 400.0, 1700.0, 1400.0));
		routingPolygon.basicRectangles.push_back(Rectangle(300.0, 700.0, 600.0, 1100.0));
		routingPolygon.basicRectangles.push_back(Rectangle(400.0, 1100.0, 1000.0, 1500.0));

		child0Polygon.basicRectangles.clear();
		child0Polygon.basicRectangles.push_back(Rectangle(900.0, 400.0, 1100.0, 600.0));

		child1Polygon.basicRectangles.clear();
		child1Polygon.basicRectangles.push_back(Rectangle(1400.0, 700.0, 1500.0, 800.0));
		child1Polygon.basicRectangles.push_back(Rectangle(1200.0, 600.0, 1600.0, 700.0));
		child1Polygon.basicRectangles.push_back(Rectangle(1400.0, 500.0, 1500.0, 600.0));

		child2Polygon.basicRectangles.clear();
		child2Polygon.basicRectangles.push_back(Rectangle(100.0, 600.0, 400.0, 700.0));
		child2Polygon.basicRectangles.push_back(Rectangle(300.0, 700.0, 400.0, 900.0));

		child3Polygon.basicRectangles.clear();
		child3Polygon.basicRectangles.push_back(Rectangle(400.0, 1000.0, 500.0, 1200.0));

		child4Polygon.basicRectangles.clear();
		child4Polygon.basicRectangles.push_back(Rectangle(800.0, 1200.0, 900.0, 1300.0));
		child4Polygon.basicRectangles.push_back(Rectangle(900.0, 1100.0, 1100.0, 1400.0));

		child5Polygon.basicRectangles.clear();
		child5Polygon.basicRectangles.push_back(Rectangle(800.0, 0.0, 900.0, 300.0));

		child6Polygon.basicRectangles.clear();
		child6Polygon.basicRectangles.push_back(Rectangle(400.0, 1300.0, 700.0, 1500.0));

		child7Polygon.basicRectangles.clear();
		child7Polygon.basicRectangles.push_back(Rectangle(1500.0, 1000.0, 1600.0, 1200.0));
		child7Polygon.basicRectangles.push_back(Rectangle(1500.0, 900.0, 1700.0, 1000.0));

		IsotheticPolygon child8Polygon;
		child8Polygon.basicRectangles.push_back(Rectangle(0.0, 900.0, 100.0, 1000.0));

		IsotheticPolygon child9Polygon;
		child9Polygon.basicRectangles.push_back(Rectangle(1200.0, 900.0, 1300.0, 1100.0));
		child9Polygon.basicRectangles.push_back(Rectangle(1300.0, 800.0, 1400.0, 1000.0));

		IsotheticPolygon child10Polygon;
		child10Polygon.basicRectangles.push_back(Rectangle(1200.0, 1300.0, 1600.0, 1400.0));

		IsotheticPolygon child11Polygon;
		child11Polygon.basicRectangles.push_back(Rectangle(600.0, 400.0, 800.0, 600.0));
		child11Polygon.basicRectangles.push_back(Rectangle(500.0, 500.0, 600.0, 800.0));

		// Organize into a tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(routingPolygon);

		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(child0Polygon);
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(child1Polygon);
		routing->children.push_back(child2);
		routing->boundingBoxes.push_back(child2Polygon);
		routing->children.push_back(child4);
		routing->boundingBoxes.push_back(child4Polygon);
		routing->children.push_back(child5);
		routing->boundingBoxes.push_back(child5Polygon);
		routing->children.push_back(child6);
		routing->boundingBoxes.push_back(child6Polygon);
		routing->children.push_back(child7);
		routing->boundingBoxes.push_back(child7Polygon);
		routing->children.push_back(child8);
		routing->boundingBoxes.push_back(child8Polygon);
		routing->children.push_back(child9);
		routing->boundingBoxes.push_back(child9Polygon);
		routing->children.push_back(child10);
		routing->boundingBoxes.push_back(child10Polygon);
		routing->children.push_back(child11);
		routing->boundingBoxes.push_back(child11Polygon);

		// Split the node in two
		// sr = routing->splitNode(child3, child3Polygon);

		// TODO: Test the split
		// assert(root->boundingBoxes.size() == 2);
		// assert(root->children.size() == 2);
		// assert(routing->boundingBoxes.size() == 2);
		// assert(routing->children.size() == 2);
		// assert(sr.first->boundingBoxes.size() == 1);
		// assert(sr.first->children.size() == 1);

		pr.printToPencil(root);

		// Cleanup
		root->deleteSubtrees();
		delete root;
		// delete sr.left;
		// delete sr.right;
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
		// cluster6->condenseTree();

		// Test the condensing
		assert(cluster6->parent == nullptr);
		assert(cluster6->boundingBoxes.size() == 0);
		assert(cluster6->children.size() == 0);
		assert(cluster6->data.size() == 7);
		assert(cluster6->data[0] == Point(-2.0, -6.0));
		assert(cluster6->data[6] == Point(-3.0, -11.0));

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
		// (-3, -11) left out so the bounding box should change

		Node *root = new Node();
		cluster6->parent = root;
		root->boundingBoxes.push_back(Rectangle(-3.0, -10.0, 3.0, 6.0));
		root->children.push_back(cluster6);

		// Condense the tree
		// cluster6->condenseTree();

		// Test the condensing
		assert(root->parent == nullptr);
		assert(root->boundingBoxes.size() == 1);
		assert(root->children.size() == 1);
		assert(root->boundingBoxes[0] == Rectangle(-2.0, -9.0, 3.0, -6.0));
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
		cluster4b->data.push_back(Point(-10.0, -5.0));

		Node *cluster4 = new Node();
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
		root = new Node();
		cluster4->parent = root;
		root->boundingBoxes.push_back(cluster4->boundingBox());
		root->children.push_back(cluster4);
		cluster5->parent = root;
		root->boundingBoxes.push_back(cluster5->boundingBox());
		root->children.push_back(cluster5);

		// Condense the tree
		// Node *newRoot = cluster4b->condenseTree();

		// Test the condensing
		// assert(newRoot == root);
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(root->children[0]->children.size() == 4);
		assert(root->children[1]->children.size() == 2);
		assert(root->children[1]->children[0]->data.size() == 2);
		assert(root->children[1]->children[0]->data[0] == Point(-9.0, -3.0));
		assert(root->children[1]->children[0]->data[1] == Point(-7.0, -3.0));

		// Cleanup
		root->deleteSubtrees();
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

		Node *right = new Node();
		cluster3a->parent = right;
		right->boundingBoxes.push_back(cluster3a->boundingBox());
		right->children.push_back(cluster3a);
		cluster3b->parent = right;
		right->boundingBoxes.push_back(cluster3b->boundingBox());
		right->children.push_back(cluster3b);

		Node *root = new Node();
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
		assert(v1.size() == 3);
		assert(v1[0] == Point(-8.0, 10.0));
		assert(v1[1] == Point(-9.0, 10.0));
		assert(v1[2] == Point(-5.0, 12.0));

		// Test set two
		Rectangle sr2 = Rectangle(-8.0, 4.0, -5.0, 8.0);
		std::vector<Point> v2 = root->search(sr2);
		assert(v2.size() == 2);
		assert(v2[0] == Point(-5.0, 4.0));
		assert(v2[1] == Point(-8.0, 8.0));

		// Test set three
		Rectangle sr3 = Rectangle(-8.0, 0.0, -4.0, 16.0);
		std::vector<Point> v3 = root->search(sr3);
		assert(v3.size() == 12);
		assert(v3[0] == Point(-5.0, 4.0));
		assert(v3[1] == Point(-4.0, 3.0));
		assert(v3[2] == Point(-4.0, 1.0));
		assert(v3[3] == Point(-6.0, 2.0));
		assert(v3[4] == Point(-8.0, 10.0));
		assert(v3[5] == Point(-8.0, 9.0));
		assert(v3[6] == Point(-8.0, 8.0));
		assert(v3[7] == Point(-5.0, 12.0));
		assert(v3[8] == Point(-5.0, 15.0));
		assert(v3[9] == Point(-6.0, 14.0));
		assert(v3[10] == Point(-8.0, 16.0));
		assert(v3[11] == Point(-4.0, 13.0));

		// Test set four
		Rectangle sr4 = Rectangle(2.0, -4.0, 4.0, -2.0);
		std::vector<Point> v4 = root->search(sr4);
		assert(v4.size() == 0);

		// Test set five
		Rectangle sr5 = Rectangle(-3.5, 1.0, -1.5, 3.0);
		std::vector<Point> v5 = root->search(sr5);
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
		assert(root->children[0]->parent == root);
		assert(root->children[0]->boundingBoxes.size() == 0);
		assert(root->children[0]->children.size() == 0);
		assert(root->children[0]->data.size() == 3);
		assert(root->children[1]->parent == root);
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
		assert(root->children[0]->parent == root);
		assert(root->children[0]->boundingBoxes.size() == 3);
		assert(root->children[0]->children.size() == 3);
		assert(root->children[0]->data.size() == 0);
		assert(root->children[1]->parent == root);
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
		assert(root->children[0]->parent == root);
		assert(root->children[0]->boundingBoxes.size() == 5);
		assert(root->children[0]->children.size() == 5);
		assert(root->children[0]->data.size() == 0);
		assert(root->children[1]->parent == root);
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
		assert(root->children[0]->parent == root);
		assert(root->children[0]->boundingBoxes.size() == 5);
		assert(root->children[0]->children.size() == 5);
		assert(root->children[0]->data.size() == 0);
		assert(root->children[1]->parent == root);
		assert(root->children[1]->boundingBoxes.size() == 3);
		assert(root->children[1]->children.size() == 3);
		assert(root->children[1]->data.size() == 0);
		assert(root->children[2]->parent == root);
		assert(root->children[2]->boundingBoxes.size() == 3);
		assert(root->children[2]->children.size() == 3);
		assert(root->children[2]->data.size() == 0);

		// Cleanup
		root->deleteSubtrees();
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
		Node *root = new Node();
		cluster3->parent = root;
		root->boundingBoxes.push_back(cluster3->boundingBox());
		root->children.push_back(cluster3);
		cluster5->parent = root;
		root->boundingBoxes.push_back(cluster5->boundingBox());
		root->children.push_back(cluster5);
		cluster1->parent = root;
		root->boundingBoxes.push_back(cluster1->boundingBox());
		root->children.push_back(cluster1);

		assert(root->boundingBoxes.size() == 3);

		// Remove an element, no other changes in tree
		root->remove(Point(-12.0, -15.0));

		// Test the removal
		assert(cluster5d->data.size() == 3);
		assert(cluster5d->data[0] == Point(-15.0, -15.0));
		assert(cluster5d->data[1] == Point(-14.0, -15.0));
		assert(cluster5d->data[2] == Point(-13.0, -15.0));
		assert(cluster5->parent == root);
		assert(cluster5->boundingBoxes.size() == 4);
		assert(cluster5->children.size() == 4);
		assert(root->parent == nullptr);
		assert(root->boundingBoxes.size() == 3);
		assert(root->children.size() == 3);

		// Remove an element, parent is altered
		root->remove(Point(-6.0, 2.0));

		// Test the removal
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(root->children[0]->parent == root);
		assert(root->children[0]->boundingBoxes.size() == 4);
		assert(root->children[0]->children.size() == 4);
		assert(root->children[1]->parent == root);
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
		// (-3.0, -11.0) held out so we get a shrinking root

		// Root
		root = new Node();
		cluster6a->parent = root;
		root->boundingBoxes.push_back(cluster6a->boundingBox());
		root->children.push_back(cluster6a);
		cluster6b->parent = root;
		root->boundingBoxes.push_back(cluster6b->boundingBox());
		root->children.push_back(cluster6b);

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
}
