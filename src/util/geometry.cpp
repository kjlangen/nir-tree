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
	float infinity = std::numeric_limits<float>::infinity();
	lowerLeft = Point(infinity, infinity);
	upperRight = Point(infinity, infinity);
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

// Does not work if rectangles do not intersect
float Rectangle::computeIntersectionArea(Rectangle givenRectangle)
{
	return fabs((fmin(upperRight.x, givenRectangle.upperRight.x) - fmax(lowerLeft.x, givenRectangle.lowerLeft.x)) *
				(fmin(upperRight.y, givenRectangle.upperRight.y) - fmax(lowerLeft.y, givenRectangle.lowerLeft.y)));
}

float Rectangle::computeExpansionArea(Point givenPoint)
{
	// Expanded rectangle area computed directly
	float expandedArea = fabs((fmin(lowerLeft.x, givenPoint.x) - fmax(upperRight.x, givenPoint.x)) *
							  (fmin(lowerLeft.y, givenPoint.y) - fmax(upperRight.y, givenPoint.y)));

	// Compute the difference
	return expandedArea - area();
}

float Rectangle::computeExpansionArea(Rectangle givenRectangle)
{
	// Expanded rectangle area computed directly
	float expandedArea = fabs((fmin(givenRectangle.lowerLeft.x, lowerLeft.x) - fmax(givenRectangle.upperRight.x, upperRight.x)) *
							  (fmin(givenRectangle.lowerLeft.y, lowerLeft.y) - fmax(givenRectangle.upperRight.y, upperRight.y)));

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

bool Rectangle::intersectsRectangle(Rectangle givenRectangle)
{
	// Compute the range intersections
	bool intervalX = lowerLeft.x <= givenRectangle.upperRight.x && upperRight.x >= givenRectangle.lowerLeft.x;
	bool intervalY = lowerLeft.y <= givenRectangle.upperRight.y && upperRight.y >= givenRectangle.lowerLeft.y;

	return intervalX && intervalY;
}

bool Rectangle::strictIntersectsRectangle(Rectangle givenRectangle)
{
	// Compute the range intersections
	bool intervalX = lowerLeft.x < givenRectangle.upperRight.x && upperRight.x > givenRectangle.lowerLeft.x;
	bool intervalY = lowerLeft.y < givenRectangle.upperRight.y && upperRight.y > givenRectangle.lowerLeft.y;

	return intervalX && intervalY;
}

bool Rectangle::containsPoint(Point requestedPoint)
{
	return lowerLeft <= requestedPoint && requestedPoint <= upperRight;
}

Rectangle Rectangle::intersectionRectangle(Rectangle clippingRectangle)
{
	// Return rectangle
	Rectangle r = Rectangle(lowerLeft, upperRight);

	// Revise inward whenever the clippingRectangle is inside us

	// Define the top
	if (lowerLeft.y < clippingRectangle.upperRight.y && clippingRectangle.upperRight.y < upperRight.y)
	{
		r.upperRight.y = clippingRectangle.upperRight.y;
	}

	// Define the bottom
	if (lowerLeft.y < clippingRectangle.lowerLeft.y && clippingRectangle.lowerLeft.y < upperRight.y)
	{
		r.lowerLeft.y = clippingRectangle.lowerLeft.y;
	}

	// Define the right vertical
	if (lowerLeft.x < clippingRectangle.upperRight.x && clippingRectangle.upperRight.x < upperRight.x)
	{
		r.upperRight.x = clippingRectangle.upperRight.x;
	}

	// Define the left vertical
	if (lowerLeft.x < clippingRectangle.lowerLeft.x && clippingRectangle.lowerLeft.x < upperRight.x)
	{
		r.lowerLeft.x = clippingRectangle.lowerLeft.x;
	}

	return r;
}

std::vector<Rectangle> Rectangle::fragmentRectangle(Rectangle clippingRectangle)
{
	// Return vector
	std::vector<Rectangle> v;

	// Quick exit
	if (!intersectsRectangle(clippingRectangle) || computeIntersectionArea(clippingRectangle) == 0)
	{
		v.push_back(*this);
		return v;
	}

	// This array represents the rectangles that might have to be created when fragmenting
	// 0 -> top slab
	// 2 -> bottom slab
	// 1 -> right vertical, not intersecting the top or bottom slabs
	// 3 -> left vertical, not intersecting the top or bottom slabs
	Rectangle slots[] = { Rectangle(), Rectangle(), Rectangle(), Rectangle() };

	// If the top slab is defined this will be revised downward
	float maxVertical = upperRight.y;
	float minVertical = lowerLeft.y;

	// Define the top slab
	if (lowerLeft.y < clippingRectangle.upperRight.y && clippingRectangle.upperRight.y < upperRight.y)
	{
		maxVertical = clippingRectangle.upperRight.y;
		slots[0] = Rectangle(lowerLeft.x, maxVertical, upperRight.x, upperRight.y);
	}

	// Define the bottom slab
	if (lowerLeft.y < clippingRectangle.lowerLeft.y && clippingRectangle.lowerLeft.y < upperRight.y)
	{
		minVertical = clippingRectangle.lowerLeft.y;
		slots[2] = Rectangle(lowerLeft.x, lowerLeft.y, upperRight.x, minVertical);
	}

	// Define the right vertical
	if (lowerLeft.x < clippingRectangle.upperRight.x && clippingRectangle.upperRight.x < upperRight.x)
	{
		slots[1] = Rectangle(clippingRectangle.upperRight.x, minVertical, upperRight.x, maxVertical);
	}

	// Define the left vertical
	if (lowerLeft.x < clippingRectangle.lowerLeft.x && clippingRectangle.lowerLeft.x < upperRight.x)
	{
		slots[3] = Rectangle(lowerLeft.x, minVertical, clippingRectangle.lowerLeft.x, maxVertical);
	}

	// TODO: Maybe optimize this away and just return the array?
	float infinity = std::numeric_limits<float>::infinity();
	for (unsigned i = 0; i < 4; ++i)
	{
		if (slots[i].upperRight.y != infinity)
		{
			v.push_back(slots[i]);
		}
	}

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

IsotheticPolygon::IsotheticPolygon()
{
	// TODO: Should we create a rectangle of 0 size?
}

IsotheticPolygon::IsotheticPolygon(Rectangle baseRectangle)
{
	basicRectangles.push_back(baseRectangle);
}

IsotheticPolygon::IsotheticPolygon(const IsotheticPolygon &basePolygon)
{
	basicRectangles.insert(basicRectangles.end(), basePolygon.basicRectangles.begin(), basePolygon.basicRectangles.end());
}

float IsotheticPolygon::area()
{
	float area = 0.0;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		area += basicRectangles[i].area();
	}

	return area;
}

float IsotheticPolygon::computeIntersectionArea(Rectangle givenRectangle)
{
	float runningTotal = 0.0;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if (basicRectangles[i].intersectsRectangle(givenRectangle))
		{
			runningTotal += basicRectangles[i].computeIntersectionArea(givenRectangle);
		}
	}

	return runningTotal;
}

