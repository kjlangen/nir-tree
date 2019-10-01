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

	// println!("[computeExpansionArea] Data is self = {:?} and requested = {:?}", self, requestedRectangle);
	// println!("[computeExpansionArea] Area information is current = {} and expanded = {}", existingArea, newArea);

	return newArea - existingArea;
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
	bool intervalX = abs(centre.x - requestedPoint.x) <= radiusX;
	bool intervalY = abs(centre.y - requestedPoint.y) <= radiusY;

	return intervalX && intervalY;
}

std::vector<Rectangle> Rectangle::splitRectangle(Rectangle clippingRectangle)
{
	// We can have side-on-side/corner-on-corner intersections with 0, 1, or 2 corners inside the
	// other rectangle

	std::vector<Rectangle> v;

	if (true/* 1 corner */)
	{
		// ***************************************************************************************
		// This is all in the case of 1 corner contained
		// ***************************************************************************************

		// The overlap size in each dimension can be comptued by considering how much larger the sum of
		// the two radii are than the distance between centres. Consider when the rectangles touch only
		// and exactly on one side. Then the distance between the centres is the same as the sum of the
		// two radii, and as the centres get closer the radii do not shrink.
		
		double overlapWidth = clippingRectangle.radiusX + radiusX - abs(clippingRectangle.centre.x - centre.x);
		double overlapHeight = clippingRectangle.radiusY + radiusY - abs(clippingRectangle.centre.y - centre.y);

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
	else if (true/* 2 corners */)
	{
		// ***************************************************************************************
		// This is all in the case of 2 corners contained
		// ***************************************************************************************

		// There are three new rectangles that must be built here.
		// In this case the overlap will be the full extent of one of the rectangles in one dimension
		// and then the other dimension will exhibit overlap in the same way as the previous 1
		// corner case.

		// Note in this case we assume the clipping rectangle is smaller than our rectangle. Maybe
		// this always has to be the case because we can't just compress ourselves, we are a
		// paricular height for a reason.

		double centroidDistanceX = abs(clippingRectangle.centre.x - centre.x);
		double overlapHeight = clippingRectangle.radiusY + radiusY - abs(clippingRectangle.centre.y - centre.y);

		double centreRadiusX = clippingRectangle.radiusX;
		double rightRadiusX = (radiusX - centreRadiusX - centroidDistanceX) / 2;
		double leftRadiusX = radiusX - rightRadiusX - centreRadiusX;

		double leftRadiusY = radiusY;
		double centreRadiusY = radiusY - overlapHeight / 2;
		double rightRadiusY = radiusY;

		Rectangle centreRect = Rectangle(centre.x + centroidDistanceX, centre.y - overlapHeight / 2, centreRadiusX, centreRadiusY);
		Rectangle rightRect = Rectangle(centreRect.centre.x + centreRadiusX + leftRadiusX, centre.y, rightRadiusX, rightRadiusY);
		Rectangle leftRect = Rectangle(centreRect.centre.x - centreRadiusX - rightRadiusX, centre.y, leftRadiusX, leftRadiusY);

		v.push_back(centreRect);
		v.push_back(rightRect);
		v.push_back(leftRect);
	}
	
	return v;
}

void Rectangle::print()
{
	std::cout << "Rectangle {(" << centre.x << "," << centre.y << "), " << radiusX << "," << radiusY << "}" << std::endl;
}
