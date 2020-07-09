#ifndef __INDEX__
#define __INDEX__

#include <iostream>
#include <util/geometry.h>

class Index {
public:
	virtual std::vector<Point> search(Point requestedPoint) const { throw std::exception(); };
	virtual std::vector<Point> search(Rectangle requestedRectangle) const { throw std::exception(); };
	virtual void insert(Point givenPoint) { throw std::exception(); };
	virtual void remove(Point givenPoint) { throw std::exception(); };
	virtual unsigned checksum() { throw std::exception(); };
};

#endif  // __INDEX__