float IsotheticPolygon::computeExpansionArea(Point givenPoint)
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

float IsotheticPolygon::computeExpansionArea(Rectangle givenRectangle)
{
	// Take the minimum expansion area
	float minArea = basicRectangles[0].computeExpansionArea(givenRectangle);
	float evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(givenRectangle);
		minArea = evalArea < minArea ? evalArea : minArea;
	}

	return minArea;
}

Rectangle IsotheticPolygon::boundingBox()
{
	assert(basicRectangles.size() > 0);

	Rectangle r = basicRectangles[0];

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		r.expand(basicRectangles[i]);
	}

	return r;
}

// TODO: Incomplete
// float IsotheticPolygon::computeExpansionArea(IsotheticPolygon requestedIsotheticPolygon)
// {
// 	// Take the minimum expansion area
// 	float minArea = basicRectangles[0].computeExpansionArea(givenRectangle);
// 	// float evalArea;

// 	// for (unsigned i = 1; i < basicRectangles.size(); ++i)
// 	// {
// 	// 	evalArea = basicRectangles[i].computeExpansionArea(givenRectangle);
// 	// 	minArea = evalArea < minArea ? evalArea : minArea;
// 	// }

// 	return minArea;
// }

// TODO: This is a simple functional first-pass at expanding. It could get far more complex and most
// likely will as we deal with the special case of spirals
// TODO: There's a special case where this doesn't work when the expansion of the isothetic polygon
// bridges a U shape indent of the constraint polygon. The new expansion might be non-contigous
// after being intersected with the constraint polygon.
void IsotheticPolygon::expand(Point givenPoint, IsotheticPolygon &constraintPolygon)
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

	// By expanding naively the expanded rectangle could intersect some other of our own rectangles.
	// To fix this we move the newly expanded out of the array (setting it's slot to the rectangle
	// at infinity which intersects nothing) and then pretend it is a clipping rectangle for
	// which we need to increase our resolution. After increasing our resolution we put the newly
	// expanded rectangle back in at its old spot. This way we don't have to resize the vec more
	// than normally required by clipping/upping the resolution.
	Rectangle bb = basicRectangles[minIndex];
	basicRectangles[minIndex] = Rectangle(); // The degenerate rectangle at infinity
	increaseResolution(bb);
	basicRectangles[minIndex] = bb;

	// Intersect bb with the constraint polygon to get a set of polygons who are a subset of bb and
	// whose union equals bb with any area outside the constraint polyogn removed
	std::vector<Rectangle> v;
	for (unsigned i = 0; i < constraintPolygon.basicRectangles.size(); ++i)
	{
		if (constraintPolygon.basicRectangles[i].intersectsRectangle(bb))
		{
			v.push_back(constraintPolygon.basicRectangles[i].intersectionRectangle(bb));
		}
	}

	basicRectangles[minIndex] = v[0];
	for (unsigned i = 1; i < v.size(); ++i)
	{
		basicRectangles.push_back(v[i]);
	}
}

