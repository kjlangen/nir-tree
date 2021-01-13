#include <util/geometry.h>

Point Point::atInfinity = Point(std::numeric_limits<double>::infinity());
Point Point::atNegInfinity = Point(- std::numeric_limits<double>::infinity());
Point Point::atOrigin = Point(0.0);

Point::Point()
{
	double infinity = std::numeric_limits<double>::infinity();

	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] = infinity;
	}
}

Point::Point(double x, double y)
{
	values[0] = x;
	values[1] = y;
}

Point::Point(double value)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] = value;
	}
}

Point &Point::operator-=(const Point &rhs)
{
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] -= rhs.values[i];
	}

	return *this;
}

Point &Point::operator+=(const Point &rhs)
{
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] += rhs.values[i];
	}

	return *this;
}

Point &Point::operator/=(double scalar)
{
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] /= scalar;
	}

	return *this;
}

Point &Point::operator*=(double scalar)
{
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] *= scalar;
	}

	return *this;
}

Point &Point::operator*=(const Point &rhs)
{
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] *= rhs[i];
	}

	return *this;
}

double &Point::operator[](unsigned index)
{
	return values[index];
}

const double Point::operator[](unsigned index) const
{
	return values[index];
}

Point &Point::operator<<(const Point &p)
{
	// Set this point to be the Hamming minimum between itself and p
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] = values[i] < p[i] ? values[i] : p[i];
	}

	return *this;
}

Point &Point::operator>>(const Point &p)
{
	// Set this point to be the Hamming maximum between itself and p
	for (unsigned i = 0; i < dimensions; ++i)
	{
		values[i] = values[i] > p[i] ? values[i] : p[i];
	}

	return *this;
}

Point operator-(const Point &lhs, const Point &rhs)
{
	Point r;

	for (unsigned i = 0; i < dimensions; ++i)
	{
		r.values[i] = lhs[i] - rhs[i];
	}

	return r;
}

Point operator+(const Point &lhs, const Point &rhs)
{
	Point r;

	for (unsigned i = 0; i < dimensions; ++i)
	{
		r.values[i] = lhs[i] + rhs[i];
	}

	return r;
}

Point operator*(const Point &lhs, const double scalar)
{
	Point r;

	for (unsigned i = 0; i < dimensions; ++i)
	{
		r.values[i] = lhs[i] * scalar;
	}

	return r;
}

