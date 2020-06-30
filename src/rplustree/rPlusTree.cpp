#include <rplustree/rPlusTree.h>

/*** constructor and destructor ***/

RPlusTree::RPlusTree(unsigned int minBranchFactor, unsigned int maxBranchFactor) : minBranchFactor(minBranchFactor),
																				   maxBranchFactor(maxBranchFactor)
{
}

RPlusTree::~RPlusTree()
{
	// TODO
}

/*** general functions ***/

RPlusTreeNode * RPlusTree::getRoot() const {
	return root;
}

int RPlusTree::height() const {
	int height = 0;
	RPlusTreeNode* n = root;
	while(!n->isLeaf()) {
		n = n->children.at(0);
		height++;
	}
	return height;
}

int RPlusTree::numDataElements() const {
	int result = 0;
	RPlusTreeNode* currentNode;

	std::stack<RPlusTreeNode*> stack;
	stack.push(root);

	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();

		if (currentNode->isLeaf()) {
			result += (int)currentNode->numDataEntries();
		} else {
			for (auto & child : currentNode->children) {
				stack.push(child);
			}
		}
	}

	return result;
}

bool RPlusTree::exists(Point requestedPoint) {
	RPlusTreeNode * n = root;
	while (!n->isLeaf()) {
		bool childFound = false;
		for (auto & child : n->children) {
			if (child->boundingBox.containsPoint(requestedPoint)) {
				n = child;
				childFound = true;
				break;
			}
		}
		if (!childFound) {
			return false;
		}
	}

	for (auto & data : n->data) {
		if (data == requestedPoint) {
			return true;
		}
	}
	return false;
}

std::vector<Point> RPlusTree::search(Point requestedPoint)
{
	std::vector<Point> result;
	std::stack<RPlusTreeNode*> stack;
	stack.push(root);
	RPlusTreeNode * currentNode;

	// do DFS to find all points contained in `requestedRectangle`
	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();

		if (currentNode->isLeaf()) {
			for (auto & data: currentNode->data) {
				if (data == requestedPoint) {
					result.push_back(data);
				}
			}
		} else {
			for (auto & child : currentNode->children) {
				if (child->boundingBox.containsPoint(requestedPoint)) {
					stack.push(child);
				}
			}
		}
	}

	return result;
}

std::vector<Point> RPlusTree::search(Rectangle requestedRectangle)
{
	std::vector<Point> result;
	std::stack<RPlusTreeNode*> stack;
	stack.push(root);
	RPlusTreeNode * currentNode;

	// do DFS to find all points contained in `requestedRectangle`
	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();

		if (currentNode->isLeaf()) {
			for (auto & data: currentNode->data) {
				if (requestedRectangle.containsPoint(data)) {
					result.push_back(data);
				}
			}
		} else {
			for (auto & child : currentNode->children) {
				if (child->boundingBox.intersectsRectangle(requestedRectangle)) {
					stack.push(child);
				}
			}
		}
	}

	return result;
}

unsigned RPlusTree::checksum()
{
	unsigned sum = 0;
	RPlusTreeNode* currentNode;

	std::stack<RPlusTreeNode*> stack;
	stack.push(root);

	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();

		if (currentNode->isLeaf()) {
			for (auto & data: currentNode->data) {
				sum += (unsigned)data.x;
				sum += (unsigned)data.y;
			}
		} else {
			for (auto & child : currentNode->children) {
				stack.push(child);
			}
		}
	}

	return sum;
}

void RPlusTree::print()
{
	// TODO
}

/*** helper functions ***/

void RPlusTree::tighten(RPlusTreeNode* n)
{
	if (n->isLeaf()) {
		// reset bounding box
		Point p = n->data.at(0);
		n->boundingBox = Rectangle(p, p);
		// iterate through data to set new bounding box
		for (auto & data : n->data) {
			n->boundingBox.expand(data);
		}
	} else {
		// reset bounding box
		Rectangle r = n->children.at(0)->boundingBox;
		n->boundingBox = r;
		// iterate through children to set new bounding box
		for (auto & child : n->children) {
			n->boundingBox.expand(child->boundingBox);
		}
	}
}

