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
		bool operator!=(Point p);

		void print();
};

class Rectangle
{
	public:
		Point centre;
		double radiusX;
		double radiusY;

		Rectangle(double x, double y, double radiusX, double radiusY);
		double area();
		double computeExpansionArea(Point givenPoint);
		double computeExpansionArea(Rectangle requestedRectangle);
		void expand(Point givenPoint);
		void expand(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle requestedRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> splitRectangle(Rectangle clippingRectangle);

		bool operator==(Rectangle r);
		bool operator!=(Rectangle r);

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

void testPointEquality();
void testRectangleArea();
void testRectangleComputeExpansionArea();
void testRectangleExpansion();
void testRectangleIntersection();
void testRectanglePointContainment();
void testRectangleSplits();

#endif
