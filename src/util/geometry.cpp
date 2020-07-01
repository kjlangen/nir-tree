#include "util/geometry.h"

Point::Point()
{
	this->x = 0;
	this->y = 0;
}

Point::Point(float x, float y)
{
	this->x = x;
	this->y = y;
}

bool Point::operator<(Point p) const
{
	return x < p.x && y < p.y;
}

bool Point::operator>(Point p) const
{
	return x > p.x && y > p.y;
}

bool Point::operator<=(Point p) const
{
	return x <= p.x && y <= p.y;
}

bool Point::operator>=(Point p) const
{
	return x >= p.x && y >= p.y;
}

bool Point::operator==(Point p) const
{
	return x == p.x && y == p.y;
}

bool Point::operator!=(Point p) const
{
	return x != p.x || y != p.y;
}

void Point::print()
{
	std::cout << "(" << x << ", " << y << ")";
}

Rectangle::Rectangle()
{
	lowerLeft = Point(0.0, 0.0);
	upperRight = Point(0.0, 0.0);
}

Rectangle::Rectangle(float x, float y, float xp, float yp)
{
	lowerLeft = Point(x, y);
	upperRight = Point(xp, yp);
}

Rectangle::Rectangle(Point lowerLeft, Point upperRight)
{
	this->lowerLeft = lowerLeft;
	this->upperRight = upperRight;
}

float Rectangle::area()
{
	return fabs((upperRight.x - lowerLeft.x) * (upperRight.y - lowerLeft.y));
}

float Rectangle::computeExpansionArea(Point givenPoint)
{
	// Expanded rectangle area computed directly
	float expandedArea = fabs((fmin(lowerLeft.x, givenPoint.x) - fmax(upperRight.x, givenPoint.x)) *
							  (fmin(lowerLeft.y, givenPoint.y) - fmax(upperRight.y, givenPoint.y)));

	// Compute the difference
	return expandedArea - area();
}

float Rectangle::computeExpansionArea(Rectangle requestedRectangle)
{
	// Expanded rectangle area computed directly
	float expandedArea = fabs((fmin(requestedRectangle.lowerLeft.x, lowerLeft.x) - fmax(requestedRectangle.upperRight.x, upperRight.x)) *
							  (fmin(requestedRectangle.lowerLeft.y, lowerLeft.y) - fmax(requestedRectangle.upperRight.y, upperRight.y)));

	// Compute the difference
	return expandedArea - area();
}

// TODO: Optimize
void Rectangle::expand(Point givenPoint)
{
	lowerLeft = Point(fmin(lowerLeft.x, givenPoint.x), fmin(lowerLeft.y, givenPoint.y));
	upperRight = Point(fmax(upperRight.x, givenPoint.x), fmax(upperRight.y, givenPoint.y));
}

// TODO: Optimize computing the centre x & y coords to one computation
void Rectangle::expand(Rectangle givenRectangle)
{
	lowerLeft = Point(fmin(givenRectangle.lowerLeft.x, lowerLeft.x), fmin(givenRectangle.lowerLeft.y, lowerLeft.y));
	upperRight = Point(fmax(givenRectangle.upperRight.x, upperRight.x), fmax(givenRectangle.upperRight.y, upperRight.y));
}

bool Rectangle::intersectsRectangle(Rectangle requestedRectangle)
{
	// Compute the range intersections
	bool intervalX = lowerLeft.x <= requestedRectangle.upperRight.x && upperRight.x >= requestedRectangle.lowerLeft.x;
	bool intervalY = lowerLeft.y <= requestedRectangle.upperRight.y && upperRight.y >= requestedRectangle.lowerLeft.y;

	return intervalX && intervalY;
}

bool Rectangle::strictIntersectsRectangle(Rectangle requestedRectangle)
{
	// Compute the range intersections
	bool intervalX = lowerLeft.x < requestedRectangle.upperRight.x && upperRight.x > requestedRectangle.lowerLeft.x;
	bool intervalY = lowerLeft.y < requestedRectangle.upperRight.y && upperRight.y > requestedRectangle.lowerLeft.y;

	return intervalX && intervalY;
}

bool Rectangle::containsPoint(Point requestedPoint)
{
	return lowerLeft <= requestedPoint && requestedPoint <= upperRight;
}

