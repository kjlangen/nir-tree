#include <rplustree/rPlusTreeNode.h>

using namespace rplustree;

bool Node::isRoot() const
{
	return parent == nullptr;
}

bool Node::isLeaf() const
{
	return children.empty();
}

unsigned int Node::numChildren() const
{
	return children.size();
}

unsigned int Node::numDataEntries() const
{
	return data.size();
}

void Node::tighten()
{
	if (this->isLeaf())
	{
		// reset bounding box
		Point p = this->data.at(0);
		this->boundingBox = Rectangle(p, p);
		// iterate through data to set new bounding box
		for (auto dt : this->data)
		{
			this->boundingBox.expand(dt);
		}
	}
	else
	{
		// reset bounding box
		Rectangle r = this->children.at(0)->boundingBox;
		this->boundingBox = r;
		// iterate through children to set new bounding box
		for (auto &child : this->children)
		{
			this->boundingBox.expand(child->boundingBox);
		}
	}
}

Node::Node()
{
	this->parent = nullptr;
}

Node::Node(const Node &other)
{
	this->data = other.data;
	this->children = other.children;
	this->parent = other.parent;
}

Node &Node::operator=(const Node &other)
{
	if (this != &other)
	{
		this->data = other.data;
		this->children = other.children;
		this->parent = other.parent;
	}
	return *this;
}
