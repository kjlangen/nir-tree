#include <rstartree/rstartree.h>

namespace rstartree
{
	RStarTree::RStarTree(unsigned minBranchFactor, unsigned maxBranchFactor)
	{
		root = new RStarTreeNode(minBranchFactor, maxBranchFactor);
	}

	RStarTree::RStarTree(RStarTreeNode *root)
	{
		this->root = root;
	}

	RStarTree::~RStarTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> RStarTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> RStarTree::search(Point requestedPoint)
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> RStarTree::search(Rectangle requestedRectangle)
	{
		return root->search(requestedRectangle);
	}

	void RStarTree::insert(Point givenPoint)
	{
		root = root->insert(givenPoint);
	}

	void RStarTree::remove(Point givenPoint)
	{
		root = root->remove(givenPoint);
	}

	unsigned RStarTree::checksum()
	{
		return root->checksum();
	}

	void RStarTree::print()
	{
		root->printTree();
	}
}