std::vector<Rectangle> Rectangle::fragmentRectangle(Rectangle clippingRectangle)
{
	std::vector<Rectangle> v;

	// Enumerate each of the points of the bounding rectangle/cube/hypercube
	Point upperLeft = Point(clippingRectangle.lowerLeft.x, clippingRectangle.upperRight.y);
	Point lowerRight = Point(clippingRectangle.upperRight.x, clippingRectangle.lowerLeft.y);

	// Upper right is inside us
	if (containsPoint(clippingRectangle.upperRight))
	{
		// Always divide horizontally
		v.push_back(Rectangle(lowerLeft.x, clippingRectangle.upperRight.y, upperRight.x, upperRight.y));
		v.push_back(Rectangle(clippingRectangle.upperRight.x, lowerLeft.y, upperRight.x, clippingRectangle.upperRight.y));
	}

	// Lower right is inside us
	if (containsPoint(lowerRight))
	{
		// Always divide horizontally
		v.push_back(Rectangle(lowerLeft.x, lowerLeft.y, upperRight.x, lowerRight.y));
		v.push_back(Rectangle(lowerRight.x, lowerRight.y, upperRight.x, upperRight.y));
	}

	// Lower left is inside us
	if (containsPoint(clippingRectangle.lowerLeft))
	{
		// Always divide horizontally
		v.push_back(Rectangle(lowerLeft.x, lowerLeft.y, upperRight.x, clippingRectangle.lowerLeft.y));
		v.push_back(Rectangle(lowerLeft.x, clippingRectangle.lowerLeft.y, clippingRectangle.lowerLeft.x, upperRight.y));
	}

	// Upper left is inside us
	if (containsPoint(upperLeft))
	{
		// Always divide horizontally
		v.push_back(Rectangle(lowerLeft.x, upperLeft.y, upperRight.x, upperRight.y));
		v.push_back(Rectangle(lowerLeft.x, lowerLeft.y, upperLeft.x, upperLeft.y));
	}

	// TODO: If there are too many rectangles we can safely remove one of the rectangles although
	// it will cost us computation time to find out which we can eliminate

	return v;
}

bool Rectangle::operator==(Rectangle r) const
{
	return lowerLeft == r.lowerLeft && upperRight == r.upperRight;
}

bool Rectangle::operator!=(Rectangle r) const
{
	return lowerLeft != r.lowerLeft || upperRight != r.upperRight;
}

void Rectangle::print()
{
	std::cout.precision(std::numeric_limits<double>::max_digits10+3);
	std::cout << "Rectangle {";
	lowerLeft.print();
	std::cout << ", ";
	upperRight.print();
	std::cout << "}" << std::endl;
}

DynamicRectangle::DynamicRectangle()
{
	// TODO: Should we create a rectangle of 0 size?
}

DynamicRectangle::DynamicRectangle(Rectangle baseRectangle)
{
	basicRectangles.push_back(baseRectangle);
}

float DynamicRectangle::area()
{
	float area = 0.0;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		area += basicRectangles[i].area();
	}

	return area;
}

float DynamicRectangle::computeExpansionArea(Point givenPoint)
{
	// Take the minimum expansion area
	float minArea = basicRectangles[0].computeExpansionArea(givenPoint);
	float evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(givenPoint);
		minArea = evalArea < minArea ? evalArea : minArea;
	}

	return minArea;
}

float DynamicRectangle::computeExpansionArea(Rectangle requestedRectangle)
{
	// Take the minimum expansion area
	float minArea = basicRectangles[0].computeExpansionArea(requestedRectangle);
	float evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(requestedRectangle);
		minArea = evalArea < minArea ? evalArea : minArea;
	}

	return minArea;
}

// TODO: This is a simple functional first-pass at expanding. It could get far more complex and most
// likely will as we deal with the special case of spirals
void DynamicRectangle::expand(Point givenPoint)
{
	unsigned minIndex = 0;
	float minArea = basicRectangles[0].computeExpansionArea(givenPoint);
	float evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(givenPoint);
		if (evalArea < minArea)
		{
			minArea = evalArea;
			minIndex = i;
		}
	}

	basicRectangles[minIndex].expand(givenPoint);
}

// TODO: This is a simple functional first-pass at expanding. It could get far more complex and most
// likely will as we deal with the special case of spirals
void DynamicRectangle::expand(Rectangle givenRectangle)
{
	unsigned minIndex = 0;
	float minArea = basicRectangles[0].computeExpansionArea(givenRectangle);
	float evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(givenRectangle);
		if (evalArea < minArea)
		{
			minArea = evalArea;
			minIndex = i;
		}
	}

	basicRectangles[minIndex].expand(givenRectangle);
}

bool DynamicRectangle::intersectsRectangle(Rectangle &requestedRectangle)
{
	bool running = false;

	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size() || running; ++i)
	{
		running = running || requestedRectangle.intersectsRectangle(basicRectangles[i]);
	}

	return running;
}

