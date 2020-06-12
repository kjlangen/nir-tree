#include <rplustree/rPlusTreeNode.h>

bool RPlusTreeNode::isRoot() const
{
	return parent == nullptr;
}

bool RPlusTreeNode::isLeaf() const
{
	return children.empty();
}

unsigned int RPlusTreeNode::numChildren() const
{
	return children.size();
}

unsigned int RPlusTreeNode::numDataEntries() const
{
	return data.size();
}
