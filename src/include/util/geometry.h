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
#include <iterator>
#include <functional>
#include <util/debug.h>
#include <globals/globals.h>

class Point
{


	public:

        double values[dimensions];

		static Point atInfinity;
		static Point atNegInfinity;
		static Point atOrigin;

		Point();

		Point(double x, double y);
		Point(double value);
		Point(const Point &o) = default;

		bool orderedCompare(const Point &rhs, unsigned startingDimension) const;
		double distance(const Point &p) const;

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
		friend Point operator/(const Point &lsh, const double scalar);
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
		Rectangle(const Rectangle &o) = default;
		double area() const;
		double margin() const;
		double computeIntersectionArea(const Rectangle &givenRectangle) const;
		double computeExpansionArea(const Point &givenPoint) const;
		double computeExpansionArea(const Rectangle &givenRectangle) const;
		void expand(const Point &givenPoint);
		void expand(const Rectangle &givenRectangle);
		bool alignedForMerging(const Rectangle &givenRectangle) const;
		bool alignedOpposingBorders(const Rectangle &givenRectangle) const;
		bool intersectsRectangle(const Rectangle &givenRectangle) const;
		bool strictIntersectsRectangle(const Rectangle &givenRectangle) const;
		bool borderOnlyIntersectsRectangle(const Rectangle &givenRectangle) const;
		bool containsPoint(const Point &givenPoint) const;
		bool strictContainsPoint(const Point &givenPoint) const;
		bool containsRectangle(const Rectangle &givenRectangle) const;
		Point centrePoint() const;
		Rectangle intersection(const Rectangle &clippingRectangle) const;
		std::vector<Rectangle> fragmentRectangle(const Rectangle &clippingRectangle) const;

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
		IsotheticPolygon(const Rectangle &baseRectangle);
		IsotheticPolygon(const IsotheticPolygon &basePolygon);
		double area() const;
		double computeIntersectionArea(const Rectangle &givenRectangle) const;
		OptimalExpansion computeExpansionArea(const Point &givenPoint) const;
		OptimalExpansion computeExpansionArea(const Rectangle &givenRectangle) const;
		void expand(const Point &givenPoint);
		void expand(const Point &givenPoint, const OptimalExpansion &expansion);
		bool intersectsRectangle(const Rectangle &givenRectangle) const;
		bool intersectsPolygon(const IsotheticPolygon &givenPolygon) const;
		bool borderOnlyIntersectsRectangle(const Rectangle &givenRectangle) const;
		bool containsPoint(const Point &givenPoint) const ;
		bool disjoint(const IsotheticPolygon &givenPolygon) const;
		std::vector<Rectangle> intersection(const Rectangle &givenRectangle) const;
		void intersection(const IsotheticPolygon &constraintPolygon);
		void increaseResolution(const Point &givenPoint, const Rectangle &clippingRectangle);
		void increaseResolution(const Point &givenPoint, const IsotheticPolygon &clippingPolygon);
		void maxLimit(double limit, unsigned d=0);
		void minLimit(double limit, unsigned d=0);
		void merge(const IsotheticPolygon &mergePolygon);
		void remove(unsigned basicRectangleIndex);
		void deduplicate();
		void refine();
		void shrink(const std::vector<Point> &pinPoints);

		bool exists() const;
		bool valid() const;
		bool unique() const;
		bool lineFree() const;
		bool infFree() const;

		friend bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
		friend std::ostream& operator<<(std::ostream &os, const IsotheticPolygon &polygon);
};

bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);
bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs);

#endif
