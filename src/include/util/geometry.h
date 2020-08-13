#ifndef __GEOMETRY__
#define __GEOMETRY__

#include <iostream>
#include <algorithm>
#include <queue>
#include <cassert>
#include <vector>
#include <cmath>
#include <cstring>
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
		void shrink(std::vector<Rectangle> &pinSet, std::vector<Point> &constraintSet);
		void shrink(std::vector<Rectangle> &pinSet, std::vector<Rectangle> &constraintSet);
		bool alignedOpposingBorders(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle givenRectangle);
		bool strictIntersectsRectangle(Rectangle givenRectangle);
		bool borderOnlyIntersectsRectanlge(Rectangle givenRectangle);
		bool containsPoint(Point givenPoint);
		bool strictContainsPoint(Point givenPoint);
		bool containsRectangle(Rectangle givenRectangle);
		Rectangle intersection(Rectangle clippingRectangle);
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
		bool quickIntersectsRectangle(Rectangle &givenRectangle);
		bool intersectsPolygon(IsotheticPolygon &givenPolygon);
		bool quickIntersectsPolygon(IsotheticPolygon &givenPolygon);
		bool borderOnlyIntersectsRectanlge(Rectangle givenRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> intersection(Rectangle givenRectangle);
		void intersection(IsotheticPolygon &constraintPolygon);
		void increaseResolution(Rectangle clippingRectangle);
		void increaseResolution(IsotheticPolygon &clippingPolygon);
		void refine();
		void sort(bool min, unsigned d=0);

		bool operator==(IsotheticPolygon r);
		bool operator!=(IsotheticPolygon r);
		bool unique();
		bool infFree();
		bool contiguous();

		void print();
};

void testPointEquality();
void testRectangleArea();
void testRectangleComputeExpansionArea();
void testRectangleExpansion();
void testRectangleIntersection();
void testRectanglePointContainment();
void testRectangleFragmentation();
void testIsotheticPolygonRefinement();

#endif
