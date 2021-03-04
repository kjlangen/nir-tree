#include <rstartree/rstartree.h>

namespace rstartree
{
	RStarTree::RStarTree(unsigned minBranchFactor, unsigned maxBranchFactor) : minBranchFactor(minBranchFactor), maxBranchFactor(maxBranchFactor)
	{
		hasReinsertedOnLevel = {false};
		root = new Node(*this);
		root->level = 0;
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

	std::vector<Point> RStarTree::search(Point requestedPoint) CONST_IF_NOT_STAT
	{
		assert(root->parent == nullptr);

		return root->search(requestedPoint);
	}

	std::vector<Point> RStarTree::search(Rectangle requestedRectangle) CONST_IF_NOT_STAT
	{
		return root->search(requestedRectangle);
	}

	void RStarTree::insert(Point givenPoint)
	{
		assert(root->parent == nullptr);

		std::fill(hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false);
		root = root->insert(givenPoint, hasReinsertedOnLevel);
	}

	void RStarTree::remove(Point givenPoint)
	{
		std::fill(hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false);
		root = root->remove(givenPoint, hasReinsertedOnLevel);
        assert(root->parent == nullptr);
	}

	unsigned RStarTree::checksum()
	{
		return root->checksum();
	}

	void RStarTree::print()
	{
		root->printTree();
	}

	bool RStarTree::validate()
	{
		return true;
	}

	void RStarTree::stat()
	{
		root->stat();
	}

	void RStarTree::visualize()
	{
		BMPPrinter p(1000, 1000);

		p.printToBMP(root);
	}
}
