#ifndef __GEOMETRY__
#define __GEOMETRY__

#include <iostream>
#include <algorithm>
#include <cassert>
#include <vector>
#include <cmath>
#include <limits>
#include <cfenv>

class Point {
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

class Rectangle {
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

    bool operator==(Rectangle r);

    bool operator!=(Rectangle r);

    void print();
};

class DynamicRectangle {
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

    bool operator==(DynamicRectangle r);

    bool operator!=(DynamicRectangle r);

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
