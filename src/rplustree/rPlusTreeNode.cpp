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

void RPlusTreeNode::tighten() {
	if (this->isLeaf()) {
		// reset bounding box
		Point p = this->data.at(0);
		this->boundingBox = Rectangle(p, p);
		// iterate through data to set new bounding box
		for (auto dt : this->data) {
			this->boundingBox.expand(dt);
		}
	} else {
		// reset bounding box
		Rectangle r = this->children.at(0)->boundingBox;
		this->boundingBox = r;
		// iterate through children to set new bounding box
		for (auto & child : this->children) {
			this->boundingBox.expand(child->boundingBox);
		}
	}
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
