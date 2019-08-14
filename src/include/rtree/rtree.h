#ifndef __RTREE__
#define __RTREE__
#include "util/geometry.h"
#include <vector>

class Node
{
	int id;
	std::vector<Rectangle> boundingPolygons;
	std::vector<Node> children;

	public:
	std::vector<Rectangle> searchRectangle(Rectangle requestedRectangle);
	bool searchPoint(Point requestedPoint);
};

void test();

#endif
