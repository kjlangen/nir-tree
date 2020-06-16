#ifndef __INDEX__
#define __INDEX__

#include <iostream>
#include <util/geometry.h>

class Index {
public:
	virtual std::vector<Point> search(Point requestedPoint) { return std::vector<Point>(); };
	virtual std::vector<Point> search(Rectangle requestedRectangle) { return std::vector<Point>(); };
	virtual void insert(Point givenPoint) {};
	virtual void remove(Point givenPoint) {};
	virtual unsigned checksum() { return 0; };
};

#endif  // __INDEX__
