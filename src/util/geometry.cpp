#include <util/geometry.h>

Point::Point()
{
	float infinity = std::numeric_limits<float>::infinity();
	memset(values, infinity, sizeof(float) * dimensions);
}

Point::Point(float x, float y)
{
	values[0] = x;
	values[1] = y;
}

Point::Point(float *values)
{
	for (unsigned i = 0; i < dimensions; ++i)
	{
		this->values[i] = values[i];
	}
}

float Point::operator[](unsigned index) const
{
	return values[index];
}

void Point::operator<<(Point &p)
{
	// Set this point to be the Hamming minimum between itself and p
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] = values[i] < p[i] ? values[i] : p[i];
	}
}

void Point::operator>>(Point &p)
{
	// Set this point to be the Hamming maximum between itself and p
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] = values[i] > p[i] ? values[i] : p[i];
	}
}

bool operator<(const Point &lhs, const Point &rhs)
{
	bool result = true;

	for (unsigned i = 0; i < dimensions && result; ++i)
	{
		result = result && lhs[i] < rhs[i];
	}

	return result;
}

bool operator>(const Point &lhs, const Point &rhs)
{
	bool result = true;

	for (unsigned i = 0; i < dimensions && result; ++i)
	{
		result = result && lhs[i] > rhs[i];
	}

	return result;
}

bool operator<=(const Point &lhs, const Point &rhs)
{
	bool result = true;

	for (unsigned i = 0; i < dimensions && result; ++i)
	{
		result = result && lhs[i] <= rhs[i];
	}

	return result;
}

bool operator>=(const Point &lhs, const Point &rhs)
{
	bool result = true;

	for (unsigned i = 0; i < dimensions && result; ++i)
	{
		result = result && lhs[i] >= rhs[i];
	}

	return result;
}

bool operator==(const Point &lhs, const Point &rhs)
{
	bool result = true;

	for (unsigned i = 0; i < dimensions && result; ++i)
	{
		result = result && lhs[i] == rhs[i];
	}

	return result;
}

bool operator!=(const Point &lhs, const Point &rhs)
{
	bool result = false;

	for (unsigned i = 0; i < dimensions && !result; ++i)
	{
		result = result || lhs[i] != rhs[i];
	}

	return result;
}

std::ostream& operator<<(std::ostream& os, const Point& point)
{
	os.precision(std::numeric_limits<double>::max_digits10+3);
	os << "(" << point[0];
	for (unsigned i = 1; i < dimensions; ++i)
	{
		os << ", " << point[i];
	}
	os << ")";

	return os;
}

Rectangle::Rectangle()
{
	lowerLeft = Point();
	upperRight = Point();
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
	float a = fabs(upperRight[0] - lowerLeft[0]);

	for (unsigned i = 1; i < dimensions; ++i)
	{
		a = a * fabs(upperRight[i] - lowerLeft[i]);
	}

	return a;
}

float Rectangle::computeIntersectionArea(Rectangle givenRectangle)
{
	// Quick exit
	if (!intersectsRectangle(givenRectangle))
	{
		return 0;
	}

	float intersectionArea = fabs(fmin(upperRight[0], givenRectangle.upperRight[0]) - fmax(lowerLeft[0], givenRectangle.lowerLeft[0]));

	for (unsigned i = 1; i < dimensions; ++i)
	{
		intersectionArea = intersectionArea * fabs(fmin(upperRight[i], givenRectangle.upperRight[i]) - fmax(lowerLeft[i], givenRectangle.lowerLeft[i]));
	}

	return intersectionArea;
}

float Rectangle::computeExpansionArea(Point givenPoint)
{
	// Expanded rectangle area computed directly
	float expandedArea = fabs(fmin(lowerLeft[0], givenPoint[0]) - fmax(upperRight[0], givenPoint[0]));

	for (unsigned i = 1; i < dimensions; ++i)
	{
		expandedArea = expandedArea * fabs(fmin(lowerLeft[i], givenPoint[i]) - fmax(upperRight[i], givenPoint[i]));
	}

	// Compute the difference
	return expandedArea - area();
}

float Rectangle::computeExpansionArea(Rectangle givenRectangle)
{
	// Expanded rectangle area computed directly
	float expandedArea = fabs(fmin(givenRectangle.lowerLeft[0], lowerLeft[0]) - fmax(givenRectangle.upperRight[0], upperRight[0]));

	for (unsigned i = 0; i < dimensions; ++i)
	{
		expandedArea = expandedArea * fabs(fmin(givenRectangle.lowerLeft[i], lowerLeft[i]) - fmax(givenRectangle.upperRight[i], upperRight[i]));
	}

	// Compute the difference
	return expandedArea - area();
}