void RPlusTree::adjustTree(RPlusTreeNode* n, RPlusTreeNode* nn)
{
	if (n->isRoot()) {
		if (nn != nullptr) {
			root = new RPlusTreeNode();
			root->children.push_back(n);
			n->parent = root;
			root->children.push_back(nn);
			nn->parent = root;
		}
		tighten(root);
		return;
	}
	tighten(n);
	if (nn != nullptr) {
		tighten(nn);
		// propagate split upwards, if needed
		if (n->parent->numChildren() > maxBranchFactor) {
			Partition splits = splitNode(n->parent);
			adjustTree(splits.first, splits.second);
		}
	}

	if (!n->isRoot()) {
		adjustTree(n->parent, nullptr);
	}
}

RPlusTreeNode* RPlusTree::chooseLeaf(RPlusTreeNode* node, Point& givenPoint)
{
	if (node->isLeaf()) {
		return node;
	}
	// Find the bounding box with least required expansion/overlap
	RPlusTreeNode* chosenChild = node->children.at(0);
	float smallestExpansionArea = node->children.at(0)->boundingBox.computeExpansionArea(givenPoint);
	for (auto & child : node->children) {
		float testExpansionArea = child->boundingBox.computeExpansionArea(givenPoint);
		if (smallestExpansionArea > testExpansionArea) {
			smallestExpansionArea = testExpansionArea;
			chosenChild = child;
		}
	}
	return chooseLeaf(chosenChild, givenPoint);
}

/*** insert functions ***/

void RPlusTree::insert(Point givenPoint)
{
	// choose the leaves where the data will go
	RPlusTreeNode * leaf = chooseLeaf(root, givenPoint);
	leaf->data.push_back(givenPoint);

	// need to split leaf node
	if (leaf->numDataEntries() > maxBranchFactor) {
		Partition split = splitNode(leaf);
		adjustTree(split.first, split.second);
	} else {
		adjustTree(leaf, nullptr);
	}
}

Cost RPlusTree::sweepData(std::vector<Point>& points, Orientation orientation)
{
	std::vector<float> values;
	for (auto &p : points) {
		if (orientation == ALONG_X_AXIS) {
			values.push_back(p.x);
		} else {
			values.push_back(p.y);
		}
	}
	std::sort(values.begin(), values.end());

	std::vector<float> dedup;
	for (int i = 0; i < values.size() - 1; i++) {
		if (values.at(i) != values.at(i + 1)) {
			dedup.push_back(values.at(i));
		}
	}
	dedup.push_back(values.at(values.size() - 1));

	if (dedup.at(0) == dedup.at(dedup.size() - 1)) {
		return {values.size(), 0.0f};
	}

	unsigned mid = dedup.size() / 2;
	if (dedup.size() % 2 == 0) {
		return {0.0f, (dedup.at(mid - 1) + dedup.at(mid)) / 2.0f};
	}
	return {0.0f, dedup.at(mid)};
}

