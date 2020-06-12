#include <rplustree/rPlusTree.h>

void RPlusTree::removeSubtree(RPlusTreeNode *r)
{
	// TODO
}

void RPlusTree::tighten(RPlusTreeNode* n)
{
	if (n->isLeaf()) {
		// reset bounding box
		Point p = n->data.at(0);
		n->boundingBox.lowerLeft.x = p.x;
		n->boundingBox.lowerLeft.y = p.y;
		n->boundingBox.upperRight.x = p.x;
		n->boundingBox.upperRight.y = p.y;
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

Cost RPlusTree::sweepData(std::vector<Point>& points, Orientation orientation)
{
	std::sort(points.begin(), points.end(),
		[orientation](const Point & p1, const Point & p2) {
			if (orientation == ALONG_X_AXIS) {
				return p1.x < p2.x;
			}
			return p1.y < p2.y;
	});

	// Determine split line
	float splitLine, cost;
	if (orientation == ALONG_X_AXIS) {
		splitLine = points.at(points.size() / 2).x;
		if (points.at(0).x == splitLine || splitLine == points.at(points.size()-1).x) {
			cost = points.size();
		} else {
			cost = 0.0f;
		}
	} else {
		splitLine = points.at(points.size() / 2).y;
		if (points.at(0).y == splitLine || splitLine == points.at(points.size()-1).y) {
			cost = points.size();
		} else {
			cost = 0.0f;
		}
	}

	return {cost, splitLine};
}

Cost RPlusTree::sweep(std::vector<RPlusTreeNode*>& nodeList, Orientation orientation)
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

	// Determine split line
	float splitLine = rightBounds.at(rightBounds.size() / 2);

	// Edge case
	if (rightBounds.at(0) == splitLine && splitLine == rightBounds.at(leftBounds.size()-1)) {
		splitLine = (splitLine - leftBounds.at(0)) / 2;
	}

	// Compute cost
	float cost = 0.0f;
	for (int i=0; i<leftBounds.size(); i++) {
		if (leftBounds.at(i) < splitLine && splitLine < rightBounds.at(i)) {
			cost += 1.0f;
		}
	}

	return {cost, splitLine};
}

Partition RPlusTree::splitNodeAlongLine(RPlusTreeNode *n, float splitLine, RPlusTree::Orientation splitAxis)
{
	// create new node and set parameters
	auto* newNode = new RPlusTreeNode();
	Partition partition = {n, newNode};
	newNode->parent = n->parent;
	if (newNode->parent != nullptr) {
		newNode->parent->children.push_back(newNode);
	}

	if (n->isLeaf()) {
		std::vector<Point> pointsClone = n->data;  // copy
		n->data.clear();  // clear old entries
		for (auto & point : pointsClone) {
			float value = splitAxis == ALONG_X_AXIS ? point.x : point.y;
			if (value <= splitLine) {
				partition.first->data.push_back(point);
			} else {
				partition.second->data.push_back(point);
			}
		}
	} else {
		std::vector<RPlusTreeNode*> childrenClone = n->children;  // copy
		n->children.clear();  // clear old entries
		for (auto & child : childrenClone) {
			float leftBound = splitAxis == ALONG_X_AXIS ? child->boundingBox.upperRight.x : child->boundingBox.upperRight.y;
			float rightBound = splitAxis == ALONG_X_AXIS ? child->boundingBox.lowerLeft.x : child->boundingBox.lowerLeft.y;
			if (leftBound <= splitLine) {
				partition.first->children.push_back(child);
			} else if (rightBound >= splitLine) {
				partition.second->children.push_back(child);
			} else {
				Partition split = splitNodeAlongLine(child, splitLine, splitAxis);  // propagate changes downwards
				partition.first->children.push_back(split.first);
				split.first->parent = partition.first;
				partition.second->children.push_back(split.second);
				split.second->parent = partition.second;
			}
		}
	}

	return partition;
}

Partition RPlusTree::splitNode(RPlusTreeNode* n)
{
	// create new node and set parameters
	auto* newNode = new RPlusTreeNode();
	Partition partition = {n, newNode};
	newNode->parent = n->parent;
	if (newNode->parent != nullptr) {
		newNode->parent->children.push_back(newNode);
	}

	if (n->isLeaf()) {
		// determine optimal partition
		Cost costX = sweepData(n->data, ALONG_X_AXIS);
		Cost costY = sweepData(n->data, ALONG_Y_AXIS);
		float splitLine = costX.first < costY.first ? costX.second : costY.second;
		Orientation splitAxis = costX.first < costY.first ? ALONG_X_AXIS : ALONG_Y_AXIS;
		std::vector<Point> pointsClone = n->data;  // copy
		n->data.clear();  // clear old entries
		for (auto & point : pointsClone) {
			float value = splitAxis == ALONG_X_AXIS ? point.x : point.y;
			if (value <= splitLine) {
				partition.first->data.push_back(point);
			} else {
				partition.second->data.push_back(point);
			}
		}
	} else {
		// determine optimal partition
		Cost costX = sweep(n->children, ALONG_X_AXIS);
		Cost costY = sweep(n->children, ALONG_Y_AXIS);
		float splitLine = costX.first < costY.first ? costX.second : costY.second;
		Orientation splitAxis = costX.first < costY.first ? ALONG_X_AXIS : ALONG_Y_AXIS;

		// partition children
		std::vector<RPlusTreeNode*> childrenClone = n->children;  // copy
		n->children.clear();  // clear old entries
		for (auto & child : childrenClone) {
			float leftBound = splitAxis == ALONG_X_AXIS ? child->boundingBox.upperRight.x : child->boundingBox.upperRight.y;
			float rightBound = splitAxis == ALONG_X_AXIS ? child->boundingBox.lowerLeft.x : child->boundingBox.lowerLeft.y;
			if (leftBound <= splitLine) {
				partition.first->children.push_back(child);
			} else if (rightBound >= splitLine) {
				partition.second->children.push_back(child);
			} else {
				Partition split = splitNodeAlongLine(child, splitLine, splitAxis);  // propagate changes downwards
				partition.first->children.push_back(split.first);
				split.first->parent = partition.first;
				partition.second->children.push_back(split.second);
				split.second->parent = partition.second;
			}
		}
	}

	// tighten the new nodes
	tighten(partition.first);
	tighten(partition.second);
	return partition;
}

RPlusTree::RPlusTree(unsigned int minBranchFactor, unsigned int maxBranchFactor) : minBranchFactor(minBranchFactor),
																				   maxBranchFactor(maxBranchFactor)
{
}

RPlusTree::RPlusTree(RPlusTreeNode *root)
{
	// TODO
}

RPlusTree::~RPlusTree()
{
	// TODO
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

std::vector<Point> RPlusTree::search(Rectangle requestedRectangle)
{
	// TODO
	return std::vector<Point>();
}

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

void RPlusTree::remove(Point givenPoint)
{
	// TODO
}

unsigned RPlusTree::checksum()
{
	// TODO
	return 0;
}

void RPlusTree::print()
{
	// TODO
}