Point operator*(const Point &lhs, const Point &rhs)
{
	Point r;

	for (unsigned i = 0; i < dimensions; ++i)
	{
		r.values[i] = lhs[i] * rhs[i];
	}

	return r;
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

std::ostream& operator<<(std::ostream &os, const Point &p)
{
	os.precision(std::numeric_limits<double>::max_digits10+3);

	os << "(" << p[0];
	for (unsigned d = 1; d < dimensions; ++d)
	{
		os << ", " << p[d];
	}
	os << ")";

	return os;
}

Rectangle Rectangle::atInfinity = Rectangle(Point::atInfinity, Point::atInfinity);
Rectangle Rectangle::atNegInfinity = Rectangle(Point::atNegInfinity, Point::atNegInfinity);
Rectangle Rectangle::atOrigin = Rectangle(Point::atOrigin, Point::atOrigin);

Rectangle::Rectangle()
{
	lowerLeft = Point();
	upperRight = Point();
}

Rectangle::Rectangle(double x, double y, double xp, double yp)
{
	lowerLeft = Point(x, y);
	upperRight = Point(xp, yp);
}

Rectangle::Rectangle(Point lowerLeft, Point upperRight)
{
	this->lowerLeft = lowerLeft;
	this->upperRight = upperRight;
}

double Rectangle::area()
{
	double a = fabs(upperRight[0] - lowerLeft[0]);

	for (unsigned i = 1; i < dimensions; ++i)
	{
		a = a * fabs(upperRight[i] - lowerLeft[i]);
	}

	return a;
}

double Rectangle::computeIntersectionArea(Rectangle givenRectangle)
{
	// Quick exit
	if (!intersectsRectangle(givenRectangle))
	{
		return 0;
	}

	double intersectionArea = fabs(fmin(upperRight[0], givenRectangle.upperRight[0]) - fmax(lowerLeft[0], givenRectangle.lowerLeft[0]));

	for (unsigned i = 1; i < dimensions; ++i)
	{
		intersectionArea = intersectionArea * fabs(fmin(upperRight[i], givenRectangle.upperRight[i]) - fmax(lowerLeft[i], givenRectangle.lowerLeft[i]));
	}

	return intersectionArea;
}

double Rectangle::computeExpansionArea(Point givenPoint)
{
	// // Expanded rectangle area computed directly
	// double expandedArea = fabs(fmin(lowerLeft[0], givenPoint[0]) - fmax(upperRight[0], givenPoint[0]));

	// for (unsigned i = 1; i < dimensions; ++i)
	// {
	// 	expandedArea = expandedArea * fabs(fmin(lowerLeft[i], givenPoint[i]) - fmax(upperRight[i], givenPoint[i]));
	// }

	// // Compute the difference
	// return expandedArea - area();

	Rectangle expanded(lowerLeft, upperRight);
	expanded.lowerLeft << givenPoint;
	expanded.upperRight >> givenPoint;

	return expanded.area() - area();
}

double Rectangle::computeExpansionArea(Rectangle givenRectangle)
{
	// Expanded rectangle area computed directly
	double expandedArea = fabs(fmin(givenRectangle.lowerLeft[0], lowerLeft[0]) - fmax(givenRectangle.upperRight[0], upperRight[0]));

	for (unsigned i = 1; i < dimensions; ++i)
	{
		expandedArea = expandedArea * fabs(fmin(givenRectangle.lowerLeft[i], lowerLeft[i]) - fmax(givenRectangle.upperRight[i], upperRight[i]));
	}

	// Compute the difference
	return expandedArea - area();
}

void Rectangle::expand(Point givenPoint)
{
	DPRINT1("expand");
	DPRINT2("givenPoint = ", givenPoint);
	DPRINT2("current rectangle = ", *this);
	lowerLeft << givenPoint;
	upperRight >> givenPoint;

	DPRINT2("expanded rectangle = ", *this);

	DASSERT(lowerLeft <= upperRight);
	DPRINT1("expand finished");
}

void Rectangle::expand(Rectangle givenRectangle)
{
	lowerLeft << givenRectangle.lowerLeft;
	upperRight >> givenRectangle.upperRight;

	DASSERT(lowerLeft <= upperRight);
}

bool Rectangle::aligned(Rectangle givenRectangle)
{
	unsigned alignedDimensions = 0;

	for (unsigned d = 0; d < dimensions; ++d)
	{
		if (lowerLeft[d] == givenRectangle.lowerLeft[d] && upperRight[d] == givenRectangle.upperRight[d])
		{
			alignedDimensions++;
		}
	}

	return alignedDimensions == dimensions - 1;
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

bool Rectangle::borderOnlyIntersectsRectangle(Rectangle givenRectangle)
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

// NOTE: Will return the degenerate inf rectangle if the intersection is border-only
Rectangle Rectangle::intersection(Rectangle clippingRectangle)
{
	// Return rectangle
	Rectangle r = Rectangle(lowerLeft, upperRight);

	// Quick exit
	if (!intersectsRectangle(clippingRectangle))
	{
		return Rectangle::atInfinity;
	}

	// Revise inward whenever the clippingRectangle is inside us
	r.lowerLeft >> clippingRectangle.lowerLeft;
	r.upperRight << clippingRectangle.upperRight;

	DASSERT(r.lowerLeft <= r.upperRight);

	return r;
}

std::vector<Rectangle> Rectangle::fragmentRectangle(Rectangle clippingRectangle)
{
	DPRINT1("fragmentRectangle");

	// Return vector
	std::vector<Rectangle> v;
	v.reserve(dimensions);

	// Quick exit
	if (!intersectsRectangle(clippingRectangle) || borderOnlyIntersectsRectangle(clippingRectangle))
	{
		DEXEC(Rectangle rAtInfinity);
		DASSERT(*this != rAtInfinity);
		v.push_back(Rectangle(lowerLeft, upperRight));
		return v;
	}

	// Cycle through all dimensions
	Point maximums = Point::atInfinity;
	Point minimums = Point::atNegInfinity;

	DPRINT2("maximums = ", maximums);
	DPRINT2("minimums = ", minimums);

	unsigned finalSize = 0;
	for (unsigned d = 0; d < dimensions; ++d)
	{
		// Alter the original rectangle to respect the clipping rectangle. In the y-dimension this
		// corresponds to making a 'floor' and 'ceiling' rectangle around the clipping rectangle and
		// then doing the same for all other dimensions. If creating a 'floor' or 'ceiling' is not
		// possible because the clipping rectangle clips through the original in a particular
		// dimension then do not create a rectangle

		// Start with copies of the original rectangle
		Rectangle ceiling(lowerLeft, upperRight);
		Rectangle floor(lowerLeft, upperRight);

		DPRINT2("d = ", d);
		DPRINT2("ceiling = ", ceiling);
		DPRINT2("floor = ", floor);

		// Make the original into a 'ceiling' in this d dimension
		if (clippingRectangle.upperRight[d] < upperRight[d])
		{
			// New ceiling
			ceiling.lowerLeft[d] = clippingRectangle.upperRight[d];
			ceiling.upperRight << maximums;
			ceiling.lowerLeft >> minimums;
			DPRINT2("ceiling = ", ceiling);
			maximums[d] = ceiling.lowerLeft[d];
			DPRINT2("maximums = ", maximums);
			v.push_back(ceiling);
			++finalSize;
		}

		// Make the original into a 'floor' in this d dimension
		if (clippingRectangle.lowerLeft[d] > lowerLeft[d])
		{
			// New floor
			floor.upperRight[d] = clippingRectangle.lowerLeft[d];
			floor.upperRight << maximums;
			floor.lowerLeft >> minimums;
			DPRINT2("floor = ", floor);
			minimums[d] = floor.upperRight[d];
			DPRINT2("minimums = ", minimums);
			v.push_back(floor);
			++finalSize;
		}

		DASSERT(ceiling.lowerLeft <= ceiling.upperRight);
		DASSERT(floor.lowerLeft <= floor.upperRight);
	}

	v.resize(finalSize);

	DPRINT1("fragmentRectangle finished");
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

std::ostream& operator<<(std::ostream &os, const Rectangle &rectangle)
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
	boundingBox = baseRectangle;
	basicRectangles.push_back(baseRectangle);
}

IsotheticPolygon::IsotheticPolygon(const IsotheticPolygon &basePolygon)
{
	boundingBox = basePolygon.boundingBox;
	basicRectangles.clear();
	basicRectangles.insert(basicRectangles.end(), basePolygon.basicRectangles.begin(), basePolygon.basicRectangles.end());
}

double IsotheticPolygon::area()
{
	double area = 0.0;

	for (Rectangle basicRectangle : basicRectangles)
	{
		area += basicRectangle.area();
	}

	return area;
}

double IsotheticPolygon::computeIntersectionArea(Rectangle givenRectangle)
{
	double runningTotal = 0.0;

	for (Rectangle basicRectangle : basicRectangles)
	{
		runningTotal += basicRectangle.computeIntersectionArea(givenRectangle);
	}

	return runningTotal;
}

IsotheticPolygon::OptimalExpansion IsotheticPolygon::computeExpansionArea(Point givenPoint)
{
	DPRINT1("computeExpansionArea point");

	// Take the minimum expansion area
	DASSERT(basicRectangles.size() > 0);
	OptimalExpansion expansion = {0, basicRectangles[0].computeExpansionArea(givenPoint)};
	double evalArea;

	unsigned basicsSize = basicRectangles.size();

	for (unsigned i = 1; i < basicsSize; ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(givenPoint);

		if (evalArea < expansion.area)
		{
			expansion.index = i;
			expansion.area = evalArea;
		}
	}

	DPRINT1("computeExpansionArea point finished");
	return expansion;
}

IsotheticPolygon::OptimalExpansion IsotheticPolygon::computeExpansionArea(Rectangle givenRectangle)
{
	DPRINT1("computeExpansionArea rectangle");

	// Take the minimum expansion area
	DASSERT(basicRectangles.size() > 0);
	OptimalExpansion expansion = {0, basicRectangles[0].computeExpansionArea(givenRectangle)};
	double evalArea;

	unsigned basicsSize = basicRectangles.size();

	for (unsigned i = 1; i < basicsSize; ++i)
	{
		evalArea = basicRectangles[i].computeExpansionArea(givenRectangle);
		
		if (evalArea < expansion.area)
		{
			expansion.index = i;
			expansion.area = evalArea;
		}
	}

	DPRINT1("computeExpansionArea rectangle finished");
	return expansion;
}

void IsotheticPolygon::expand(Point givenPoint)
{
	DPRINT1("expand");
	unsigned minIndex = 0;
	double minArea = basicRectangles[0].computeExpansionArea(givenPoint);
	double evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		if (basicRectangles[i].containsPoint(givenPoint))
		{
			minIndex = i;
			break;
		}
		else if ((evalArea = basicRectangles[i].computeExpansionArea(givenPoint)) < minArea)
		{
			minArea = evalArea;
			minIndex = i;
		}
	}

	DPRINT2("expanding rectangle ", minIndex);
	DPRINT1(basicRectangles[minIndex]);
	basicRectangles[minIndex].expand(givenPoint);
	boundingBox.expand(givenPoint);
	DPRINT1(basicRectangles[minIndex]);
	DASSERT(basicRectangles[minIndex].containsPoint(givenPoint));
	DASSERT(basicRectangles.size() > 0);
	DASSERT(boundingBox.lowerLeft != Point::atOrigin);
	DASSERT(basicRectangles[minIndex].lowerLeft <= basicRectangles[minIndex].upperRight);
	// DASSERT(unique());
	DPRINT1("expand finished");
}

void IsotheticPolygon::expand(Point givenPoint, IsotheticPolygon::OptimalExpansion expansion)
{
	DPRINT1("expand");
	DPRINT2("expanding rectangle ", expansion.index);
	DPRINT1(basicRectangles[expansion.index]);
	basicRectangles[expansion.index].expand(givenPoint);
	boundingBox.expand(givenPoint);
	DPRINT1(basicRectangles[expansion.index]);
	DASSERT(basicRectangles[expansion.index].containsPoint(givenPoint));
	DASSERT(basicRectangles.size() > 0);
	DASSERT(boundingBox.lowerLeft != Point::atOrigin);
	DASSERT(basicRectangles[expansion.index].lowerLeft <= basicRectangles[expansion.index].upperRight);
	// DASSERT(unique());
	DPRINT1("expand finished");
}

void IsotheticPolygon::expand(Point givenPoint, IsotheticPolygon &constraintPolygon)
{
	DPRINT1("expand constraint");
	// Expand as if without restraint
	// DASSERT(unique());
	unsigned minIndex = 0;
	double minArea = basicRectangles[0].computeExpansionArea(givenPoint);
	double evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		if (basicRectangles[i].containsPoint(givenPoint))
		{
			minIndex = i;
			break;
		}
		else if ((evalArea = basicRectangles[i].computeExpansionArea(givenPoint)) < minArea)
		{
			minArea = evalArea;
			minIndex = i;
		}
	}

	basicRectangles[minIndex].expand(givenPoint);
	boundingBox.expand(givenPoint);
	// DASSERT(unique());
	DASSERT(basicRectangles[minIndex].containsPoint(givenPoint));

	// Introduce the constraint
	std::vector<Rectangle> intersectionPieces = constraintPolygon.intersection(basicRectangles[minIndex]);

	DASSERT(boundingBox.lowerLeft <= boundingBox.upperRight);
	for (Rectangle intersectionPiece : intersectionPieces)
	{
		DASSERT(intersectionPiece.lowerLeft <= intersectionPiece.upperRight);
	}

	unsigned basicsSize = basicRectangles.size();

	// Replace the original expansion with the constrained expansion
	basicRectangles[minIndex] = basicRectangles[basicsSize - 1];
	basicRectangles.pop_back();
	basicRectangles.reserve(basicsSize + intersectionPieces.size());
	basicRectangles.insert(basicRectangles.end(), intersectionPieces.begin(), intersectionPieces.end());

	DASSERT(basicRectangles.size() > 0);
	DASSERT(boundingBox.lowerLeft != Point::atOrigin);
	// DASSERT(unique());
	DPRINT1("expand constraint finished");
}