Cost RPlusTree::sweepNodes(std::vector<RPlusTreeNode*>& nodeList, Orientation orientation)
{
	std::vector<float> leftBounds;
	std::vector<float> rightBounds;

	for (auto & node : nodeList) {
		if (orientation == ALONG_X_AXIS) {
			leftBounds.push_back(node->boundingBox.lowerLeft.x);
			rightBounds.push_back(node->boundingBox.upperRight.x);
		} else {
			leftBounds.push_back(node->boundingBox.lowerLeft.y);
			rightBounds.push_back(node->boundingBox.upperRight.y);
		}
	}

	// Sort ascending
	std::sort(leftBounds.begin(), leftBounds.end());
	std::sort(rightBounds.begin(), rightBounds.end());

	// De-duplicated left bounds
	std::vector<float> dedup;
	for (int i = 0; i < leftBounds.size() - 1; i++) {
		if (leftBounds.at(i) != leftBounds.at(i + 1)) {
			dedup.push_back(leftBounds.at(i));
		}
	}
	dedup.push_back(leftBounds.at(leftBounds.size() - 1));

	// Edge case
	if (dedup.at(0) == dedup.at(dedup.size() - 1)) {
		return {leftBounds.size(), 0.0f};
	}

	// Set split line to be middle element
	float splitLine = dedup.at(dedup.size() / 2);

	// Compute cost
	float cost = 0.0f;
	for (int i = 0; i < leftBounds.size(); i++) {
		if (leftBounds.at(i) < splitLine && splitLine < rightBounds.at(i)) {
			cost += 1.0f;
		}
	}

	return {cost, splitLine};
}

void RPlusTree::partition(RPlusTreeNode *n, float splitLine, Orientation splitAxis, RPlusTreeNode* left, RPlusTreeNode* right)
{
	if (n->isLeaf()) {
		std::vector<Point> pointsClone = n->data;  // copy
		n->data.clear();  // clear old entries
		for (auto & point : pointsClone) {
			float value = splitAxis == ALONG_X_AXIS ? point.x : point.y;
			if (value < splitLine) {
				left->data.push_back(point);
			} else {
				right->data.push_back(point);
			}
		}
	} else {
		std::vector<RPlusTreeNode*> childrenClone = n->children;  // copy
		n->children.clear();  // clear old entries
		int vectorSize = childrenClone.size();
		for (int i = 0; i < vectorSize; i++) {
			RPlusTreeNode* child = childrenClone.at(i);
			float rightEdge = splitAxis == ALONG_X_AXIS ? child->boundingBox.upperRight.x : child->boundingBox.upperRight.y;
			float leftEdge = splitAxis == ALONG_X_AXIS ? child->boundingBox.lowerLeft.x : child->boundingBox.lowerLeft.y;
			if (rightEdge < splitLine) {
				left->children.push_back(child);
				child->parent = left;   // set new parent
			} else if (splitLine <= leftEdge) {
				right->children.push_back(child);
				child->parent = right;  // set new parent
			} else {
				// propagate changes downwards
				auto * newLeftNode = new RPlusTreeNode();
				auto * newRightNode = new RPlusTreeNode();
				partition(child, splitLine, splitAxis, newLeftNode, newRightNode);
				delete child;  // cleanup no longer needed node

				// add new nodes to array or remove if empty
				if (newLeftNode->numChildren() == 0 && newLeftNode->numDataEntries() == 0) {
					delete newLeftNode;
				} else {
					tighten(newLeftNode);
					childrenClone.push_back(newLeftNode);
					vectorSize++;
				}
				if (newRightNode->numChildren() == 0 && newRightNode->numDataEntries() == 0) {
					delete newRightNode;
				} else {
					tighten(newRightNode);
					childrenClone.push_back(newRightNode);
					vectorSize++;
				}
			}
		}
	}
}

Partition RPlusTree::splitNode(RPlusTreeNode* n)
{
	Cost costX, costY;
	if (n->isLeaf()) {
		// determine optimal partition for data
		costX = sweepData(n->data, ALONG_X_AXIS);
		costY = sweepData(n->data, ALONG_Y_AXIS);
	} else {
		// determine optimal partition for intermediate node
		costX = sweepNodes(n->children, ALONG_X_AXIS);
		costY = sweepNodes(n->children, ALONG_Y_AXIS);
	}
	float splitLine = costX.first <= costY.first ? costX.second : costY.second;
	Orientation splitAxis = costX.first <= costY.first ? ALONG_X_AXIS : ALONG_Y_AXIS;
	return splitNodeAlongLine(n, splitLine, splitAxis);
}

