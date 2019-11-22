#include "util/geometry.h"

Point::Point()
{
	this->x = 0;
	this->y = 0;
}

Point::Point(double x, double y)
{
	this->x = x;
	this->y = y;
}

bool Point::operator==(Point p)
{
	return x == p.x && y == p.y;
}

bool Point::operator!=(Point p)
{
	return x != p.x || y != p.y;
}

void Point::print()
{
	std::cout << "(" << x << "," << y << ")";
}

Rectangle::Rectangle()
{
	centre = Point(0.0, 0.0);
	radiusX = 0.0;
	radiusY = 0.0;
}

Rectangle::Rectangle(double x, double y, double radiusX, double radiusY)
{
	this->centre = Point(x, y);
	this->radiusX = radiusX;
	this->radiusY = radiusY;
}

Rectangle::Rectangle(Point centre, double radiusX, double radiusY)
{
	this->centre = centre;
	this->radiusX = radiusX;
	this->radiusY = radiusY;
}

double Rectangle::area()
{
	return (radiusX + radiusX) * (radiusY + radiusY);
}

double Rectangle::computeExpansionArea(Point givenPoint)
{
	// Distance from centre
	Point centroidDistance = Point(fabs(centre.x - givenPoint.x), fabs(centre.y - givenPoint.y));

	// The extra area we will need to include
	double xExtra = centroidDistance.x > radiusX ? (centroidDistance.x - radiusX) : 0;
	double yExtra = centroidDistance.y > radiusY ? (centroidDistance.y - radiusY) : 0;

	// Use addition here b/c it's faster
	double diameterX = radiusX + radiusX;
	double diameterY = radiusY + radiusY;
	return (diameterX + xExtra) * (diameterY + yExtra) - (diameterX) * (diameterY);
}

double Rectangle::computeExpansionArea(Rectangle requestedRectangle)
{
	// Distance from centres
	Point centroidDistance = Point(fabs(centre.x - requestedRectangle.centre.x), fabs(centre.y - requestedRectangle.centre.y));

	double diameterX = centroidDistance.x + requestedRectangle.radiusX > radiusX ? centroidDistance.x + radiusX + requestedRectangle.radiusX : radiusX + radiusX;
	double diameterY = centroidDistance.y + requestedRectangle.radiusY > radiusY ? centroidDistance.y + radiusY + requestedRectangle.radiusY : radiusY + radiusY;

	// Use addition here b/c it's faster than multiplying by 2
	return (diameterX * diameterY) - ((radiusX + radiusX) * (radiusY + radiusY));
}

// TODO: Optimize
void Rectangle::expand(Point givenPoint)
{
	// Distance from centre
	Point centroidDistance = Point(fabs(centre.x - givenPoint.x), fabs(centre.y - givenPoint.y));

	// Step the centre over half the distance between the box's edge and the point
	// Then increase the radius by half the distance between the box's edge and the point
	if (centroidDistance.x > radiusX)
	{
		double halfDistBoxEdgeToPoint = (centroidDistance.x - radiusX) / 2;
		radiusX += halfDistBoxEdgeToPoint;
		halfDistBoxEdgeToPoint = givenPoint.x > centre.x ? halfDistBoxEdgeToPoint : -halfDistBoxEdgeToPoint;
		centre.x += halfDistBoxEdgeToPoint;
	}

	if (centroidDistance.y > radiusY)
	{
		double halfDistBoxEdgeToPoint = (centroidDistance.y - radiusY) / 2;
		radiusY += halfDistBoxEdgeToPoint;
		halfDistBoxEdgeToPoint = givenPoint.y > centre.y ? halfDistBoxEdgeToPoint : -halfDistBoxEdgeToPoint;
		centre.y += halfDistBoxEdgeToPoint;
	}
}