void IsotheticPolygon::expand(Point givenPoint, IsotheticPolygon &constraintPolygon, IsotheticPolygon::OptimalExpansion expansion)
{
	DPRINT1("expand constraint");
	// Expand as if without restraint
	// DASSERT(unique());
	basicRectangles[expansion.index].expand(givenPoint);
	boundingBox.expand(givenPoint);
	DASSERT(basicRectangles[expansion.index].containsPoint(givenPoint));
	// DASSERT(unique());

	// Introduce the constraint
	std::vector<Rectangle> intersectionPieces = constraintPolygon.intersection(basicRectangles[expansion.index]);

	DASSERT(boundingBox.lowerLeft <= boundingBox.upperRight);
	for (Rectangle intersectionPiece : intersectionPieces)
	{
		DASSERT(intersectionPiece.lowerLeft <= intersectionPiece.upperRight);
	}

	unsigned basicsSize = basicRectangles.size();

	// Replace the original expansion with the constrained expansion
	basicRectangles[expansion.index] = basicRectangles[basicsSize - 1];
	basicRectangles.pop_back();
	basicRectangles.reserve(basicsSize + intersectionPieces.size());
	basicRectangles.insert(basicRectangles.end(), intersectionPieces.begin(), intersectionPieces.end());

	DASSERT(basicRectangles.size() > 0);
	DASSERT(boundingBox.lowerLeft != Point::atOrigin);
	// DASSERT(unique());
	DPRINT1("expand constraint finished");
}

