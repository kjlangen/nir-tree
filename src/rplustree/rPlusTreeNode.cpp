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

RPlusTreeNode::RPlusTreeNode() {
	this->parent = nullptr;
}

RPlusTreeNode::RPlusTreeNode(const RPlusTreeNode &other) {
	this->data = other.data;
	this->children = other.children;
	this->parent = other.parent;
}

RPlusTreeNode & RPlusTreeNode::operator=(const RPlusTreeNode &other) {
	if (this != &other) {
		this->data = other.data;
		this->children = other.children;
		this->parent = other.parent;
	}
	return *this;
}