// TODO: Optimize computing the centre x & y coords to one computation
void Rectangle::expand(Rectangle givenRectangle)
{
	Point centroidDistance = Point(fabs(centre.x - givenRectangle.centre.x), fabs(centre.y - givenRectangle.centre.y));

	// Adjust centre's x coord and radius
	if (centroidDistance.x + givenRectangle.radiusX > radiusX)
	{
		if (givenRectangle.centre.x < centre.x)
		{
			centre.x = (centre.x + radiusX + givenRectangle.centre.x - givenRectangle.radiusX) / 2;
		}
		else
		{
			centre.x = (centre.x - radiusX + givenRectangle.centre.x + givenRectangle.radiusX) / 2;
		}

		radiusX = (centroidDistance.x + radiusX + givenRectangle.radiusX) / 2;
	}

	// Adjust centre's y coord and radius
	if (centroidDistance.y + givenRectangle.radiusY > radiusY)
	{
		if (givenRectangle.centre.y < centre.y)
		{
			centre.y = (centre.y + radiusY + givenRectangle.centre.y - givenRectangle.radiusY) / 2;
		}
		else
		{
			centre.y = (centre.y - radiusY + givenRectangle.centre.y + givenRectangle.radiusY) / 2;
		}

		radiusY = (centroidDistance.y + radiusY + givenRectangle.radiusY) / 2;
	}
}

bool Rectangle::intersectsRectangle(Rectangle requestedRectangle)
{
	// Compute the range intersections
	bool intervalX = fabs(centre.x - requestedRectangle.centre.x) < radiusX + requestedRectangle.radiusX;
	bool intervalY = fabs(centre.y - requestedRectangle.centre.y) < radiusY + requestedRectangle.radiusY;

	return intervalX && intervalY;
}

bool Rectangle::containsPoint(Point requestedPoint)
{
	bool intervalX = fabs(centre.x - requestedPoint.x) <= radiusX;
	bool intervalY = fabs(centre.y - requestedPoint.y) <= radiusY;

	return intervalX && intervalY;
}