bool IsotheticPolygon::intersectsRectangle(Rectangle &givenRectangle)
{
	if (!boundingBox.intersectsRectangle(givenRectangle))
	{
		return false;
	}

	// Short circuit checking if we find a positive
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		if (givenRectangle.intersectsRectangle(basicRectangles[i]))
		{
			return true;
		}
	}

	return false;
}

bool IsotheticPolygon::intersectsPolygon(IsotheticPolygon &givenPolygon)
{
	if (!boundingBox.intersectsRectangle(givenPolygon.boundingBox))
	{
		return false;
	}

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

bool IsotheticPolygon::borderOnlyIntersectsRectangle(Rectangle givenRectangle)
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
	if (!boundingBox.containsPoint(requestedPoint))
	{
		return false;
	}

	// Short circuit checking if we find a positive
	for (Rectangle basicRectangle : basicRectangles)
	{
		if (basicRectangle.containsPoint(requestedPoint))
		{
			return true;
		}
	}

	return false;
}

std::vector<Rectangle> IsotheticPolygon::intersection(Rectangle givenRectangle)
{
	std::vector<Rectangle> v;

	for (Rectangle basicRectangle : basicRectangles)
	{
		Rectangle r = basicRectangle.intersection(givenRectangle);
		if (r != Rectangle::atInfinity)
		{
			v.push_back(r);
			DASSERT(r.lowerLeft <= r.upperRight);
		}
	}

	// DASSERT(unique());

	return v;
}

