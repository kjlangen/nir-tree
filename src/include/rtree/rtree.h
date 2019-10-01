#ifndef __RTREE__
#define __RTREE__
#include "util/geometry.h"
#include <cassert>
#include <vector>
#include <stack>
#include <iostream>

class Node
{
	public:
	unsigned id;
	Node *parent;
	std::vector<Rectangle> boundingPolygons;
	std::vector<Node *> children;

	Node();
	Node(unsigned id);
	void setParent(Node *parent);
	void addPolygon(Rectangle newRectangle);
	void addChild(Node *newChild);
	Rectangle computeBoundary();
	std::vector<Rectangle> searchRectangle(Rectangle requestedRectangle);
	bool searchPoint(Point requestedPoint);
	Node *splitNode();
	Node *insert(Rectangle newRectangle);
	void print();
};

class RTree
{
	Node *root;
	public:
	RTree();
	RTree(Node *root);
	std::vector<Rectangle> searchRectangle(Rectangle requestedRectangle);
	bool searchPoint(Point requestedPoint);
	void insert(Rectangle newRectangle);
};


void testSimpleSearch();
void testSimpleInsert();
void expandRootTest();

#endif
