#include <util/geometry.h>

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

float Rectangle::computeIntersectionArea(Rectangle givenRectangle)
{
	// Quick exit
	if (!intersectsRectangle(givenRectangle))
	{
		return 0;
	}

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

void Rectangle::expand(Point givenPoint)
{
	lowerLeft = Point(fmin(lowerLeft.x, givenPoint.x), fmin(lowerLeft.y, givenPoint.y));
	upperRight = Point(fmax(upperRight.x, givenPoint.x), fmax(upperRight.y, givenPoint.y));
}

void Rectangle::expand(Rectangle givenRectangle)
{
	lowerLeft = Point(fmin(givenRectangle.lowerLeft.x, lowerLeft.x), fmin(givenRectangle.lowerLeft.y, lowerLeft.y));
	upperRight = Point(fmax(givenRectangle.upperRight.x, upperRight.x), fmax(givenRectangle.upperRight.y, upperRight.y));
}

bool Rectangle::intersectsRectangle(Rectangle givenRectangle)
{
	// Compute the range intersections
	bool intervalX = lowerLeft.x <= givenRectangle.lowerLeft.x && givenRectangle.lowerLeft.x <= upperRight.x;
	intervalX = intervalX || (givenRectangle.lowerLeft.x <= lowerLeft.x && lowerLeft.x <= givenRectangle.upperRight.x);

	bool intervalY = lowerLeft.y <= givenRectangle.lowerLeft.y && givenRectangle.lowerLeft.y <= upperRight.y;
	intervalY = intervalY || (givenRectangle.lowerLeft.y <= lowerLeft.y && lowerLeft.y <= givenRectangle.upperRight.y);

	return intervalX && intervalY;
}

bool Rectangle::strictIntersectsRectangle(Rectangle givenRectangle)
{
	// Compute the range intersections strictly
	bool intervalX = lowerLeft.x < givenRectangle.lowerLeft.x && givenRectangle.lowerLeft.x < upperRight.x;
	intervalX = intervalX || (givenRectangle.lowerLeft.x < lowerLeft.x && lowerLeft.x < givenRectangle.upperRight.x);

	bool intervalY = lowerLeft.y < givenRectangle.lowerLeft.y && givenRectangle.lowerLeft.y < upperRight.y;
	intervalY = intervalY || (givenRectangle.lowerLeft.y < lowerLeft.y && lowerLeft.y < givenRectangle.upperRight.y);

	// std::cout << "xy = " << intervalX << intervalY  << std::endl;

	return intervalX && intervalY;
}

bool Rectangle::borderOnlyIntersectsRectanlge(Rectangle givenRectangle)
{
	bool a = lowerLeft.x == givenRectangle.upperRight.x || lowerLeft.y == givenRectangle.upperRight.y;
	bool b = upperRight.x == givenRectangle.lowerLeft.x || upperRight.y == givenRectangle.lowerLeft.y;

	return a || b;
}

bool Rectangle::containsPoint(Point givenPoint)
{
	return lowerLeft <= givenPoint && givenPoint <= upperRight;
}

bool Rectangle::strictContainsPoint(Point givenPoint)
{
	return lowerLeft < givenPoint && givenPoint < upperRight;
}

bool Rectangle::containsRectangle(Rectangle givenRectangle)
{
	return containsPoint(givenRectangle.lowerLeft) && containsPoint(givenRectangle.upperRight);
}

// NOTE: Will return the degenerate inf rectangle if the intersection is border-only or non-existent
Rectangle Rectangle::intersection(Rectangle clippingRectangle)
{
	// Return rectangle
	Rectangle r = Rectangle(lowerLeft, upperRight);

	// Quick exit
	if (!intersectsRectangle(clippingRectangle) || borderOnlyIntersectsRectanlge(clippingRectangle))
	{
		return Rectangle();
	}

	// Revise inward whenever the clippingRectangle is inside us
	r.lowerLeft = Point(fmax(clippingRectangle.lowerLeft.x, lowerLeft.x), fmax(clippingRectangle.lowerLeft.y, lowerLeft.y));
	r.upperRight = Point(fmin(clippingRectangle.upperRight.x, upperRight.x), fmin(clippingRectangle.upperRight.y, upperRight.y));

	return r;
}

std::vector<Rectangle> Rectangle::fragmentRectangle(Rectangle clippingRectangle)
{
	// Return vector
	std::vector<Rectangle> v;

	// Quick exit
	if (!intersectsRectangle(clippingRectangle) || borderOnlyIntersectsRectanlge(clippingRectangle))
	{
		assert(*this != Rectangle());
		v.push_back(Rectangle(lowerLeft, upperRight));
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
	Rectangle rAtInfinity = Rectangle();
	for (unsigned i = 0; i < 4; ++i)
	{
		if (slots[i] != rAtInfinity)
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
	basicRectangles.clear();
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
		runningTotal += basicRectangles[i].computeIntersectionArea(givenRectangle);
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
	assert(basicRectangles[minIndex].containsPoint(givenPoint));

	// By expanding naively the expanded rectangle could intersect some of our own rectangles.
	// To fix this take the expanded rectangle out of the polygon, treat it as a clipping
	// rectangle, then put it back.
	Rectangle swap = basicRectangles[minIndex];
	basicRectangles[minIndex] = basicRectangles[basicRectangles.size() - 1];
	basicRectangles.pop_back();
	assert(swap.containsPoint(givenPoint));
	assert(!containsPoint(givenPoint));
	increaseResolution(swap);
	basicRectangles.push_back(swap);
	assert(containsPoint(givenPoint));

	// Cleanup
	refine();
}

// TODO: This is a simple functional first-pass at expanding. It could get far more complex and most
// likely will as we deal with the special case of spirals
// TODO: There's a special case where this doesn't work when the expansion of the isothetic polygon
// bridges a U shape indent of the constraint polygon. The new expansion might be non-contigous
// after being intersected with the constraint polygon.
void IsotheticPolygon::expand(Point givenPoint, IsotheticPolygon &constraintPolygon)
{
	assert(unique());
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
	assert(unique());
	assert(basicRectangles[minIndex].containsPoint(givenPoint));
	assert(contiguous());

	// By expanding naively the expanded rectangle could intersect some of our own rectangles.
	// To fix this take the expanded rectangle out of the polygon, treat it as a clipping
	// rectangle, then put it back.
	Rectangle swap = basicRectangles[minIndex];
	basicRectangles[minIndex] = basicRectangles.back();//[basicRectangles.size() - 1];
	basicRectangles.pop_back();
	assert(swap.containsPoint(givenPoint));
	assert(unique());

	basicRectangles.push_back(swap);
	assert(contiguous());
	basicRectangles.pop_back();

	increaseResolution(swap);

	assert(swap.containsPoint(givenPoint));
	assert(unique());
	basicRectangles.push_back(swap);
	assert(contiguous());
	basicRectangles.pop_back();
	// basicRectangles.push_back(swap);
	// assert(unique());

	// Ensure the expansion remains within the constraint polygon
	assert(constraintPolygon.containsPoint(givenPoint));
	Rectangle swapIntersectionPiece;
	Rectangle rAtInfinity = Rectangle();
	// std::cout << "swap "; swap.print();
	for (unsigned i = 0; i < constraintPolygon.basicRectangles.size(); ++i)
	{
		swapIntersectionPiece = swap.intersection(constraintPolygon.basicRectangles[i]);
		if (swapIntersectionPiece != rAtInfinity)
		{
			// std::cout << "constraintRectanlge "; constraintPolygon.basicRectangles[i].print();
			// std::cout << "swapIntersectionPiece "; swapIntersectionPiece.print();
			// std::cout << "Pushing swapIntersectionPiece" << std::endl;
			basicRectangles.push_back(swapIntersectionPiece);
		}
	}
	assert(containsPoint(givenPoint));
	assert(unique());
	assert(contiguous());

	// Cleanup
	refine();
}

// TODO: FIX! Make similar to above
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

	// By expanding naively the expanded rectangle could intersect some of our own rectangles.
	// To fix this take the expanded rectangle out of the polygon, treat it as a clipping
	// rectangle, then put it back.
	Rectangle swap = basicRectangles[minIndex];
	basicRectangles[minIndex] = basicRectangles[basicRectangles.size() - 1];
	basicRectangles.pop_back();
	increaseResolution(swap);
	basicRectangles.push_back(swap);

	// Cleanup
	refine();
}

// TODO: FIX! Make similar to above
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

	// By expanding naively the expanded rectangle could intersect some of our own rectangles.
	// To fix this take the expanded rectangle out of the polygon, treat it as a clipping
	// rectangle, then put it back.
	Rectangle swap = basicRectangles[minIndex];
	basicRectangles[minIndex] = basicRectangles[basicRectangles.size() - 1];
	basicRectangles.pop_back();
	increaseResolution(swap);
	basicRectangles.push_back(swap);

	// Ensure the expansion remains within the constraint polygon
	std::vector<Rectangle> v;
	unsigned endIndex = basicRectangles.size() - 1;
	for (unsigned i = 0; i < constraintPolygon.basicRectangles.size(); ++i)
	{
		swap = basicRectangles[endIndex].intersection(constraintPolygon.basicRectangles[i]);
		v.push_back(swap);
		if (swap == basicRectangles[endIndex])
		{
			break;
		}
	}
	basicRectangles.pop_back();
	basicRectangles.insert(basicRectangles.end(), v.begin(), v.end());

	// Cleanup
	refine();
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

/*
bool IsotheticPolygon::quickIntersectsRectangle(Rectangle &givenRectangle)
{
	// Compute the root
	unsigned lstack[basicRectangles.size()];
	unsigned rstack[basicRectangles.size()];
	unsigned topOfStack = 1;
	unsigned root;

	// Prime the stack
	lstack[0] = 0;
	rstack[0] = basicRectangles.size() - 1;

	// DFS for intersections
	for (;topOfStack != 0;)
	{
		// std::cout << "topOfStack = " << topOfStack << std::endl;
		lBound = lstack[topOfStack - 1];
		rBound = rstack[topOfStack - 1];
		root = (lBound + rBound) / 2;
		topOfStack--;
		// std::cout << "lBound = " << lBound << std::endl;
		// std::cout << "rBound = " << rBound << std::endl;
		// std::cout << "root = " << root << std::endl;
		// std::cout << "topOfStack = " << topOfStack << std::endl;

		// Check against root, short circuit the search if we find a positive
		// std::cout << "DFS1" << std::endl;
		if (givenRectangle.intersectsRectangle(basicRectangles[root]))
		{
			return true;
		}

		// std::cout << "DFS3" << std::endl;
		// Check for leaf
		if (lBound == rBound)
		{
			continue;
		}

		// Might intersect things on the left
		// std::cout << "DFS4" << std::endl;
		if (root != lBound && queryRectangle.r.lowerLeft.x <= vTagged[root].r.lowerLeft.x)
		{
			// std::cout << "DFS5" << std::endl;
			// Go Left
			lstack[topOfStack] = lBound;
			rstack[topOfStack] = root - 1;
			++topOfStack;
			// querySubtree(queryRectangle, weights, vTagged, lBound, root - 1);
		}

		// Might intersect things on the right
		// std::cout << "DFS6" << std::endl;
		if (root != rBound && queryRectangle.r.upperRight.x >= vTagged[root].r.lowerLeft.x && queryRectangle.r.lowerLeft.x <= weights[root])
		{
			// std::cout << "DFS7" << std::endl;
			// Go Right
			lstack[topOfStack] = root + 1;
			rstack[topOfStack] = rBound;
			++topOfStack;
			// querySubtree(queryRectangle, weights, vTagged, root + 1, rBound);
		}
	}
}
*/

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
	Rectangle r;
	Rectangle rAtInfinity = Rectangle();
	std::vector<Rectangle> v;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < constraintPolygon.basicRectangles.size(); ++j)
		{
			r = basicRectangles[i].intersection(constraintPolygon.basicRectangles[j]);
			if (r != rAtInfinity)
			{
				v.push_back(r);
			}
		}
	}
	basicRectangles.clear();
	basicRectangles.swap(v);

	// Cleanup
	refine();
}