bool DynamicRectangle::intersectsRectangle(DynamicRectangle &requestedRectangle)
{
	bool running = false;

	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size() || running; ++i)
	{
		running = running || requestedRectangle.intersectsRectangle(basicRectangles[i]);
	}

	return running;
}

bool DynamicRectangle::containsPoint(Point requestedPoint)
{
	bool running = false;

	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size() || running; ++i)
	{
		running = running || basicRectangles[i].containsPoint(requestedPoint);
	}

	return running;
}

void DynamicRectangle::increaseResolution(Rectangle clippingRectangle)
{
	// Test each of the rectangles that define us, splitting them whenever necessary
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if (clippingRectangle.intersectsRectangle(basicRectangles[i]))
		{
			// Break the rectangle
			std::vector<Rectangle> v = basicRectangles[i].fragmentRectangle(clippingRectangle);

			// Replace the rectangle with one of its new constituent pieces and append the others
			basicRectangles[i] = v[0];
			for (unsigned j = 1; j < v.size(); ++j)
			{
				basicRectangles.push_back(v[j]);
			}
		}
	}
}

bool DynamicRectangle::operator==(DynamicRectangle r)
{
	// ???
	return false;
}

bool DynamicRectangle::operator!=(DynamicRectangle r)
{
	// ???
	return false;
}

void DynamicRectangle::print()
{
	std::cout << "DynamicRectangle |";
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		basicRectangles[i].lowerLeft.print();
		basicRectangles[i].upperRight.print();
		std::cout << '|';
	}
	std::cout << '|' << std::endl;
}

void testPointEquality()
{
	// Test set one
	Point p1 = Point(0.0, 0.0);
	Point p2 = Point(0.0, 0.0);
	assert(p1 == p2);

	// Test set two
	Point p3 = Point(1.0, 1.0);
	Point p4 = Point(1.0, 1.0);
	assert(p3 == p4);

	// Test set three
	Point p5 = Point(-1.0, -1.0);
	Point p6 = Point(-1.0, -1.0);
	assert(p5 == p6);

	// Test set four
	Point p7 = Point(2.0, 1.0);
	Point p8 = Point(-2.0, -1.0);
	assert(p7 != p8);

	// Test set five
	Point p9 = Point(2.0, 1.0);
	Point p10 = Point(1.0, 2.0);
	assert(p9 != p10);
}

void testRectangleArea()
{
	// Test set one
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	assert(r1.area() == 0.0);

	// Test set two
	Rectangle r2 = Rectangle(0.0, 0.0, 1.0, 1.0);
	assert(r2.area() == 1.0);

	// Test set three
	Rectangle r3 = Rectangle(-5.0, -3.0, 13.0, 11.0);
	assert(r3.area() == 252.0);

	// Test set four, floating point math is super busted so it's not quite 259.86 as it should be
	Rectangle r4 = Rectangle(-5.0, -3.0, 13.3, 11.2);
	assert(r4.area() == 259.8599853515625);
}

void testRectangleComputeExpansionArea()
{
	// Test computing expansion area for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	assert(r1.computeExpansionArea(p1) == 45.0);

	// Test computing expansion area for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	assert(r2.computeExpansionArea(r3) == 18.0);

	// Test computing expansion area for a rectangle partially inside another rectangle
	Rectangle r4 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r5 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	assert(r4.computeExpansionArea(r5) == 26.0);

	// Test computing expansion area for a rectangle wholly inside another rectangle
	Rectangle r6 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r7 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	assert(r6.computeExpansionArea(r7) == 0.0);
}

void testRectangleExpansion()
{
	// Test computing expansion for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	r1.expand(p1);
	assert(r1.area() == 49.0);
	assert(r1.lowerLeft == Point(2.0, 8.0));
	assert(r1.upperRight == Point(9.0, 15.0));

	// Test computing expansion for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	r2.expand(r3);
	assert(r2.area() == 22.0);
	assert(r2.lowerLeft == Point(1.0, -2.5));
	assert(r2.upperRight == Point(5.0, 3.0));

	// Test computing expansion area for a rectangle partially inside another rectangle
	Rectangle r4 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r5 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	r4.expand(r5);
	assert(r4.area() == 66.0);
	assert(r4.lowerLeft == Point(-10.0, 2.0));
	assert(r4.upperRight == Point(1.0, 8.0));

	// Test computing expansion area for a rectangle wholly inside another rectangle
	Rectangle r6 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r7 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	r6.expand(r7);
	assert(r6.area() == 56.0);
	assert(r6.lowerLeft == Point(-4.0, -3.0));
	assert(r6.upperRight == Point(3.0, 5.0));
}

