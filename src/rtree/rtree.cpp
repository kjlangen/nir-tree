#include <rtree/rtree.h>

RTree::RTree(unsigned minBranchFactor, unsigned maxBranchFactor)
{
	root = new Node(minBranchFactor, maxBranchFactor);
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

std::vector<Point> RTree::search(Rectangle requestedRectangle)
{
	std::vector<Point> v;
	root->search(requestedRectangle, v);

	return v;
}

void RTree::insert(Point givenPoint)
{
	root = root->insert(givenPoint);
}

void RTree::remove(Point givenPoint)
{
	root = root->remove(givenPoint);
	assert(root != nullptr);
}

void RTree::print()
{
	std::cout << "-----------------------------------------------" << std::endl;
	root->print();
	for (int i = 0; i < root->children.size(); ++i)
	{
		std::cout << "-----------------------------------------------" << std::endl;
		root->children[i]->print();
	}
}
