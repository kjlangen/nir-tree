#include <rplustree/rPlusTree.h>

void RPlusTree::removeSubtree(RPlusTreeNode *r)
{
	// TODO
}

void RPlusTree::tighten(RPlusTreeNode* n)
{
	// no need to tighten bounding box
	if (n->isDataNode) {
		return;
	}

	// reset bounding box
	n->boundingBox.lowerLeft.x = 0;
	n->boundingBox.lowerLeft.y = 0;
	n->boundingBox.upperRight.x = 0;
	n->boundingBox.upperRight.y = 0;

	// iterate through children to set new bounding box
	for (auto & child : n->children) {
		n->boundingBox.expand(child->boundingBox);
	}
}

void RPlusTree::adjustTree(RPlusTreeNode* n, RPlusTreeNode* nn)
{
	if (n->isRoot()) {
		if (nn != nullptr) {
			root = new RPlusTreeNode(false);
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

void RPlusTree::chooseLeaves(RPlusTreeNode* node, Rectangle& givenRectangle, std::vector<RPlusTreeNode*>& leaves)
{
	if (node->isLeaf()) {
		leaves.push_back(node);
		return;
	}

	bool overlap = false;
	for (auto & child : node->children) {
		if (child->boundingBox.computeExpansionArea(givenRectangle) == 0.0f) {
			chooseLeaves(child, givenRectangle, leaves);
			overlap = true;
		}
	}

	if (!overlap) {
		// Find the bounding box with least required expansion/overlap
		RPlusTreeNode* chosenChild = node->children.at(0);
		float smallestExpansionArea = node->children.at(0)->boundingBox.computeExpansionArea(givenRectangle);
		for (auto & child : node->children) {
			float testExpansionArea = child->boundingBox.computeExpansionArea(givenRectangle);
			if (smallestExpansionArea > testExpansionArea) {
				smallestExpansionArea = testExpansionArea;
			}
		}
		chooseLeaves(chosenChild, givenRectangle, leaves);
	}
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
	// duplicate data values
	if (n->isDataNode) {
		auto* dataClone = new RPlusTreeNode(true);
		if (splitAxis == ALONG_X_AXIS) {
			n->boundingBox = Rectangle(
				n->boundingBox.lowerLeft.x, n->boundingBox.lowerLeft.y,
				splitLine, n->boundingBox.upperRight.y
			);
			dataClone->boundingBox = Rectangle(
				splitLine, n->boundingBox.lowerLeft.y,
				n->boundingBox.upperRight.x, n->boundingBox.upperRight.y
			);
		} else {
			n->boundingBox = Rectangle(
				n->boundingBox.lowerLeft.x, n->boundingBox.lowerLeft.y,
				n->boundingBox.upperRight.x, splitLine
			);
			dataClone->boundingBox = Rectangle(
				n->boundingBox.lowerLeft.x, splitLine,
				n->boundingBox.upperRight.x, n->boundingBox.upperRight.y
			);
		}
		return {n, dataClone};
	}

	// otherwise partition children, recurse downwards if needed
	// note similarity of code with `splitNode` function
	Partition partition;
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
	return partition;
}


Partition RPlusTree::splitNode(RPlusTreeNode* n)
{
	// create new node and set parameters
	auto* newNode = new RPlusTreeNode(false);
	Partition partition = {n, newNode};
	newNode->parent = n->parent;
	if (newNode->parent != nullptr) {
		newNode->parent->children.push_back(newNode);
	}

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

	// tighten newly created node
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

bool RPlusTree::exists(Rectangle requestedRectangle) {
	RPlusTreeNode * n = root;
	while (!n->isLeaf()) {
		bool childFound = false;
		for (auto & child : n->children) {
			if (child->boundingBox.intersectsRectangle(requestedRectangle)) {
				n = child;
				childFound = true;
				break;
			}
		}
		if (!childFound) {
			return false;
		}
	}

	for (auto & data : n->children) {
		if (data->boundingBox == requestedRectangle) {
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

void RPlusTree::insert(Rectangle givenRectangle)
{
	// choose the leaves where the data will go
	std::vector<RPlusTreeNode*> leaves;
	chooseLeaves(root, givenRectangle, leaves);

	for (auto & leaf : leaves) {
		auto* newDataNode = new RPlusTreeNode(true);
		newDataNode->boundingBox = givenRectangle;
		leaf->children.push_back(newDataNode);
		// need to split leaf node
		if (leaf->numChildren() > maxBranchFactor) {
			Partition split = splitNode(leaf);
			adjustTree(split.first, split.second);
		} else {
			adjustTree(leaf, nullptr);
		}
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