Partition RPlusTree::splitNodeAlongLine(RPlusTreeNode *n, float splitLine, Orientation splitAxis) {
	// create new node and set parameters
	auto* newRightNode = new RPlusTreeNode();
	Partition result = {n, newRightNode};
	newRightNode->parent = n->parent;
	partition(n, splitLine, splitAxis, result.first, result.second);

	unsigned numLeftElements = result.first->numChildren();
	unsigned numRightElements = result.second->numChildren();
	if (n->isLeaf()) {
		numLeftElements = result.first->numDataEntries();
		numRightElements = result.second->numDataEntries();
	}

	// adjust left and right sections
	if (numLeftElements == 0) {
		delete result.first;
		result.first = result.second;
		result.second = nullptr;
	} else if (numRightElements == 0) {
		delete result.second;
		result.second = nullptr;
	}

	// adjust bounding boxes and add new node to parent if applicable
	tighten(result.first);
	if (result.second != nullptr) {
		tighten(result.second);
		if (result.second->parent != nullptr) {
			result.second->parent->children.push_back(result.second);
		}
	}
	return result;
}

/*** remove functions ***/

void RPlusTree::remove(Point givenPoint)
{
	RPlusTreeNode* leaf = chooseLeaf(root, givenPoint);
	auto iter = std::find(leaf->data.begin(), leaf->data.end(), givenPoint);
	if (iter == leaf->data.end()) {
		return;  // element does not exist
	}
	leaf->data.erase(iter);  // otherwise, remove data

	if (leaf->numDataEntries() >= minBranchFactor || leaf->isRoot()) {
		return;  // no need to do anything else, return
	}

	// need to remove node from tree
	auto it = std::find(leaf->parent->children.begin(), leaf->parent->children.end(), leaf);
	leaf->parent->children.erase(it);

	std::vector<Point> dataClone = leaf->data; // copy
	leaf->data.clear();
	findDataPoints(leaf->parent, dataClone);

	// reinsert data points
	for (auto & data : dataClone) {
		insert(data);
	}
}

void RPlusTree::findDataPoints(RPlusTreeNode *n, std::vector<Point>& dataClone) {
	// Find out which nodes need to be re-inserted
	std::vector<RPlusTreeNode*> reinsertion;
	while (!n->isRoot() && n->numChildren() < minBranchFactor) {
		auto iter = std::find(n->parent->children.begin(), n->parent->children.end(), n);
		n->parent->children.erase(iter);
		for (auto & child : n->children) {
			reinsertion.push_back(child);
		}
		n = n->parent;
	}

	// Edge case for removing root node
	if (n->isRoot() && n->numChildren() == 1) {
		RPlusTreeNode* tempNode = root;
		root = root->children.at(0);
		root->parent = nullptr;
		delete tempNode;
	}

	// Find data to re-insert back into the tree
	int numElements = reinsertion.size();
	for (int i = 0; i < numElements; i++) {
		RPlusTreeNode * curNode = reinsertion.at(i);
		if (curNode->isLeaf()) {
			// copy all data from curNode to dataClone
			dataClone.insert(dataClone.end(), curNode->data.begin(), curNode->data.end());
		} else {
			// copy all nodes from curNode to reinsertion and adjust size
			reinsertion.insert(reinsertion.end(), curNode->children.begin(), curNode->children.end());
			numElements += curNode->children.size();
		}
	}

	// Cleanup associated memory
	for (auto & node : reinsertion) {
		delete node;
	}
}

/*** tree traversals ***/

void RPlusTree::bfs() {
	std::queue<RPlusTreeNode*> queue;
	queue.push(root);

	RPlusTreeNode* currentNode;
	while (!queue.empty()) {
		currentNode = queue.front();
		queue.pop();
		currentNode->boundingBox.print();
		if (currentNode->isLeaf()) {
			for (auto & data : currentNode->data) {
				data.print();
			}
			std::cout << std::endl << std::endl;
		} else {
			for (auto & child : currentNode->children) {
				queue.push(child);
			}
		}
	}
}