void IsotheticPolygon::intersection(IsotheticPolygon &constraintPolygon)
{
	DPRINT1("intersection poly-poly");
	Rectangle r;
	std::vector<Rectangle> v;

	for (Rectangle basicRectangle : basicRectangles)
	{
		for (Rectangle constraintRectangle : constraintPolygon.basicRectangles)
		{
			DPRINT4("basicRectangle ", basicRectangle, " intersecting ", constraintRectangle);
			r = basicRectangle.intersection(constraintRectangle);
			DPRINT2("intersection result r = ", r);
			if (r != Rectangle::atInfinity)
			{
				v.push_back(r);
				DASSERT(r.lowerLeft <= r.upperRight);
			}
		}
	}
	basicRectangles.clear();
	basicRectangles.swap(v);

	deduplicate();

	// DASSERT(unique());
	DASSERT(basicRectangles.size() > 0);
	DPRINT1("intersection poly-poly finished");
}

void IsotheticPolygon::increaseResolution(Rectangle clippingRectangle)
{
	DPRINT1("increaseResolution rectangle");
	// Fragment each of our constiuent rectangles based on the clippingRectangle. This may result in
	// no splitting of the constiuent rectangles and that's okay.
	std::vector<Rectangle> extraRectangles;

	for (Rectangle basicRectangle : basicRectangles)
	{
		if (!basicRectangle.intersectsRectangle(clippingRectangle))
		{
			DASSERT(basicRectangle.lowerLeft <= basicRectangle.upperRight);
			extraRectangles.push_back(basicRectangle);
		}
		else
		{
			// Break the rectangle and add the fragments to extras
			for (Rectangle fragment : basicRectangle.fragmentRectangle(clippingRectangle))
			{
				DASSERT(fragment != Rectangle::atInfinity);
				DASSERT(fragment.lowerLeft <= fragment.upperRight);
				extraRectangles.push_back(fragment);
			}
			DPRINT2("extraRectangles.size() = ", extraRectangles.size());
		}
	}

	// The new bounding polygon is now entirely defined by the fragments in extraRectangles
	basicRectangles.clear();
	basicRectangles.swap(extraRectangles);
	deduplicate();
	DPRINT2("basicRectangles.size() = ", basicRectangles.size());
	DASSERT(basicRectangles.size() > 0);
	// DASSERT(disjoint());
	// DASSERT(unique());
	DPRINT1("increaseResolution rectangle finished");
}

