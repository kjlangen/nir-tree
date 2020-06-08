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
	// TOOD
}

Cost RPlusTree::sweep(std::vector<Rectangle>& rectangles, Orientation orientation)
{
	// TODO
	return {};
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
	// TOOD
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
