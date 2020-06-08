#include <rplustree/rPlusTreeNode.h>

RPlusTreeNode::RPlusTreeNode(bool isDataNode): isDataNode(isDataNode)
{
}

bool RPlusTreeNode::isRoot() const
{
	return parent == nullptr;
}

bool RPlusTreeNode::isLeaf() const
{
	if (children.empty()) {
		return true;
	}
	return children.at(0)->isDataNode;
}

unsigned int RPlusTreeNode::numChildren() const
{
	return children.size();
}
