#ifndef __BTREE__
#define __BTREE__

namespace btree
{
	struct Node
	{
		int key;
		Node *left;
		Node *right;
	};

	void insert(Node &root, Node &newLeaf);
}

#endif
