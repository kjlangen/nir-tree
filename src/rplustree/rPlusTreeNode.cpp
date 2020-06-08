#include <rplustree/rPlusTreeNode.h>

bool RPlusTreeNode::isLeaf() const
{
	return children.empty();
}

unsigned int RPlusTreeNode::numEntries() const
{
	return data.size();
}


unsigned int RPlusTreeNode::numChildren() const
{
	return children.size();
}

bool RPlusTreeNode::isRoot() const
{
	return parent == nullptr;
}
