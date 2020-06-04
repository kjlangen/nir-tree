#include <rplustree/rPlusTree.h>

void RPlusTree::removeSubtree(RPlusTreeNode *r)
{
	// TODO
}

RPlusTreeNode *RPlusTree::chooseLeaf(Rectangle givenRectangle)
{
	// CL1 [Initialize]
	RPlusTreeNode *node = root;
	for (;;)
	{
		// CL2 [Leaf check]
		if (node->isLeaf())
		{
			return node;
		} else
		{
			// CL3 [Choose subtree]
			// Find the bounding box with least required expansion/overlap?
			// TODO: Break ties by using smallest area
			unsigned smallestExpansionIndex = 0;
			float smallestExpansionArea = node->boundingBoxes[0].computeExpansionArea(givenRectangle);
			for (unsigned i = 0; i < node->boundingBoxes.size(); ++i)
			{
				float testExpansionArea = node->boundingBoxes[i].computeExpansionArea(givenRectangle);
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

Cost RPlusTree::sweep(std::vector<Rectangle> &rectangles, Orientation orientation)
{
	std::sort(rectangles.begin(), rectangles.end(),
			  [orientation](Rectangle const &r1, Rectangle const &r2) -> bool
			  {
				  if (orientation == Orientation::ALONG_X_AXIS)
				  {
					  return r1.lowerLeft.x < r2.lowerLeft.x;
				  }
				  return r1.lowerLeft.y < r2.lowerLeft.y;
			  }
	);

	float splitLine;
	float cost = 0.0f;

	// For now, split using middle element and count the number of rectangles that
	// intersect with the chosen split
	if (orientation == Orientation::ALONG_X_AXIS)
	{
		splitLine = rectangles.at(rectangles.size() / 2).upperRight.x;
		for (auto &rect : rectangles)
		{
			if (rect.lowerLeft.x < splitLine && splitLine < rect.upperRight.x)
			{
				cost += 1.0f;
			}
		}
	} else
	{
		splitLine = rectangles.at(rectangles.size() / 2).upperRight.y;
		for (auto &rect : rectangles)
		{
			if (rect.lowerLeft.y < splitLine && splitLine < rect.upperRight.y)
			{
				cost += 1.0f;
			}
		}
	}

	return {cost, splitLine};
}

Partition RPlusTree::splitNodeDownwards(RPlusTreeNode *node, RPlusTree::Orientation orientation, float splitLine)
{
	Partition result;

	// same code as case 3 in `splitNode` function
	for (int i = 0; i < node->numChildren(); i++)
	{
		Rectangle rect = node->boundingBoxes.at(i);
		RPlusTreeNode *child = node->children.at(i);
		if (rect.upperRight.x < splitLine)
		{
			// lies completely in the left subspace
			result.leftNode->boundingBoxes.push_back(rect);
			result.leftNode->children.push_back(child);
			result.leftBoundingBox.expand(rect);
		} else if (rect.lowerLeft.x > splitLine)
		{
			// lies completely in the right subspace
			result.rightNode->boundingBoxes.push_back(rect);
			result.rightNode->children.push_back(child);
			result.rightBoundingBox.expand(rect);
		} else
		{
			// lies across the left and right subspace
			Partition partition = splitNodeDownwards(child, orientation, splitLine);
			// copy data from left partition
			result.leftNode->boundingBoxes.push_back(partition.leftBoundingBox);
			result.leftNode->children.push_back(partition.leftNode);
			result.leftBoundingBox.expand(partition.leftBoundingBox);
			// copy data from right partition
			result.rightNode->boundingBoxes.push_back(partition.rightBoundingBox);
			result.rightNode->children.push_back(partition.rightNode);
			result.rightBoundingBox.expand(partition.rightBoundingBox);
		}
	}
	return result;
}

void RPlusTree::splitNode(RPlusTreeNode *node, RPlusTree::Orientation orientation, float splitLine)
{
	auto *leftNode = new RPlusTreeNode();
	auto leftBoundingBox = Rectangle();

	auto *rightNode = new RPlusTreeNode();
	auto rightBoundingBox = Rectangle();

	if (node->isLeaf())
	{
		// Note use of `data` field
		for (auto &rect : node->data)
		{
			if (rect.upperRight.x < splitLine)
			{
				// lies completely in the left subspace
				leftNode->data.push_back(rect);
				leftBoundingBox.expand(rect);
			} else if (rect.lowerLeft.x > splitLine)
			{
				// lies completely in the right subspace
				rightNode->data.push_back(rect);
				rightBoundingBox.expand(rect);
			} else
			{
				// lies across the left and right subspace
				// insert to both sides according to paper
				leftNode->data.push_back(rect);
				leftBoundingBox.expand(rect);
				rightNode->data.push_back(rect);
				rightBoundingBox.expand(rect);
			}
		}
	} else
	{	// is an intermediary node
		// Note call to `numChildren`
		for (int i = 0; i < node->numChildren(); i++)
		{
			Rectangle rect = node->boundingBoxes.at(i);
			RPlusTreeNode *child = node->children.at(i);
			if (rect.upperRight.x < splitLine)
			{
				// lies completely in the left subspace
				leftNode->boundingBoxes.push_back(rect);
				leftNode->children.push_back(child);
				leftBoundingBox.expand(rect);
			} else if (rect.lowerLeft.x > splitLine)
			{
				// lies completely in the right subspace
				rightNode->boundingBoxes.push_back(rect);
				rightNode->children.push_back(child);
				rightBoundingBox.expand(rect);
			} else
			{
				// lies across the left and right subspace
				Partition partition = splitNodeDownwards(child, orientation, splitLine);
				// copy data from left partition
				leftNode->boundingBoxes.push_back(partition.leftBoundingBox);
				leftNode->children.push_back(partition.leftNode);
				leftBoundingBox.expand(partition.leftBoundingBox);
				// copy data from right partition
				rightNode->boundingBoxes.push_back(partition.rightBoundingBox);
				rightNode->children.push_back(partition.rightNode);
				rightBoundingBox.expand(partition.rightBoundingBox);
			}
		}
	}

	if (node->isRoot())
	{
		auto *newRoot = new RPlusTreeNode();
		newRoot->boundingBoxes.push_back(leftBoundingBox);
		newRoot->boundingBoxes.push_back(rightBoundingBox);
		newRoot->parent->children.push_back(leftNode);
		newRoot->parent->children.push_back(rightNode);
		root = newRoot;  // set as new root, end of upwards recursion
	} else
	{
		// Find existing entry's index
		int nodeIndex = 0;
		for (int i = 0; i < node->parent->children.size(); i++)
		{
			if (node->parent->children.at(i) == node)
			{
				nodeIndex = i;
				break;
			}
		}

		// update parent's bounding boxes vector
		node->parent->boundingBoxes.erase(node->parent->boundingBoxes.begin() + nodeIndex);
		node->parent->boundingBoxes.push_back(leftBoundingBox);
		node->parent->boundingBoxes.push_back(rightBoundingBox);

		// update parent's nodes vector
		delete node->parent->children.at(nodeIndex);
		node->parent->children.erase(node->parent->children.begin() + nodeIndex);
		node->parent->children.push_back(leftNode);
		node->parent->children.push_back(rightNode);

		// propagate changes upwards if needed
		if (node->parent->numChildren() > maxBranchFactor)
		{
			splitNode(node->parent, orientation, splitLine);
		}
	}
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

std::vector<Point> RPlusTree::search(Point requestedPoint)
{
	// TODO
	return std::vector<Point>();
}

std::vector<Point> RPlusTree::search(Rectangle requestedRectangle)
{
	// TODO
	return std::vector<Point>();
}

void RPlusTree::insert(Point givenPoint)
{
	// TODO
}

void RPlusTree::insert(Rectangle givenRectangle)
{
	// Add data to chosen leaf node, begin splitNode process if necessary
	RPlusTreeNode *leaf = chooseLeaf(givenRectangle);
	leaf->data.push_back(givenRectangle);
	if (leaf->numEntries() > maxBranchFactor)
	{
		Cost costX = sweep(leaf->data, Orientation::ALONG_X_AXIS);
		Cost costY = sweep(leaf->data, Orientation::ALONG_Y_AXIS);
		if (costX.first < costY.first)
		{
			splitNode(leaf, Orientation::ALONG_X_AXIS, costX.second);
		} else
		{
			splitNode(leaf, Orientation::ALONG_Y_AXIS, costY.second);
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
