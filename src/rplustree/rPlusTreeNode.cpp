#include <rplustree/rPlusTreeNode.h>

namespace rplustree
{
	Node::Node()
	{
		parent = nullptr;
	}

	Node::Node(const Node &other)
	{
		data = other.data;
		children = other.children;
		parent = other.parent;
	}

	bool Node::isRoot()
	{
		return parent == nullptr;
	}

	bool Node::isLeaf()
	{
		return children.empty();
	}

	unsigned Node::numChildren()
	{
		return children.size();
	}

	unsigned Node::numDataEntries()
	{
		return data.size();
	}

	void Node::tighten()
	{
		if (isLeaf())
		{
			// Reset bounding box
			boundingBox = Rectangle(data[0], data[0]);

			// Iterate through data to set new bounding box
			for (auto dataPoint : data)
			{
				boundingBox.expand(dataPoint);
			}
		}
		else
		{
			// Reset bounding box
			boundingBox = children[0]->boundingBox;
			
			// Iterate through children to set new bounding box
			for (auto &child : children)
			{
				boundingBox.expand(child->boundingBox);
			}
		}
	}

	Node &Node::operator=(const Node &other)
	{
		if (this != &other)
		{
			data = other.data;
			children = other.children;
			parent = other.parent;
		}

		return *this;
	}
}
