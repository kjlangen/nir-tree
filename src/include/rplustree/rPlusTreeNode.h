#ifndef __RPLUSTREE_NODE__
#define __RPLUSTREE_NODE__

#include <vector>
#include <util/geometry.h>

namespace RPlusTree
{
	class Node
	{
	public:
		Rectangle boundingBox;
		std::vector<Node *> children;
		std::vector<Point> data;
		Node *parent = nullptr;

		bool isRoot() const;

		bool isLeaf() const;

		unsigned int numChildren() const;

		unsigned int numDataEntries() const;

		void tighten();

		Node();

		Node(const Node &other);

		Node &operator=(const Node &other);
	};
}

#endif // __RPLUSTREE_NODE__
