#ifndef __GEOMETRY__
#define __GEOMETRY__
#include <iostream>
#include <algorithm>
#include <cassert>
#include <vector>
#include <cmath>
#include <limits>
#include <cfenv>

class Point
{
	public:
		float x;
		float y;

		Point();
		Point(float x, float y);
		bool operator<(Point p);
		bool operator>(Point p);
		bool operator<=(Point p);
		bool operator>=(Point p);
		bool operator==(Point p);
		bool operator!=(Point p);

		void print();
};

class Rectangle
{
	public:
		Point centre;
		float radiusX;
		float radiusY;

		Rectangle();
		Rectangle(float x, float y, float radiusX, float radiusY);
		Rectangle(Point centre, float radiusX, float radiusY);
		float area();
		float computeExpansionArea(Point givenPoint);
		float computeExpansionArea(Rectangle requestedRectangle);
		void expand(Point givenPoint);
		void expand(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle requestedRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> fragmentRectangle(Rectangle clippingRectangle);

		bool operator==(Rectangle r);
		bool operator!=(Rectangle r);

		void print();
};

class DynamicRectangle
{
	public:
		std::vector<Rectangle> boundingBoxes;

		DynamicRectangle();
		float computeExpansionArea();
		bool intersectsRectangle();
};

void testPointEquality();
void testRectangleArea();
void testRectangleComputeExpansionArea();
void testRectangleExpansion();
void testRectangleIntersection();
void testRectanglePointContainment();
void testRectangleFragmentation();

#endif
