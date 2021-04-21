#include <revisedrstartree/revisedrstartree.h>

namespace revisedrstartree
{
	RevisedRStarTree::RevisedRStarTree(unsigned minBranchFactor, unsigned maxBranchFactor) : minBranchFactor(minBranchFactor), maxBranchFactor(maxBranchFactor)
	{
		root = new Node(*this);
	}

	RevisedRStarTree::~RevisedRStarTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> RevisedRStarTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> RevisedRStarTree::search(Point requestedPoint) CONST_IF_NOT_STAT
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> RevisedRStarTree::search(Rectangle requestedRectangle) CONST_IF_NOT_STAT
	{
		return root->search(requestedRectangle);
	}

	void RevisedRStarTree::insert(Point givenPoint)
	{
		root = root->insert(givenPoint);
	}

	void RevisedRStarTree::remove(Point givenPoint)
	{
		root = root->remove(givenPoint);
	}

	unsigned RevisedRStarTree::checksum()
	{
		return root->checksum();
	}

	bool RevisedRStarTree::validate()
	{
		return root->validate(nullptr, 0);
	}

	void RevisedRStarTree::stat()
	{
		root->stat();
	}

	void RevisedRStarTree::print()
	{
		root->printTree();
	}

	void RevisedRStarTree::visualize()
	{
		BMPPrinter p(1000, 1000);

		p.printToBMP(root);
	}
}
