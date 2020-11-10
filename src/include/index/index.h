#ifndef __INDEX__
#define __INDEX__

#include <iostream>
#include <util/geometry.h>

class Index
{
	public:
		virtual std::vector<Point> exhaustiveSearch(Point requestedPoint) = 0;
		virtual std::vector<Point> search(Point requestedPoint) = 0;
		virtual std::vector<Point> search(Rectangle requestedRectangle) = 0;
		virtual void insert(Point givenPoint) = 0;
		virtual void remove(Point givenPoint) = 0;
		virtual unsigned checksum() = 0;
		virtual void stat() = 0;
		virtual void print() = 0;
};

#endif
