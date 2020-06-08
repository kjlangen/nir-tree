#include <rplustree/rPlusTree.h>

void RPlusTree::removeSubtree(RPlusTreeNode *r)
{
	// TODO
}

void RPlusTree::tighten(RPlusTreeNode* n)
{
	// TODO
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

Cost RPlusTree::sweep(std::vector<Rectangle>& rectangles, Orientation orientation)
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

Partition RPlusTree::splitNode(RPlusTreeNode* n)
{
	// TOOD
	return {};
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
		leaf->data.push_back(givenRectangle);
		// need to split leaf node
		if (leaf->numEntries() > maxBranchFactor) {
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