void IsotheticPolygon::increaseResolution(IsotheticPolygon &clippingPolygon)
{
	DPRINT1("increaseResolution polygon");
	// Quick exit
	if (!boundingBox.intersectsRectangle(clippingPolygon.boundingBox))
	{
		return;
	}

	for (Rectangle basicClippingRectangle : clippingPolygon.basicRectangles)
	{
		DPRINT2("    Removing ", basicClippingRectangle);
		increaseResolution(basicClippingRectangle);
		DPRINT2("    Result ", this);
	}

	DASSERT(basicRectangles.size() > 0);
	// DASSERT(disjoint());
	// DASSERT(unique());
	DPRINT1("increaseResolution polygon finished");
}

void IsotheticPolygon::maxLimit(double limit, unsigned d)
{
	DPRINT1("maxLimit");
	// DASSERT(unique());

	unsigned startingSize = basicRectangles.size();

	// Set the maximum dimension of all rectangles to be limit
	for (unsigned i = 0; i < startingSize; ++i)
	{
		DPRINT2("before limit basicRectangle[i] = ", basicRectangles[i]);
		basicRectangles[i].upperRight[d] = std::min(basicRectangles[i].upperRight[d], limit);
		DPRINT2("after limit basicRectangle[i] = ", basicRectangles[i]);
	}

	// Remove all rectangles whose minimum in d is greater than limit and at the same time resize
	// the bounding box
	for (unsigned i = 0; i < startingSize; ++i)
	{
		if (basicRectangles[i].lowerLeft[d] > limit || basicRectangles[i].lowerLeft[d] > basicRectangles[i].upperRight[d])
		{
			basicRectangles[i] = basicRectangles[startingSize - 1];
			--i;
			--startingSize;
			basicRectangles.pop_back();
		}
	}

	boundingBox = basicRectangles[0];
	for (unsigned i = 1; i < startingSize; ++i)
	{
		boundingBox.expand(basicRectangles[i]);
		DASSERT(basicRectangles[i].lowerLeft <= basicRectangles[i].upperRight);
	}

	deduplicate();

	DASSERT(boundingBox.lowerLeft <= boundingBox.upperRight);

	DASSERT(basicRectangles.size() > 0);
	DASSERT(boundingBox.upperRight != Point::atOrigin);
	// DASSERT(disjoint());
	// DASSERT(unique());
	DPRINT1("maxLimit finished");
}

