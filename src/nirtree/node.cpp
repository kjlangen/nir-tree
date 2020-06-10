#include <nirtree/node.h>

namespace nirtree
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

	// TODO: Convert
	// TODO: Optimize maybe
	// void Node::removeChild(Node *child)
	// {
	// 	for (unsigned i = 0; i < children.size(); ++i)
	// 	{
	// 		if (children[i] == child)
	// 		{
	// 			boundingBoxes.erase(boundingBoxes.begin() + i);
	// 			children.erase(children.begin() + i);
	// 			break;
	// 		}
	// 	}
	// }

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
	// This top-to-bottom sweep is only for adjusting bounding boxes to contain the point and choosing
	// a particular leaf
	Node *Node::chooseLeaf(Point givenPoint)
	{
		// CL1 [Initialize]
		Node *node = this;
		unsigned prevSmallestExpansionIndex = 0;

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

				// CL3.1 Expand the chosen bounding polygon to cover the new point and keep it
				// within the area defined by our bounding polygon. The root has no bounding polygon
				if (node->parent == nullptr)
				{
					node->boundingBoxes[smallestExpansionIndex].expand(givenPoint);
				}
				else
				{
					node->boundingBoxes[smallestExpansionIndex].expand(givenPoint, node->parent->boundingBoxes[prevSmallestExpansionIndex]);
				}

				// CL3.2 Trim back the chosen bounding polygon so it plays nice with our other
				// children
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					if (node->boundingBoxes[i].intersectsRectangle(node->boundingBoxes[smallestExpansionIndex]))
					{
						node->boundingBoxes[smallestExpansionIndex].increaseResolution(node->boundingBoxes[i]);
					}
				}

				// CL4 [Descend until a leaf is reached]
				node = node->children[smallestExpansionIndex];
				prevSmallestExpansionIndex = smallestExpansionIndex;
			}
		}
	}

	// Always called on root, this = root
	// Node *Node::chooseNode(ReinsertionEntry e)
	// {
	// 	// CL1 [Initialize]
	// 	Node *node = this;

	// 	for (;;)
	// 	{
	// 		// CL2 [Leaf check]
	// 		if (node->children.size() == 0)
	// 		{
	// 			for (unsigned i = 0; i < e.level; ++i)
	// 			{
	// 				node = node->parent;
	// 			}

	// 			return node;
	// 		}
	// 		else
	// 		{
	// 			// CL3 [Choose subtree]
	// 			// Find the bounding box with least required expansion/overlap?
	// 			// TODO: Break ties by using smallest area
	// 			unsigned smallestExpansionIndex = 0;
	// 			float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(e.boundingBox);
	// 			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
	// 			{
	// 				float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(e.boundingBox);
	// 				if (smallestExpansionArea > testExpansionArea)
	// 				{
	// 					smallestExpansionIndex = i;
	// 					smallestExpansionArea = testExpansionArea;
	// 				}
	// 			}

	// 			// CL4 [Descend until a leaf is reached]
	// 			node = node->children[smallestExpansionIndex];
	// 		}
	// 	}
	// }

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

	std::vector<Rectangle> Node::decomposeNode(IsotheticPolygon &boundingPolygon)
	{
		// Copy our bounding polygon to start
		IsotheticPolygon temp = boundingPolygon;

		// Decompose our bounding box using code for increasing a polygon's "resolution"
		for (unsigned i = 0; i < boundingBoxes.size(); ++i)
		{
			std::cout << "Current polygon "; temp.print(); std::cout << std::endl;
			std::cout << "Cutting out "; boundingBoxes[i].print();
			temp.increaseResolution(boundingBoxes[i]);
			std::cout << std::endl << "Result "; temp.print(); std::cout << std::endl;
		}

		return temp.basicRectangles;
	}

	Node::SplitResult Node::splitNode(Node *newChild, IsotheticPolygon newPolygon)
	{
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);

		newChild->parent = this;
		boundingBoxes.push_back(newPolygon);
		children.push_back(newChild);

		const unsigned childrenSize = children.size();
		unsigned polygonIndex;
		IsotheticPolygon ourPolygon;

		std::cout << "Checking edge case" << std::endl;
		if (parent == nullptr)
		{
			for (unsigned i = 0; i < children.size(); ++i)
			{
				ourPolygon.basicRectangles.insert(ourPolygon.basicRectangles.begin(), boundingBoxes[i].basicRectangles.begin(), boundingBoxes[i].basicRectangles.end());
			}
			Rectangle r = ourPolygon.boundingBox();
			ourPolygon.basicRectangles.clear();
			ourPolygon.basicRectangles[0] = r;
		}
		else
		{
			for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}

			ourPolygon = parent->boundingBoxes[polygonIndex];
		}

		// Decompose polygon
		std::cout << "Decomposing node" << std::endl;
		std::vector<Rectangle> decomposed = decomposeNode(ourPolygon);

		const unsigned decomposedSize = decomposed.size();
		const unsigned totalSize = childrenSize + decomposedSize;
		const unsigned root = totalSize / 2;

		PencilPrinter p1;
		p1.printToPencil(decomposed);

		// Build graph
		std::cout << "Builidng graph" << std::endl;
		bool graph[totalSize][totalSize];
		std::memset(graph, false, totalSize * totalSize);
		for (unsigned i = 0; i < childrenSize; ++i)
		{
			// Children X Children
			std::cout << "Children X Children" << std::endl;
			for (unsigned j = 0; j < i && j < childrenSize; ++j)
			{
				if (boundingBoxes[i].intersectsRectangle(boundingBoxes[j]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
			std::cout << "Done" << std::endl;

			// Children X Decomposed Polygon
			std::cout << "Children X Decomposed" << std::endl;
			for (unsigned j = childrenSize; j < totalSize; ++j)
			{
				if (boundingBoxes[i].intersectsRectangle(decomposed[j - childrenSize]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
			std::cout << "Done" << std::endl;
		}
		std::cout << "Decomposed polygon X Decomposed polygon" << std::endl;
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
		std::cout << "Done" << std::endl;

		std::cout << "Graph" << std::endl;
		for (unsigned i = 0; i < totalSize; ++i)
		{
			std::cout << "|";
			for (unsigned j = 0; j < totalSize; ++j)
			{
				std::cout << " " << graph[i][j];
			}
			std::cout << "| " << i << std::endl;
		}
		std::cout << std::endl;

		// Build tree
		std::cout << "Building tree" << std::endl;
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
		explored[root] = true;
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();

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
			explorationQ.pop();
		}

		std::cout << "Tree:" << std::endl;
		for (unsigned i = 0; i < totalSize; ++i)
		{
			std::cout << "|";
			for (unsigned j = 0; j < totalSize; ++j)
			{
				std::cout << " " << tree[i][j];
			}
			std::cout << "| " << i << std::endl;
		}
		std::cout << std::endl;

		// Push weights up the tree
		std::cout << "Pushing weights up the tree" << std::endl;
		std::cout << "parentStack.size(): " << parentStack.size() << std::endl;
		std::cout << "WeightStack.size(): " << weightStack.size() << std::endl;
		std::cout << "totalSize: " << totalSize << std::endl;
		for (;parentStack.size() && weightStack.size();)
		{
			weights[parentStack.top()] += weights[weightStack.top()];
			weightStack.pop();
			parentStack.pop();
		}

		// Find a seperator
		std::cout << "Finding a separator" << std::endl;
		unsigned delta = std::numeric_limits<unsigned>::max();
		unsigned subtreeRoot, subtreeParent;

		std::memset(explored, false, totalSize);

		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			for (unsigned neighbouringVertex = 0; neighbouringVertex < totalSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					// |(weight of whole tree - weight of this subtree) - weight of this subtree| < delta?
					// If yes that means that the subtree rooted at neighbouring vertex is more
					// balanced than any of our previous splits.
					// TODO: The math here can be simplified, I'm sure of it, we know total weight
					// and so we should be able to do some set-complement-y stuff...
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
			explorationQ.pop();
		}

		// Split along seperator
		std::cout << "Splitting along separator" << std::endl;
		bool switchboard[totalSize];
		tree[subtreeRoot][subtreeParent] = tree[subtreeParent][subtreeRoot] = false;

		std::memset(switchboard, false, totalSize);
		std::memset(explored, false, totalSize);

		switchboard[subtreeRoot] = true;

		explorationQ.push(subtreeRoot);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			for (unsigned neighbouringVertex = 0; neighbouringVertex < totalSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					switchboard[neighbouringVertex] = true;
					explorationQ.push(neighbouringVertex);
				}
			}
			explored[currentVertex] = true;
			explorationQ.pop();
		}

		std::cout << "Tree:" << std::endl;
		for (unsigned i = 0; i < totalSize; ++i)
		{
			std::cout << "|";
			for (unsigned j = 0; j < totalSize; ++j)
			{
				std::cout << " " << tree[i][j];
			}
			std::cout << "| " << i << std::endl;
		}
		std::cout << std::endl;

		// Recompose into two polygons by building polygons representing what we have now and then
		// reducing in size their representation
		std::cout << "Recomposing polygons" << std::endl;
		IsotheticPolygon currentNewPolygon;
		IsotheticPolygon currentOldPolygon;
		IsotheticPolygon finalNewPolygon;
		IsotheticPolygon finalOldPolygon;

		for (unsigned i = 0; i < totalSize; ++i)
		{
			if (switchboard[i])
			{
				if (i < childrenSize)
				{
					currentNewPolygon.basicRectangles.insert(currentNewPolygon.basicRectangles.end(), boundingBoxes[i].basicRectangles.begin(), boundingBoxes[i].basicRectangles.end());
				}
				else
				{
					currentNewPolygon.basicRectangles.emplace_back(decomposed[i - childrenSize]);
				}
			}
			else
			{
				if (i < childrenSize)
				{
					currentOldPolygon.basicRectangles.insert(currentOldPolygon.basicRectangles.end(), boundingBoxes[i].basicRectangles.begin(), boundingBoxes[i].basicRectangles.end());
				}
				else
				{
					currentOldPolygon.basicRectangles.emplace_back(decomposed[i - childrenSize]);
				}
			}
		}

		finalNewPolygon.basicRectangles.push_back(currentNewPolygon.boundingBox());
		finalOldPolygon.basicRectangles.push_back(currentOldPolygon.boundingBox());

		// Intersect with original polygon => No going outside original boundaries
		finalNewPolygon.intersection(ourPolygon);
		finalOldPolygon.intersection(ourPolygon);

		// Remove from the new polygon the old polygon defined by the separator found earlier
		finalNewPolygon.increaseResolution(currentOldPolygon);
		// Remove from the old polygon the new polygon defined by the separator found earlier (use finalNewPolygon b/c it is smaller)
		finalOldPolygon.increaseResolution(finalNewPolygon);

		// Split children in two
		std::cout << "Splitting children" << std::endl;
		std::vector<IsotheticPolygon> oldBoundingBoxes;
		std::vector<IsotheticPolygon> newBoundingBoxes;
		std::vector<Node *> oldChildren;
		std::vector<Node *> newChildren;

		for (unsigned i = 0; i < childrenSize; ++i)
		{
			if (switchboard[i])
			{
				newBoundingBoxes.emplace_back(boundingBoxes[i]);
				newChildren.emplace_back(children[i]);
			}
			else
			{
				oldBoundingBoxes.emplace_back(boundingBoxes[i]);
				oldChildren.emplace_back(children[i]);
			}
		}

		// Update new node's children
		newSibling->boundingBoxes.clear();
		newSibling->boundingBoxes.swap(newBoundingBoxes);
		newSibling->children.clear();
		newSibling->children.swap(newChildren);
		// Update our children
		boundingBoxes.clear();
		boundingBoxes.swap(oldBoundingBoxes);
		children.clear();
		children.swap(oldChildren);

		// Update our bounding box if we are not the root
		std::cout << "Updating bounding boxes" << std::endl;
		if (parent != nullptr)
		{
			parent->boundingBoxes[polygonIndex].basicRectangles.clear();
			parent->boundingBoxes[polygonIndex] = finalOldPolygon;
		}

		std::cout << "Return" << std::endl;
		return {newSibling, finalNewPolygon, finalOldPolygon};
	}

	Node::SplitResult Node::splitNodeSpecialCase(Point newData)
	{
		// Setup the new node and its bounding polygon
		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		IsotheticPolygon newSiblingIsotheticPolygon;

		data.push_back(newData);

		const unsigned dataSize = data.size();
		const unsigned dataHalfSize = dataSize / 2;

		// Always split along the first dimension
		// TODO: To get really fancy we could do some variance analysis and split along the
		// dimension with highest variance.
		std::sort(data.begin(), data.end(), [](Point a, Point b){return a.x < b.x;});

		// Fill new node with data and remove that data from this node
		for (unsigned i = dataHalfSize; i < dataSize; ++i)
		{
			newSibling->data.push_back(data[i]);
		}
		data.resize(dataHalfSize);

		// Create the bounding box for newSibling
		float newMinY = newSibling->data[0].y;
		float newMaxY = newSibling->data[0].y;
		for (unsigned i = 1; i < newSibling->data.size(); ++i)
		{
			newMinY = std::min(newMinY, newSibling->data[i].y);
			newMaxY = std::max(newMaxY, newSibling->data[i].y);
		}

		float oldMinY = data[0].y;
		float oldMaxY = data[0].y;
		for (unsigned i = 1; i < data.size(); ++i)
		{
			oldMinY = std::min(oldMinY, data[i].y);
			oldMaxY = std::max(oldMaxY, data[i].y);
		}

		return
		{
			newSibling,
			IsotheticPolygon(Rectangle(newSibling->data[0].x, newMinY, newSibling->data[newSibling->data.size() - 1].x, newMaxY)),
			IsotheticPolygon(Rectangle(data[0].x, oldMinY, data[data.size() - 1].x, oldMaxY))
		};
	}

	// TODO: Because we're using vectors and didn't exactly implement the original R-Tree rewriting this
	// with sets will necessitate rewriting the entire R-Tree with sets.
	Node::SplitResult Node::splitNode(Point newData)
	{
		// Special case where the leaf to be split is the root
		if (parent == nullptr)
		{
			return splitNodeSpecialCase(newData);
		}

		unsigned polygonIndex;
		for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}
		std::vector<Rectangle> &basics = parent->boundingBoxes[polygonIndex].basicRectangles;
		const unsigned basicsSize = basics.size();
		const unsigned dataSize = data.size();
		const unsigned root = basicsSize / 2;

		// Special case where the leaf is bounded by a simple rectangle
		if (basicsSize == 1)
		{
			SplitResult sr = splitNodeSpecialCase(newData);
			parent->boundingBoxes[polygonIndex] = sr.third;
			return sr;
		}

		Node *newSibling = new Node(minBranchFactor, maxBranchFactor, parent);
		data.push_back(newData);
		
		// Build graph
		// std::cout << "Building graph..." << std::endl;
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

		// Printing...
		// std::cout << "    0 1 2 3 4  " << std::endl;
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << i << " | ";
		// 	for (unsigned j = 0; j < basics.size(); ++j)
		// 	{
		// 		std::cout << graph[i][j] << " ";
		// 	}
		// 	std::cout << "|" << std::endl;
		// }

		// Build tree
		// std::cout << "Building tree..." << std::endl;
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
		explored[root] = true;
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();

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

			explored[currentVertex] = true;
			explorationQ.pop();
		}

		// Printing...
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout <<  i  << " | ";
		// 	for (unsigned j = 0; j < basics.size(); ++j)
		// 	{
		// 		std::cout << tree[i][j] << " ";
		// 	}
		// 	std::cout << "|" << std::endl;
		// }

		// Printing...
		// std::cout << "explored = [";
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << " " << explored[i];
		// }
		// std::cout << " ]" << std::endl;

		// Printing...
		// std::cout << "connected = [";
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << " " << connected[i];
		// }
		// std::cout << " ]" << std::endl;

		// Printing...
		// std::cout << "weights = [";
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << " " << weights[i];
		// }
		// std::cout << " ]" << std::endl;

		// Weight the tree so we can quickly find a separator
		// std::cout << "Combining weights..." << std::endl;
		for (unsigned i = 0; i < basicsSize - 1; ++i)
		{
			weights[parentStack.top()] += weights[weightStack.top()];
			weightStack.pop();
			parentStack.pop();
		}

		// Printing...
		// std::cout << "weights = [";
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << " " << weights[i];
		// }
		// std::cout << " ]" << std::endl;

		// Find a separator
		// std::cout << "Finding a separator..." << std::endl;
		unsigned delta = std::numeric_limits<unsigned>::max();
		unsigned subtreeRoot, subtreeParent;

		std::memset(explored, false, basicsSize);

		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			for (unsigned neighbouringVertex = 0; neighbouringVertex < basicsSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					// |(weight of whole tree - weight of this subtree) - weight of this subtree| < delta?
					// If yes that means that the subtree rooted at neighbouring vertex is more
					// balanced than any of our previous splits.
					// TODO: The math here can be simplified, I'm sure of it, we know total weight
					// and so we should be able to do some set-complement-y stuff...
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
			explorationQ.pop();
		}

		// Split along seperator
		// std::cout << "Splitting along separator..." << std::endl;
		bool switchboard[basicsSize];
		tree[subtreeRoot][subtreeParent] = tree[subtreeParent][subtreeRoot] = false;

		std::memset(switchboard, false, basicsSize);
		std::memset(explored, false, basicsSize);

		switchboard[subtreeRoot] = true;

		explorationQ.push(subtreeRoot);
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			for (unsigned neighbouringVertex = 0; neighbouringVertex < basicsSize; ++neighbouringVertex)
			{
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					switchboard[neighbouringVertex] = true;
					explorationQ.push(neighbouringVertex);
				}
			}
			explored[currentVertex] = true;
			explorationQ.pop();
		}

		// Printing...
		// std::cout << "switchboard = [";
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << " " << switchboard[i];
		// }
		// std::cout << " ]" << std::endl;

		// Break the tree into two components
		// TODO: Optimize, fairly certain swapping could be made faster
		// std::cout << "Splitting and swapping..." << std::endl;


		// Break the bounding polygon in two
		IsotheticPolygon finalOldPolygon;
		IsotheticPolygon finalNewPolygon;

		for (unsigned i = 0; i < basicsSize; ++i)
		{
			if (switchboard[i])
			{
				finalNewPolygon.basicRectangles.emplace_back(basics[i]);
			}
			else
			{
				finalOldPolygon.basicRectangles.emplace_back(basics[i]);
			}
		}

		// Update our bounding box
		parent->boundingBoxes[polygonIndex] = finalOldPolygon;

		// Break the data in two
		std::vector<Point> oldPoints;
		std::vector<Point> newPoints;

		for (unsigned i = 0; i < dataSize; ++i)
		{
			if (finalNewPolygon.containsPoint(data[i]))
			{
				newPoints.emplace_back(data[i]);
			}
			else
			{
				oldPoints.emplace_back(data[i]);
			}
		}

		// Update new node's data
		newSibling->data.clear();
		newSibling->data.swap(newPoints);
		// Update our data
		data.clear();
		data.swap(oldPoints);

		// std::cout << "Old stuff recombobulated:" << std::endl;
		// parent->boundingBoxes[polygonIndex].print();
		// this->print();
		// std::cout << "New stuff recombobulated:" << std::endl;
		// newSiblingIsotheticPolygon.print();
		// newSibling->print();

		return {newSibling, finalNewPolygon, finalOldPolygon};
	}

	// This bottom-to-top sweep is only for splitting bounding boxes as necessary
	Node::SplitResult Node::adjustTree(Node *sibling, IsotheticPolygon polygon)
	{
		// AT1 [Initialize]
		Node *node = this;
		Node *siblingNode = sibling;
		IsotheticPolygon siblingPolygon = polygon;

		for (;;)
		{
			// AT2 [If node is the root, stop]
			if (node->parent == nullptr)
			{
				break;
			}
			else
			{
				// If one of children split then integrate the new node otherwise keep ascending the tree
				if (siblingNode != nullptr)
				{
					// AT4 [Propogate the node split upwards]
					if (node->parent->children.size() < node->parent->maxBranchFactor)
					{
						node->parent->boundingBoxes.push_back(siblingPolygon);
						node->parent->children.push_back(siblingNode);
						siblingNode->parent = node->parent;
						// No split so clear the variable
						siblingNode = nullptr;
					}
					else
					{
						SplitResult sr = node->parent->splitNode(siblingNode, siblingPolygon);

						// Is a split so set the variables
						siblingNode = sr.first;
						siblingPolygon = sr.second;
					}

					// AT5 [Move up to next level]
					node = node->parent;
				}
				else
				{
					// AT5 [Move up to next level]
					node = node->parent;
				}
			}
		}

		return {siblingNode, polygon, IsotheticPolygon()};
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		// I1 [Find position for new record]
		Node *leaf = chooseLeaf(givenPoint);
		Node *siblingLeaf = nullptr;
		IsotheticPolygon siblingPolygon;

		// I2 [Add record to leaf node]
		if (leaf->data.size() < leaf->maxBranchFactor)
		{
			leaf->data.push_back(givenPoint);
		}
		else
		{
			Node::SplitResult sr = leaf->splitNode(givenPoint);
			siblingLeaf = sr.first;
			siblingPolygon = sr.second;
		}

		// I3 [Propogate changes upward]
		SplitResult adjustmentResult = leaf->adjustTree(siblingLeaf, siblingPolygon);

		// I4 [Grow tree taller]
		if (adjustmentResult.first != nullptr)
		{
			Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

			// Push the modified old root node into the new root node
			this->parent = newRoot;
			newRoot->boundingBoxes.push_back(adjustmentResult.third);
			newRoot->children.push_back(this);

			// Push the new node into the new root node
			adjustmentResult.first->parent = newRoot;
			newRoot->boundingBoxes.push_back(adjustmentResult.second);
			newRoot->children.push_back(adjustmentResult.first);

			return newRoot;
		}
		else
		{
			return this;
		}
	}

	// Always called on root, this = root
	// TODO: Convert
	// Node *Node::insert(ReinsertionEntry e)
	// {
	// 	// If reinserting a leaf then use normal insert
	// 	if (e.level == 0)
	// 	{
	// 		return insert(e.data);
	// 	}

	// 	// I1 [Find position for new record]
	// 	Node *node = chooseNode(e);
	// 	Node *siblingNode = nullptr;

	// 	// I2 [Add record to node]
	// 	if (node->children.size() < node->maxBranchFactor)
	// 	{
	// 		e.child->parent = node;
	// 		node->boundingBoxes.push_back(e.boundingBox);
	// 		node->children.push_back(e.child);
	// 	}
	// 	else
	// 	{
	// 		siblingNode = node->splitNode(e.child);
	// 	}

	// 	// I3 [Propogate changes upward]
	// 	siblingNode = node->adjustTree(siblingNode);

	// 	// I4 [Grow tree taller]
	// 	if (siblingNode != nullptr)
	// 	{
	// 		Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

	// 		this->parent = newRoot;
	// 		newRoot->boundingBoxes.push_back(this->boundingBox());
	// 		newRoot->children.push_back(this);

	// 		siblingNode->parent = newRoot;
	// 		newRoot->boundingBoxes.push_back(siblingNode->boundingBox());
	// 		newRoot->children.push_back(siblingNode);

	// 		return newRoot;
	// 	}
	// 	else
	// 	{
	// 		return this;
	// 	}
	// }

	// To be called on a leaf
	// TODO: Convert
	// Node *Node::condenseTree()
	// {
	// 	// CT1 [Initialize]
	// 	Node *node = this;
	// 	unsigned level = 0;

	// 	std::vector<ReinsertionEntry> Q;

	// 	// CT2 [Find parent entry]
	// 	unsigned nodeBoundingBoxesSize, nodeDataSize;
	// 	while (node->parent != nullptr)
	// 	{
	// 		nodeBoundingBoxesSize = node->boundingBoxes.size();
	// 		nodeDataSize = node->data.size();
	// 		// CT3 & CT4 [Eliminate under-full node. & Adjust covering rectangle.]
	// 		if (nodeBoundingBoxesSize >= node->minBranchFactor || nodeDataSize >= node->minBranchFactor)
	// 		{
	// 			node->parent->updateBoundingBox(node, node->boundingBox());

	// 			// CT5 [Move up one level in the tree]
	// 			// Move up a level without deleting ourselves
	// 			node = node->parent;
	// 			level++;
	// 		}
	// 		else
	// 		{
	// 			// Remove ourselves from our parent
	// 			node->parent->removeChild(node);

	// 			// Add a reinsertion entry for each data point or branch of this node
	// 			for (unsigned i = 0; i < nodeDataSize; ++i)
	// 			{
	// 				ReinsertionEntry e = {};
	// 				e.child = nullptr;
	// 				e.data = node->data[i];
	// 				e.level = 0;
	// 				Q.push_back(e);
	// 			}
	// 			for (unsigned i = 0; i < nodeBoundingBoxesSize; ++i)
	// 			{
	// 				ReinsertionEntry e = {};
	// 				e.boundingBox = node->boundingBoxes[i];
	// 				e.child = node->children[i];
	// 				e.level = level;
	// 				Q.push_back(e);
	// 			}

	// 			// Prepare for garbage collection
	// 			Node *garbage = node;

	// 			// CT5 [Move up one level in the tree]
	// 			// Move up a level before deleting ourselves
	// 			node = node->parent;
	// 			level++;

	// 			// Cleanup ourselves without deleting children b/c they will be reinserted
	// 			delete garbage;
	// 		}
	// 	}

	// 	// CT6 [Re-insert oprhaned entries]
	// 	for (unsigned i = 0; i < Q.size(); ++i)
	// 	{
	// 		node = node->insert(Q[i]);
	// 	}

	// 	return node;
	// }

	// // Always called on root, this = root
	// Node *Node::remove(Point givenPoint)
	// {
	// 	// D1 [Find node containing record]
	// 	Node *leaf = findLeaf(givenPoint);

	// 	if (leaf == nullptr)
	// 	{
	// 		return nullptr;
	// 	}

	// 	// D2 [Delete record]
	// 	leaf->removeData(givenPoint);

	// 	// D3 [Propagate changes]
	// 	Node *root = leaf->condenseTree();

	// 	// D4 [Shorten tree]
	// 	if (root->children.size() == 1)
	// 	{
	// 		root->children[0]->parent = nullptr;
	// 		return root->children[0];
	// 	}
	// 	else
	// 	{
	// 		return root;
	// 	}
	// }

	void Node::print(unsigned n)
	{
		std::string indendtation(n * 4, ' ');
		std::cout << indendtation << "Node " << (void *)this << std::endl;
		std::cout << indendtation << "{" << std::endl;
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
				sum += (unsigned)data[i].x;
				sum += (unsigned)data[i].y;
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

	void testPlayground()
	{
		// Setup
		// Node *root = new Node();
		Node *leaf = new Node();
		// IsotheticPolygon p = IsotheticPolygon();

		// root->children.push_back(leaf);

		// p.basicRectangles.push_back(Rectangle(0.0, 7.0, 5.0, 10.0));
		// p.basicRectangles.push_back(Rectangle(5.0, 5.0, 8.0, 12.0));
		// p.basicRectangles.push_back(Rectangle(8.0, 10.0, 13.0, 14.0));
		// p.basicRectangles.push_back(Rectangle(4.0, 0.0, 10.0, 5.0));
		// p.basicRectangles.push_back(Rectangle(10.0, 0.0, 13.0, 10.0));
		// root->boundingBoxes.push_back(p);

		leaf->data.push_back(Point(1.0, 8.0));
		leaf->data.push_back(Point(2.0, 9.0));
		leaf->data.push_back(Point(3.0, 8.0));
		leaf->data.push_back(Point(6.5, 11.0));
		leaf->data.push_back(Point(6.0, 8.0));
		leaf->data.push_back(Point(7.0, 8.0));
		leaf->data.push_back(Point(7.0, 6.0));
		leaf->data.push_back(Point(6.0, 2.0));
		leaf->data.push_back(Point(9.0, 4.0));
		leaf->data.push_back(Point(11.0, 2.0));
		leaf->data.push_back(Point(12.0, 1.0));
		leaf->data.push_back(Point(12.0, 5.0));
		leaf->data.push_back(Point(12.0, 8.0));
		leaf->data.push_back(Point(10.0, 10.0));
		leaf->data.push_back(Point(13.0, 10.0));
		leaf->data.push_back(Point(12.0, 12.0));

		std::cout << "Setup complete." << std::endl;


		// Split
		// auto timerStart = std::chrono::high_resolution_clock::now();
		// std::pair<Node *, IsotheticPolygon> p = leaf->splitNode(Point(8.0, 1.0));
		// auto timerEnd = std::chrono::high_resolution_clock::now();

		// for (unsigned i = 0; i < leaf->data.size(); ++i)
		// {
		// 	leaf->data[i].print();
		// }
		// std::cout << std::endl;

		// for (unsigned i = 0; i < p.first->data.size(); ++i)
		// {
		// 	p.first->data[i].print();
		// }
		// std::cout << std::endl;
		// p.second.print();


		// // Print results
		// std::cout << "It took " << std::chrono::duration_cast<std::chrono::microseconds>(timerEnd - timerStart).count() << "us to split." << std::endl;
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

	// void testRemoveChild()
	// {
	// 	// Setup a node with some children
	// 	Node parentNode = Node();

	// 	Node *child0 = new Node();
	// 	child0->parent = &parentNode;
	// 	parentNode.boundingBoxes.push_back(Rectangle(8.0, -6.0, 10.0, -4.0));
	// 	parentNode.children.push_back(child0);

	// 	Node *child1 = new Node();
	// 	child1->parent = &parentNode;
	// 	parentNode.boundingBoxes.push_back(Rectangle(12.0, -4.0, 16.0, -2.0));
	// 	parentNode.children.push_back(child1);

	// 	Node *child2 = new Node();
	// 	child2->parent = &parentNode;
	// 	parentNode.boundingBoxes.push_back(Rectangle(10.0, 12.0, 12.0, 14.0));
	// 	parentNode.children.push_back(child2);

	// 	Node *child3 = new Node();
	// 	child3->parent = &parentNode;
	// 	parentNode.boundingBoxes.push_back(Rectangle(12.0, 12.0, 14.0, 14.0));
	// 	parentNode.children.push_back(child3);

	// 	// Remove one of the children
	// 	parentNode.removeChild(child3);
	// 	assert(parentNode.boundingBoxes.size() == 3);
	// 	assert(parentNode.children.size() == 3);

	// 	// Cleanup
	// 	delete child0;
	// 	delete child1;
	// 	delete child2;
	// 	delete child3;
	// }

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

	// void testFindLeaf()
	// {
	// 	// Setup the tree

	// 	// Cluster 4, n = 7
	// 	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// 	// Organized into two nodes
	// 	Node *cluster4a = new Node();
	// 	cluster4a->data.push_back(Point(-10.0, -2.0));
	// 	cluster4a->data.push_back(Point(-12.0, -3.0));
	// 	cluster4a->data.push_back(Point(-11.0, -3.0));
	// 	cluster4a->data.push_back(Point(-10.0, -3.0));

	// 	Node *cluster4b = new Node();
	// 	cluster4b->data.push_back(Point(-9.0, -3.0));
	// 	cluster4b->data.push_back(Point(-7.0, -3.0));
	// 	cluster4b->data.push_back(Point(-10.0, -5.0));

	// 	Node *cluster4 = new Node();
	// 	cluster4a->parent = cluster4;
	// 	cluster4->boundingBoxes.push_back(cluster4a->boundingBox());
	// 	cluster4->children.push_back(cluster4a);
	// 	cluster4b->parent = cluster4;
	// 	cluster4->boundingBoxes.push_back(cluster4b->boundingBox());
	// 	cluster4->children.push_back(cluster4b);

	// 	// Cluster 5, n = 16
	// 	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// 	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// 	// (-14, -15), (-13, -15), (-12, -15)
	// 	// Organized into four nodes
	// 	Node *cluster5a = new Node();
	// 	cluster5a->data.push_back(Point(-14.5, -13.0));
	// 	cluster5a->data.push_back(Point(-14.0, -13.0));
	// 	cluster5a->data.push_back(Point(-13.5, -13.5));
	// 	cluster5a->data.push_back(Point(-15.0, -14.0));

	// 	Node *cluster5b = new Node();
	// 	cluster5b->data.push_back(Point(-14.0, -14.0));
	// 	cluster5b->data.push_back(Point(-13.0, -14.0));
	// 	cluster5b->data.push_back(Point(-12.0, -14.0));
	// 	cluster5b->data.push_back(Point(-13.5, -16.0));

	// 	Node *cluster5c = new Node();
	// 	cluster5c->data.push_back(Point(-15.0, -14.5));
	// 	cluster5c->data.push_back(Point(-14.0, -14.5));
	// 	cluster5c->data.push_back(Point(-12.5, -14.5));
	// 	cluster5c->data.push_back(Point(-13.5, -15.5));

	// 	Node *cluster5d = new Node();
	// 	cluster5d->data.push_back(Point(-15.0, -15.0));
	// 	cluster5d->data.push_back(Point(-14.0, -15.0));
	// 	cluster5d->data.push_back(Point(-13.0, -15.0));
	// 	cluster5d->data.push_back(Point(-12.0, -15.0));

	// 	Node *cluster5 = new Node();
	// 	cluster5a->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	// 	cluster5->children.push_back(cluster5a);
	// 	cluster5b->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	// 	cluster5->children.push_back(cluster5b);
	// 	cluster5c->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	// 	cluster5->children.push_back(cluster5c);
	// 	cluster5d->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	// 	cluster5->children.push_back(cluster5d);

	// 	// Root
	// 	Node *root = new Node();
	// 	cluster4->parent = root;
	// 	root->boundingBoxes.push_back(cluster4->boundingBox());
	// 	root->children.push_back(cluster4);
	// 	cluster5->parent = root;
	// 	root->boundingBoxes.push_back(cluster5->boundingBox());
	// 	root->children.push_back(cluster5);

	// 	// Test finding leaves
	// 	assert(root->findLeaf(Point(-11.0, -3.0)) == cluster4a);
	// 	assert(root->findLeaf(Point(-9.0, -3.0)) == cluster4b);
	// 	assert(root->findLeaf(Point(-13.5, -13.5)) == cluster5a);
	// 	assert(root->findLeaf(Point(-12.0, -14.0)) == cluster5b);
	// 	assert(root->findLeaf(Point(-12.5, -14.5)) == cluster5c);
	// 	assert(root->findLeaf(Point(-13.0, -15.0)) == cluster5d);

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;
	// }

	void testSplitNodeLeaf()
	{
		// // Test set one
		// Node *cluster0 = new Node();

		// cluster6->data.push_back(Point(-2.0, -6.0));
		// cluster6->data.push_back(Point(2.0, -6.0));
		// cluster6->data.push_back(Point(-1.0, -7.0));
		// cluster6->data.push_back(Point(1.0, -7.0));
		// cluster6->data.push_back(Point(3.0, -8.0));
		// cluster6->data.push_back(Point(-2.0, -9.0));

		// // Split the node in two
		// Node *cluster6p = cluster6->splitNode(Point(-3.0, -11.0));

		// // Test the split
		// assert(cluster6->data.size() == 2);
		// assert(cluster6->data[0] == Point(-2.0, -6.0));
		// assert(cluster6->data[1] == Point(2.0, -6.0));
		// assert(cluster6p->data.size() == 5);
		// assert(cluster6p->data[0] == Point(-1.0, -7.0));
		// assert(cluster6p->data[1] == Point(1.0, -7.0));
		// assert(cluster6p->data[2] == Point(3.0, -8.0));
		// assert(cluster6p->data[3] == Point(-2.0, -9.0));
		// assert(cluster6p->data[4] == Point(-3.0, -11.0));

		// // Cleanup
		// delete cluster6;
		// delete cluster6p;

		// // Test set two
		// // Cluster 2, n = 8
		// // (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
		// Node *cluster2 = new Node();
		// cluster2->data.push_back(Point(-14.0, 8.0));
		// cluster2->data.push_back(Point(-10.0, 8.0));
		// cluster2->data.push_back(Point(-9.0, 10.0));
		// cluster2->data.push_back(Point(-9.0, 9.0));
		// cluster2->data.push_back(Point(-8.0, 10.0));
		// cluster2->data.push_back(Point(-9.0, 7.0));
		// cluster2->data.push_back(Point(-8.0, 8.0));

		// // Split the node in two
		// Node *cluster2p = cluster2->splitNode(Point(-8.0, 9.0));

		// // Test the split
		// assert(cluster2->data.size() == 2);
		// assert(cluster2->data[0] == Point(-14.0, 8.0));
		// assert(cluster2->data[1] == Point(-10.0, 8.0));
		// assert(cluster2p->data.size() == 6);
		// assert(cluster2p->data[0] == Point(-9.0, 10.0));
		// assert(cluster2p->data[1] == Point(-9.0, 9.0));
		// assert(cluster2p->data[2] == Point(-8.0, 10.0));
		// assert(cluster2p->data[3] == Point(-9.0, 7.0));
		// assert(cluster2p->data[4] == Point(-8.0, 8.0));
		// assert(cluster2p->data[5] == Point(-8.0, 9.0));

		// // Cleanup
		// delete cluster2;
		// delete cluster2p;

		// // Test set three
		// // Cluster 3, n = 9
		// // (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
		// // {(-5, 4), 1, 1}, {(-2, 4), 1, 1}, {(-1, 3), 1, 1}, {(-1, 1), 1, 1}, {(-3, 0), 1, 1},
		// // {(-6, 2), 1, 1}
		// Node *cluster3 = new Node();
		// Node *dummys[6] = {new Node(), new Node(), new Node(), new Node(), new Node(), new Node()};
		// dummys[0]->parent = cluster3;
		// cluster3->boundingBoxes.push_back(Rectangle(-6.0, 3.0, -4.0, 5.0));
		// cluster3->children.push_back(dummys[0]);
		// dummys[1]->parent = cluster3;
		// cluster3->boundingBoxes.push_back(Rectangle(-3.0, 3.0, -1.0, 5.0));
		// cluster3->children.push_back(dummys[1]);
		// dummys[2]->parent = cluster3;
		// cluster3->boundingBoxes.push_back(Rectangle(-2.0, 2.0, 0.0, 4.0));
		// cluster3->children.push_back(dummys[2]);
		// dummys[3]->parent = cluster3;
		// cluster3->boundingBoxes.push_back(Rectangle(-2.0, 0.0, 0.0, 2.0));
		// cluster3->children.push_back(dummys[3]);
		// dummys[4]->parent = cluster3;
		// cluster3->boundingBoxes.push_back(Rectangle(-4.0, -1.0, -2.0, 1.0));
		// cluster3->children.push_back(dummys[4]);
		// dummys[5]->parent = cluster3;
		// cluster3->boundingBoxes.push_back(Rectangle(-7.0, 1.0, -5.0, 3.0));
		// cluster3->children.push_back(dummys[5]);

		// // Extra node causing the split
		// Node *cluster3extra = new Node();
		// cluster3extra->data.push_back(Point(1.0, 1.0));
		// cluster3extra->data.push_back(Point(2.0, 2.0));

		// // Test the split
		// Node *cluster3p = cluster3->splitNode(cluster3extra);

		// assert(cluster3->children.size() == 5);
		// assert(cluster3->boundingBoxes[0] == Rectangle(-4.0, -1.0, -2.0, 1.0));
		// assert(cluster3->boundingBoxes[1] == Rectangle(-3.0, 3.0, -1.0, 5.0));
		// assert(cluster3->boundingBoxes[2] == Rectangle(-2.0, 2.0, 0.0, 4.0));
		// assert(cluster3->boundingBoxes[3] == Rectangle(-2.0, 0.0, 0.0, 2.0));
		// assert(cluster3->boundingBoxes[4] == Rectangle(1.0, 1.0, 2.0, 2.0));
		// assert(cluster3->children[4] == cluster3extra);

		// assert(cluster3p->children.size() == 2);
		// assert(cluster3p->boundingBoxes[0] == Rectangle(-6.0, 3.0, -4.0, 5.0));
		// assert(cluster3p->boundingBoxes[1] == Rectangle(-7.0, 1.0, -5.0, 3.0));

		// // Cleanup
		// delete cluster3;
		// delete cluster3p;
		// delete cluster3extra;
		// delete dummys[0];
		// delete dummys[1];
		// delete dummys[2];
		// delete dummys[3];
		// delete dummys[4];
		// delete dummys[5];
	}

	void testSplitNodeRoutingSimple()
	{
		// Test set one
		Node *root = new Node();
		Node *routing = new Node(3, 3, root);
		Node *child0 = new Node(3, 3, routing);
		Node *child1 = new Node(3, 3, routing);
		Node *child2 = new Node(3, 3, routing);

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
		Node::SplitResult sr = routing->splitNode(child2, ip);

		// Adjust tree
		root->children.push_back(sr.first);
		root->boundingBoxes.push_back(sr.second);

		// Test the split
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(routing->boundingBoxes.size() == 2);
		assert(routing->children.size() == 2);
		assert(sr.first->boundingBoxes.size() == 1);
		assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;


		// Test set two
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);

		// Organize the tree
		root->children.push_back(routing);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 500.0, 500.0)));
		routing->children.push_back(child0);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 200.0, 100.0, 300.0)));
		routing->children.push_back(child1);
		routing->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 200.0, 500.0, 300.0)));

		// Split the node in two
		sr = routing->splitNode(child2, IsotheticPolygon(Rectangle(200.0, 0.0, 300.0, 500.0)));

		// Adjust the tree
		root->children.push_back(sr.first);
		root->boundingBoxes.push_back(sr.second);

		// Test the split
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(routing->boundingBoxes.size() == 2);
		assert(routing->children.size() == 2);
		assert(sr.first->boundingBoxes.size() == 1);
		assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;


		// Test set three
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);

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
		sr = routing->splitNode(child2, ip);

		// Adjust the tree
		root->children.push_back(sr.first);
		root->boundingBoxes.push_back(sr.second);

		// Test the split
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(routing->boundingBoxes.size() == 2);
		assert(routing->children.size() == 2);
		assert(sr.first->boundingBoxes.size() == 1);
		assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;


		// Test set four
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);

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
		sr = routing->splitNode(child2, ip);

		// Adjust the tree
		root->children.push_back(sr.first);
		root->boundingBoxes.push_back(sr.second);

		// Test the split
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(routing->boundingBoxes.size() == 2);
		assert(routing->children.size() == 2);
		assert(sr.first->boundingBoxes.size() == 1);
		assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;


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
		sr = routing->splitNode(child1, ip1);

		// Adjust the tree
		root->children.push_back(sr.first);
		root->boundingBoxes.push_back(sr.second);

		// Test the split
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(routing->boundingBoxes.size() == 1);
		assert(routing->children.size() == 1);
		assert(sr.first->boundingBoxes.size() == 1);
		assert(sr.first->children.size() == 1);

		// Cleanup
		root->deleteSubtrees();
		delete root;


		// Test set six
		root = new Node();
		routing = new Node(3, 3, root);
		child0 = new Node(3, 3, routing);
		child1 = new Node(3, 3, routing);
		child2 = new Node(3, 3, routing);

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
		sr = routing->splitNode(child2, ip);

		// Adjust the tree
		root->children.push_back(sr.first);
		root->boundingBoxes.push_back(sr.second);

		// Test the split
		assert(root->boundingBoxes.size() == 2);
		assert(root->children.size() == 2);
		assert(routing->boundingBoxes.size() == 1);
		assert(routing->children.size() == 1);
		assert(sr.first->boundingBoxes.size() == 2);
		assert(sr.first->children.size() == 2);

		// Cleanup
		root->deleteSubtrees();
		delete root;
	}

	// void testAdjustTree()
	// {
	// 	// Leaf Node and new sibling leaf
	// 	// Cluster 4, n = 7
	// 	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// 	Node *cluster4a = new Node();
	// 	cluster4a->data.push_back(Point(-10.0, -2.0));
	// 	cluster4a->data.push_back(Point(-12.0, -3.0));
	// 	cluster4a->data.push_back(Point(-11.0, -3.0));
	// 	cluster4a->data.push_back(Point(-10.0, -3.0));

	// 	Node *cluster4b = new Node();
	// 	cluster4b->data.push_back(Point(-9.0, -3.0));
	// 	cluster4b->data.push_back(Point(-7.0, -3.0));
	// 	cluster4b->data.push_back(Point(-10.0, -5.0));

	// 	// Middle Node
	// 	Node *middle = new Node();
	// 	Node *dummys[4] = {new Node(), new Node(), new Node(), new Node()};
	// 	dummys[0]->parent = middle;
	// 	middle->boundingBoxes.push_back(Rectangle(-10.0, 2.0, -8.0, 4.0));
	// 	middle->children.push_back(dummys[0]);
	// 	dummys[1]->parent = middle;
	// 	middle->boundingBoxes.push_back(Rectangle(-12.0, -10.0, -10.0, -8.0));
	// 	middle->children.push_back(dummys[1]);
	// 	cluster4a->parent = middle;
	// 	middle->boundingBoxes.push_back(Rectangle(-12.0, -5.0, -7.0, -2.0));
	// 	middle->children.push_back(cluster4a);
	// 	dummys[2]->parent = middle;
	// 	middle->boundingBoxes.push_back(Rectangle(6.0, 6.0, 8.0, 8.0));
	// 	middle->children.push_back(dummys[2]);
	// 	dummys[3]->parent = middle;
	// 	middle->boundingBoxes.push_back(Rectangle(15.0, -17.0, 17.0, -15.0));
	// 	middle->children.push_back(dummys[3]);

	// 	// Root Node
	// 	Node *root = new Node();
	// 	middle->parent = root;
	// 	root->boundingBoxes.push_back(middle->boundingBox());
	// 	root->children.push_back(middle);

	// 	// Adjust the tree
	// 	Node *result = cluster4a->adjustTree(cluster4b);

	// 	// Test the adjustment
	// 	assert(result == nullptr);
	// 	assert(root->children.size() == 2);
	// 	assert(root->boundingBoxes[0] == Rectangle(-12.0, -10.0, -7.0, 4.0));
	// 	assert(root->boundingBoxes[1] == Rectangle(6.0, -17.0, 17.0, 8.0));
	// 	assert(middle->children.size() == 4);
	// 	assert(middle->boundingBoxes[2] == Rectangle(-12.0, -3.0, -10.0, -2.0));
	// 	assert(middle->boundingBoxes[3] == Rectangle(-10.0, -5.0, -7.0, -3.0));

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;
	// }

	// void testCondenseTree()
	// {
	// 	// Test where the leaf is the root
	// 	// Cluster 6, n = 7
	// 	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	// 	Node *cluster6 = new Node();
	// 	cluster6->data.push_back(Point(-2.0, -6.0));
	// 	cluster6->data.push_back(Point(2.0, -6.0));
	// 	cluster6->data.push_back(Point(-1.0, -7.0));
	// 	cluster6->data.push_back(Point(1.0, -7.0));
	// 	cluster6->data.push_back(Point(3.0, -8.0));
	// 	cluster6->data.push_back(Point(-2.0, -9.0));
	// 	cluster6->data.push_back(Point(-3.0, -11.0));

	// 	// Condense the tree
	// 	cluster6->condenseTree();

	// 	// Test the condensing
	// 	assert(cluster6->parent == nullptr);
	// 	assert(cluster6->boundingBoxes.size() == 0);
	// 	assert(cluster6->children.size() == 0);
	// 	assert(cluster6->data.size() == 7);
	// 	assert(cluster6->data[0] == Point(-2.0, -6.0));
	// 	assert(cluster6->data[6] == Point(-3.0, -11.0));

	// 	// Cleanup
	// 	delete cluster6;

	// 	// Test where condensing is confined to a leaf != root
	// 	// Cluster 6, n = 7
	// 	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	// 	cluster6 = new Node();
	// 	cluster6->data.push_back(Point(-2.0, -6.0));
	// 	cluster6->data.push_back(Point(2.0, -6.0));
	// 	cluster6->data.push_back(Point(-1.0, -7.0));
	// 	cluster6->data.push_back(Point(1.0, -7.0));
	// 	cluster6->data.push_back(Point(3.0, -8.0));
	// 	cluster6->data.push_back(Point(-2.0, -9.0));
	// 	// (-3, -11) left out so the bounding box should change

	// 	Node *root = new Node();
	// 	cluster6->parent = root;
	// 	root->boundingBoxes.push_back(Rectangle(-3.0, -10.0, 3.0, 6.0));
	// 	root->children.push_back(cluster6);

	// 	// Condense the tree
	// 	cluster6->condenseTree();

	// 	// Test the condensing
	// 	assert(root->parent == nullptr);
	// 	assert(root->boundingBoxes.size() == 1);
	// 	assert(root->children.size() == 1);
	// 	assert(root->boundingBoxes[0] == Rectangle(-2.0, -9.0, 3.0, -6.0));
	// 	assert(root->children[0] == cluster6);
	// 	assert(cluster6->parent == root);
	// 	assert(cluster6->boundingBoxes.size() == 0);
	// 	assert(cluster6->children.size() == 0);
	// 	assert(cluster6->data.size() == 6);

	// 	// Cleanup
	// 	delete cluster6;
	// 	delete root;

	// 	// Test where condensing is unconfined to a leaf != root
	// 	// Cluster 4, n = 7
	// 	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// 	// Organized into two nodes
	// 	Node *cluster4a = new Node();
	// 	cluster4a->data.push_back(Point(-10.0, -2.0));
	// 	cluster4a->data.push_back(Point(-12.0, -3.0));
	// 	cluster4a->data.push_back(Point(-11.0, -3.0));
	// 	cluster4a->data.push_back(Point(-10.0, -3.0));

	// 	Node *cluster4b = new Node();
	// 	cluster4b->data.push_back(Point(-9.0, -3.0));
	// 	cluster4b->data.push_back(Point(-7.0, -3.0));
	// 	// cluster4b->data.push_back(Point(-10.0, -5.0)); left out to precipitate condensing

	// 	Node *cluster4 = new Node();
	// 	cluster4a->parent = cluster4;
	// 	cluster4->boundingBoxes.push_back(cluster4a->boundingBox());
	// 	cluster4->children.push_back(cluster4a);
	// 	cluster4b->parent = cluster4;
	// 	cluster4->boundingBoxes.push_back(cluster4b->boundingBox());
	// 	cluster4->children.push_back(cluster4b);

	// 	// Cluster 5, n = 16
	// 	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// 	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// 	// (-14, -15), (-13, -15), (-12, -15)
	// 	// Organized into four nodes
	// 	Node *cluster5a = new Node();
	// 	cluster5a->data.push_back(Point(-14.5, -13.0));
	// 	cluster5a->data.push_back(Point(-14.0, -13.0));
	// 	cluster5a->data.push_back(Point(-13.5, -13.5));
	// 	cluster5a->data.push_back(Point(-15.0, -14.0));
	// 	Node *cluster5b = new Node();
	// 	cluster5b->data.push_back(Point(-14.0, -14.0));
	// 	cluster5b->data.push_back(Point(-13.0, -14.0));
	// 	cluster5b->data.push_back(Point(-12.0, -14.0));
	// 	cluster5b->data.push_back(Point(-13.5, -16.0));
	// 	Node *cluster5c = new Node();
	// 	cluster5c->data.push_back(Point(-15.0, -14.5));
	// 	cluster5c->data.push_back(Point(-14.0, -14.5));
	// 	cluster5c->data.push_back(Point(-12.5, -14.5));
	// 	cluster5c->data.push_back(Point(-13.5, -15.5));
	// 	Node *cluster5d = new Node();
	// 	cluster5d->data.push_back(Point(-15.0, -15.0));
	// 	cluster5d->data.push_back(Point(-14.0, -15.0));
	// 	cluster5d->data.push_back(Point(-13.0, -15.0));
	// 	cluster5d->data.push_back(Point(-12.0, -15.0));
	// 	Node *cluster5 = new Node();
	// 	cluster5a->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	// 	cluster5->children.push_back(cluster5a);
	// 	cluster5b->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	// 	cluster5->children.push_back(cluster5b);
	// 	cluster5c->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	// 	cluster5->children.push_back(cluster5c);
	// 	cluster5d->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	// 	cluster5->children.push_back(cluster5d);

	// 	// Root
	// 	root = new Node();
	// 	cluster4->parent = root;
	// 	root->boundingBoxes.push_back(cluster4->boundingBox());
	// 	root->children.push_back(cluster4);
	// 	cluster5->parent = root;
	// 	root->boundingBoxes.push_back(cluster5->boundingBox());
	// 	root->children.push_back(cluster5);

	// 	// Condense the tree
	// 	Node *newRoot = cluster4b->condenseTree();

	// 	// Test the condensing
	// 	assert(newRoot == root);
	// 	assert(root->boundingBoxes.size() == 2);
	// 	assert(root->children.size() == 2);
	// 	assert(root->children[0]->children.size() == 4);
	// 	assert(root->children[1]->children.size() == 2);
	// 	assert(root->children[1]->children[0]->data.size() == 2);
	// 	assert(root->children[1]->children[0]->data[0] == Point(-9.0, -3.0));
	// 	assert(root->children[1]->children[0]->data[1] == Point(-7.0, -3.0));

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;
	// }

	// void testSearch()
	// {
	// 	// Build the tree directly

	// 	// Cluster 1, n = 7
	// 	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
	// 	Node *cluster1a = new Node();
	// 	cluster1a->data.push_back(Point(-3.0, 16.0));
	// 	cluster1a->data.push_back(Point(-3.0, 15.0));
	// 	cluster1a->data.push_back(Point(-4.0, 13.0));

	// 	Node *cluster1b = new Node();
	// 	cluster1b->data.push_back(Point(-5.0, 12.0));
	// 	cluster1b->data.push_back(Point(-5.0, 15.0));
	// 	cluster1b->data.push_back(Point(-6.0, 14.0));
	// 	cluster1b->data.push_back(Point(-8.0, 16.0));

	// 	// Cluster 2, n = 8
	// 	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	// 	Node *cluster2a = new Node();
	// 	cluster2a->data.push_back(Point(-8.0, 10.0));
	// 	cluster2a->data.push_back(Point(-9.0, 10.0));
	// 	cluster2a->data.push_back(Point(-8.0, 9.0));
	// 	cluster2a->data.push_back(Point(-9.0, 9.0));
	// 	cluster2a->data.push_back(Point(-8.0, 8.0));

	// 	Node *cluster2b = new Node();
	// 	cluster2b->data.push_back(Point(-14.0, 8.0));
	// 	cluster2b->data.push_back(Point(-10.0, 8.0));
	// 	cluster2b->data.push_back(Point(-9.0, 7.0));

	// 	// Cluster 3, n = 9
	// 	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// 	Node *cluster3a = new Node();
	// 	cluster3a->data.push_back(Point(-3.0, 4.0));
	// 	cluster3a->data.push_back(Point(-3.0, 0.0));
	// 	cluster3a->data.push_back(Point(-2.0, 4.0));
	// 	cluster3a->data.push_back(Point(-1.0, 3.0));
	// 	cluster3a->data.push_back(Point(-1.0, 1.0));

	// 	Node *cluster3b = new Node();
	// 	cluster3b->data.push_back(Point(-5.0, 4.0));
	// 	cluster3b->data.push_back(Point(-4.0, 3.0));
	// 	cluster3b->data.push_back(Point(-4.0, 1.0));
	// 	cluster3b->data.push_back(Point(-6.0, 2.0));

	// 	// High level nodes
	// 	Node *left = new Node();
	// 	cluster1a->parent = left;
	// 	left->boundingBoxes.push_back(cluster1a->boundingBox());
	// 	left->children.push_back(cluster1a);
	// 	cluster1b->parent = left;
	// 	left->boundingBoxes.push_back(cluster1b->boundingBox());
	// 	left->children.push_back(cluster1b);
	// 	cluster2a->parent = left;
	// 	left->boundingBoxes.push_back(cluster2a->boundingBox());
	// 	left->children.push_back(cluster2a);
	// 	cluster2b->parent = left;
	// 	left->boundingBoxes.push_back(cluster2b->boundingBox());
	// 	left->children.push_back(cluster2b);

	// 	Node *right = new Node();
	// 	cluster3a->parent = right;
	// 	right->boundingBoxes.push_back(cluster3a->boundingBox());
	// 	right->children.push_back(cluster3a);
	// 	cluster3b->parent = right;
	// 	right->boundingBoxes.push_back(cluster3b->boundingBox());
	// 	right->children.push_back(cluster3b);

	// 	Node *root = new Node();
	// 	left->parent = root;
	// 	root->boundingBoxes.push_back(left->boundingBox());
	// 	root->children.push_back(left);
	// 	right->parent = root;
	// 	root->boundingBoxes.push_back(right->boundingBox());
	// 	root->children.push_back(right);

	// 	// Test search

	// 	// Test set one
	// 	Rectangle sr1 = Rectangle(-9.0, 9.5, -5.0, 12.5);
	// 	std::vector<Point> v1 = root->search(sr1);
	// 	assert(v1.size() == 3);
	// 	assert(v1[0] == Point(-8.0, 10.0));
	// 	assert(v1[1] == Point(-9.0, 10.0));
	// 	assert(v1[2] == Point(-5.0, 12.0));

	// 	// Test set two
	// 	Rectangle sr2 = Rectangle(-8.0, 4.0, -5.0, 8.0);
	// 	std::vector<Point> v2 = root->search(sr2);
	// 	assert(v2.size() == 2);
	// 	assert(v2[0] == Point(-5.0, 4.0));
	// 	assert(v2[1] == Point(-8.0, 8.0));

	// 	// Test set three
	// 	Rectangle sr3 = Rectangle(-8.0, 0.0, -4.0, 16.0);
	// 	std::vector<Point> v3 = root->search(sr3);
	// 	assert(v3.size() == 12);
	// 	assert(v3[0] == Point(-5.0, 4.0));
	// 	assert(v3[1] == Point(-4.0, 3.0));
	// 	assert(v3[2] == Point(-4.0, 1.0));
	// 	assert(v3[3] == Point(-6.0, 2.0));
	// 	assert(v3[4] == Point(-8.0, 10.0));
	// 	assert(v3[5] == Point(-8.0, 9.0));
	// 	assert(v3[6] == Point(-8.0, 8.0));
	// 	assert(v3[7] == Point(-5.0, 12.0));
	// 	assert(v3[8] == Point(-5.0, 15.0));
	// 	assert(v3[9] == Point(-6.0, 14.0));
	// 	assert(v3[10] == Point(-8.0, 16.0));
	// 	assert(v3[11] == Point(-4.0, 13.0));

	// 	// Test set four
	// 	Rectangle sr4 = Rectangle(2.0, -4.0, 4.0, -2.0);
	// 	std::vector<Point> v4 = root->search(sr4);
	// 	assert(v4.size() == 0);

	// 	// Test set five
	// 	Rectangle sr5 = Rectangle(-3.5, 1.0, -1.5, 3.0);
	// 	std::vector<Point> v5 = root->search(sr5);
	// 	assert(v5.size() == 0);

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;
	// }

	// void testInsert()
	// {
	// 	// Setup the tree
	// 	Node *root = new Node();

	// 	// Cluster 2, n = 8
	// 	// (-14, 8), (-10, 8), (-9, 10), (-9, 9), (-8, 10), (-9, 7), (-8, 8), (-8, 9)
	// 	root = root->insert(Point(-14.0, 8.0));
	// 	root = root->insert(Point(-10.0, 8.0));
	// 	root = root->insert(Point(-9.0, 10.0));
	// 	root = root->insert(Point(-9.0, 9.0));
	// 	root = root->insert(Point(-8.0, 10.0));
	// 	root = root->insert(Point(-9.0, 7.0));
	// 	root = root->insert(Point(-8.0, 8.0));
	// 	root = root->insert(Point(-8.0, 9.0));

	// 	// Test set one
	// 	assert(root->boundingBoxes.size() == 2);
	// 	assert(root->children.size() == 2);
	// 	assert(root->children[0]->parent == root);
	// 	assert(root->children[0]->boundingBoxes.size() == 0);
	// 	assert(root->children[0]->children.size() == 0);
	// 	assert(root->children[0]->data.size() == 3);
	// 	assert(root->children[1]->parent == root);
	// 	assert(root->children[1]->boundingBoxes.size() == 0);
	// 	assert(root->children[1]->children.size() == 0);
	// 	assert(root->children[1]->data.size() == 5);

	// 	// Cluster 1, n = 7
	// 	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12)
	// 	root = root->insert(Point(-8.0, 16.0));
	// 	root = root->insert(Point(-3.0, 16.0));
	// 	root = root->insert(Point(-5.0, 15.0));
	// 	root = root->insert(Point(-3.0, 15.0));
	// 	root = root->insert(Point(-6.0, 14.0));
	// 	root = root->insert(Point(-4.0, 13.0));
	// 	root = root->insert(Point(-5.0, 12.0));

	// 	// Test set two
	// 	assert(root->boundingBoxes.size() == 2);
	// 	assert(root->children.size() == 2);
	// 	assert(root->children[0]->parent == root);
	// 	assert(root->children[0]->boundingBoxes.size() == 3);
	// 	assert(root->children[0]->children.size() == 3);
	// 	assert(root->children[0]->data.size() == 0);
	// 	assert(root->children[1]->parent == root);
	// 	assert(root->children[1]->boundingBoxes.size() == 3);
	// 	assert(root->children[1]->children.size() == 3);
	// 	assert(root->children[1]->data.size() == 0);

	// 	// Cluster 4, n = 7
	// 	// (-10, -2), (-12, -3), (-11, -3), (-10, -3), (-9, -3), (-7, -3), (-10, -5)
	// 	root = root->insert(Point(-10.0, -2.0));
	// 	root = root->insert(Point(-12.0, -3.0));
	// 	root = root->insert(Point(-11.0, -3.0));
	// 	root = root->insert(Point(-10.0, -3.0));
	// 	root = root->insert(Point(-10.0, -3.0));
	// 	root = root->insert(Point(-9.0, -3.0));
	// 	root = root->insert(Point(-7.0, -3.0));
	// 	root = root->insert(Point(-10.0, -5.0));

	// 	// Test set three
	// 	assert(root->boundingBoxes.size() == 2);
	// 	assert(root->children.size() == 2);
	// 	assert(root->children[0]->parent == root);
	// 	assert(root->children[0]->boundingBoxes.size() == 5);
	// 	assert(root->children[0]->children.size() == 5);
	// 	assert(root->children[0]->data.size() == 0);
	// 	assert(root->children[1]->parent == root);
	// 	assert(root->children[1]->boundingBoxes.size() == 3);
	// 	assert(root->children[1]->children.size() == 3);
	// 	assert(root->children[1]->data.size() == 0);

	// 	// Cluster 3, n = 9
	// 	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// 	root = root->insert(Point(-5.0, 4.0));
	// 	root = root->insert(Point(-3.0, 4.0));
	// 	root = root->insert(Point(-2.0, 4.0));
	// 	root = root->insert(Point(-4.0, 3.0));
	// 	root = root->insert(Point(-1.0, 3.0));
	// 	root = root->insert(Point(-6.0, 2.0));
	// 	root = root->insert(Point(-4.0, 1.0));
	// 	root = root->insert(Point(-3.0, 0.0));
	// 	root = root->insert(Point(-1.0, 1.0));

	// 	// Test set four
	// 	assert(root->boundingBoxes.size() == 3);
	// 	assert(root->children.size() == 3);
	// 	assert(root->children[0]->parent == root);
	// 	assert(root->children[0]->boundingBoxes.size() == 5);
	// 	assert(root->children[0]->children.size() == 5);
	// 	assert(root->children[0]->data.size() == 0);
	// 	assert(root->children[1]->parent == root);
	// 	assert(root->children[1]->boundingBoxes.size() == 3);
	// 	assert(root->children[1]->children.size() == 3);
	// 	assert(root->children[1]->data.size() == 0);
	// 	assert(root->children[2]->parent == root);
	// 	assert(root->children[2]->boundingBoxes.size() == 3);
	// 	assert(root->children[2]->children.size() == 3);
	// 	assert(root->children[2]->data.size() == 0);

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;
	// }

	// void testRemove()
	// {
	// 	// Cluster 5, n = 16
	// 	// (-14.5, -13), (-14, -13), (-13.5, -13.5), (-15, -14), (-14, -14), (-13, -14), (-12, -14),
	// 	// (-13.5, -16), (-15, -14.5), (-14, -14.5), (-12.5, -14.5), (-13.5, -15.5), (-15, -15),
	// 	// (-14, -15), (-13, -15), (-12, -15)
	// 	// Organized into four nodes
	// 	Node *cluster5a = new Node();
	// 	cluster5a->data.push_back(Point(-14.5, -13.0));
	// 	cluster5a->data.push_back(Point(-14.0, -13.0));
	// 	cluster5a->data.push_back(Point(-13.5, -13.5));
	// 	cluster5a->data.push_back(Point(-15.0, -14.0));

	// 	Node *cluster5b = new Node();
	// 	cluster5b->data.push_back(Point(-14.0, -14.0));
	// 	cluster5b->data.push_back(Point(-13.0, -14.0));
	// 	cluster5b->data.push_back(Point(-12.0, -14.0));
	// 	cluster5b->data.push_back(Point(-13.5, -16.0));

	// 	Node *cluster5c = new Node();
	// 	cluster5c->data.push_back(Point(-15.0, -14.5));
	// 	cluster5c->data.push_back(Point(-14.0, -14.5));
	// 	cluster5c->data.push_back(Point(-12.5, -14.5));
	// 	cluster5c->data.push_back(Point(-13.5, -15.5));

	// 	Node *cluster5d = new Node();
	// 	cluster5d->data.push_back(Point(-15.0, -15.0));
	// 	cluster5d->data.push_back(Point(-14.0, -15.0));
	// 	cluster5d->data.push_back(Point(-13.0, -15.0));
	// 	cluster5d->data.push_back(Point(-12.0, -15.0));

	// 	Node *cluster5 = new Node();
	// 	cluster5a->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5a->boundingBox());
	// 	cluster5->children.push_back(cluster5a);
	// 	cluster5b->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5b->boundingBox());
	// 	cluster5->children.push_back(cluster5b);
	// 	cluster5c->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5c->boundingBox());
	// 	cluster5->children.push_back(cluster5c);
	// 	cluster5c->parent = cluster5;
	// 	cluster5->boundingBoxes.push_back(cluster5d->boundingBox());
	// 	cluster5->children.push_back(cluster5d);

	// 	// Cluster 3, n = 9
	// 	// (-5, 4), (-3, 4), (-2, 4), (-4, 3), (-1, 3), (-6, 2), (-4, 1), (-3, 0), (-1, 1)
	// 	Node *cluster3a = new Node();
	// 	cluster3a->data.push_back(Point(-5.0, 4.0));
	// 	cluster3a->data.push_back(Point(-3.0, 4.0));
	// 	cluster3a->data.push_back(Point(-2.0, 4.0));

	// 	Node *cluster3b = new Node();
	// 	cluster3b->data.push_back(Point(-4.0, 1.0));
	// 	cluster3b->data.push_back(Point(-3.0, 0.0));
	// 	cluster3b->data.push_back(Point(-1.0, 1.0));

	// 	Node *cluster3c = new Node();
	// 	cluster3c->data.push_back(Point(-4.0, 3.0));
	// 	cluster3c->data.push_back(Point(-1.0, 3.0));
	// 	cluster3c->data.push_back(Point(-6.0, 2.0));

	// 	Node *cluster3 = new Node();
	// 	cluster3a->parent = cluster3;
	// 	cluster3->boundingBoxes.push_back(cluster3a->boundingBox());
	// 	cluster3->children.push_back(cluster3a);
	// 	cluster3b->parent = cluster3;
	// 	cluster3->boundingBoxes.push_back(cluster3b->boundingBox());
	// 	cluster3->children.push_back(cluster3b);
	// 	cluster3c->parent = cluster3;
	// 	cluster3->boundingBoxes.push_back(cluster3c->boundingBox());
	// 	cluster3->children.push_back(cluster3c);

	// 	// Cluster 1, n = 7
	// 	// (-8, 16), (-3, 16), (-5, 15), (-3, 15), (-6, 14), (-4, 13), (-5, 12), + (-4.5, 15.5), (-2.0, 13.5)
	// 	Node *cluster1a = new Node();
	// 	cluster1a->data.push_back(Point(-3.0, 16.0));
	// 	cluster1a->data.push_back(Point(-3.0, 15.0));
	// 	cluster1a->data.push_back(Point(-4.0, 13.0));

	// 	Node *cluster1b = new Node();
	// 	cluster1b->data.push_back(Point(-5.0, 12.0));
	// 	cluster1b->data.push_back(Point(-5.0, 15.0));
	// 	cluster1b->data.push_back(Point(-6.0, 14.0));

	// 	Node *cluster1c = new Node();
	// 	cluster1c->data.push_back(Point(-8.0, 16.0));
	// 	cluster1c->data.push_back(Point(-4.5, 15.5));
	// 	cluster1c->data.push_back(Point(-2.0, 13.5));

	// 	Node *cluster1 = new Node();
	// 	cluster1a->parent = cluster1;
	// 	cluster1->boundingBoxes.push_back(cluster1a->boundingBox());
	// 	cluster1->children.push_back(cluster1a);
	// 	cluster1b->parent = cluster1;
	// 	cluster1->boundingBoxes.push_back(cluster1b->boundingBox());
	// 	cluster1->children.push_back(cluster1b);
	// 	cluster1c->parent = cluster1;
	// 	cluster1->boundingBoxes.push_back(cluster1c->boundingBox());
	// 	cluster1->children.push_back(cluster1c);

	// 	// Root
	// 	Node *root = new Node();
	// 	cluster3->parent = root;
	// 	root->boundingBoxes.push_back(cluster3->boundingBox());
	// 	root->children.push_back(cluster3);
	// 	cluster5->parent = root;
	// 	root->boundingBoxes.push_back(cluster5->boundingBox());
	// 	root->children.push_back(cluster5);
	// 	cluster1->parent = root;
	// 	root->boundingBoxes.push_back(cluster1->boundingBox());
	// 	root->children.push_back(cluster1);

	// 	assert(root->boundingBoxes.size() == 3);

	// 	// Remove an element, no other changes in tree
	// 	root->remove(Point(-12.0, -15.0));

	// 	// Test the removal
	// 	assert(cluster5d->data.size() == 3);
	// 	assert(cluster5d->data[0] == Point(-15.0, -15.0));
	// 	assert(cluster5d->data[1] == Point(-14.0, -15.0));
	// 	assert(cluster5d->data[2] == Point(-13.0, -15.0));
	// 	assert(cluster5->parent == root);
	// 	assert(cluster5->boundingBoxes.size() == 4);
	// 	assert(cluster5->children.size() == 4);
	// 	assert(root->parent == nullptr);
	// 	assert(root->boundingBoxes.size() == 3);
	// 	assert(root->children.size() == 3);

	// 	// Remove an element, parent is altered
	// 	root->remove(Point(-6.0, 2.0));

	// 	// Test the removal
	// 	assert(root->boundingBoxes.size() == 2);
	// 	assert(root->children.size() == 2);
	// 	assert(root->children[0]->parent == root);
	// 	assert(root->children[0]->boundingBoxes.size() == 4);
	// 	assert(root->children[0]->children.size() == 4);
	// 	assert(root->children[1]->parent == root);
	// 	assert(root->children[1]->boundingBoxes.size() == 5);
	// 	assert(root->children[1]->children.size() == 5);

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;

	// 	// Remove an element, tree shrinks

	// 	// Cluster 6, n = 7
	// 	// (-2, -6), (2, -6), (-1, -7), (1, -7), (3, -8), (-2, -9), (-3, -11)
	// 	Node *cluster6a = new Node();
	// 	cluster6a->data.push_back(Point(-2.0, -6.0));
	// 	cluster6a->data.push_back(Point(2.0, -6.0));
	// 	cluster6a->data.push_back(Point(-1.0, -7.0));

	// 	Node *cluster6b = new Node();
	// 	cluster6b->data.push_back(Point(1.0, -7.0));
	// 	cluster6b->data.push_back(Point(3.0, -8.0));
	// 	cluster6b->data.push_back(Point(-2.0, -9.0));
	// 	// (-3.0, -11.0) held out so we get a shrinking root

	// 	// Root
	// 	root = new Node();
	// 	cluster6a->parent = root;
	// 	root->boundingBoxes.push_back(cluster6a->boundingBox());
	// 	root->children.push_back(cluster6a);
	// 	cluster6b->parent = root;
	// 	root->boundingBoxes.push_back(cluster6b->boundingBox());
	// 	root->children.push_back(cluster6b);

	// 	// Remove an element, the tree should shrink and cluster6a should be the new root
	// 	root = root->remove(Point(3.0, -8.0));

	// 	// Test the removal
	// 	assert(root == cluster6a);
	// 	assert(root->boundingBoxes.size() == 0);
	// 	assert(root->children.size() == 0);
	// 	assert(root->data.size() == 5);

	// 	// Cleanup
	// 	root->deleteSubtrees();
	// 	delete root;
	// }
}