std::vector<Rectangle> Rectangle::splitRectangle(Rectangle clippingRectangle)
{
	// We can have side-on-side or corner-on-corner intersections with 0, 1, or 2 corners inside the
	// other rectangle

	std::vector<Rectangle> v;
	Point centroidDistance = Point(fabs(centre.x - clippingRectangle.centre.x), fabs(centre.y - clippingRectangle.centre.y));
	bool xMeasure = centroidDistance.x + clippingRectangle.radiusX < radiusX;
	bool yMeasure = centroidDistance.y + clippingRectangle.radiusY < radiusY;

	if (!(xMeasure || yMeasure))
	{
		// ***************************************************************************************
		// This is all in the case of 1 corner contained
		// ***************************************************************************************

		// The overlap size in each dimension can be comptued by considering how much larger the sum
		// of the two radii are than the distance between centres. Consider when the rectangles
		// touch only and exactly on one side. Then the distance between the centres is the same as
		// the sum of the two radii, and as the centres get closer the radii do not shrink.
		
		double overlapWidth = clippingRectangle.radiusX + radiusX - centroidDistance.x;
		double overlapHeight = clippingRectangle.radiusY + radiusY - centroidDistance.y;

		// Now depending on the orientation we have to add/subtract these quantities from our centre
		// in a particular order to get the centres of the two new rectangles.

		// Determine the x coord and radius of both new rectangles.
		// The radius of the rectangles can be computed without knowledge of intersection orientation
		double smallRadiusX = overlapWidth / 2;
		double largeRadiusX = radiusX - smallRadiusX;
		double smallXCoord, largeXCoord;
		if (centre.x < clippingRectangle.centre.x)
		{
			smallXCoord = centre.x + radiusX - smallRadiusX;
			largeXCoord = centre.x - radiusX + largeRadiusX;
		}
		else
		{
			smallXCoord = centre.x - radiusX + smallRadiusX; 
			largeXCoord = centre.x + radiusX - largeRadiusX;
		}

		// Determine the y coord and radius of the small new retangle, the large rectangle doesn't
		// change. The radius of the rectangles can be computed without knowledge of intersection
		// orientation. Additionally we can assert the large y coord since it does not change.
		double smallRadiusY = radiusY - overlapHeight / 2;
		double smallYCoord;
		if (centre.y < clippingRectangle.centre.y)
		{
			smallYCoord = centre.y - radiusY + smallRadiusY;
		}
		else
		{
			smallYCoord = centre.y + radiusY - smallRadiusY;
		}

		Rectangle smallBoi = Rectangle(smallXCoord, smallYCoord, smallRadiusX, smallRadiusY);
		Rectangle largeBoi = Rectangle(largeXCoord, centre.y, largeRadiusX, radiusY);
		
		v.push_back(smallBoi);
		v.push_back(largeBoi);
	}
	else if (xMeasure && !yMeasure)
	{
		// ***************************************************************************************
		// This is all in the case of 2 corners contained, intersection from above/below
		// ***************************************************************************************

		// There are three new rectangles that must be built here.
		// In this case the overlap will be the full extent of one of the rectangles in one dimension
		// and then the other dimension will exhibit overlap in the same way as the previous 1
		// corner case.

		// Note in this case we assume the clipping rectangle is smaller than our rectangle. Maybe
		// this always has to be the case because we can't just compress ourselves, we are a
		// paricular height for a reason.

		double overlapHeight = clippingRectangle.radiusY + radiusY - centroidDistance.y;

		double centreRadiusX = clippingRectangle.radiusX;
		double rightRadiusX = fabs((centre.x + radiusX) - (clippingRectangle.centre.x + clippingRectangle.radiusX)) / 2;
		double leftRadiusX = fabs((centre.x - radiusX) - (clippingRectangle.centre.x - clippingRectangle.radiusX)) / 2;

		double leftRadiusY = radiusY;
		double centreRadiusY = radiusY - overlapHeight / 2;
		double rightRadiusY = radiusY;

		double centreYCoord;
		if (centre.y < clippingRectangle.centre.y)
		{
			centreYCoord = clippingRectangle.centre.y - clippingRectangle.radiusY - centreRadiusY;
		}
		else
		{
			centreYCoord = clippingRectangle.centre.y + clippingRectangle.radiusY + centreRadiusY;
		}

		Rectangle centreRect = Rectangle(clippingRectangle.centre.x, centreYCoord, centreRadiusX, centreRadiusY);
		Rectangle rightRect = Rectangle(centreRect.centre.x + centreRadiusX + rightRadiusX, centre.y, rightRadiusX, rightRadiusY);
		Rectangle leftRect = Rectangle(centreRect.centre.x - centreRadiusX - leftRadiusX, centre.y, leftRadiusX, leftRadiusY);

		v.push_back(leftRect);
		v.push_back(centreRect);
		v.push_back(rightRect);
	}
	else if (!xMeasure && yMeasure)
	{
		// ***************************************************************************************
		// This is all in the case of 2 corners contained, intersection from sides
		// ***************************************************************************************

		// There are three new rectangles that must be built here.
		// In this case the overlap will be the full extent of one of the rectangles in one
		// dimension and then the other dimension will exhibit overlap in the same way as the
		// previous 1 corner case.

		// Note in this case we assume the clipping rectangle is smaller than our rectangle. Maybe
		// this always has to be the case because we can't just compress ourselves, we are a
		// paricular height for a reason.

		double overlapWidth = clippingRectangle.radiusX + radiusX - centroidDistance.x;

		double upperRadiusY = fabs((centre.y + radiusY) - (clippingRectangle.centre.y + clippingRectangle.radiusY)) / 2;
		double centreRadiusY = clippingRectangle.radiusY;
		double lowerRadiusY = fabs((centre.y - radiusY) - (clippingRectangle.centre.y - clippingRectangle.radiusY)) / 2;

		double upperRadiusX = radiusX;
		double centreRadiusX = radiusX - overlapWidth / 2;
		double lowerRadiusX = radiusX;

		double centreXCoord;
		if (centre.x < clippingRectangle.centre.x)
		{
			centreXCoord = centre.x - overlapWidth / 2;
		}
		else
		{
			centreXCoord = centre.x + overlapWidth / 2;
		}

		Rectangle upperRect = Rectangle(centre.x, clippingRectangle.centre.y + centreRadiusY + upperRadiusY, upperRadiusX, upperRadiusY);
		Rectangle centreRect = Rectangle(centreXCoord, clippingRectangle.centre.y, centreRadiusX, centreRadiusY);
		Rectangle lowerRect = Rectangle(centre.x, clippingRectangle.centre.y - centreRadiusY - lowerRadiusY, lowerRadiusX, lowerRadiusY);

		v.push_back(upperRect);
		v.push_back(centreRect);
		v.push_back(lowerRect);
	}
	
	return v;
}

