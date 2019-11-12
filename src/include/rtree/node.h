#ifndef __NODE__
#define __NODE__
#include <cassert>
#include <vector>
#include <stack>
#include <map>
#include <list>
#include <utility>
#include <iostream>
#include <util/geometry.h>

const std::size_t PAGESIZE = 256;
const unsigned MINBRANCHFACTOR = 3;
const unsigned MAXBRANCHFACTOR = 5;

class Node
{
	public:
		unsigned id;
		std::vector<Rectangle> boundingBoxes;
		Node *parent;
		std::vector<Node *> children;
		std::vector<Point> data;

		Node();
		Node(unsigned id, Node *p=nullptr);
		Rectangle boundingBox();
		void updateBoundingBox(Node *child, Rectangle updatedBoundingBox);
		void removeChild(Node *child);
		void removeData(Point givenPoint);
		std::vector<Point> search(Rectangle requestedRectangle);
		Node *chooseLeaf(Point givenPoint);
		Node *findLeaf(Point givenPoint);
		Node *splitNode(Node *newChild);
		Node *splitNode(Point givenPoint);
		Node *adjustTree(Node *ll);
		Node *insert(Point givenPoint);
		void condenseTree();
		Node *remove(Point givenPoint);
		// Node *remove(Point givenPoint, std::map<unsigned, std::list<std::pair<Rectangle, Node *>>> &q);
		void print();
};

#endif
