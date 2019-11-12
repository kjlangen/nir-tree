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
	return this->x == p.x && this->y == p.y;
}

void Point::print()
{
	std::cout << "(" << x << ", " << y << ")" << std::endl;
}

Rectangle::Rectangle(double x, double y, double radiusX, double radiusY)
{
	this->centre = Point(x, y);
	this->radiusX = radiusX;
	this->radiusY = radiusY;
}

// TODO: Optimize
double Rectangle::computeExpansionArea(Rectangle requestedRectangle)
{
	// Min/Max along X
	double minX = std::min(centre.x - radiusX, requestedRectangle.centre.x - requestedRectangle.radiusX);
	double maxX = std::max(centre.x + radiusX, requestedRectangle.centre.x + requestedRectangle.radiusX);

	// Min/Max along Y
	double minY = std::min(centre.y - radiusY, requestedRectangle.centre.y - requestedRectangle.radiusY);
	double maxY = std::max(centre.y + radiusY, requestedRectangle.centre.y + requestedRectangle.radiusY);

	double newArea = (maxX - minX) * (maxY - minY);
	double existingArea = (radiusX + radiusX) * (radiusY + radiusY);

	return newArea - existingArea;
}

double Rectangle::computeExpansionArea(Point givenPoint)
{
	// Distance from centre
	Point centroidDistance = Point(fabs(centre.x - givenPoint.x), fabs(centre.y - givenPoint.y));

	double newRadiusX = std::max(centroidDistance.x, radiusX);
	double newRadiusY = std::max(centroidDistance.y, radiusY);

	return (newRadiusX * newRadiusY) - (radiusX * radiusY);
}

// TODO: This is a naive expand and we should instead shift the centre not just increase radius
void Rectangle::expand(Point givenPoint)
{
	// Distance from centre
	Point centroidDistance = Point(fabs(centre.x - givenPoint.x), fabs(centre.y - givenPoint.y));

	double newRadiusX = std::max(centroidDistance.x, radiusX);
	double newRadiusY = std::max(centroidDistance.y, radiusY);

	radiusX = newRadiusX;
	radiusY = newRadiusY;
}

bool Rectangle::intersectsRectangle(Rectangle requestedRectangle)
{
	// Compute the range intersections
	bool intervalX = abs(centre.x - requestedRectangle.centre.x) < radiusX + requestedRectangle.radiusX;
	bool intervalY = abs(centre.y - requestedRectangle.centre.y) < radiusY + requestedRectangle.radiusY;

	// println!("[intersectsRectangle] Data is self = {:?} and requested = {:?}", self, requestedRectangle);
	// println!("[intersectsRectangle] Containment information is ({}, {})", intervalX, intervalY);

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
	// We can have side-on-side/corner-on-corner intersections with 0, 1, or 2 corners inside the
	// other rectangle

	std::vector<Rectangle> v;
	Point centroidDistance = Point(fabs(centre.x - clippingRectangle.centre.x), fabs(centre.y - clippingRectangle.centre.y));
	std::cout << "centroidDistance = (" << centroidDistance.x << ", " << centroidDistance.y << ")" << std::endl;
	bool xMeasure = centroidDistance.x + clippingRectangle.radiusX < radiusX;
	bool yMeasure = centroidDistance.y + clippingRectangle.radiusY < radiusY;

	if (!(xMeasure || yMeasure))
	{
		// ***************************************************************************************
		// This is all in the case of 1 corner contained
		// ***************************************************************************************

		// The overlap size in each dimension can be comptued by considering how much larger the sum of
		// the two radii are than the distance between centres. Consider when the rectangles touch only
		// and exactly on one side. Then the distance between the centres is the same as the sum of the
		// two radii, and as the centres get closer the radii do not shrink.
		
		double overlapWidth = clippingRectangle.radiusX + radiusX - centroidDistance.x;
		double overlapHeight = clippingRectangle.radiusY + radiusY - centroidDistance.y;
		std::cout << "overlapWidth = " << overlapWidth << " overlapHeight = " << overlapHeight << std::endl;

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
		// In this case the overlap will be the full extent of one of the rectangles in one dimension
		// and then the other dimension will exhibit overlap in the same way as the previous 1
		// corner case.

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

void Rectangle::print()
{
	std::cout << "Rectangle {(" << centre.x << "," << centre.y << "), " << radiusX << ", " << radiusY << "}" << std::endl;
}
