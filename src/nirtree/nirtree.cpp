#include <nirtree/nirtree.h>

namespace nirtree
{
	NIRTree::NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor) :
		minBranchFactor(minBranchFactor), maxBranchFactor(maxBranchFactor)
	{
		root = new Node(*this, nullptr, 0);
		hasReinsertedOnLevel = {false};
	}

	NIRTree::~NIRTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> NIRTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> NIRTree::search(Point requestedPoint) CONST_IF_NOT_STAT
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> NIRTree::search(Rectangle requestedRectangle) CONST_IF_NOT_STAT
	{
		return root->search(requestedRectangle);
	}

	void NIRTree::insert(Point givenPoint)
	{
		std::fill(hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false);
		root->insert(givenPoint, hasReinsertedOnLevel);

		// Cleanup
		for (Node *g : garbage)
		{
			delete g;
		}
		garbage.clear();
	}

	void NIRTree::remove(Point givenPoint)
	{
		std::fill(hasReinsertedOnLevel.begin(), hasReinsertedOnLevel.end(), false);
		root = root->remove(givenPoint);

		// Cleanup
		for (Node *g : garbage)
		{
			delete g;
		}
		garbage.clear();
	}

	unsigned NIRTree::checksum()
	{
		return root->checksum();
	}

	bool NIRTree::validate()
	{
		return root->validate(nullptr, 0);
	}

	void NIRTree::stat()
	{
		root->stat();
	}

	void NIRTree::print()
	{
		root->printTree();
	}

	void NIRTree::visualize()
	{
		BMPPrinter p(1000, 1000);

		p.printToBMP(root);
	}
}
