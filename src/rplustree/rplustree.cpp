#include <rplustree/rplustree.h>

namespace rplustree
{
	RPlusTree::RPlusTree(unsigned minBranchFactor, unsigned maxBranchFactor)
	{
		root = new Node(minBranchFactor, maxBranchFactor);
	}

	RPlusTree::RPlusTree(Node *root)
	{
		this->root = root;
	}

	RPlusTree::~RPlusTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> RPlusTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> RPlusTree::search(Point requestedPoint)
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> RPlusTree::search(Rectangle requestedRectangle)
	{
		return root->search(requestedRectangle);
	}

	void RPlusTree::insert(Point givenPoint)
	{
		root = root->insert(givenPoint);
	}

	void RPlusTree::remove(Point givenPoint)
	{
		root = root->remove(givenPoint);
	}

	unsigned RPlusTree::checksum()
	{
		return root->checksum();
	}

	void RPlusTree::stat()
	{
		root->stat();
	}

	void RPlusTree::print()
	{
		BMPPrinter p(10000, 10000);

		p.printToBMP(root);
		// root->printTree();
	}

	void RPlusTree::pencilPrint()
	{
		PencilPrinter p;

		p.printToPencil(root);
	}
}