bool Rectangle::operator==(Rectangle r)
{
	return centre == r.centre && radiusX == r.radiusX && radiusY == r.radiusY;
}

bool Rectangle::operator!=(Rectangle r)
{
	return centre != r.centre || radiusX != r.radiusX || radiusY != r.radiusY;
}

void Rectangle::print()
{
	std::cout << "Rectangle {(" << centre.x << "," << centre.y << "), " << radiusX << ", " << radiusY << "}";
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
	Rectangle r2 = Rectangle(1.0, 1.0, 0.0, 0.0);
	assert(r2.area() == 0.0);

	// Test set three
	Rectangle r3 = Rectangle(-5.0, -3.0, 13.0, 11.0);
	assert(r3.area() == 572.0);

	// Test set four
	Rectangle r4 = Rectangle(-5.0, -3.0, 13.3, 11.2);
	assert(r4.area() == 595.84);
}

void testRectangleComputeExpansionArea()
{
	// Test computing expansion area for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 2.0);
	Point p1 = Point(9.0, 5.0);
	assert(r1.computeExpansionArea(p1) == 23.0);

	// Test computing expansion area for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 3.0, 1.5);
	Rectangle r3 = Rectangle(5.0, 3.0, 1.0, 1.0);
	assert(r2.computeExpansionArea(r3) == 30.0);

	// Test computing expansion area for a rectangle partially inside another rectangle
	Rectangle r4 = Rectangle(-9.0, 8.0, 1.0, 4.0);
	Rectangle r5 = Rectangle(-8.0, 4.0, 1.0, 1.0);
	assert(r4.computeExpansionArea(r5) == 11.0);

	// Test computing expansion area for a rectangle wholly inside another rectangle
	Rectangle r6 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r7 = Rectangle(-2.5, -5.0, 0.5, 2.0);
	assert(r6.computeExpansionArea(r7) == 0.0);
}

void testRectangleExpansion()
{
	// Test computing expansion for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 2.0);
	Point p1 = Point(9.0, 5.0);
	r1.expand(p1);
	assert(r1.radiusX == 5.5);
	assert(r1.radiusY == 2.5);
	assert(r1.centre == Point(3.5, 7.5));

	// Test computing expansion for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 3.0, 1.5);
	Rectangle r3 = Rectangle(5.0, 3.0, 1.0, 1.0);
	r2.expand(r3);
	assert(r2.radiusX == 3.0);
	assert(r2.radiusY == 4.0);
	assert(r2.centre == Point(4.0, 0.0));

	// Test computing expansion area for a rectangle partially inside another rectangle
	Rectangle r4 = Rectangle(-9.0, 8.0, 1.0, 4.0);
	Rectangle r5 = Rectangle(-8.0, 4.0, 1.0, 1.0);
	r4.expand(r5);
	assert(r4.radiusX == 1.5);
	assert(r4.radiusY == 4.5);
	assert(r4.centre == Point(-8.5, 7.5));

	// Test computing expansion area for a rectangle wholly inside another rectangle
	Rectangle r6 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r7 = Rectangle(-2.5, -5.0, 0.5, 2.0);
	r6.expand(r7);
	assert(r6.radiusX == 3.0);
	assert(r6.radiusY == 5.0);
	assert(r6.centre == Point(-4.0, -3.0));
}

