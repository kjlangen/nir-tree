#include <quadtree/quadtree.h>

namespace quadtree
{
	QuadTree::QuadTree()
	{
		root = nullptr;

		// Compute 2^d only once so we can iterate over all possible quadrants
		quadrants = 1;
		for (unsigned d = 0; d < dimensions; ++d)
		{
			quadrants *= 2;
		}

		assert(quadrants == 8);
	}

	QuadTree::QuadTree(Node *root)
	{
		this->root = root;
	}

	QuadTree::~QuadTree()
	{
		root->deleteSubtrees();
		delete root;
	}

	std::vector<Point> QuadTree::exhaustiveSearch(Point requestedPoint)
	{
		std::vector<Point> v;
		root->exhaustiveSearch(requestedPoint, v);

		return v;
	}

	std::vector<Point> QuadTree::search(Point requestedPoint) CONST_IF_NOT_STAT
	{
		return root->search(requestedPoint);
	}

	std::vector<Point> QuadTree::search(Rectangle requestedRectangle) CONST_IF_NOT_STAT
	{
		return root->search(requestedRectangle);
	}

	void QuadTree::insert(Point givenPoint)
	{
		// Root special case
		if (root == nullptr)
		{
			root = new Node(*this, givenPoint);
		}
		else
		{
			root->insert(givenPoint);
		}
	}

	void QuadTree::remove(Point givenPoint)
	{
		root->remove(givenPoint);
	}

	unsigned QuadTree::checksum()
	{
		return root->checksum();
	}

	bool QuadTree::validate()
	{
		return root->validate(nullptr);
	}

	void QuadTree::stat()
	{
		root->stat();
	}

	void QuadTree::print()
	{
		root->printTree();
	}

	void QuadTree::visualize()
	{
	}
}
