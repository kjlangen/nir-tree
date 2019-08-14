#ifndef __GEOMETRY__
#define __GEOMETRY__

class Point
{
public:
	int x;
	int y;

	Point();
	Point(int x, int y);
};

class Rectangle
{
public:
	Point lowerLeft;
	Point upperRight;

	Rectangle(Point lowerLeft, Point upperRight);
	Rectangle computeExpansionArea(Rectangle requestedRectangle);
	bool intersectsRectangle(Rectangle requestedRectangle);
	bool containsPoint(Point requestedPoint);
};

#endif
