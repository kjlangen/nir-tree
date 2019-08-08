#ifndef __RTREE__
#define __RTREE__
#include <vector>

namespace rtree
{
	struct Point
	{
		int x;
		int y;
	};

	struct Rectangle
	{
		Point lowerLeft;
		Point upperRight;
	};

	struct NavNode
	{
		Rectangle boundingBox;
		Node *child;
	};

	struct Node
	{
		std::vector<NavNode> children;
	};

	void insert(Node &root, Node &newLeaf);
}

#endif
