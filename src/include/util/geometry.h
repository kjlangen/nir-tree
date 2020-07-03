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
		bool operator<(Point p) const;
		bool operator>(Point p) const;
		bool operator<=(Point p) const;
		bool operator>=(Point p) const;
		bool operator==(Point p) const;
		bool operator!=(Point p) const;

		void print();
		friend std::ostream& operator<<(std::ostream& os, const Point& point);
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
		float computeExpansionArea(Point givenPoint);
		float computeExpansionArea(Rectangle requestedRectangle);
		void expand(Point givenPoint);
		void expand(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle requestedRectangle);
		bool strictIntersectsRectangle(Rectangle requestedRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> fragmentRectangle(Rectangle clippingRectangle);

		bool operator==(Rectangle r) const;
		bool operator!=(Rectangle r) const;

		void print();
		friend std::ostream& operator<<(std::ostream& os, const Rectangle& rectangle);
};

class DynamicRectangle
{
	public:
		std::vector<Rectangle> basicRectangles;

		DynamicRectangle();
		DynamicRectangle(Rectangle baseRectangle);
		float area();
		float computeExpansionArea(Point givenPoint);
		float computeExpansionArea(Rectangle requestedRectangle);
		void expand(Point givenPoint);
		void expand(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle &requestedRectangle);
		bool intersectsRectangle(DynamicRectangle &requestedRectangle);
		bool containsPoint(Point requestedPoint);
		void increaseResolution(Rectangle clippingRectangle);

		bool operator==(const DynamicRectangle& r);
		bool operator!=(const DynamicRectangle& r);

		void print();
};

#endif
