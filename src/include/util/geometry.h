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
		Point lowerLeft;
		Point upperRight;

		Rectangle();
		Rectangle(float x, float y, float xp, float yp);
		Rectangle(Point lowerLeft, Point upperRight);
		float area();
		float computeIntersectionArea(Rectangle givenRectangle);
		float computeExpansionArea(Point givenPoint);
		float computeExpansionArea(Rectangle givenRectangle);
		void expand(Point givenPoint);
		void expand(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle givenRectangle);
		bool strictIntersectsRectangle(Rectangle givenRectangle);
		bool containsPoint(Point requestedPoint);
		Rectangle intersectionRectangle(Rectangle clippingRectangle);
		std::vector<Rectangle> fragmentRectangle(Rectangle clippingRectangle);

		bool operator==(Rectangle r);
		bool operator!=(Rectangle r);

		void print();
};

class IsotheticPolygon
{
	public:
		std::vector<Rectangle> basicRectangles;

		IsotheticPolygon();
		IsotheticPolygon(Rectangle baseRectangle);
		IsotheticPolygon(const IsotheticPolygon &basePolygon);
		float area();
		float computeIntersectionArea(Rectangle givenRectangle);
		float computeExpansionArea(Point givenPoint);
		float computeExpansionArea(Rectangle givenRectangle);
		Rectangle boundingBox();
		void expand(Point givenPoint);
		void expand(Point givenPoint, IsotheticPolygon &constraintPolygon);
		void expand(Rectangle givenRectangle);
		void expand(IsotheticPolygon &targetPolygon, IsotheticPolygon &constraintPolygon);
		bool intersectsRectangle(Rectangle &givenRectangle);
		bool intersectsRectangle(IsotheticPolygon &givenPolygon);
		bool containsPoint(Point requestedPoint);
		void intersection(IsotheticPolygon &constraintPolygon);
		void increaseResolution(Rectangle clippingRectangle);
		void increaseResolution(IsotheticPolygon &clippingPolygon);

		bool operator==(IsotheticPolygon r);
		bool operator!=(IsotheticPolygon r);

		void print();
};

void testPointEquality();
void testRectangleArea();
void testRectangleComputeExpansionArea();
void testRectangleExpansion();
void testRectangleIntersection();
void testRectanglePointContainment();
void testRectangleFragmentation();

#endif