void IsotheticPolygon::increaseResolution(Rectangle clippingRectangle)
{
	// std::cout << "this B: "; print();
	// Fragment each of our constiuent rectangles based on the clippingRectangle. This may result in
	// no splitting of the constiuent rectangles and that's okay.
	std::vector<Rectangle> extraRectangles;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		// Break the rectangle
		// std::cout << std::endl << "basic: "; basicRectangles[i].print();
		// std::cout << "clipping: "; clippingRectangle.print();
		std::vector<Rectangle> fragments = basicRectangles[i].fragmentRectangle(clippingRectangle);
		// std::cout << "fragments.size() = " << fragments.size() << std::endl;

		// Add the fragments to extras
		for (unsigned j = 0; j < fragments.size(); ++j)
		{
			extraRectangles.push_back(fragments[j]);
			// if (fragments[j] == Rectangle())
			// {
			// 	std::cout << "fragments[" << j << "] = "; fragments[j].print();
			// }
			// assert(fragments[j] != Rectangle());
		}
		// std::cout << "extraRectangles.size() = " << extraRectangles.size() << std::endl;
	}

	basicRectangles.clear();

	// The new bounding polygon is now entirely defined by the fragments in extraRectangles
	basicRectangles.swap(extraRectangles);
	// std::cout << "basicRectangles.size() = " << basicRectangles.size() << std::endl;

	// Cleanup
	refine();
}

