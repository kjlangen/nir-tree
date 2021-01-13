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
	double values[dimensions];

	public:
		static Point atInfinity;
		static Point atNegInfinity;
		static Point atOrigin;

		Point();
		Point(double x, double y);
		Point(double value);

		Point &operator-=(const Point &rhs);
		Point &operator+=(const Point &rhs);
		Point &operator/=(double scalar);
		Point &operator*=(double scalar);
		Point &operator*=(const Point &rhs);
		double &operator[](unsigned index);
		const double operator[](unsigned index) const;
		Point &operator<<(const Point &p);
		Point &operator>>(const Point &p);

		friend Point operator-(const Point &lhs, const Point &rhs);
		friend Point operator+(const Point &lhs, const Point &rhs);
		friend Point operator*(const Point &lhs, const double scalar);
		friend Point operator*(const Point &lhs, const Point &rhs);
		friend bool operator<(const Point &lhs, const Point &rhs);
		friend bool operator>(const Point &lhs, const Point &rhs);
		friend bool operator<=(const Point &lhs, const Point &rhs);
		friend bool operator>=(const Point &lhs, const Point &rhs);
		friend bool operator==(const Point &lhs, const Point &rhs);
		friend bool operator!=(const Point &lhs, const Point &rhs);
		friend std::ostream& operator<<(std::ostream &os, const Point &point);
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
		static Rectangle atInfinity;
		static Rectangle atNegInfinity;
		static Rectangle atOrigin;

		Point lowerLeft;
		Point upperRight;

		Rectangle();
		Rectangle(double x, double y, double xp, double yp);
		Rectangle(Point lowerLeft, Point upperRight);
		double area();
		double computeIntersectionArea(Rectangle givenRectangle);
		double computeExpansionArea(Point givenPoint);
		double computeExpansionArea(Rectangle givenRectangle);
		void expand(Point givenPoint);
		void expand(Rectangle givenRectangle);
		bool aligned(Rectangle givenRectangle);
		bool alignedOpposingBorders(Rectangle givenRectangle);
		bool intersectsRectangle(Rectangle givenRectangle);
		bool strictIntersectsRectangle(Rectangle givenRectangle);
		bool borderOnlyIntersectsRectangle(Rectangle givenRectangle);
		bool containsPoint(Point givenPoint);
		bool strictContainsPoint(Point givenPoint);
		bool containsRectangle(Rectangle givenRectangle);
		Rectangle intersection(Rectangle clippingRectangle);
		std::vector<Rectangle> fragmentRectangle(Rectangle clippingRectangle);

		friend bool operator==(const Rectangle &lr, const Rectangle &rr);
		friend bool operator!=(const Rectangle &lr, const Rectangle &rr);
		friend std::ostream& operator<<(std::ostream& os, const Rectangle &rectangle);
};

bool operator==(const Rectangle &lhs, const Rectangle &rhs);
bool operator!=(const Rectangle &lhs, const Rectangle &rhs);

class IsotheticPolygon
{
	public:
		struct OptimalExpansion
		{
			unsigned index;
			double area;
		};

		Rectangle boundingBox;
		std::vector<Rectangle> basicRectangles;

		IsotheticPolygon();
		IsotheticPolygon(Rectangle baseRectangle);
		IsotheticPolygon(const IsotheticPolygon &basePolygon);
		double area();
		double computeIntersectionArea(Rectangle givenRectangle);
		OptimalExpansion computeExpansionArea(Point givenPoint);
		OptimalExpansion computeExpansionArea(Rectangle givenRectangle);
		void expand(Point givenPoint);
		void expand(Point givenPoint, OptimalExpansion expansion);
		void expand(Point givenPoint, IsotheticPolygon &constraintPolygon);
		void expand(Point givenPoint, IsotheticPolygon &constraintPolygon, OptimalExpansion expansion);
		bool intersectsRectangle(Rectangle &givenRectangle);
		bool intersectsPolygon(IsotheticPolygon &givenPolygon);
		bool borderOnlyIntersectsRectangle(Rectangle givenRectangle);
		bool containsPoint(Point requestedPoint);
		std::vector<Rectangle> intersection(Rectangle givenRectangle);
		void intersection(IsotheticPolygon &constraintPolygon);
		void increaseResolution(Rectangle clippingRectangle);
		void increaseResolution(IsotheticPolygon &clippingPolygon);
		void maxLimit(double limit, unsigned d=0);
		void minLimit(double limit, unsigned d=0);
		void merge(const IsotheticPolygon &mergePolygon);
		void remove(unsigned basicRectangleIndex);
		void deduplicate();
		void refine();
		void sort(bool min, unsigned d=0);

		bool disjoint();
		bool exists();
		bool unique();
		bool infFree();

		friend bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend std::ostream& operator<<(std::ostream &os, const IsotheticPolygon &polygon);
};

bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);

#endif
