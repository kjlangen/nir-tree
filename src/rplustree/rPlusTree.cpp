#include <rplustree/rPlusTree.h>

namespace rplustree
{
	RPlusTree::RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor)
	{
		this->minBranchFactor = minBranchFactor;
		this->maxBranchFactor = maxBranchFactor;
		root = new Node();
	}

	RPlusTree::~RPlusTree()
	{
		std::stack<Node *> stack;
		stack.push(root);
		root = nullptr;
		Node *currentNode;

		while (!stack.empty())
		{
			currentNode = stack.top();
			stack.pop();
			for (auto &child : currentNode->children)
			{
				stack.push(child);
			}

			// Cleanup
			delete currentNode;
		}
	}

	bool RPlusTree::isEmpty()
	{
		return root->numChildren() == 0 && root->numDataEntries() == 0;
	}

	Node *RPlusTree::getRoot()
	{
		return root;
	}

	unsigned RPlusTree::height()
	{
		unsigned height = 0;
		Node *n = root;

		while (!n->isLeaf())
		{
			n = n->children[0];
			height++;
		}

		return height;
	}

	unsigned RPlusTree::numDataElements()
	{
		unsigned result = 0;
		Node *currentNode;

		std::stack<Node *> stack;
		stack.push(root);

		while (!stack.empty())
		{
			currentNode = stack.top();
			stack.pop();

			if (currentNode->isLeaf())
			{
				result += (unsigned) currentNode->numDataEntries();
			}
			else
			{
				for (auto &child : currentNode->children)
				{
					stack.push(child);
				}
			}
		}

		return result;
	}

	bool RPlusTree::exists(Point requestedPoint)
	{
		return findLeaf(requestedPoint) != nullptr;
	}

	void RPlusTree::adjustTree(Node *n, Node *nn)
	{
		if (n->isRoot())
		{
			if (nn != nullptr)
			{
				root = new Node();
				root->children.push_back(n);
				n->parent = root;
				root->children.push_back(nn);
				nn->parent = root;
			}

			root->tighten();

			return;
		}

		n->tighten();

		if (nn != nullptr)
		{
			nn->tighten();
			nn->parent->children.push_back(nn);

			// Propagate split upwards if neccessary
			if (n->parent->numChildren() > maxBranchFactor)
			{
				Partition splits = splitNode(n->parent);
				adjustTree(splits.first, splits.second);
			}
		}

		if (!n->isRoot())
		{
			adjustTree(n->parent, nullptr);
		}
	}

	Node *RPlusTree::chooseLeaf(Node *node, Point &givenPoint)
	{
		// Found leaf
		if (node->isLeaf())
		{
			return node;
		}

		// Variables for overlap and no overlap cases
		Node *chosenChildNoOverlap = nullptr;
		float smallestAreaNoOverlap = std::numeric_limits<float>::max();
		Node *chosenChildOverlap = node->children[0];
		float smallestAreaOverlap = node->children[0]->boundingBox.computeExpansionArea(givenPoint);

		// Iterate through child nodes
		for (auto child : node->children)
		{
			// Containment case, simple break
			if (child->boundingBox.containsPoint(givenPoint))
			{
				chosenChildNoOverlap = child;
				break;
			}

			// Check for future overlap
			bool noOverlap = true;
			Rectangle newBoundingBox = child->boundingBox;
			newBoundingBox.expand(givenPoint);
			for (auto other : node->children)
			{
				if (child != other && newBoundingBox.computeIntersectionArea(other->boundingBox) != 0.0)
				{
					noOverlap = false;
					break;
				}
			}

			// Find smallest expansion area
			float testExpansionArea = child->boundingBox.computeExpansionArea(givenPoint);
			if (noOverlap && testExpansionArea < smallestAreaNoOverlap)
			{
				smallestAreaNoOverlap = testExpansionArea;
				chosenChildNoOverlap = child;
			}
			else if (testExpansionArea < smallestAreaOverlap)
			{
				smallestAreaOverlap = testExpansionArea;
				chosenChildOverlap = child;
			}
		}

		// Recurse
		if (chosenChildNoOverlap != nullptr)
		{
			return chooseLeaf(chosenChildNoOverlap, givenPoint);
		}
		else
		{
			return chooseLeaf(chosenChildOverlap, givenPoint);
		}
	}

	Node *RPlusTree::chooseLeaf(Node *node, Rectangle &givenRectangle)
	{
		// Found leaf
		if (node->isLeaf())
		{
			return node;
		}

		// Variables for overlap and no overlap cases
		Node *chosenChildNoOverlap = nullptr;
		float smallestAreaNoOverlap = std::numeric_limits<float>::max();
		Node *chosenChildOverlap = node->children[0];
		float smallestAreaOverlap = node->children[0]->boundingBox.computeExpansionArea(givenRectangle);

		// Iterate through child nodes
		for (auto child : node->children)
		{
			// Check for future overlap
			bool noOverlap = true;
			Rectangle newBoundingBox = child->boundingBox;
			newBoundingBox.expand(givenRectangle);
			for (auto other : node->children)
			{
				if (child != other && newBoundingBox.computeIntersectionArea(other->boundingBox) != 0.0)
				{
					noOverlap = false;
					break;
				}
			}

			// Find smallest expansion area
			float testExpansionArea = child->boundingBox.computeExpansionArea(givenRectangle);
			if (noOverlap && testExpansionArea < smallestAreaNoOverlap)
			{
				smallestAreaNoOverlap = testExpansionArea;
				chosenChildNoOverlap = child;
			}
			else if (testExpansionArea < smallestAreaOverlap)
			{
				smallestAreaOverlap = testExpansionArea;
				chosenChildOverlap = child;
			}
		}

		// Recurse
		if (chosenChildNoOverlap != nullptr)
		{
			return chooseLeaf(chosenChildNoOverlap, givenRectangle);
		}
		else
		{
			return chooseLeaf(chosenChildOverlap, givenRectangle);
		}
	}

	Node *RPlusTree::findLeaf(Point requestedPoint)
	{
		std::stack<Node *> stack;
		stack.push(root);
		Node *currentNode;

		// Run DFS to find leaf node that contains requestedPoint
		while (!stack.empty())
		{
			currentNode = stack.top();
			stack.pop();

			if (currentNode->isLeaf())
			{
				auto iter = std::find(currentNode->data.begin(), currentNode->data.end(), requestedPoint);
				if (iter != currentNode->data.end())
				{
					return currentNode;
				}
			}
			else
			{
				for (auto &child : currentNode->children)
				{
					if (child->boundingBox.containsPoint(requestedPoint))
					{
						stack.push(child);
					}
				}
			}
		}

		return nullptr;
	}

	Cost RPlusTree::sweepData(std::vector<Point> &points, Orientation orientation)
	{
		std::vector<float> values;
		for (auto &p : points)
		{
			if (orientation == ALONG_X_AXIS)
			{
				values.push_back(p.x);
			}
			else
			{
				values.push_back(p.y);
			}
		}

		std::sort(values.begin(), values.end());

		std::vector<float> dedup;
		for (unsigned i = 0; i < values.size() - 1; ++i)
		{
			if (values[i] != values[i + 1])
			{
				dedup.push_back(values[i]);
			}
		}
		dedup.push_back(values[values.size() - 1]);

		// Edge case: unable to find split due to distribution
		if (dedup[0] == dedup[dedup.size() - 1])
		{
			return {values.size(), std::nanf("")};
		}

		unsigned mid = dedup.size() / 2;
		if (dedup.size() % 2 == 0)
		{
			return {0.0, (dedup[mid - 1] + dedup[mid]) / 2.0};
		}
		else
		{
			return {0.0, dedup[mid]};
		}
	}

	Cost RPlusTree::sweepNodes(std::vector<Node *> &nodeList, Orientation orientation)
	{
		std::vector<float> leftBounds;
		std::vector<float> rightBounds;

		for (auto &node : nodeList)
		{
			if (orientation == ALONG_X_AXIS)
			{
				leftBounds.push_back(node->boundingBox.lowerLeft.x);
				rightBounds.push_back(node->boundingBox.upperRight.x);
			}
			else
			{
				leftBounds.push_back(node->boundingBox.lowerLeft.y);
				rightBounds.push_back(node->boundingBox.upperRight.y);
			}
		}

		// Sort ascending
		std::sort(leftBounds.begin(), leftBounds.end());
		std::sort(rightBounds.begin(), rightBounds.end());

		// De-duplicated left bounds
		std::vector<float> dedup;
		for (unsigned i = 0; i < leftBounds.size() - 1; i++)
		{
			if (leftBounds[i] != leftBounds[i + 1])
			{
				dedup.push_back(leftBounds[i]);
			}
		}
		dedup.push_back(leftBounds[leftBounds.size() - 1]);

		// Edge case: unable to find split due to distribution
		if (dedup[0] == dedup[dedup.size() - 1])
		{
			return {leftBounds.size(), std::nanf("")};
		}

		// Set split line to be middle element
		float splitLine = dedup[dedup.size() / 2];

		// Compute cost
		float cost = 0.0;
		for (unsigned i = 0; i < leftBounds.size(); i++)
		{
			if (leftBounds[i] < splitLine && splitLine < rightBounds[i])
			{
				cost += 1.0;
			}
		}

		return {cost, splitLine};
	}

	Partition RPlusTree::partition(Node *n, float splitLine, Orientation splitAxis)
	{
		auto *left = n;
		auto *right = new Node();
		right->parent = left->parent;

		if (n->isLeaf())
		{
			// Leaf case
			std::vector<Point> pointsClone = n->data;
			n->data.clear();
			for (auto &point : pointsClone)
			{
				float value = splitAxis == ALONG_X_AXIS ? point.x : point.y;
				if (value < splitLine)
				{
					left->data.push_back(point);
				}
				else
				{
					right->data.push_back(point);
				}
			}
		}
		else
		{
			// Routing node case
			std::vector<Node *> childrenClone = n->children;
			n->children.clear();
			unsigned vectorSize = childrenClone.size();
			for (unsigned i = 0; i < vectorSize; ++i)
			{
				Node *child = childrenClone[i];
				float rightEdge = splitAxis == ALONG_X_AXIS ? child->boundingBox.upperRight.x : child->boundingBox.upperRight.y;
				float leftEdge = splitAxis == ALONG_X_AXIS ? child->boundingBox.lowerLeft.x : child->boundingBox.lowerLeft.y;
				
				if (rightEdge <= splitLine)
				{
					left->children.push_back(child);
					child->parent = left;
				}
				else if (splitLine <= leftEdge)
				{
					right->children.push_back(child);
					child->parent = right;
				}
				else
				{
					// Propagate changes downwards
					Partition split = partition(child, splitLine, splitAxis);
					if (split.first != nullptr)
					{
						childrenClone.push_back(split.first);
						vectorSize++;
					}
					if (split.second != nullptr)
					{
						childrenClone.push_back(split.second);
						vectorSize++;
					}
				}
			}
		}

		// Adjust bounding boxes
		if (std::max(left->numDataEntries(), left->numChildren()) > 0)
		{
			left->tighten();
		}
		if (std::max(right->numDataEntries(), right->numChildren()) > 0)
		{
			right->tighten();
		}

		// Adjust left node after split and right nodes after split
		if (std::max(left->numDataEntries(), left->numChildren()) == 0)
		{
			// Replace original pointer to left child with right child
			std::replace(left->parent->children.begin(), left->parent->children.end(), left, right);
			left = right;
			right->data.clear();
			right->children.clear();
		}

		// Delete right node if needed
		if (std::max(right->numDataEntries(), right->numChildren()) == 0)
		{
			delete right;
			right = nullptr;
		}

		return {left, right};
	}

	Partition RPlusTree::splitNode(Node *n)
	{
		Cost costX, costY;
		if (n->isLeaf())
		{
			// Determine optimal partition for data
			costX = sweepData(n->data, ALONG_X_AXIS);
			costY = sweepData(n->data, ALONG_Y_AXIS);
		}
		else
		{
			// Determine optimal partition for intermediate node
			costX = sweepNodes(n->children, ALONG_X_AXIS);
			costY = sweepNodes(n->children, ALONG_Y_AXIS);
		}

		// Edge case where we are unable to find split due to distribution
		if (std::isnan(costX.second))
		{
			return partition(n, costY.second, ALONG_Y_AXIS);
		}
		if (std::isnan(costY.second))
		{
			return partition(n, costX.second, ALONG_X_AXIS);
		}

		// Decide split based on cost
		float splitLine = costX.first <= costY.first ? costX.second : costY.second;
		Orientation splitAxis = costX.first <= costY.first ? ALONG_X_AXIS : ALONG_Y_AXIS;

		return partition(n, splitLine, splitAxis);
	}

	void RPlusTree::reinsert(Node *n, unsigned level, std::vector<Point> &dataClone)
	{
		// Special single element case / empty bounding box case
		if (n->boundingBox.lowerLeft == n->boundingBox.upperRight)
		{
			if (n->numChildren() == 1)
			{
				reinsert(n->children[0], level - 1, dataClone);
			}
			if (n->numDataEntries() == 1)
			{
				dataClone.push_back(n->data[0]);
			}

			return;
		}

		// Find where to re-insert intermediate nodes
		Node *baseNode = chooseLeaf(root, n->boundingBox);

		// Move up the tree to the correct level
		for (unsigned i = 0; i < level; ++i)
		{
			baseNode = baseNode->parent;
		}

		// Add node back into tree, adjust if needed
		baseNode->children.push_back(n);
		n->parent = baseNode;
		if (baseNode->numChildren() > maxBranchFactor)
		{
			Partition split = splitNode(baseNode);
			adjustTree(split.first, split.second);
		}
		else
		{
			adjustTree(baseNode, nullptr);
		}
	}

	void RPlusTree::condenseTree(Node *n, std::vector<Point> &dataClone)
	{
		unsigned lvl = 0;  // Number of levels above leaf
		std::vector<unsigned> levels;
		std::vector<Node *> reinsertion;

		while (!n->isRoot() && n->numChildren() < minBranchFactor)
		{
			auto iter = std::find(n->parent->children.begin(), n->parent->children.end(), n);
			n->parent->children.erase(iter);

			if (n->parent->numChildren() > 0)
			{
				adjustTree(n->parent, nullptr);
			}

			for (auto &child : n->children)
			{
				reinsertion.push_back(child);
				levels.push_back(lvl);
				child->parent = nullptr;
			}
			n = n->parent;
			lvl++;
		}

		// Root removal case
		if (n->isRoot() && n->numChildren() == 1)
		{
			Node *tempNode = root;
			root = root->children[0];
			root->parent = nullptr;
			delete tempNode;
		}

		// Reinsert intermediate nodes
		for (unsigned i = 0; i < reinsertion.size(); ++i)
		{
			reinsert(reinsertion[i], levels[i], dataClone);
		}
	}

	std::vector<Point> RPlusTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		return v;
	}

	std::vector<Point> RPlusTree::search(Point requestedPoint)
	{
		return findLeaf(requestedPoint) == nullptr ? std::vector<Point>() : std::vector<Point>{requestedPoint};
	}

	std::vector<Point> RPlusTree::search(Rectangle requestedRectangle)
	{
		std::vector<Point> result;
		std::stack<Node *> stack;
		Node *currentNode;

		// do DFS to find all points contained in requestedRectangle
		stack.push(root);
		while (!stack.empty())
		{
			currentNode = stack.top();
			stack.pop();

			if (currentNode->isLeaf())
			{
				for (auto &data: currentNode->data)
				{
					if (requestedRectangle.containsPoint(data))
					{
						result.push_back(data);
					}
				}
			}
			else
			{
				for (auto &child : currentNode->children)
				{
					if (child->boundingBox.intersectsRectangle(requestedRectangle))
					{
						stack.push(child);
					}
				}
			}
		}

		return result;
	}

	void RPlusTree::insert(Point givenPoint)
	{
		// Choose the leaves where the data will go
		Node *leaf = chooseLeaf(root, givenPoint);
		leaf->data.push_back(givenPoint);

		// Split leaf node
		if (leaf->numDataEntries() > maxBranchFactor)
		{
			Partition split = splitNode(leaf);
			adjustTree(split.first, split.second);
		}
		else
		{
			adjustTree(leaf, nullptr);
		}
	}

	void RPlusTree::remove(Point givenPoint)
	{
		Node *leaf = findLeaf(givenPoint);
		if (leaf == nullptr)
		{
			// Element does not exist
			return;
		}
		auto iter = std::find(leaf->data.begin(), leaf->data.end(), givenPoint);
		leaf->data.erase(iter);

		// Root case
		if (leaf->isRoot() && leaf->numChildren() == 0)
		{
			return;
		}

		// Simple removal case
		if (leaf->numDataEntries() >= minBranchFactor)
		{
			adjustTree(leaf, nullptr);
			return;
		}

		std::vector<Point> dataClone = leaf->data;
		leaf->data.clear();
		condenseTree(leaf, dataClone);

		// Reinsert data points
		for (auto &data : dataClone)
		{
			insert(data);
		}
	}

	void RPlusTree::checkBoundingBoxes()
	{
		std::stack<Node *> stack;
		Node *currentNode;

		stack.push(root);
		while (!stack.empty())
		{
			currentNode = stack.top();
			stack.pop();

			if (currentNode->isLeaf())
			{
				continue;
			}
			for (auto &c1 : currentNode->children)
			{
				stack.push(c1);
				for (auto &c2 : currentNode->children)
				{
					if (c1 < c2)
					{
						assert(c1->boundingBox.computeIntersectionArea(c2->boundingBox) == 0.0);
					}
				}
			}
		}
	}

	unsigned RPlusTree::checksum()
	{
		unsigned sum = 0;
		Node *currentNode;
		std::stack<Node *> stack;

		stack.push(root);
		while (!stack.empty())
		{
			currentNode = stack.top();
			stack.pop();

			if (currentNode->isLeaf())
			{
				for (auto &data: currentNode->data)
				{
					sum += (unsigned) data.x;
					sum += (unsigned) data.y;
				}
			}
			else
			{
				for (auto &child : currentNode->children)
				{
					stack.push(child);
				}
			}
		}

		return sum;
	}

	std::ostream &operator<<(std::ostream &os, RPlusTree &tree)
	{
		std::queue<Node *> queue;
		Node *currentNode;

		queue.push(tree.getRoot());
		while (!queue.empty())
		{
			currentNode = queue.front();
			queue.pop();
			if (currentNode->isLeaf())
			{
				os << currentNode->boundingBox << ": ";
				for (auto &data : currentNode->data)
				{
					os << data << ", ";
				}
				os << std::endl;
			}
			else
			{
				os << currentNode->boundingBox << std::endl;
				for (auto &child : currentNode->children)
				{
					queue.push(child);
				}
			}
		}

		return os;
	}
}