void testRectangleIntersection()
{
	// Test corner on corner intersection
	Rectangle r1 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r2 = Rectangle(46.0, 34.8, 72.8, 61.6);
	assert(r1.intersectsRectangle(r2));

	// Test side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	assert(r3.intersectsRectangle(r4));
}

void testStrictRectangleIntersection()
{
	// Test corner on corner intersection
	Rectangle r1 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r2 = Rectangle(46.0, 34.8, 72.8, 61.6);
	assert(r1.intersectsRectangle(r2));

	// Test side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	assert(r3.intersectsRectangle(r4));

	// Test corner on corner intersection
	Rectangle r5 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r6 = Rectangle(48.8, 47.9, 72.8, 61.6);
	assert(!r5.intersectsRectangle(r6));

	// Test side on side intersection
	Rectangle r7 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r8 = Rectangle(-56.4, -8.0, -19.2, 5.0);
	assert(!r7.intersectsRectangle(r8));
}

void testRectanglePointContainment()
{
	// Test set one
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	Point p1 = Point(0.0, 0.0);
	assert(r1.containsPoint(p1));

	// Test set two
	Rectangle r2 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p2 = Point(-1.0, -1.0);
	assert(r2.containsPoint(p2));

	// Test set three
	Rectangle r3 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p3 = Point(217.3, 527.7);
	assert(!r3.containsPoint(p3));

	// Test set four
	Rectangle r4 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p4 = Point(3.0, 1.0);
	assert(r4.containsPoint(p4));
}

void testRectangleFragmentation()
{
	// Test set one
	Rectangle r1 = Rectangle(1.0, 1.0, 5.0, 5.0);
	Rectangle r2 = Rectangle(4.0, 4.0, 8.0, 8.0);
	std::vector<Rectangle> v = r1.fragmentRectangle(r2);
	assert(v.size() == 2);
	assert(v[0] == Rectangle(1.0, 1.0, 5.0, 4.0));
	assert(v[1] == Rectangle(1.0, 4.0, 4.0, 5.0));

	// Test set two
	r1 = Rectangle(0.0, 0.0, 4.0, 4.0);
	r2 = Rectangle(1.0, -3.0, 6.0, 1.0);
	v = r1.fragmentRectangle(r2);
	assert(v.size() == 2);
	assert(v[0] == Rectangle(0.0, 1.0, 4.0, 4.0));
	assert(v[1] == Rectangle(0.0, 0.0, 1.0, 1.0));

	// Test set three
	r1 = Rectangle(-7.0, -5.0, 0.0, 0.0);
	r2 = Rectangle(-8.0, -8.0, -4.0, -3.0);
	v = r1.fragmentRectangle(r2);
	assert(v.size() == 2);
	assert(v[0] == Rectangle(-7.0, -3.0, 0.0, 0.0));
	assert(v[1] == Rectangle(-4.0, -5.0, 0.0, -3.0));

	// Test set four
	r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	r2 = Rectangle(3.0, 3.0, 7.0, 7.0);
	v = r1.fragmentRectangle(r2);
	assert(v.size() == 4);
	assert(v[0] == Rectangle(0.0, 0.0, 4.0, 3.0));
	assert(v[1] == Rectangle(0.0, 3.0, 3.0, 8.0));
	assert(v[2] == Rectangle(0.0, 7.0, 4.0, 8.0));
	assert(v[3] == Rectangle(0.0, 0.0, 3.0, 7.0));

	// Test set five
	r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	r2 = Rectangle(-3.0, 3.0, 1.0, 7.0);
	v = r1.fragmentRectangle(r2);
	assert(v.size() == 4);
	assert(v[0] == Rectangle(0.0, 7.0, 4.0, 8.0));
	assert(v[1] == Rectangle(1.0, 0.0, 4.0, 7.0));
	assert(v[2] == Rectangle(0.0, 0.0, 4.0, 3.0));
	assert(v[3] == Rectangle(1.0, 3.0, 4.0, 8.0));

	// Test set six
	r1 = Rectangle(0.0, 0.0, 8.0, 4.0);
	r2 = Rectangle(1.0, 2.0, 5.0, 6.0);
	v = r1.fragmentRectangle(r2);
	assert(v.size() == 4);
	assert(v[0] == Rectangle(0.0, 0.0, 8.0, 2.0));
	assert(v[1] == Rectangle(5.0, 2.0, 8.0, 4.0));
	assert(v[2] == Rectangle(0.0, 0.0, 8.0, 2.0));
	assert(v[3] == Rectangle(0.0, 2.0, 1.0, 4.0));
}
