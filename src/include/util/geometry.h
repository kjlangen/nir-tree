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
#include <util/debug.h>

class Point
{
	public:
		float x;
		float y;

		Point();
		Point(float x, float y);

		friend bool operator<(const Point &lhs, const Point &rhs);
		friend bool operator>(const Point &lhs, const Point &rhs);
		friend bool operator<=(const Point &lhs, const Point &rhs);
		friend bool operator>=(const Point &lhs, const Point &rhs);
		friend bool operator==(const Point &lhs, const Point &rhs);
		friend bool operator!=(const Point &lhs, const Point &rhs);
		friend std::ostream& operator<<(std::ostream& os, const Point& point);
};

bool operator<(const Point &lhs, const Point &rhs);
bool operator>(const Point &lhs, const Point &rhs);
bool operator<=(const Point &lhs, const Point &rhs);
bool operator>=(const Point &lhs, const Point &rhs);
bool operator==(const Point &lhs, const Point &rhs);
bool operator!=(const Point &lhs, const Point &rhs);

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

		friend bool operator==(const Rectangle &lr, const Rectangle &rr);
		friend bool operator!=(const Rectangle &lr, const Rectangle &rr);
		friend std::ostream& operator<<(std::ostream& os, const Rectangle& rectangle);
};

bool operator==(const Rectangle &lhs, const Rectangle &rhs);
bool operator!=(const Rectangle &lhs, const Rectangle &rhs);

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
		bool intersectsRectangle(Rectangle &givenRectangle);
		bool intersectsPolygon(IsotheticPolygon &givenPolygon);
		bool borderOnlyIntersectsRectanlge(Rectangle givenRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> intersection(Rectangle givenRectangle);
		void intersection(IsotheticPolygon &constraintPolygon);
		void increaseResolution(Rectangle clippingRectangle);
		void increaseResolution(IsotheticPolygon &clippingPolygon);
		void refine();
		void sort(bool min, unsigned d=0);

		bool unique();
		bool infFree();

		friend bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend std::ostream& operator<<(std::ostream& os, const IsotheticPolygon& polygon);
};

bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);

#endif
