#ifndef __GEOMETRY__
#define __GEOMETRY__
#include <iostream>
#include <algorithm>
#include <cassert>
#include <vector>
#include <cmath>

class Point
{
	public:
		double x;
		double y;

		Point();
		Point(double x, double y);
		bool operator==(Point p);
		void print();
};

class Rectangle
{
	public:
		Point centre;
		double radiusX;
		double radiusY;

		Rectangle(double x, double y, double radiusX, double radiusY);
		double computeExpansionArea(Rectangle requestedRectangle);
		double computeExpansionArea(Point givenPoint);
		void expand(Point givenPoint);
		bool intersectsRectangle(Rectangle requestedRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> splitRectangle(Rectangle clippingRectangle);
		void print();
};

class DynamicRectangle
{
	public:
		std::vector<Rectangle> boundingBoxes;

		DynamicRectangle();
		double computeExpansionArea();
		bool intersectsRectangle();
};

#endif