void IsotheticPolygon::increaseResolution(IsotheticPolygon &clippingPolygon)
{
	// std::cout << "this A: "; print();
	for (unsigned i = 0; i < clippingPolygon.basicRectangles.size(); ++i)
	{
		// std::cout << "    Removing "; clippingPolygon.basicRectangles[i].print();
		increaseResolution(clippingPolygon.basicRectangles[i]);
		// std::cout << "    Result "; print();
	}

	// Cleanup
	refine();
}

void IsotheticPolygon::refine()
{
	// std::cout << "Before |";
	// unsigned j;
	// for (j = 0; j < basicRectangles.size(); ++j)
	// {
	// 	basicRectangles[j].lowerLeft.print();
	// 	basicRectangles[j].upperRight.print();
	// 	std::cout << '|';
	// }
	// if (j == 0)
	// {
	// 	std::cout << '|';
	// }
	// std::cout << std::endl;

	for (unsigned k = 0; k < 7 && basicRectangles.size(); ++k)
	{
		Rectangle r;
		std::vector<Rectangle> rectangleSetRefined;

		// Refine along x
		r = basicRectangles[0];
		std::sort(basicRectangles.begin(), basicRectangles.end(), [](Rectangle a, Rectangle b){return a.lowerLeft.x < b.lowerLeft.x;});
		r = basicRectangles[0];
		for (unsigned i = 1; i < basicRectangles.size(); ++i)
		{
			if (r.lowerLeft.x == basicRectangles[i].lowerLeft.x && r.upperRight.x == basicRectangles[i].upperRight.x && r.intersectsRectangle(basicRectangles[i]))
			{
				r.lowerLeft.y = fmin(r.lowerLeft.y, basicRectangles[i].lowerLeft.y);
				r.upperRight.y = fmax(r.upperRight.y, basicRectangles[i].upperRight.y);
			}
			else
			{
				rectangleSetRefined.push_back(r);
				r = basicRectangles[i];
			}
		}
		rectangleSetRefined.push_back(r);
		basicRectangles.swap(rectangleSetRefined);
		rectangleSetRefined.clear();

		// Refine along y
		std::sort(basicRectangles.begin(), basicRectangles.end(), [](Rectangle a, Rectangle b){return a.lowerLeft.y < b.lowerLeft.y;});
		r = basicRectangles[0];
		for (unsigned i = 1; i < basicRectangles.size(); ++i)
		{
			if (r.lowerLeft.y == basicRectangles[i].lowerLeft.y && r.upperRight.y == basicRectangles[i].upperRight.y && r.intersectsRectangle(basicRectangles[i]))
			{
				r.lowerLeft.x = fmin(r.lowerLeft.x, basicRectangles[i].lowerLeft.x);
				r.upperRight.x = fmax(r.upperRight.x, basicRectangles[i].upperRight.x);
			}
			else
			{
				rectangleSetRefined.push_back(r);
				r = basicRectangles[i];
			}
		}
		rectangleSetRefined.push_back(r);
		basicRectangles.swap(rectangleSetRefined);
		rectangleSetRefined.clear();
	}

	// std::cout << "After |";
	// for (j = 0; j < basicRectangles.size(); ++j)
	// {
	// 	basicRectangles[j].lowerLeft.print();
	// 	basicRectangles[j].upperRight.print();
	// 	std::cout << '|';
	// }
	// if (j == 0)
	// {
	// 	std::cout << '|';
	// }
	// std::cout << std::endl;

	sort(true);
}

