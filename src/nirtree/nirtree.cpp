#include <nirtree/nirtree.h>

namespace nirtree
{
	NIRTree::NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor)
	{
		root = new Node(minBranchFactor, maxBranchFactor);
	}

	NIRTree::NIRTree(Node *root)
	{
		this->root = root;
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

	std::vector<Point> NIRTree::search(Point requestedPoint)
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> NIRTree::search(Rectangle requestedRectangle)
	{
		return root->search(requestedRectangle);
	}

	void NIRTree::insert(Point givenPoint)
	{
		root = root->insert(givenPoint);
	}

	void NIRTree::remove(Point givenPoint)
	{
		root = root->remove(givenPoint);
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
		BMPPrinter p(10000, 10000);

		p.printToBMP(root);
	}
}
