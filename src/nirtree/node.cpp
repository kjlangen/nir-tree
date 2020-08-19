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
			// validate(nullptr, 0);
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
			// validate(nullptr, 0);
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
		// PencilPrinter p;

		for (;;)
		{
			// CL2 [Leaf check]
			if (node->children.size() == 0)
			{
				return node;
			}
			else
			{
				// assert(node->parent == nullptr || node->parent->boundingBoxes[prevSmallestExpansionIndex].containsPoint(givenPoint));
				// node->print();
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

				// std::cout << "Chose index " << smallestExpansionIndex << std::endl;

				// std::cout << "A: node->boundingBoxes[" << smallestExpansionIndex << "].size() = " << node->boundingBoxes[smallestExpansionIndex].basicRectangles.size() << std::endl;
				// std::cout << "Chose ";
				// node->boundingBoxes[smallestExpansionIndex].print();
				// p.printToPencil(node->boundingBoxes);
				// assert(node->boundingBoxes[smallestExpansionIndex].unique());
				// assert(node->boundingBoxes[smallestExpansionIndex].contiguous());
				// std::cout << "Chose successfully" << std::endl;

				// CL3.1 Expand the chosen bounding polygon to cover the new point and keep it
				// within the area defined by our bounding polygon. The root has no bounding polygon
				if (node->parent == nullptr)
				{
					node->boundingBoxes[smallestExpansionIndex].expand(givenPoint);
					// assert(node->boundingBoxes[smallestExpansionIndex].unique());
				}
				else
				{
					node->boundingBoxes[smallestExpansionIndex].expand(givenPoint, node->parent->boundingBoxes[prevSmallestExpansionIndex]);
					// assert(node->boundingBoxes[smallestExpansionIndex].unique());
				}

				// std::cout << "Expanded to ";
				// std::cout << "B: node->boundingBoxes[" << smallestExpansionIndex << "].size() = " << node->boundingBoxes[smallestExpansionIndex].basicRectangles.size() << std::endl;
				// node->boundingBoxes[smallestExpansionIndex].print();
				// p.printToPencil(node->boundingBoxes);
				// assert(node->boundingBoxes[smallestExpansionIndex].unique());
				// assert(node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint));
				// assert(node->boundingBoxes[smallestExpansionIndex].contiguous());
				// std::cout << "Expanded successfully" << std::endl;

				// CL3.2 Trim back the chosen bounding polygon so it plays nice with the other
				// children
				for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
				{
					if (i == smallestExpansionIndex)
					{
						continue;
					}

					// std::cout << "Pre chooseLeaf increaseRes" << std::endl;
					node->boundingBoxes[smallestExpansionIndex].increaseResolution(node->boundingBoxes[i]);
					// std::cout << "Post chooseLeaf increaseRes" << std::endl;
					// if (!node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint))
					// {
					// 	std::cout << "i = " << i << std::endl;
					// 	std::cout << "node->boundingBoxes[i] = "; node->boundingBoxes[i].print();
					// 	std::cout << "node->boundingBoxes[smallestExpansionIndex] = "; node->boundingBoxes[smallestExpansionIndex].print();
					// }
					// assert(node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint));
				}

				// std::cout << "Trimmed back to ";
				// std::cout << "C: node->boundingBoxes[" << smallestExpansionIndex << "].size() = " << node->boundingBoxes[smallestExpansionIndex].basicRectangles.size() << std::endl;
				// node->boundingBoxes[smallestExpansionIndex].print();
				// p.printToPencil(node->boundingBoxes);
				// assert(node->boundingBoxes[smallestExpansionIndex].unique());
				// assert(node->boundingBoxes[smallestExpansionIndex].containsPoint(givenPoint));
				// assert(node->boundingBoxes[smallestExpansionIndex].contiguous());
				// std::cout << "Trimmed successfully" << std::endl;

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
			// std::cout << "Current polygon "; temp.print(); std::cout << std::endl;
			// std::cout << "Cutting out "; boundingBoxes[i].print();
			// std::cout << "Pre decompose increaseRes" << std::endl;
			temp.increaseResolution(boundingBoxes[i]);
			// std::cout << "Post decompose increaseRes" << std::endl;
			// std::cout << std::endl << "Result "; temp.print(); std::cout << std::endl;
		}

		return temp.basicRectangles;
	}

	Node::SplitResult Node::splitNode(SplitResult &lowerSplit)
	{
		// std::cout << "Split starting" << std::endl;
		// Setup the split
		Node *left = new Node(minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(minBranchFactor, maxBranchFactor, parent);

		// Insert the lower split as if we had capacity
		// lowerSplit.leftPolygon.print();
		// lowerSplit.rightPolygon.print();
		boundingBoxes.push_back(lowerSplit.leftPolygon);
		children.push_back(lowerSplit.left);
		boundingBoxes.push_back(lowerSplit.rightPolygon);
		children.push_back(lowerSplit.right);

		const unsigned childrenSize = children.size();
		unsigned polygonIndex;
		IsotheticPolygon unsplitPolygon;

		// std::cout << "Checking edge case" << std::endl;
		if (parent == nullptr)
		{
			// std::cout << "CHKPT13" << std::endl;
			assert(boundingBoxes.size() != 0);
			Rectangle r = boundingBoxes[0].boundingBox();
			for (unsigned i = 0; i < children.size(); ++i)
			{
				r.expand(boundingBoxes[i].boundingBox());
			}
			unsplitPolygon = IsotheticPolygon(r);
			// std::cout << "unsplitPolygon: "; unsplitPolygon.print();
			// std::cout << "CHKPT15" << std::endl;
		}
		else
		{
			for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}

			unsplitPolygon = parent->boundingBoxes[polygonIndex];
		}

		// Decompose polygon
		// std::cout << "Decomposing node" << std::endl;
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		std::vector<Rectangle> decomposed = decomposeNode(unsplitPolygon);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		// std::cout << "Decompose time = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

		const unsigned decomposedSize = decomposed.size();
		const unsigned totalSize = childrenSize + decomposedSize;
		const unsigned root = totalSize / 2;

		// PencilPrinter p1;
		// p1.printToPencil(decomposed);

		begin = std::chrono::high_resolution_clock::now();
		// Build graph
		// std::cout << "Builidng graph" << std::endl;
		// std::cout << "totalSize = " << totalSize << std::endl;
		// std::vector<IsotheticPolygon> u(boundingBoxes.size() + decomposed.size());
		// u.insert(u.end(), boundingBoxes.begin(), boundingBoxes.end());
		// for (auto decompositionRectangle : decomposed)
		// {
		// 	u.push_back(IsotheticPolygon(decompositionRectangle));
		// }
		// Graph graph(u);
		bool graph[totalSize][totalSize];
		// std::cout << "CHKPT1" << std::endl;
		std::memset(graph, false, totalSize * totalSize);
		// std::cout << "CHKPT2" << std::endl;
		for (unsigned i = 0; i < childrenSize; ++i)
		{
			// Children X Children
			// std::cout << "Children X Children" << std::endl;
			for (unsigned j = 0; j < i && j < childrenSize; ++j)
			{
				if (boundingBoxes[i].intersectsRectangle(boundingBoxes[j]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
			// std::cout << "Done" << std::endl;

			// Children X Decomposed Polygon
			// std::cout << "Children X Decomposed" << std::endl;
			for (unsigned j = childrenSize; j < totalSize; ++j)
			{
				if (boundingBoxes[i].intersectsRectangle(decomposed[j - childrenSize]))
				{
					graph[i][j] = true;
					graph[j][i] = true;
				}
			}
			// std::cout << "Done" << std::endl;
		}
		// std::cout << "Decomposed polygon X Decomposed polygon" << std::endl;
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
		end = std::chrono::high_resolution_clock::now();
		// std::cout << "Graph build time = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;
		// std::cout << "Done" << std::endl;

		// std::cout << "Graph" << std::endl;
		// for (unsigned i = 0; i < totalSize; ++i)
		// {
		// 	std::cout << "|";
		// 	for (unsigned j = 0; j < totalSize; ++j)
		// 	{
		// 		std::cout << " " << graph[i][j];
		// 	}
		// 	std::cout << "| " << i << std::endl;
		// }
		// std::cout << std::endl;

		// Build tree
		// std::cout << "Building tree" << std::endl;
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

		// std::cout << "Tree:" << std::endl;
		// for (unsigned i = 0; i < totalSize; ++i)
		// {
		// 	std::cout << "|";
		// 	for (unsigned j = 0; j < totalSize; ++j)
		// 	{
		// 		std::cout << " " << tree[i][j];
		// 	}
		// 	std::cout << "| " << i << std::endl;
		// }
		// std::cout << std::endl;

		// Push weights up the tree
		// std::cout << "Pushing weights up the tree" << std::endl;
		// std::cout << "parentStack.size(): " << parentStack.size() << std::endl;
		// std::cout << "WeightStack.size(): " << weightStack.size() << std::endl;
		// std::cout << "totalSize: " << totalSize << std::endl;
		for (;parentStack.size() && weightStack.size();)
		{
			weights[parentStack.top()] += weights[weightStack.top()];
			weightStack.pop();
			parentStack.pop();
		}

		// Find a seperator
		// std::cout << "Finding a separator" << std::endl;
		unsigned delta = std::numeric_limits<unsigned>::max();
		unsigned subtreeRoot, subtreeParent;

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
		}

		// Split along separator
		// std::cout << "Splitting along separator" << std::endl;
		bool switchboard[totalSize];
		tree[subtreeRoot][subtreeParent] = tree[subtreeParent][subtreeRoot] = false;
		// std::cout << "CHKPT15" << std::endl;

		std::memset(switchboard, false, totalSize);
		std::memset(explored, false, totalSize);
		// std::cout << "CHKPT16" << std::endl;

		switchboard[subtreeRoot] = true;
		// std::cout << "CHKPT17" << std::endl;

		explorationQ.push(subtreeRoot);
		// std::cout << "CHKPT18" << std::endl;
		for (;explorationQ.size();)
		{
			currentVertex = explorationQ.front();
			explorationQ.pop();
			// std::cout << "CHKPT19" << std::endl;

			if (explored[currentVertex])
			{
				continue;
			}
			// std::cout << "CHKPT20" << std::endl;

			for (unsigned neighbouringVertex = 0; neighbouringVertex < totalSize; ++neighbouringVertex)
			{
				// std::cout << "CHKPT21" << std::endl;
				if (tree[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
				{
					// std::cout << "CHKPT22" << std::endl;
					switchboard[neighbouringVertex] = true;
					explorationQ.push(neighbouringVertex);
					// std::cout << "CHKPT23" << std::endl;
				}
				// std::cout << "CHKPT24" << std::endl;
			}
			explored[currentVertex] = true;
			// std::cout << "CHKPT25" << std::endl;
		}
		end = std::chrono::high_resolution_clock::now();
		// std::cout << "Graph time = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

		// std::cout << "Tree:" << std::endl;
		// for (unsigned i = 0; i < totalSize; ++i)
		// {
		// 	std::cout << "|";
		// 	for (unsigned j = 0; j < totalSize; ++j)
		// 	{
		// 		std::cout << " " << tree[i][j];
		// 	}
		// 	std::cout << "| " << i << std::endl;
		// }
		// std::cout << std::endl;

		begin = std::chrono::high_resolution_clock::now();
		// Recompose into two polygons by building polygons representing what we have now and then
		// reducing in size their representation
		// std::cout << "Recomposing polygons" << std::endl;
		// IsotheticPolygon currentNewPolygon;
		// IsotheticPolygon currentOldPolygon;
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

		// std::cout << "Decomposition union in two groups" << std::endl;
		// currentOldPolygon.print();
		// currentNewPolygon.print();

		// leftPolygon.basicRectangles.push_back(currentNewPolygon.boundingBox());
		// rightPolygon.basicRectangles.push_back(currentOldPolygon.boundingBox());

		// // std::cout << "L/R polygons start" << std::endl;
		// // leftPolygon.print();
		// // rightPolygon.print();

		// // Intersect with original polygon => No going outside original boundaries
		// std::cout << "CHKPT0" << std::endl;
		// assert(leftPolygon.infFree());
		// assert(rightPolygon.infFree());
		// std::cout << "CHKPT1" << std::endl;
		// leftPolygon.intersection(unsplitPolygon);
		// assert(leftPolygon.infFree());
		// rightPolygon.intersection(unsplitPolygon);
		// assert(rightPolygon.infFree());

		// // std::cout << "L/R polygons post intersection with original polygon" << std::endl;
		// // leftPolygon.print();
		// // rightPolygon.print();

		// std::cout << "Pre splitNode increaseRes" << std::endl;
		// // Remove from the new polygon the old polygon defined by the separator found earlier
		// leftPolygon.increaseResolution(currentOldPolygon);
		// // Remove from the old polygon the new polygon defined by the separator found earlier (use finalNewPolygon b/c it is smaller)
		// rightPolygon.increaseResolution(leftPolygon);
		// std::cout << "Post splitNode increaseRes" << std::endl;

		PencilPrinter pp;
		pp.printToPencil(leftPolygon.basicRectangles);
		leftPolygon.refine();
		pp.printToPencil(leftPolygon.basicRectangles);
		pp.printToPencil(rightPolygon.basicRectangles);
		rightPolygon.refine();
		pp.printToPencil(rightPolygon.basicRectangles);

		// Because we touch the polygon's rectangles directly we must leave them in a valid sorted state
		leftPolygon.sort(true);
		rightPolygon.sort(true);

		// std::cout << "L/R polygons post increase resolution" << std::endl;
		// leftPolygon.print();
		// rightPolygon.print();

		// Split children in two
		// std::cout << "Splitting children" << std::endl;
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
		end = std::chrono::high_resolution_clock::now();
		// std::cout << "Move time = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

		// std::cout << "Return" << std::endl;
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

	// TODO: Because we're using vectors and didn't exactly implement the original R-Tree rewriting this
	// with sets will necessitate rewriting the entire R-Tree with sets.
	// TODO: START Make sure isothetic polygon constituent rectangles are always sorted forever and for always from here down
	Node::SplitResult Node::splitNode(Point newData)
	{
		// Special case where the leaf to be split is the root
		if (parent == nullptr)
		{
			// std::cout << "Nullptr case" << std::endl;
			return splitNodeSpecialCase(newData);
		}

		unsigned polygonIndex;
		for(polygonIndex = 0; parent->children[polygonIndex] != this; ++polygonIndex) {}
		// parent->boundingBoxes[polygonIndex].print();
		std::vector<Rectangle> &basics = parent->boundingBoxes[polygonIndex].basicRectangles;
		const unsigned basicsSize = basics.size();

		// Special case where the leaf is bounded by a simple rectangle
		if (basicsSize == 1)
		{
			// std::cout << "basicsSize = 1 case" << std::endl;
			return splitNodeSpecialCase(newData);
		}

		Node *left = new Node(minBranchFactor, maxBranchFactor, parent);
		Node *right = new Node(minBranchFactor, maxBranchFactor, parent);
		data.push_back(newData);
		const unsigned dataSize = data.size();
		const unsigned root = basicsSize / 2;

		// PencilPrinter p;
		// p.printToPencil(basics);
		
		// Build graph
		// std::cout << "Building graph..." << std::endl;
		// Graph graph(basicsSize);
		// Graph graph(basics);
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
		// std::cout << "Base Graph" << std::endl;
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << i << " | ";
		// 	for (unsigned j = 0; j < basics.size(); ++j)
		// 	{
		// 		std::cout << graph[i][j] << " ";
		// 	}
		// 	std::cout << "| " << i << std::endl;
		// }
		// std::cout << std::endl;

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

		// Printing...
		// std::cout << "weights (cleared) = [";
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << " " << weights[i];
		// }
		// std::cout << " ]" << std::endl;

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

		// Printing...
		// std::cout << "Tree Graph" << std::endl;
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << i << " | ";
		// 	for (unsigned j = 0; j < basics.size(); ++j)
		// 	{
		// 		std::cout << tree[i][j] << " ";
		// 	}
		// 	std::cout << "| " << i << std::endl;
		// }
		// std::cout << std::endl;

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
		// std::cout << "basicsSize = " << basicsSize << std::endl;
		// std::cout << "parentStack.size() = " << parentStack.size() << std::endl;
		// assert(parentStack.size() == weightStack.size());
		// assert(basicsSize - 1 == parentStack.size());
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

		// Printing...
		// std::cout << "Separated Graph" << std::endl;
		// for (unsigned i = 0; i < basics.size(); ++i)
		// {
		// 	std::cout << i << " | ";
		// 	for (unsigned j = 0; j < basics.size(); ++j)
		// 	{
		// 		std::cout << tree[i][j] << " ";
		// 	}
		// 	std::cout << "| " << i << std::endl;
		// }
		// std::cout << std::endl;

		// Break the tree into two components
		// TODO: Optimize, fairly certain swapping could be made faster
		// std::cout << "Splitting and swapping..." << std::endl;


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
			// std::cout << "CHKPT5" << std::endl;
			if (node == nullptr)
			{
				// std::cout << "CHKPT6" << std::endl;
				// Stop if at the top of the tree
				return lowerSplit;
			}
			else if (node->children.size() + 2 <= node->maxBranchFactor)
			{
				// std::cout << "CHKPT7" << std::endl;
				// Add additional children gracefully
				node->boundingBoxes.push_back(lowerSplit.leftPolygon);
				node->children.push_back(lowerSplit.left);
				node->boundingBoxes.push_back(lowerSplit.rightPolygon);
				node->children.push_back(lowerSplit.right);
				// assert(node->boundingBoxes.size() <= node->maxBranchFactor);
				// assert(node->children.size() <= node->maxBranchFactor);
				// std::cout << "CHKPT8" << std::endl;

				// Stop there is no split on this level
				return {nullptr, IsotheticPolygon(), nullptr, IsotheticPolygon()};
			}
			else
			{
				// std::cout << "CHKPT9" << std::endl;
				// Split this node
				// std::cout << "Before routing split" << std::endl;
				// node->printTree();
				lowerSplit = node->splitNode(lowerSplit);
				// std::cout << "After routing split" << std::endl;
				// assert(lowerSplit.left->boundingBoxes.size() <= node->maxBranchFactor);
				// assert(lowerSplit.left->children.size() <= node->maxBranchFactor);
				// assert(lowerSplit.right->boundingBoxes.size() <= node->maxBranchFactor);
				// assert(lowerSplit.right->children.size() <= node->maxBranchFactor);

				// std::cout << "CHKPT10" << std::endl;
				// Remove the single old over-capacity node from the tree
				if (node->parent != nullptr)
				{
					// std::cout << "node->parent->printTree" << std::endl;
					// node->parent->printTree();
					// std::cout << "CHKPT11" << std::endl;
					node->parent->removeChild(node);
				}
				// std::cout << "Left ";
				// lowerSplit.leftPolygon.print();
				// lowerSplit.left->printTree();
				// std::cout << "Right ";
				// lowerSplit.rightPolygon.print();
				// lowerSplit.right->printTree();
				// std::cout << "CHKPT12" << std::endl;
				// Continue to move up the tree
				node = lowerSplit.left->parent;
			}
		}

		return lowerSplit;
	}

	// Always called on root, this = root
	Node *Node::insert(Point givenPoint)
	{
		// std::cout << "Inserting "; givenPoint.print(); std::cout << std::endl;
		// PencilPrinter p;
		// p.printToPencil(this);
		// I1 [Find position for new record]
		// std::cout << "Before the leaf choosing" << std::endl;
		// printTree();
		Node *leaf = chooseLeaf(givenPoint);
		// std::cout << "After the leaf choosing" << std::endl;
		// printTree();
		// validate(nullptr, 0);
		// std::cout << "I1 complete" << std::endl;
		// printTree();

		// I2 [Add record to leaf node]
		if (leaf->data.size() < leaf->maxBranchFactor)
		{
			// std::cout << "CHKPT0" << std::endl;
			leaf->data.push_back(givenPoint);
			// validate(nullptr, 0);
			// std::cout << "I2a complete" << std::endl;
		}
		else
		{
			// std::cout << "CHKPT1" << std::endl;
			SplitResult sr = leaf->splitNode(givenPoint);
			// validate(nullptr, 0);
			// std::cout << "I2b complete" << std::endl;

			if (sr.left->parent != nullptr)
			{
				// std::cout << "CHKPT2" << std::endl;
				sr.left->parent->removeChild(leaf);
				// validate(nullptr, 0);
				// std::cout << "CHKPT3" << std::endl;
				// I3 [Propogate changes upward]
				sr = sr.left->parent->adjustTree(sr);
				// validate(nullptr, 0);
				// std::cout << "I3 complete" << std::endl;
			}

			// validate(nullptr, 0);

			// I4 [Grow tree taller]
			if (sr.left != nullptr && sr.right != nullptr)
			{
				// std::cout << "CHKPT4" << std::endl;
				Node *newRoot = new Node(minBranchFactor, maxBranchFactor);

				sr.left->parent = newRoot;
				newRoot->boundingBoxes.push_back(sr.leftPolygon);
				newRoot->children.push_back(sr.left);
				// std::cout << "CHKPT5" << std::endl;
				sr.right->parent = newRoot;
				newRoot->boundingBoxes.push_back(sr.rightPolygon);
				newRoot->children.push_back(sr.right);
				// std::cout << "I4a complete" << std::endl;
				// newRoot->validate(nullptr, 0);
				return newRoot;
			}
		}

		// std::cout << "I4b complete" << std::endl;
		// validate(nullptr, 0);
		return this;
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
	Node *Node::condenseTree()
	{
		// CT1 [Initialize]
		Node *node = this;

		for (;node->parent != nullptr;)
		{
			// TODO: Shrink bounding box. How???

			if (node->boundingBoxes.size() == 0 && node->data.size() == 0)
			{
				// Remove ourselves from our parent
				node->parent->removeChild(node);

				// Prepare for garbage collection
				Node *garbage = node;

				// Move up a level before deleting ourselves
				node = node->parent;

				// Garbage collect current node
				delete garbage;
			}
			else
			{
				// TODO: Maybe add early stop since there are no more deletes to propagate?
				node = node->parent;
			}
		}

		return node;
	}

	// // Always called on root, this = root
	Node *Node::remove(Point givenPoint)
	{
		// std::cout << "Removing "; givenPoint.print(); std::cout << std::endl;
		// std::cout << "CHKPT0" << std::endl;
		// D1 [Find node containing record]
		Node *leaf = findLeaf(givenPoint);
		// validate(nullptr, 0);

		// std::cout << "CHKPT1" << std::endl;

		if (leaf == nullptr)
		{
			return nullptr;
		}

		// std::cout << "CHKPT2" << std::endl;

		// D2 [Delete record]
		// leaf->parent->updateBoundingBox(leaf, givenPoint);
		leaf->removeData(givenPoint);
		// validate(nullptr, 0);
		// std::cout << "CHKPT3" << std::endl;

		// D3 [Propagate changes]
		Node *root = leaf->condenseTree();
		// validate(nullptr, 0);
		// std::cout << "CHKPT4" << std::endl;

		// D4 [Shorten tree]
		if (root->children.size() == 1)
		{
			// std::cout << "CHKPT5" << std::endl;
			Node *newRoot = root->children[0];
			delete root;
			newRoot->parent = nullptr;
			// newRoot->validate(nullptr, 0);
			return newRoot;
		}
		else
		{
			// validate(nullptr, 0);
			// std::cout << "CHKPT6" << std::endl;
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

	void testPlayground()
	{
		// Setup
		std::vector<Rectangle> v;
		const float sizeF = 300.0;
		const unsigned sizeU = 300;

		for (float i = 0.0; i < sizeF; i += 1.0)
		{
			for (float j = 0.0; j < sizeF; j += 1.0)
			{
				v.push_back(Rectangle(j, i, j + 1.0, i + 1.0));
			}
		}

		// Build graph new way
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		Graph g1(v);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::cout << "New time = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

		// Build graph old way
		begin = std::chrono::high_resolution_clock::now();
		auto g0 = new std::multimap<unsigned, unsigned>();// bool[sizeU * sizeU];
		// std::memset(g0, false, sizeU * sizeU);
		// end = std::chrono::high_resolution_clock::now();
		// std::cout << "Allocate & clear 2.0 = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;
		for (unsigned i = 0; i < sizeU; ++i)
		{
			for (unsigned j = 0; j < i; ++j)
			{
				if (v[i].intersectsRectangle(v[j]))
				{
					g0->insert(std::pair<unsigned, unsigned>(i, j));
					// g0[i * sizeU + j] = true;
					// g0[j * sizeU + i] = true;
				}
			}
		}
		end = std::chrono::high_resolution_clock::now();
		std::cout << "Old time = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

		// Assert correctness
		// assert(memcmp(g0, g1[0], sizeU * sizeU));
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
		child2 = new Node(3, 3, routing);

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