void Rectangle::expand(Point givenPoint)
{
	lowerLeft << givenPoint;
	upperRight >> givenPoint;
}

void Rectangle::expand(Rectangle givenRectangle)
{
	lowerLeft << givenRectangle.lowerLeft;
	upperRight >> givenRectangle.upperRight;
}

void Rectangle::shrink(std::vector<Rectangle> &pointSet, std::vector<Point> &constraintSet)
{
}

void Rectangle::shrink(std::vector<Rectangle> &pointSet, std::vector<Rectangle> &constraintSet)
{
}

bool Rectangle::aligned(Rectangle givenRectangle)
{
	unsigned unalignedDimensions = 0;

	for (unsigned d = 0; d < dimensions && unalignedDimensions < 2; ++d)
	{
		if (lowerLeft[d] != givenRectangle.lowerLeft[d] && upperRight[d] != givenRectangle.upperRight[d])
		{
			unalignedDimensions++;
		}
	}

	return unalignedDimensions == 1;
}

bool Rectangle::alignedOpposingBorders(Rectangle givenRectangle)
{
	bool result = false;

	for (unsigned i = 0; i < dimensions && !result; ++i)
	{
		result = result || lowerLeft[i] == givenRectangle.upperRight[i] || upperRight[i] == givenRectangle.lowerLeft[i];
	}

	return result;
}

bool Rectangle::intersectsRectangle(Rectangle givenRectangle)
{
	// Compute the range intersections
	bool interval = true;

	for (unsigned i = 0; i < dimensions && interval; ++i)
	{
		interval =
			interval &&
			((lowerLeft[i] <= givenRectangle.lowerLeft[i] && givenRectangle.lowerLeft[i] <= upperRight[i]) ||
			(givenRectangle.lowerLeft[i] <= lowerLeft[i] && lowerLeft[i] <= givenRectangle.upperRight[i]));
	}

	return interval;
}

bool Rectangle::strictIntersectsRectangle(Rectangle givenRectangle)
{
	// Compute the range intersections
	bool interval = true;

	for (unsigned i = 0; i < dimensions && interval; ++i)
	{
		interval =
			interval &&
			((lowerLeft[i] < givenRectangle.lowerLeft[i] && givenRectangle.lowerLeft[i] < upperRight[i]) ||
			(givenRectangle.lowerLeft[i] < lowerLeft[i] && lowerLeft[i] < givenRectangle.upperRight[i]));
	}

	return interval;
}