// TODO: Convert each occurance of .x to used fully generalized dimension d
void IsotheticPolygon::sort(bool min, unsigned d)
{
	if (min)
	{
		std::sort(basicRectangles.begin(), basicRectangles.end(), [](Rectangle a, Rectangle b){return a.lowerLeft.x < b.lowerLeft.x;});
	}
	else
	{
		std::sort(basicRectangles.begin(), basicRectangles.end(), [](Rectangle a, Rectangle b){return a.upperRight.x < b.upperRight.x;});
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

bool IsotheticPolygon::unique()
{
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < basicRectangles.size(); ++j)
		{
			if (i != j && basicRectangles[i] == basicRectangles[j])
			{
				std::cout << "Rectangle " << i << " and " << j << " are the same" << std::endl;
				return false;
			}
		}
	}

	return true;
}

bool IsotheticPolygon::infFree()
{
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if (basicRectangles[i] == Rectangle())
		{
			std::cout << "Rectangle " << i << " is degenerate" << std::endl;
			return false;
		}
	}

	return true;
}

bool IsotheticPolygon::contiguous()
{
	unsigned basicsSize = basicRectangles.size();
	bool graph[basicsSize][basicsSize];
	std::memset(graph, false, basicsSize * basicsSize);
	for (unsigned i = 0; i < basicsSize; ++i)
	{
		for (unsigned j = 0; j < i; ++j)
		{
			if (basicRectangles[i].intersectsRectangle(basicRectangles[j]))
			{
				graph[i][j] = true;
				graph[j][i] = true;
			}
		}
	}

	unsigned currentVertex;
	bool explored[basicsSize];
	std::queue<unsigned> explorationQ;

	std::memset(explored, false, basicsSize);

	explorationQ.push(basicsSize / 2);
	for (;explorationQ.size();)
	{
		currentVertex = explorationQ.front();
		explorationQ.pop();

		if (explored[currentVertex])
		{
			continue;
		}

		// Connect children of this node to the tree
		for (unsigned neighbouringVertex = 0; neighbouringVertex < basicsSize; ++neighbouringVertex)
		{
			if (graph[currentVertex][neighbouringVertex] && !explored[neighbouringVertex])
			{
				explorationQ.push(neighbouringVertex);
			}
		}

		// Done with this vertex
		explored[currentVertex] = true;
	}

	bool contigous = true;
	for (unsigned i = 0; i < basicsSize; ++i)
	{
		contigous = contigous && explored[i];
	}

	if (!contigous)
	{
		std::cout << "Not contigous! "; print();
	}

	return contigous;
}

void IsotheticPolygon::print()
{
	std::cout << "IsotheticPolygon |";
	unsigned i;
	for (i = 0; i < basicRectangles.size(); ++i)
	{
		basicRectangles[i].lowerLeft.print();
		basicRectangles[i].upperRight.print();
		std::cout << '|';
	}
	if (i == 0)
	{
		std::cout << '|';
	}
	std::cout << std::endl;
}
