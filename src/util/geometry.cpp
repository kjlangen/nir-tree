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

std::ostream& operator<<(std::ostream& os, const Point& point)
{
	os << "(" << point.x << ", " << point.y << ")";
	return os;
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

float Rectangle::computeOverlapArea(Rectangle requestedRectangle)
{
	float widthOverlap = fmax(0, fmin(upperRight.x, requestedRectangle.upperRight.x) - fmax(lowerLeft.x, requestedRectangle.lowerLeft.x));
	float heightOverlap = fmax(0, fmin(upperRight.y, requestedRectangle.upperRight.y) - fmax(lowerLeft.y, requestedRectangle.lowerLeft.y));
	return widthOverlap * heightOverlap;
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

std::ostream& operator<<(std::ostream& os, const Rectangle& rectangle)
{
	os << "[" << rectangle.lowerLeft << "; " << rectangle.upperRight << "]";
	return os;
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

bool DynamicRectangle::operator==(const DynamicRectangle& r)
{
	// ???
	return false;
}

bool DynamicRectangle::operator!=(const DynamicRectangle& r)
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