bool Rectangle::borderOnlyIntersectsRectanlge(Rectangle givenRectangle)
{
	return intersectsRectangle(givenRectangle) && alignedOpposingBorders(givenRectangle);
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
	r.lowerLeft >> lowerLeft;
	r.upperRight << upperRight;

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
	Rectangle slots[2 * dimensions];
	for (unsigned i = 0; i < 2 * dimensions; ++i)
	{
		slots[i] = Rectangle();
	}

	// // If the top slab is defined this will be revised downward
	// float maxVertical = upperRight.y;
	// float minVertical = lowerLeft.y;

	// // Define the top slab
	// if (lowerLeft.y < clippingRectangle.upperRight.y && clippingRectangle.upperRight.y < upperRight.y)
	// {
	// 	maxVertical = clippingRectangle.upperRight.y;
	// 	slots[0] = Rectangle(lowerLeft.x, maxVertical, upperRight.x, upperRight.y);
	// }

	// // Define the bottom slab
	// if (lowerLeft.y < clippingRectangle.lowerLeft.y && clippingRectangle.lowerLeft.y < upperRight.y)
	// {
	// 	minVertical = clippingRectangle.lowerLeft.y;
	// 	slots[2] = Rectangle(lowerLeft.x, lowerLeft.y, upperRight.x, minVertical);
	// }

	// // Define the right vertical
	// if (lowerLeft.x < clippingRectangle.upperRight.x && clippingRectangle.upperRight.x < upperRight.x)
	// {
	// 	slots[1] = Rectangle(clippingRectangle.upperRight.x, minVertical, upperRight.x, maxVertical);
	// }

	// // Define the left vertical
	// if (lowerLeft.x < clippingRectangle.lowerLeft.x && clippingRectangle.lowerLeft.x < upperRight.x)
	// {
	// 	slots[3] = Rectangle(lowerLeft.x, minVertical, clippingRectangle.lowerLeft.x, maxVertical);
	// }

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

bool operator==(const Rectangle &lhs, const Rectangle &rhs)
{
	return lhs.lowerLeft == rhs.lowerLeft && lhs.upperRight == rhs.upperRight;
}

bool operator!=(const Rectangle &lhs, const Rectangle &rhs)
{
	return lhs.lowerLeft != rhs.lowerLeft || lhs.upperRight != rhs.upperRight;
}

std::ostream& operator<<(std::ostream& os, const Rectangle& rectangle)
{
	os.precision(std::numeric_limits<double>::max_digits10+3);
	os << "[" << rectangle.lowerLeft << "; " << rectangle.upperRight << "]";
	return os;
}

IsotheticPolygon::IsotheticPolygon()
{
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
	DASSERT(basicRectangles[minIndex].containsPoint(givenPoint));

	// By expanding naively the expanded rectangle could intersect some of our own rectangles.
	// To fix this take the expanded rectangle out of the polygon, treat it as a clipping
	// rectangle, then put it back.
	Rectangle swap = basicRectangles[minIndex];
	basicRectangles[minIndex] = basicRectangles[basicRectangles.size() - 1];
	basicRectangles.pop_back();
	DASSERT(swap.containsPoint(givenPoint));
	DASSERT(!containsPoint(givenPoint));
	increaseResolution(swap);
	basicRectangles.push_back(swap);
	DASSERT(containsPoint(givenPoint));

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
	DASSERT(unique());
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
	DASSERT(unique());
	DASSERT(basicRectangles[minIndex].containsPoint(givenPoint));

	// By expanding naively the expanded rectangle could intersect some of our own rectangles.
	// To fix this take the expanded rectangle out of the polygon, treat it as a clipping
	// rectangle, then put it back.
	Rectangle swap = basicRectangles[minIndex];
	basicRectangles[minIndex] = basicRectangles.back();
	basicRectangles.pop_back();
	DASSERT(swap.containsPoint(givenPoint));
	DASSERT(unique());

	DEXEC(basicRectangles.push_back(swap));
	DEXEC(basicRectangles.pop_back());

	increaseResolution(swap);

	DASSERT(swap.containsPoint(givenPoint));
	DASSERT(unique());
	DEXEC(basicRectangles.push_back(swap));
	DEXEC(basicRectangles.pop_back());
	DASSERT(unique());

	// Ensure the expansion remains within the constraint polygon
	DASSERT(constraintPolygon.containsPoint(givenPoint));
	Rectangle swapIntersectionPiece;
	Rectangle rAtInfinity = Rectangle();
	DPRINT2("swap", swap);
	for (unsigned i = 0; i < constraintPolygon.basicRectangles.size(); ++i)
	{
		swapIntersectionPiece = swap.intersection(constraintPolygon.basicRectangles[i]);
		DPRINT2("constraintRectanlge ", constraintPolygon.basicRectangles[i]);
		DPRINT2("swapIntersectionPiece ", swapIntersectionPiece);
		if (swapIntersectionPiece != rAtInfinity)
		{
			DPRINT1("Pushing swapIntersectionPiece");
			basicRectangles.push_back(swapIntersectionPiece);
		}
	}
	DASSERT((containsPoint(givenPoint)));
	DASSERT(unique());

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

bool IsotheticPolygon::intersectsPolygon(IsotheticPolygon &givenPolygon)
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

bool IsotheticPolygon::borderOnlyIntersectsRectanlge(Rectangle givenRectangle)
{
	// Final decision
	bool result = false;

	for (auto r : basicRectangles)
	{
		if (!r.intersectsRectangle(givenRectangle))
		{
			continue;
		}
		else if (!r.alignedOpposingBorders(givenRectangle))
		{
			result = false;
			break;
		}
		else
		{
			result = true;
		}
	}

	return result;
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

std::vector<Rectangle> IsotheticPolygon::intersection(Rectangle givenRectangle)
{
	std::vector<Rectangle> v;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		Rectangle r = basicRectangles[i].intersection(givenRectangle);
		if (r != Rectangle())
		{
			v.push_back(r);
		}
	}

	return v;
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
	// Fragment each of our constiuent rectangles based on the clippingRectangle. This may result in
	// no splitting of the constiuent rectangles and that's okay.
	std::vector<Rectangle> extraRectangles;

	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		// Break the rectangle
		DPRINT2("basic: ", basicRectangles[i]);
		DPRINT2("clipping: ", clippingRectangle);
		std::vector<Rectangle> fragments = basicRectangles[i].fragmentRectangle(clippingRectangle);
		DPRINT2("fragments.size() = ", fragments.size());

		// Add the fragments to extras
		for (unsigned j = 0; j < fragments.size(); ++j)
		{
			extraRectangles.push_back(fragments[j]);
			DPRINT2("j = ", j);
			DASSERT(fragments[j] != Rectangle());
		}
		DPRINT2("extraRectangles.size() = ", extraRectangles.size());
	}

	basicRectangles.clear();

	// The new bounding polygon is now entirely defined by the fragments in extraRectangles
	basicRectangles.swap(extraRectangles);
	DPRINT2("basicRectangles.size() = ", basicRectangles.size());

	// Cleanup
	refine();
}

void IsotheticPolygon::increaseResolution(IsotheticPolygon &clippingPolygon)
{
	for (unsigned i = 0; i < clippingPolygon.basicRectangles.size(); ++i)
	{
		DPRINT2("    Removing ", clippingPolygon.basicRectangles[i]);
		increaseResolution(clippingPolygon.basicRectangles[i]);
		DPRINT2("    Result ", this);
	}

	// Cleanup
	refine();
}

void IsotheticPolygon::refine()
{
	DPRINT2("Before refinement ", this);

	for (unsigned k = 0; k < 7 && basicRectangles.size(); ++k)
	{
		Rectangle r;
		std::vector<Rectangle> rectangleSetRefined;

		for (unsigned d = 0; d < dimensions; ++d)
		{
			// Refine along x
			r = basicRectangles[0];
			std::sort(basicRectangles.begin(), basicRectangles.end(), [d](Rectangle a, Rectangle b){return a.lowerLeft[d] < b.lowerLeft[d];});
			r = basicRectangles[0];
			for (unsigned i = 1; i < basicRectangles.size(); ++i)
			{
				if (r.aligned(basicRectangles[i]) && r.intersectsRectangle(basicRectangles[i]))
				{
					r.expand(basicRectangles[i]);
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
	}

	DPRINT2("After refinement ", this);
	sort(true);
}

void IsotheticPolygon::sort(bool min, unsigned d)
{
	if (min)
	{
		std::sort(basicRectangles.begin(), basicRectangles.end(), [d](Rectangle a, Rectangle b){return a.lowerLeft[d] < b.lowerLeft[d];});
	}
	else
	{
		std::sort(basicRectangles.begin(), basicRectangles.end(), [d](Rectangle a, Rectangle b){return a.upperRight[d] < b.upperRight[d];});
	}
}

bool IsotheticPolygon::unique()
{
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < basicRectangles.size(); ++j)
		{
			if (i != j && basicRectangles[i] == basicRectangles[j])
			{
				DPRINT5("Rectangle ", i, " and ", j, " are the same");
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
			DPRINT3("Rectangle ", i, " is degenerate");
			return false;
		}
	}

	return true;
}

bool operator==(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs)
{
	if (lhs.basicRectangles.size() != rhs.basicRectangles.size())
	{
		return false;
	}
	else
	{
		for (unsigned i = 0; i < rhs.basicRectangles.size(); ++i)
		{
			if (lhs.basicRectangles[i] != rhs.basicRectangles[i])
			{
				return false;
			}
		}
	}

	return true;
}

bool operator!=(const IsotheticPolygon &lhs, const IsotheticPolygon &rhs)
{
	if (lhs.basicRectangles.size() != rhs.basicRectangles.size())
	{
		return true;
	}
	else
	{
		for (unsigned i = 0; i < rhs.basicRectangles.size(); ++i)
		{
			if (lhs.basicRectangles[i] != rhs.basicRectangles[i])
			{
				return true;
			}
		}
	}

	return false;
}

std::ostream& operator<<(std::ostream& os, const IsotheticPolygon& polygon)
{
	os.precision(std::numeric_limits<double>::max_digits10 + 3);
	os << "|";
	for (auto rectangle : polygon.basicRectangles)
	{
		os << rectangle;
	}
	os << "|";
	return os;
}
