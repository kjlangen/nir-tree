#include "util/geometry.h"
#include <iostream>

Point::Point()
{
	this->x = 0;
	this->y = 0;
}

Point::Point(int x, int y)
{
	this->x = x;
	this->y = y;
}

Rectangle::Rectangle(Point p1, Point p2)
{
	std::cout << "Building rectangle" << std::endl;
	std::cout << "Point 1: {" << p1.x << ", " << p1.y << "}" << std::endl;
	std::cout << "Point 1: {" << p2.x << ", " << p2.y << "}" << std::endl;
	this->lowerLeft = p1;
	this->upperRight = p2;
}

Rectangle Rectangle::computeExpansionArea(Rectangle requestedRectangle)
{
	int minX = 4; //min();
	int minY = 6; //min();
	Point lowerLeft = Point();//(minX, minY);

	int maxX = 7; //max();
	int maxY = 8; //max();
	Point upperRight = Point();//(maxX, maxY);

	int area = (maxX - minX) * (maxY - minY);
	Rectangle expandedRectangle = Rectangle(lowerLeft, upperRight);

	return expandedRectangle;
}

bool Rectangle::intersectsRectangle(Rectangle requestedRectangle)
{
	// Compute the range intersections
	bool intervalX = this->lowerLeft.x <= requestedRectangle.upperRight.x;
	bool intervalXPrime = requestedRectangle.lowerLeft.x <= this->upperRight.x;
	bool intervalY = this->lowerLeft.x <= requestedRectangle.upperRight.y;
	bool intervalYPrime = requestedRectangle.lowerLeft.x <= this->upperRight.y;

	// println!("[intersectsRectangle] Data is self = {:?} and requested = {:?}", self, requestedRectangle);
	// println!("[intersectsRectangle] Containment information is ({}, {}, {}, {})", intervalX, intervalXPrime, intervalY, intervalYPrime);

	if (intervalX && intervalXPrime && intervalY && intervalYPrime)
	{
		return true;
	}

	return false;
}

bool Rectangle::containsPoint(Point requestedPoint)
{
	bool intervalX = this->lowerLeft.x <= requestedPoint.x && requestedPoint.x <= this->upperRight.x;
	bool intervalY = this->lowerLeft.y <= requestedPoint.y && requestedPoint.y <= this->upperRight.y;
}
