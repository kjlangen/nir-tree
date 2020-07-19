#include <rplustree/rPlusTree.h>

/*** constructor and destructor ***/

RPlusTree::RPlusTree(unsigned int minBranchFactor, unsigned int maxBranchFactor) : minBranchFactor(minBranchFactor),
																				   maxBranchFactor(maxBranchFactor)
{
	root = new RPlusTreeNode();
}

RPlusTree::~RPlusTree()
{
	std::stack<RPlusTreeNode*> stack;
	stack.push(root);
	root = nullptr;
	RPlusTreeNode * currentNode;

	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();
		for (auto & child : currentNode->children) {
			stack.push(child);
		}
		delete currentNode;  // cleanup memory
	}
}

/*** general functions ***/

bool RPlusTree::isEmpty() const {
	return root->numChildren() == 0 && root->numDataEntries() == 0;
}

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

bool RPlusTree::exists(Point requestedPoint) const {
	return findLeaf(requestedPoint) != nullptr;
}

std::vector<Point> RPlusTree::search(Point requestedPoint) const
{
	return findLeaf(requestedPoint) == nullptr ? std::vector<Point>() : std::vector<Point>{requestedPoint};
}

std::vector<Point> RPlusTree::search(Rectangle requestedRectangle) const
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

/*** helper functions ***/

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
		root->tighten();
		return;
	}
	n->tighten();
	if (nn != nullptr) {
		nn->tighten();
		nn->parent->children.push_back(nn);
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

RPlusTreeNode* RPlusTree::chooseLeaf(RPlusTreeNode* node, Point& givenPoint) const
{
	if (node->isLeaf()) {
		return node;
	}
	// Find the bounding box with least required expansion/overlap
	RPlusTreeNode* chosenChild = nullptr;
	float smallestExpansionArea = node->children.at(0)->boundingBox.computeExpansionArea(givenPoint);
	for (auto child : node->children) {
		// containment case
		if (child->boundingBox.containsPoint(givenPoint)) {
			chosenChild = child;
			break;
		}
		// need to check for future overlap
		bool noOverlap = true;
		Rectangle newBoundingBox = child->boundingBox;
		newBoundingBox.expand(givenPoint);
		for (auto other : node->children) {
			if (child != other && newBoundingBox.computeOverlapArea(other->boundingBox) != 0.0f) {
				noOverlap = false;
			}
		}
		if (noOverlap) {
			if (chosenChild == nullptr) {
				chosenChild = child;
			} else {
				// best fit case
				float testExpansionArea = child->boundingBox.computeExpansionArea(givenPoint);
				if (testExpansionArea < smallestExpansionArea) {
					smallestExpansionArea = testExpansionArea;
					chosenChild = child;
				}
			}
		}
	}
	assert(chosenChild != nullptr);
	return chooseLeaf(chosenChild, givenPoint);
}

RPlusTreeNode* RPlusTree::findLeaf(Point requestedPoint) const {
	std::stack<RPlusTreeNode*> stack;
	stack.push(root);
	RPlusTreeNode * currentNode;

	// do DFS to find leaf node that contains `requestedPoint`
	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();

		if (currentNode->isLeaf()) {
			auto iter = std::find(currentNode->data.begin(), currentNode->data.end(), requestedPoint);
			if (iter != currentNode->data.end()) {
				return currentNode;
			}
		} else {
			for (auto & child : currentNode->children) {
				if (child->boundingBox.containsPoint(requestedPoint)) {
					stack.push(child);
				}
			}
		}
	}
	return nullptr;
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
	for (unsigned i = 0; i < values.size() - 1; i++) {
		if (values.at(i) != values.at(i + 1)) {
			dedup.push_back(values.at(i));
		}
	}
	dedup.push_back(values.at(values.size() - 1));

	// Edge case: unable to find split due to distribution
	if (dedup.at(0) == dedup.at(dedup.size() - 1)) {
		return {values.size(), std::nanf("")};
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
	for (unsigned i = 0; i < leftBounds.size() - 1; i++) {
		if (leftBounds.at(i) != leftBounds.at(i + 1)) {
			dedup.push_back(leftBounds.at(i));
		}
	}
	dedup.push_back(leftBounds.at(leftBounds.size() - 1));

	// Edge case: unable to find split due to distribution
	if (dedup.at(0) == dedup.at(dedup.size() - 1)) {
		return {leftBounds.size(), std::nanf("")};
	}

	// Set split line to be middle element
	float splitLine = dedup.at(dedup.size() / 2);

	// Compute cost
	float cost = 0.0f;
	for (unsigned i = 0; i < leftBounds.size(); i++) {
		if (leftBounds.at(i) < splitLine && splitLine < rightBounds.at(i)) {
			cost += 1.0f;
		}
	}

	return {cost, splitLine};
}

Partition RPlusTree::partition(RPlusTreeNode *n, float splitLine, Orientation splitAxis)
{
	auto* left = n;
	auto* right = new RPlusTreeNode();
	right->parent = left->parent;

	if (n->isLeaf()) {
		/*** leaf node case ***/
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
		/*** intermediate node case ***/
		std::vector<RPlusTreeNode*> childrenClone = n->children;  // copy
		n->children.clear();  // clear old entries
		int vectorSize = childrenClone.size();
		for (unsigned i = 0; i < vectorSize; i++) {
			RPlusTreeNode* child = childrenClone.at(i);
			float rightEdge = splitAxis == ALONG_X_AXIS ? child->boundingBox.upperRight.x : child->boundingBox.upperRight.y;
			float leftEdge = splitAxis == ALONG_X_AXIS ? child->boundingBox.lowerLeft.x : child->boundingBox.lowerLeft.y;
			if (rightEdge <= splitLine) {
				left->children.push_back(child);
				child->parent = left;   // set new parent
			} else if (splitLine <= leftEdge) {
				right->children.push_back(child);
				child->parent = right;  // set new parent
			} else {
				// propagate changes downwards
				Partition split = partition(child, splitLine, splitAxis);
				if (split.first != nullptr) {
					childrenClone.push_back(split.first);
					vectorSize++;
				}
				if (split.second != nullptr) {
					childrenClone.push_back(split.second);
					vectorSize++;
				}
			}
		}
	}

	// adjust bounding boxes
	if (std::max(left->numDataEntries(), left->numChildren()) > 0) {
		left->tighten();
	}
	if (std::max(right->numDataEntries(), right->numChildren()) > 0) {
		right->tighten();
	}

	// adjust left and right nodes after split
	if (std::max(left->numDataEntries(), left->numChildren()) == 0) {
		// replace original pointer to left child with right child
		std::replace(left->parent->children.begin(), left->parent->children.end(), left, right);
		left = right;  // call assignment operator to copy over class attributes
		right->data.clear();
		right->children.clear();
	}
	if (std::max(right->numDataEntries(), right->numChildren()) == 0) {
		// delete right node if needed
		delete right;
		right = nullptr;
	}
	return {left, right};
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

	// handle edge case where we are unable to find split due to distribution
	if (std::isnan(costX.second)) {
		return partition(n, costY.second, ALONG_Y_AXIS);
	}
	if (std::isnan(costY.second)) {
		return partition(n, costX.second, ALONG_X_AXIS);
	}

	// decide split based on cost
	float splitLine = costX.first <= costY.first ? costX.second : costY.second;
	Orientation splitAxis = costX.first <= costY.first ? ALONG_X_AXIS : ALONG_Y_AXIS;
	return partition(n, splitLine, splitAxis);
}

/*** remove functions ***/

void RPlusTree::findAllData(RPlusTreeNode* n, std::vector<Point> &dataClone) {
	if (n->isLeaf()) {
		dataClone.insert(dataClone.end(), n->data.begin(), n->data.end());
		return;
	}
	for (auto child : n->children) {
		findAllData(child, dataClone);
		delete child;
	}
	n->children.clear();
	n->parent = nullptr;
}

void RPlusTree::reinsert(RPlusTreeNode *n, int level, std::vector<Point>& dataClone) {
	// Special single element case / empty bounding box case
	if (n->boundingBox.lowerLeft == n->boundingBox.upperRight) {
		if (n->numChildren() == 1) {
			reinsert(n->children.at(0), level - 1, dataClone);
		}
		if (n->numDataEntries() == 1) {
			dataClone.push_back(n->data.at(0));
		}
		return;
	}

	// Find where to re-insert intermediate nodes
	RPlusTreeNode * baseNode = root;
	while (!baseNode->isLeaf()) {
		RPlusTreeNode * nextNode = nullptr;
		float smallestExpansionArea = baseNode->children.at(0)->boundingBox.computeExpansionArea(n->boundingBox);
		for (auto & child : baseNode->children) {
			// need to check for future overlap
			bool noOverlap = true;
			Rectangle newBoundingBox = child->boundingBox;
			newBoundingBox.expand(n->boundingBox);
			for (auto other : baseNode->children) {
				if (child != other && newBoundingBox.computeOverlapArea(other->boundingBox) != 0.0f) {
					noOverlap = false;
				}
			}
			if (noOverlap) {
				if (nextNode == nullptr) {
					nextNode = child;
				}
				float testExpansionArea = child->boundingBox.computeExpansionArea(n->boundingBox);
				if (testExpansionArea < smallestExpansionArea) {
					smallestExpansionArea = testExpansionArea;
					nextNode = child;
				}
			}
		}
		// Special case
		if (nextNode == nullptr) {
			findAllData(n, dataClone);
			return;
		}
		baseNode = nextNode;
	}

	// Move up the tree to the correct level
	for (unsigned i=0; i<level; i++) {
		baseNode = baseNode->parent;
	}

	// Special overlapping case
	for (auto child: baseNode->children) {
		if (n->boundingBox.computeOverlapArea(child->boundingBox) != 0.0f) {
			findAllData(n, dataClone);
			return;
		}
	}

	// Add node back into tree, adjust if needed
	baseNode->children.push_back(n);
	n->parent = baseNode;  // adjust parent pointer
	if (baseNode->numChildren() > maxBranchFactor) {
		Partition split = splitNode(baseNode);
		adjustTree(split.first, split.second);
	} else {
		adjustTree(baseNode, nullptr);  // adjust bounding box
	}
}

void RPlusTree::condenseTree(RPlusTreeNode *n, std::vector<Point>& dataClone) {
	int lvl = 0;  // number of levels above leaf
	std::vector<int> levels;
	std::vector<RPlusTreeNode*> reinsertion;
	while (!n->isRoot() && n->numChildren() < minBranchFactor) {
		auto iter = std::find(n->parent->children.begin(), n->parent->children.end(), n);
		n->parent->children.erase(iter);
		if (n->parent->numChildren() > 0) {
			adjustTree(n->parent, nullptr);  // adjust bounding box
		}
		for (auto & child : n->children) {
			reinsertion.push_back(child);
			levels.push_back(lvl);
			child->parent = nullptr;
		}
		n = n->parent;
		lvl++;
	}

	// Root removal case
	if (n->isRoot() && n->numChildren() == 1) {
		RPlusTreeNode* tempNode = root;
		root = root->children.at(0);
		root->parent = nullptr;  // set root property
		delete tempNode;
	}

	// Reinsert intermediate nodes
	for (unsigned i=0; i<reinsertion.size(); i++) {
		reinsert(reinsertion.at(i), levels.at(i), dataClone);
	}
}

void RPlusTree::remove(Point givenPoint)
{
	RPlusTreeNode* leaf = findLeaf(givenPoint);
	if (leaf == nullptr) {
		throw std::exception();  // element does not exist
	}
	auto iter = std::find(leaf->data.begin(), leaf->data.end(), givenPoint);
	leaf->data.erase(iter);  // otherwise, remove data

	// special root case
	if (leaf->isRoot() && leaf->numChildren() == 0) {
		return;
	}

	// simple removal case
	if (leaf->numDataEntries() >= minBranchFactor) {
		adjustTree(leaf, nullptr);
		return;  // no need to do anything else, return
	}

	std::vector<Point> dataClone = leaf->data; // copy
	leaf->data.clear();
	condenseTree(leaf, dataClone);

	// reinsert data points
	for (auto & data : dataClone) {
		insert(data);
	}
}

/*** correctness checks ***/

void RPlusTree::checkBoundingBoxes() {
	std::stack<RPlusTreeNode*> stack;
	stack.push(root);
	RPlusTreeNode * currentNode;

	while (!stack.empty()) {
		currentNode = stack.top();
		stack.pop();

		if (currentNode->isLeaf()) {
			continue;
		}
		for (auto & c1 : currentNode->children) {
			stack.push(c1);
			for (auto & c2 : currentNode->children) {
				if (c1 < c2) {
					assert(c1->boundingBox.computeOverlapArea(c2->boundingBox) == 0.0f);
				}
			}
		}
	}
}

/*** tree traversal ***/

std::ostream &operator<<(std::ostream &os, const RPlusTree &tree)
{
	std::queue<RPlusTreeNode*> queue;
	queue.push(tree.getRoot());

	RPlusTreeNode* currentNode;
	while (!queue.empty()) {
		currentNode = queue.front();
		queue.pop();
		if (currentNode->isLeaf()) {
			os << currentNode->boundingBox << ": ";
			for (auto & data : currentNode->data) {
				os << data << ", ";
			}
			os << std::endl;
		} else {
			os << currentNode->boundingBox << std::endl;
			for (auto & child : currentNode->children) {
				queue.push(child);
			}
		}
	}
	return os;
}
