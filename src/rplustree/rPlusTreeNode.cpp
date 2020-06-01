#include <rplustree/rPlusTreeNode.h>

bool RPlusTreeNode::isLeaf() const
{
	// assert(children.empty() && boundingBoxes.empty());
	return children.empty();
}

unsigned int RPlusTreeNode::numEntries() const
{
	// assert(children.size() == boundingBoxes.size());
	return children.size();
}
