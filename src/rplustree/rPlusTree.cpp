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
	// TODO
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
	std::sort(nodeList.begin(), nodeList.end(),
			  [orientation](const RPlusTreeNode * n1, const RPlusTreeNode * n2) -> bool
			  {
				  if (orientation == Orientation::ALONG_X_AXIS)
				  {
					  return n1->boundingBox.lowerLeft.x < n2->boundingBox.lowerLeft.x;
				  }
				  return n1->boundingBox.lowerLeft.y < n2->boundingBox.lowerLeft.y;
			  }
	);

	float splitLine;
	float cost = 0.0f;

	// For now, split using middle element and count the number of rectangles that
	// intersect with the chosen split
	if (orientation == Orientation::ALONG_X_AXIS)
	{
		splitLine = nodeList.at(nodeList.size() / 2)->boundingBox.upperRight.x;
		for (auto * node : nodeList)
		{
			if (node->boundingBox.lowerLeft.x < splitLine && splitLine < node->boundingBox.upperRight.x)
			{
				cost += 1.0f;
			}
		}
	} else
	{
		splitLine = nodeList.at(nodeList.size() / 2)->boundingBox.upperRight.y;
		for (auto * node : nodeList)
		{
			if (node->boundingBox.lowerLeft.y < splitLine && splitLine < node->boundingBox.upperRight.y)
			{
				cost += 1.0f;
			}
		}
	}

	return {cost, splitLine};
}

Partition RPlusTree::splitNodeAlongLine(RPlusTreeNode *n, float splitLine, RPlusTree::Orientation splitAxis)
{
	// duplicate data values
	if (n->isDataNode) {
		auto* dataClone = new RPlusTreeNode(true);
		dataClone->boundingBox = n->boundingBox;
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