void IsotheticPolygon::expand(Point givenPoint)
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

	// By expanding naively the expanded rectangle could intersect some other of our own rectangles.
	// To fix this we move the newly expanded out of the array (setting it's slot to the rectangle
	// at infinity which intersects nothing) and then pretend it is a clipping rectangle for
	// which we need to increase our resolution. After increasing our resolution we put the newly
	// expanded rectangle back in at its old spot. This way we don't have to resize the vec at all.
	Rectangle bb = basicRectangles[minIndex];
	basicRectangles[minIndex] = Rectangle(); // The degenerate rectangle at infinity
	increaseResolution(bb);
	basicRectangles[minIndex] = bb;
}

// TODO: This is a simple functional first-pass at expanding. It could get far more complex and most
// likely will as we deal with the special case of spirals
void IsotheticPolygon::expand(Rectangle givenRectangle)
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

void IsotheticPolygon::expand(IsotheticPolygon &targetPolygon, IsotheticPolygon &constraintPolygon)
{
	// Expand one of our rectangles to enclose the minimum bounding rectangle of the targetPolygon
	unsigned minIndex = 0;
	Rectangle bb = targetPolygon.boundingBox();
	float minArea = basicRectangles[0].computeExpansionArea(bb);
	float evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(bb);
		if (evalArea < minArea)
		{
			minArea = evalArea;
			minIndex = i;
		}
	}

	basicRectangles[minIndex].expand(bb);

	// By expanding naively the expanded rectangle could intersect some other of our own rectangles.
	// To fix this we move the newly expanded out of the array (setting it's slot to the rectangle
	// at infinity which intersects nothing) and then pretend it is a clipping rectangle for
	// which we need to increase our resolution. After increasing our resolution we put the newly
	// expanded rectangle back in at its old spot. This way we don't have to resize the vec at all.
	bb = basicRectangles[minIndex];
	basicRectangles[minIndex] = Rectangle(); // The degenerate rectangle at infinity
	increaseResolution(bb);
	basicRectangles[minIndex] = bb;

	// Intersect bb with the constraint polygon to get a set of polygons who are a subset of bb and
	// whose union equals bb with any area outside the constraint polyogn removed
	std::vector<Rectangle> v;
	for (unsigned i = 0; i < constraintPolygon.basicRectangles.size(); ++i)
	{
		if (constraintPolygon.basicRectangles[i].intersectsRectangle(bb))
		{
			v.push_back(constraintPolygon.basicRectangles[i].intersectionRectangle(bb));
		}
	}

	basicRectangles[minIndex] = v[0];
	for (unsigned i = 1; i < v.size(); ++i)
	{
		basicRectangles.push_back(v[i]);
	}
}

