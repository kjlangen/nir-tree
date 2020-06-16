#ifndef __SPATIALINDEX__
#define __SPATIALINDEX__

#include <iostream>
#include <util/geometry.h>

class SpatialIndex {
	virtual std::vector<Point> search(Point requestedPoint) = 0;
	virtual std::vector<Point> search(Rectangle requestedRectangle) = 0;
	virtual void insert(Point givenPoint) = 0;
	virtual void remove(Point givenPoint) = 0;
	virtual unsigned checksum() = 0;
};

#endif  // __SPATIALINDEX__
