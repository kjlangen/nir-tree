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
#include <globals/globals.h>

class Point
{
	float values[dimensions];

	public:
		static Point atInfinity;
		static Point atNegInfinity;
		static Point atOrigin;

		Point();
		Point(float x, float y);
		Point(float value);

		float &operator[](unsigned index);
		void operator<<(Point &p);
		void operator>>(Point &p);

		friend bool operator<(Point &lhs, Point &rhs);
		friend bool operator>(Point &lhs, Point &rhs);
		friend bool operator<=(Point &lhs, Point &rhs);
		friend bool operator>=(Point &lhs, Point &rhs);
		friend bool operator==(Point &lhs, Point &rhs);
		friend bool operator!=(Point &lhs, Point &rhs);
		friend std::ostream& operator<<(std::ostream &os, Point &point);
};

bool operator<(Point &lhs, Point &rhs);
bool operator>(Point &lhs, Point &rhs);
bool operator<=(Point &lhs, Point &rhs);
bool operator>=(Point &lhs, Point &rhs);
bool operator==(Point &lhs, Point &rhs);
bool operator!=(Point &lhs, Point &rhs);

class Rectangle
{
	public:
		static Rectangle atInfinity;
		static Rectangle atNegInfinity;
		static Rectangle atOrigin;

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
		bool aligned(Rectangle givenRectangle);
		bool alignedOpposingBorders(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle givenRectangle);
		bool strictIntersectsRectangle(Rectangle givenRectangle);
		bool borderOnlyIntersectsRectanlge(Rectangle givenRectangle);
		bool containsPoint(Point givenPoint);
		bool strictContainsPoint(Point givenPoint);
		bool containsRectangle(Rectangle givenRectangle);
		Rectangle intersection(Rectangle clippingRectangle);
		std::vector<Rectangle> fragmentRectangle(Rectangle clippingRectangle);

		friend bool operator==(Rectangle &lr, Rectangle &rr);
		friend bool operator!=(Rectangle &lr, Rectangle &rr);
		friend std::ostream& operator<<(std::ostream& os, Rectangle &rectangle);
};

bool operator==(Rectangle &lhs, Rectangle &rhs);
bool operator!=(Rectangle &lhs, Rectangle &rhs);

class IsotheticPolygon
{
	public:
		struct OptimalExpansion
		{
			unsigned index;
			float area;
		};

		Rectangle boundingBox;
		std::vector<Rectangle> basicRectangles;

		IsotheticPolygon();
		IsotheticPolygon(Rectangle baseRectangle);
		IsotheticPolygon(const IsotheticPolygon &basePolygon);
		float area();
		float computeIntersectionArea(Rectangle givenRectangle);
		OptimalExpansion computeExpansionArea(Point givenPoint);
		OptimalExpansion computeExpansionArea(Rectangle givenRectangle);
		void expand(Point givenPoint);
		void expand(Point givenPoint, OptimalExpansion expansion);
		void expand(Point givenPoint, IsotheticPolygon &constraintPolygon);
		void expand(Point givenPoint, IsotheticPolygon &constraintPolygon, OptimalExpansion expansion);
		bool intersectsRectangle(Rectangle &givenRectangle);
		bool intersectsPolygon(IsotheticPolygon &givenPolygon);
		bool borderOnlyIntersectsRectanlge(Rectangle givenRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> intersection(Rectangle givenRectangle);
		void intersection(IsotheticPolygon &constraintPolygon);
		void increaseResolution(Rectangle clippingRectangle);
		void increaseResolution(IsotheticPolygon &clippingPolygon);
		void maxLimit(float limit, unsigned d=0);
		void minLimit(float limit, unsigned d=0);
		void refine();
		void sort(bool min, unsigned d=0);

		bool unique();
		bool infFree();

		friend bool operator==(IsotheticPolygon &lhs, IsotheticPolygon &rhs);
		friend bool operator!=(IsotheticPolygon &lhs, IsotheticPolygon &rhs);
		friend std::ostream& operator<<(std::ostream &os, IsotheticPolygon &polygon);
};

bool operator==(IsotheticPolygon &lhs, IsotheticPolygon &rhs);
bool operator!=(IsotheticPolygon &lhs, IsotheticPolygon &rhs);

#endif