bool IsotheticPolygon::intersectsRectangle(Rectangle &givenRectangle)
{
	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if(givenRectangle.intersectsRectangle(basicRectangles[i]))
		{
			return true;
		}
	}

	return false;
}

// TODO: This can be optimized to be O(Nlog(N)) and take O(N) space, the algorithm is detailed in
// introduction to computational geometry by Preparata
bool IsotheticPolygon::intersectsRectangle(IsotheticPolygon &givenPolygon)
{
	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if (givenPolygon.intersectsRectangle(basicRectangles[i]))
		{
			return true;
		}
	}

	return false;
}

bool IsotheticPolygon::containsPoint(Point requestedPoint)
{
	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if(basicRectangles[i].containsPoint(requestedPoint))
		{
			return true;
		}
	}

	return false;
}

void IsotheticPolygon::intersection(IsotheticPolygon &constraintPolygon)
{
	std::vector<Rectangle> v;
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < constraintPolygon.basicRectangles.size(); ++j)
		{
			Rectangle r = basicRectangles[i].intersectionRectangle(constraintPolygon.basicRectangles[j]);

			if (r.area() != 0)
			{
				v.push_back(r);
			}
		}
	}
	basicRectangles.clear();
	basicRectangles.swap(v);
}

void IsotheticPolygon::increaseResolution(Rectangle clippingRectangle)
{
	// Fragment each of our constiuent rectangles based on the clippingRectangle. This may result in
	// no splitting of the constiuent rectangles and that's okay.
	std::vector<Rectangle> extraRectangles;

	for (unsigned i = basicRectangles.size() - 1; i < basicRectangles.size(); --i)
	{
		// Break the rectangle
		std::vector<Rectangle> v = basicRectangles[i].fragmentRectangle(clippingRectangle);

		// Add the fragments to extras
		extraRectangles.insert(extraRectangles.end(), v.begin(), v.end());
	}

	basicRectangles.clear();

	// The new bounding polygon is now entirely defined by the fragments in extraRectangles
	basicRectangles.swap(extraRectangles);
}

void IsotheticPolygon::increaseResolution(IsotheticPolygon &clippingPolygon)
{
	for (unsigned i = 0; i < clippingPolygon.basicRectangles.size(); ++i)
	{
		std::cout << "    Removing "; clippingPolygon.basicRectangles[i].print();
		increaseResolution(clippingPolygon.basicRectangles[i]);
		std::cout << "    Result "; print();
	}
}

bool IsotheticPolygon::operator==(IsotheticPolygon r)
{
	// ???
	return false;
}

bool IsotheticPolygon::operator!=(IsotheticPolygon r)
{
	// ???
	return false;
}

void IsotheticPolygon::print()
{
	std::cout << "IsotheticPolygon |";
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		basicRectangles[i].lowerLeft.print();
		basicRectangles[i].upperRight.print();
		std::cout << '|';
	}
	std::cout << std::endl;
}
