#include <rtree/rtree.h>

namespace rtree
{
	RTree::RTree(unsigned minBranchFactor, unsigned maxBranchFactor)
	{
		root = new Node(*this, minBranchFactor, maxBranchFactor);
	}

	RTree::RTree(Node *root)
	{
		this->root = root;
	}

	RTree::~RTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> RTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> RTree::search(Point requestedPoint)
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> RTree::search(Rectangle requestedRectangle)
	{
		return root->search(requestedRectangle);
	}

	void RTree::insert(Point givenPoint)
	{
		root = root->insert(givenPoint);
	}

	void RTree::remove(Point givenPoint)
	{
		root = root->remove(givenPoint);
	}

	unsigned RTree::checksum()
	{
		return root->checksum();
	}

	bool RTree::validate()
	{
		return root->validate(nullptr, 0);
	}

	void RTree::stat()
	{
		root->stat();
	}

	void RTree::print()
	{
		root->printTree();
	}

	void RTree::visualize()
	{
		BMPPrinter p(1000, 1000);

		p.printToBMP(root);
	}
}
