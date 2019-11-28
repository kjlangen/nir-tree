#ifndef __NODE__
#define __NODE__
#include <cassert>
#include <vector>
#include <stack>
#include <map>
#include <list>
#include <utility>
#include <cmath>
#include <iostream>
#include <util/geometry.h>

const std::size_t PAGESIZE = 256;
const unsigned MINBRANCHFACTOR = 3;
const unsigned MAXBRANCHFACTOR = 5;

class Node
{
	class ReinsertionEntry
	{
		public:
			Rectangle boundingBox;
			Point data;
			Node *child;
			unsigned level;
	};

	public:
		unsigned id;
		Node *parent;
		std::vector<Rectangle> boundingBoxes;
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
		Node *chooseNode(ReinsertionEntry e);
		Node *findLeaf(Point givenPoint);
		Node *splitNode(Node *newChild);
		Node *splitNode(Point newData);
		Node *adjustTree(Node *siblingLeaf);
		Node *insert(Point givenPoint);
		Node *insert(ReinsertionEntry e);
		Node *condenseTree();
		Node *remove(Point givenPoint);

		void print();
};

void testBoundingBox();
void testUpdateBoundingBox();
void testRemoveChild();
void testRemoveData();
void testChooseLeaf();
void testFindLeaf();
void testSplitNode();
void testAdjustTree();
void testCondenseTree();
void testSearch();
void testInsert();
void testRemove();

#endif
