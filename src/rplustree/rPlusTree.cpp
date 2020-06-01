#include <rplustree/rPlusTree.h>

void RPlusTree::removeSubtree(RPlusTreeNode *r)
{
	// TODO
}

RPlusTreeNode *RPlusTree::chooseLeaf(Point givenPoint)
{
	// TODO
	return nullptr;
}

Cost RPlusTree::sweep(RPlusTree::Orientation orientation, Point lowestCoordinates, float fillFactor)
{
	// TODO
	return {};
}

Partition RPlusTree::partition(RPlusTreeNode *node)
{
	// TODO
	return std::pair<std::vector<Rectangle>, std::vector<Rectangle>>();
}

void RPlusTree::splitNode(RPlusTreeNode *node)
{
	// TODO
	Partition partition = this->partition(node);
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