void IsotheticPolygon::minLimit(double limit, unsigned d)
{
	DPRINT1("minLimit");
	// DASSERT(unique());

	unsigned startingSize = basicRectangles.size();

	// Set the maximum dimension of all rectangles to be limit
	for (unsigned i = 0; i < startingSize; ++i)
	{
		DPRINT2("before limit basicRectangle[i] = ", basicRectangles[i]);
		basicRectangles[i].lowerLeft[d] = std::max(basicRectangles[i].lowerLeft[d], limit);
		DPRINT2("after limit basicRectangle[i] = ", basicRectangles[i]);
	}

	// Remove all rectangles whose minimum in d is greater than limit ant at the same time resize
	// the bounding box
	for (unsigned i = 0; i < startingSize; ++i)
	{
		if (basicRectangles[i].upperRight[d] < limit || basicRectangles[i].lowerLeft[d] > basicRectangles[i].upperRight[d])
		{
			basicRectangles[i] = basicRectangles[startingSize - 1];
			--i;
			--startingSize;
			basicRectangles.pop_back();
		}
	}

	boundingBox = basicRectangles[0];
	for (unsigned i = 1; i < startingSize; ++i)
	{
		boundingBox.expand(basicRectangles[i]);
		DASSERT(basicRectangles[i].lowerLeft <= basicRectangles[i].upperRight);
	}

	deduplicate();

	DASSERT(boundingBox.lowerLeft <= boundingBox.upperRight);

	DASSERT(basicRectangles.size() > 0);
	DASSERT(boundingBox.lowerLeft != Point::atOrigin);
	// DASSERT(disjoint());
	// DASSERT(unique());
	DPRINT1("minLimit finished");
}

void IsotheticPolygon::merge(const IsotheticPolygon &mergePolygon)
{
	boundingBox.expand(mergePolygon.boundingBox);
	basicRectangles.reserve(basicRectangles.size() + mergePolygon.basicRectangles.size());

	for (Rectangle mergeRectangle : mergePolygon.basicRectangles)
	{
		basicRectangles.push_back(mergeRectangle);
		DASSERT(mergeRectangle.lowerLeft <= mergeRectangle.upperRight);
	}

	deduplicate();
}

void IsotheticPolygon::remove(unsigned basicRectangleIndex)
{
	basicRectangles[basicRectangleIndex] = basicRectangles[basicRectangles.size() - 1];
	basicRectangles.pop_back();
}

void IsotheticPolygon::deduplicate()
{
	if (basicRectangles.size() == 1)
	{
		return;
	}

	std::vector<Rectangle> deduplicated;
	std::sort(basicRectangles.begin(), basicRectangles.end(), [](Rectangle a, Rectangle b){return a.lowerLeft[0] < b.lowerLeft[0];});

	deduplicated.push_back(basicRectangles[0]);

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		if (basicRectangles[i - 1] != basicRectangles[i])
		{
			deduplicated.push_back(basicRectangles[i]);
		}
	}

	basicRectangles.clear();
	basicRectangles.swap(deduplicated);
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

bool IsotheticPolygon::disjoint()
{
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < basicRectangles.size(); ++j)
		{
			if (i != j && basicRectangles[i].intersectsRectangle(basicRectangles[j]) && !basicRectangles[i].borderOnlyIntersectsRectangle(basicRectangles[j]))
			{
				DPRINT5("Rectangle ", i, " and ", j, " are not disjoint");
				DPRINT2("Rectangle i ", basicRectangles[i]);
				DPRINT2("Rectangle j ", basicRectangles[j]);
				return false;
			}
		}
	}

	return true;
}

bool IsotheticPolygon::exists()
{
	for (Rectangle basicRectangle : basicRectangles)
	{
		if (basicRectangle.lowerLeft == basicRectangle.upperRight)
		{
			return false;
		}
	}

	return basicRectangles.size() > 0;
}

bool IsotheticPolygon::unique()
{
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < basicRectangles.size(); ++j)
		{
			if (i != j && basicRectangles[i] == basicRectangles[j])
			{
				DPRINT6("Rectangle ", i, " and ", j, " are the same ", basicRectangles[i]);
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
		if (basicRectangles[i] == Rectangle::atInfinity)
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

std::ostream& operator<<(std::ostream &os, const IsotheticPolygon &polygon)
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