void testRectangleIntersection()
{
	// Test corner on corner intersection
	Rectangle r1 = Rectangle(43.2, 42.0, 5.6, 5.9);
	Rectangle r2 = Rectangle(59.4, 48.2, 13.4, 13.4);
	assert(r1.intersectsRectangle(r2));

	// Test side on side intersection
	Rectangle r3 = Rectangle(-4.2, 1.7, 15.0, 2.0);
	Rectangle r4 = Rectangle(-4.2, 0.0, 5.0, 2.0);
	assert(r3.intersectsRectangle(r4));
}

void testRectanglePointContainment()
{
	// Test set one
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	Point p1 = Point(0.0, 0.0);
	assert(r1.containsPoint(p1));

	// Test set two
	Rectangle r2 = Rectangle(1.0, 1.0, 2.0, 2.0);
	Point p2 = Point(-1.0, -1.0);
	assert(r2.containsPoint(p2));

	// Test set three
	Rectangle r3 = Rectangle(1.0, 1.0, 2.0, 2.0);
	Point p3 = Point(217.3, 527.7);
	assert(!r3.containsPoint(p3));

	// Test set four
	Rectangle r4 = Rectangle(1.0, 1.0, 2.0, 2.0);
	Point p4 = Point(3.0, 1.0);
	assert(r4.containsPoint(p4));
}

void testRectangleSplits()
{
	// Test set one
	Rectangle r1 = Rectangle(3.0, 3.0, 2.0, 2.0);
	Rectangle r2 = Rectangle(6.0, 6.0, 2.0, 2.0);
	std::vector<Rectangle> v = r1.splitRectangle(r2);
	assert(v.size() == 2);
	assert(v[0].radiusX == 0.5);
	assert(v[0].radiusY == 1.5);
	assert(v[1].radiusX == 1.5);
	assert(v[1].radiusY == 2.0);

	// Test set two
	r1 = Rectangle(2.0, 2.0, 2.0, 2.0);
	r2 = Rectangle(3.5, -0.5, 2.5, 1.5);
	v = r1.splitRectangle(r2);
	assert(v.size() == 2);
	assert(v[0].radiusX == 1.5);
	assert(v[0].radiusY == 1.5);
	assert(v[1].radiusX == 0.5);
	assert(v[1].radiusY == 2.0);

	// Test set three
	r1 = Rectangle(-3.5, -2.5, 3.5, 2.5);
	r2 = Rectangle(-6, -5.5, 2, 2.5);
	v = r1.splitRectangle(r2);
	assert(v.size() == 2);
	assert(v[0].radiusX == 1.5);
	assert(v[0].radiusY == 1.5);
	assert(v[1].radiusX == 2.0);
	assert(v[1].radiusY == 2.5);

	// Test set four
	r1 = Rectangle(2.0, 4.0, 2.0, 4.0);
	r2 = Rectangle(5.0, 5.0, 2.0, 2.0);
	v = r1.splitRectangle(r2);
	assert(v.size() == 3);
	assert(v[0].radiusX == 2.0);
	assert(v[0].radiusY == 0.5);
	assert(v[1].radiusX == 1.5);
	assert(v[1].radiusY == 2.0);
	assert(v[2].radiusX == 2.0);
	assert(v[2].radiusY == 1.5);

	// Test set five
	r1 = Rectangle(2.0, 4.0, 2.0, 4.0);
	r2 = Rectangle(-1.0, 5.0, 2.0, 2.0);
	v = r1.splitRectangle(r2);
	assert(v.size() == 3);
	assert(v[0].radiusX == 2.0);
	assert(v[0].radiusY == 0.5);
	assert(v[1].radiusX == 1.5);
	assert(v[1].radiusY == 2.0);
	assert(v[2].radiusX == 2.0);
	assert(v[2].radiusY == 1.5);

	// Test set six
	r1 = Rectangle(4.0, 2.0, 4.0, 2.0);
	r2 = Rectangle(3.0, 4.0, 2.0, 2.0);
	v = r1.splitRectangle(r2);
	assert(v.size() == 3);
	assert(v[0].radiusX == 0.5);
	assert(v[0].radiusY == 2.0);
	assert(v[1].radiusX == 2.0);
	assert(v[1].radiusY == 1.0);
	assert(v[2].radiusX == 1.5);
	assert(v[2].radiusY == 2.0);
}
