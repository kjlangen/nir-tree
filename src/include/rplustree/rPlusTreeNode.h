#ifndef __RPLUSTREE_NODE__
#define __RPLUSTREE_NODE__

#include <vector>
#include <util/geometry.h>

namespace rplustree
{
	class Node
	{
		public:
			Rectangle boundingBox;
			std::vector<Node *> children;
			std::vector<Point> data;
			Node *parent = nullptr;

			// Constructors and Destructors
			Node();
			Node(const Node &other);

			// Helper functions
			bool isRoot();
			bool isLeaf();
			unsigned numChildren();
			unsigned numDataEntries();
			void tighten();

			// Miscellaneous
			Node &operator=(const Node &other);
	};
}

#endif
