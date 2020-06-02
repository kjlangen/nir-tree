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

Partition RPlusTree::partition(std::vector<Rectangle> &rectangles)
{
	// PA1 [No Partition Required], Not applicable in this implementation

	// PA2 [Compute Lowest x- and y- Values]
	float lowestX = rectangles.at(0).lowerLeft.x;
	float lowestY = rectangles.at(0).lowerLeft.y;

	// PA3 [Sweep Along the x-dimension]
	Cost costX = sweep(rectangles, RPlusTree::Orientation::ALONG_X_AXIS);

	// PA4 [Sweep Along the y-dimension]
	Cost costY = sweep(rectangles, RPlusTree::Orientation::ALONG_Y_AXIS);

	// PA5 [Choose a Partition Point]

	return std::pair<std::vector<Rectangle>, std::vector<Rectangle>>();
}

void RPlusTree::splitNode(RPlusTreeNode *node)
{
	// SN1 [Find a Partition]
	Partition partition = this->partition(node->boundingBoxes);

	// SN2 [Populate New Nodes]


	// SN3 [Propagate Node Split Upward]

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
	RPlusTreeNode *leaf = chooseLeaf(givenRectangle);
	leaf->boundingBoxes.push_back(givenRectangle);
	auto *newNode = new RPlusTreeNode();
	leaf->children.push_back(newNode);
	newNode->parent = leaf;

	if (leaf->numEntries() > maxBranchFactor)
	{
		splitNode(leaf);
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
