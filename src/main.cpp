#include <iostream>
#include "btree/btree.h"
#include "rtree/rtree.h"

int main(int argc, char const *argv[])
{
	btree::Node root = { 24, nullptr, nullptr };
	btree::Node newLeaf = { 16, nullptr, nullptr };
	rtree::Node newNewLeaf = { { {1, 1}, {4, 4} }, std::vector<rtree::Node>() };
	btree::Node threeLeaf = { 20, nullptr, nullptr };

	btree::insert(root, newLeaf);
	btree::insert(root, threeLeaf);

	std::cout << "Yup it runs." << std::endl;
	std::cout << "In fact, the root's navkey = " << root.key << std::endl;

	int leftLeafKey = root.left->key;
	int betweenLeafKey = root.left->right->key;
	std::cout << leftLeafKey << std::endl;
	std::cout << betweenLeafKey << std::endl;

	return 0;
}
