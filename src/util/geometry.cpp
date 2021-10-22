// Copyright 2021 Kyle Langendoen

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

bool Point::orderedCompare(const Point &rhs, unsigned startingDimension) const
{
	unsigned currentDimension = startingDimension;
	for (unsigned d = 0; d < dimensions; ++d)
	{
		currentDimension = (startingDimension + d) % dimensions;
		if (values[currentDimension] < rhs.values[currentDimension])
		{
			return true;
		}
		else if (values[currentDimension] > rhs.values[currentDimension])
		{
			return false;
		}
	}

	return false;
}

double Point::distance(const Point &p) const
{
	double dist = 0.0;
	for( unsigned d = 0; d < dimensions; d++ ) {
		dist += pow(abs((*this)[d] - p[d]), 2);
	}
	return sqrt(dist);
}

Point &Point::operator-=(const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] -= rhs.values[d];
	}

	return *this;
}

Point &Point::operator+=(const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] += rhs.values[d];
	}

	return *this;
}

Point &Point::operator/=(double scalar)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] /= scalar;
	}

	return *this;
}

Point &Point::operator*=(double scalar)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] *= scalar;
	}

	return *this;
}

Point &Point::operator*=(const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] *= rhs[d];
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
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] = values[d] < p[d] ? values[d] : p[d];
	}

	return *this;
}

Point &Point::operator>>(const Point &p)
{
	// Set this point to be the Hamming maximum between itself and p
	for (unsigned d = 0; d < dimensions; ++d)
	{
		values[d] = values[d] > p[d] ? values[d] : p[d];
	}

	return *this;
}

Point operator-(const Point &lhs, const Point &rhs)
{
	Point r;

	for (unsigned d = 0; d < dimensions; ++d)
	{
		r.values[d] = lhs[d] - rhs[d];
	}

	return r;
}

Point operator+(const Point &lhs, const Point &rhs)
{
	Point r;

	for (unsigned d = 0; d < dimensions; ++d)
	{
		r.values[d] = lhs[d] + rhs[d];
	}

	return r;
}

Point operator*(const Point &lhs, const double scalar)
{
	Point r;

	for (unsigned d = 0; d < dimensions; ++d)
	{
		r.values[d] = lhs[d] * scalar;
	}

	return r;
}

Point operator/(const Point &lhs, const double scalar)
{
	Point r;

	for (unsigned d = 0; d < dimensions; ++d)
	{
		r.values[d] = lhs[d] / scalar;
	}

	return r;
}

Point operator*(const Point &lhs, const Point &rhs)
{
	Point r;

	for (unsigned d = 0; d < dimensions; ++d)
	{
		r.values[d] = lhs[d] * rhs[d];
	}

	return r;
}

bool operator<(const Point &lhs, const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		if (lhs[d] >= rhs[d]) return false;
	}

	return true;
}

bool operator>(const Point &lhs, const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		if (lhs[d] <= rhs[d]) return false;
	}

	return true;
}

bool operator<=(const Point &lhs, const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		if (lhs[d] > rhs[d]) return false;
	}
	return true;
}

bool operator>=(const Point &lhs, const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		if (lhs[d] < rhs[d]) return false;
	}

	return true;
}

bool operator==(const Point &lhs, const Point &rhs)
{
	for (unsigned d = 0; d < dimensions; ++d)
	{
		if (lhs[d] != rhs[d]) return false;
	}

	return true;
}

bool operator!=(const Point &lhs, const Point &rhs)
{
	return !(lhs==rhs);
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

double Rectangle::area() const
{
	double a = fabs(upperRight[0] - lowerLeft[0]);

	for (unsigned d = 1; d < dimensions; ++d)
	{
		a = a * fabs(upperRight[d] - lowerLeft[d]);
	}

	return a;
}

double Rectangle::margin() const
{
	double margin = 0.0;
	for(unsigned d = 0; d < dimensions; d++)
	{
		margin += fabs(upperRight[d] - lowerLeft[d]);
	}

	return margin;
}

double Rectangle::computeIntersectionArea(const Rectangle &givenRectangle) const
{
	// Quick exit
	if (!intersectsRectangle(givenRectangle))
	{
		return 0.0;
	}

	double intersectionArea = fabs(fmin(upperRight[0], givenRectangle.upperRight[0]) - fmax(lowerLeft[0], givenRectangle.lowerLeft[0]));

	for (unsigned d = 1; d < dimensions; ++d)
	{
		intersectionArea = intersectionArea * fabs(fmin(upperRight[d], givenRectangle.upperRight[d]) - fmax(lowerLeft[d], givenRectangle.lowerLeft[d]));
	}

	return intersectionArea;
}

double Rectangle::computeExpansionArea(const Point &givenPoint) const
{
	// Early exit
	if (containsPoint(givenPoint))
	{
		return -1.0;
	}

	// Expanded rectangle area computed directly
	double expandedArea = fabs(fmin(lowerLeft[0], givenPoint[0]) - fmax(upperRight[0], givenPoint[0]));
	double existingArea = fabs(lowerLeft[0] - upperRight[0]);

	for (unsigned d = 1; d < dimensions; ++d)
	{
		expandedArea *= fabs(fmin(lowerLeft[d], givenPoint[d]) - fmax(upperRight[d], givenPoint[d]));
		existingArea *= fabs(lowerLeft[d] - upperRight[d]);
	}

	// Compute the difference
	return expandedArea - existingArea;
}

double Rectangle::computeExpansionMargin(const Point &givenPoint) const
{
	// Early exit
	if (containsPoint(givenPoint))
	{
		return -1.0;
	}

	// Expanded rectangle area computed directly
	double expandedMargin = fabs(fmin(lowerLeft[0], givenPoint[0]) - fmax(upperRight[0], givenPoint[0]));
	double existingMargin = fabs(lowerLeft[0] - upperRight[0]);

	for (unsigned d = 1; d < dimensions; ++d)
	{
		expandedMargin += fabs(fmin(lowerLeft[d], givenPoint[d]) - fmax(upperRight[d], givenPoint[d]));
		existingMargin += fabs(lowerLeft[d] - upperRight[d]);
	}

	// Compute the difference
	return expandedMargin - existingMargin;
}

double Rectangle::computeExpansionArea(const Rectangle &givenRectangle) const
{
	// Early exit
	if (containsRectangle(givenRectangle))
	{
		return -1.0;
	}

	// Expanded rectangle area computed directly
	double expandedArea = fabs(fmin(givenRectangle.lowerLeft[0], lowerLeft[0]) - fmax(givenRectangle.upperRight[0], upperRight[0]));

	for (unsigned d = 1; d < dimensions; ++d)
	{
		expandedArea = expandedArea * fabs(fmin(givenRectangle.lowerLeft[d], lowerLeft[d]) - fmax(givenRectangle.upperRight[d], upperRight[d]));
	}

	// Compute the difference
	return expandedArea - area();
}

double Rectangle::marginDelta(const Point &givenPoint, const Rectangle &givenRectangle) const
{
	double currentIntersectionMargin = intersection(givenRectangle).margin();
	double expandedIntersectionMargin = copyExpand(givenPoint).intersection(givenRectangle).margin();
	return expandedIntersectionMargin - currentIntersectionMargin;
}

double Rectangle::areaDelta(const Point &givenPoint, const Rectangle &givenRectangle) const
{
	double currentIntersectionArea = computeIntersectionArea(givenRectangle);
	double expandedIntersectionArea = copyExpand(givenPoint).computeIntersectionArea(givenRectangle);
	return expandedIntersectionArea - currentIntersectionArea;
}

void Rectangle::expand(const Point &givenPoint)
{
	lowerLeft << givenPoint;
	upperRight >> givenPoint;
}

void Rectangle::expand(const Rectangle &givenRectangle)
{
	lowerLeft << givenRectangle.lowerLeft;
	upperRight >> givenRectangle.upperRight;
}

bool Rectangle::alignedForMerging(const Rectangle &givenRectangle) const
{
	unsigned alignedDimensions = 0;
	unsigned intersectingDimensions = 0;
    unsigned specialIntDims = 0;

	for( unsigned d = 0; d < dimensions; d++ ) {

		if( lowerLeft[d] == givenRectangle.lowerLeft[d] and 
                upperRight[d] == givenRectangle.upperRight[d] ) {
			alignedDimensions++;
		} else if( (lowerLeft[d] <= givenRectangle.lowerLeft[d] and
                    givenRectangle.lowerLeft[d] <= upperRight[d]) or 
				(givenRectangle.lowerLeft[d] <= lowerLeft[d] and 
                 lowerLeft[d] <= givenRectangle.upperRight[d]) ) {
			specialIntDims++;
			if( specialIntDims > 1 ) {
				return false;
			}
		} else if( (lowerLeft[d] < givenRectangle.lowerLeft[d] and
                    givenRectangle.lowerLeft[d] < upperRight[d]) or 
				(givenRectangle.lowerLeft[d] < lowerLeft[d] and 
                 lowerLeft[d] < givenRectangle.upperRight[d]) ) {
			intersectingDimensions++;
			if( intersectingDimensions > 1 ) {
				return false;
			}
		}
	}
    if( alignedDimensions == dimensions - 1 and specialIntDims == 1 and
            intersectingDimensions != 1 ) {
        std::cout << "Found case we would not have merged before did: " <<
            *this << " and other: " << givenRectangle << std::endl;
    }

	return alignedDimensions == dimensions - 1 and specialIntDims == 1;
}

bool Rectangle::alignedOpposingBorders(const Rectangle &givenRectangle) const
{
	bool result = false;

	for (unsigned d = 0; d < dimensions && !result; ++d)
	{
		result = result || lowerLeft[d] == givenRectangle.upperRight[d] || upperRight[d] == givenRectangle.lowerLeft[d];
	}

	return result;
}

bool Rectangle::intersectsRectangle(const Rectangle &givenRectangle) const
{
	// Compute the range intersections
	bool interval = true;

	for (unsigned d = 0; d < dimensions && interval; ++d)
	{
		interval =
			interval &&
			((lowerLeft[d] <= givenRectangle.lowerLeft[d] && givenRectangle.lowerLeft[d] <= upperRight[d]) ||
			(givenRectangle.lowerLeft[d] <= lowerLeft[d] && lowerLeft[d] <= givenRectangle.upperRight[d]));
	}

	return interval;
}

bool Rectangle::strictIntersectsRectangle(const Rectangle &givenRectangle) const
{
	// Compute the range intersections
	bool interval = true;

	for (unsigned d = 0; d < dimensions && interval; ++d)
	{
		interval =
			interval &&
			((lowerLeft[d] < givenRectangle.lowerLeft[d] && givenRectangle.lowerLeft[d] < upperRight[d]) ||
			(givenRectangle.lowerLeft[d] < lowerLeft[d] && lowerLeft[d] < givenRectangle.upperRight[d]));
	}

	return interval;
}

bool Rectangle::borderOnlyIntersectsRectangle(const Rectangle &givenRectangle) const
{
	return intersectsRectangle(givenRectangle) && alignedOpposingBorders(givenRectangle);
}

bool Rectangle::containsPoint(const Point &givenPoint) const
{
	return lowerLeft <= givenPoint && givenPoint <= upperRight;
}

bool Rectangle::strictContainsPoint(const Point &givenPoint) const
{
	return lowerLeft < givenPoint && givenPoint < upperRight;
}

bool Rectangle::containsRectangle(const Rectangle &givenRectangle) const
{
	return containsPoint(givenRectangle.lowerLeft) && containsPoint(givenRectangle.upperRight);
}

Point Rectangle::centrePoint() const
{
	return (lowerLeft + upperRight) / 2.0;
}

Rectangle Rectangle::copyExpand(const Point &givenPoint) const
{
	Rectangle r(*this);
	r.expand(givenPoint);
	return r;
}

Rectangle Rectangle::intersection(const Rectangle &clippingRectangle) const
{
	// Quick exit
	if (!intersectsRectangle(clippingRectangle))
	{
		return Rectangle::atInfinity;
	}

	// Return rectangle
	Rectangle r = Rectangle(lowerLeft, upperRight);

	// Revise inward whenever the clippingRectangle is inside us
	r.lowerLeft >> clippingRectangle.lowerLeft;
	r.upperRight << clippingRectangle.upperRight;

	return r;
}

std::vector<Rectangle> Rectangle::fragmentRectangle(const Rectangle &clippingRectangle) const
{
	// Return vector
	std::vector<Rectangle> v;
	v.reserve(dimensions);

	// Quick exit
	if (!intersectsRectangle(clippingRectangle) || alignedOpposingBorders(clippingRectangle))
	{
		v.push_back(Rectangle(lowerLeft, upperRight));
		return v;
	}

	// Cycle through all dimensions
	Point maximums = Point::atInfinity;
	Point minimums = Point::atNegInfinity;

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

		// Make the original into a 'ceiling' in this d dimension
		if (clippingRectangle.upperRight[d] <= upperRight[d])
		{
			// New ceiling
			ceiling.lowerLeft[d] = clippingRectangle.upperRight[d];
			ceiling.upperRight << maximums;
			ceiling.lowerLeft >> minimums;
			maximums[d] = ceiling.lowerLeft[d];
			v.push_back(ceiling);
			++finalSize;
		}

		// Make the original into a 'floor' in this d dimension
		if (clippingRectangle.lowerLeft[d] >= lowerLeft[d])
		{
			// New floor
			floor.upperRight[d] = clippingRectangle.lowerLeft[d];
			floor.upperRight << maximums;
			floor.lowerLeft >> minimums;
			minimums[d] = floor.upperRight[d];
			v.push_back(floor);
			++finalSize;
		}

	}

	v.resize(finalSize);

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

IsotheticPolygon::IsotheticPolygon(const Rectangle &baseRectangle)
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

void IsotheticPolygon::reset() {
    boundingBox = Rectangle();
    basicRectangles.clear();
}

double IsotheticPolygon::area() const
{
	double area = 0.0;

	for (const Rectangle &basicRectangle : basicRectangles)
	{
		area += basicRectangle.area();
	}

	return area;
}

double IsotheticPolygon::computeIntersectionArea(const Rectangle &givenRectangle) const
{
	double runningTotal = 0.0;

	for (const Rectangle &basicRectangle : basicRectangles)
	{
		runningTotal += basicRectangle.computeIntersectionArea(givenRectangle);
	}

	return runningTotal;
}

IsotheticPolygon::OptimalExpansion IsotheticPolygon::computeExpansionArea(const Point &givenPoint) const
{
	// Early exit
	if (containsPoint(givenPoint))
	{
		return {0, -1.0};
	}

	// Take the minimum expansion area, defaulting to the first rectangle in the worst case
	OptimalExpansion expansion = {0, std::numeric_limits<double>::infinity()};
	double evalArea;

	unsigned basicsSize = basicRectangles.size();
	for (unsigned i = 0; i < basicsSize; ++i)
	{
		evalArea = basicRectangles.at(i).computeExpansionArea(givenPoint);

		if (evalArea < expansion.area && basicRectangles.at(i).area() != 0.0)
		{
			expansion.index = i;
			expansion.area = evalArea;
		}
	}

	return expansion;
}

IsotheticPolygon::OptimalExpansion IsotheticPolygon::computeExpansionArea(const Rectangle &givenRectangle) const
{
	// Take the minimum expansion area
	OptimalExpansion expansion = {0, basicRectangles.at(0).computeExpansionArea(givenRectangle)};
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

	return expansion;
}

void IsotheticPolygon::expand(const Point &givenPoint)
{
	unsigned minIndex = 0;
	double minArea = basicRectangles.at(0).computeExpansionArea(givenPoint);
	double evalArea;

	for (unsigned i = 1; i < basicRectangles.size(); ++i)
	{
		if (basicRectangles.at(i).containsPoint(givenPoint))
		{
			minIndex = i;
			break;
		}
		else if ((evalArea = basicRectangles.at(i).computeExpansionArea(givenPoint)) < minArea)
		{
			minArea = evalArea;
			minIndex = i;
		}
	}

	basicRectangles.at(minIndex).expand(givenPoint);
	boundingBox.expand(givenPoint);
}

void IsotheticPolygon::expand(const Point &givenPoint, const IsotheticPolygon::OptimalExpansion &expansion)
{
	basicRectangles.at(expansion.index).expand(givenPoint);
	boundingBox.expand(givenPoint);
}

bool IsotheticPolygon::intersectsRectangle(const Rectangle &givenRectangle) const
{
	if (!boundingBox.intersectsRectangle(givenRectangle))
	{
		return false;
	}

	// Short circuit checking if we find a positive
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (givenRectangle.intersectsRectangle(basicRectangle))
		{
			return true;
		}
	}

	return false;
}

bool IsotheticPolygon::intersectsPolygon(const IsotheticPolygon &givenPolygon) const
{
	if (!boundingBox.intersectsRectangle(givenPolygon.boundingBox))
	{
		return false;
	}

	// Short circuit checking if we find a positive
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (givenPolygon.intersectsRectangle(basicRectangle))
		{
			return true;
		}
	}

	return false;
}

bool IsotheticPolygon::borderOnlyIntersectsRectangle(const Rectangle &givenRectangle) const
{
	// Final decision
	bool result = false;

	for (const Rectangle &basicRectangle: basicRectangles)
	{
		if (!basicRectangle.intersectsRectangle(givenRectangle))
		{
			continue;
		}
		else if (!basicRectangle.alignedOpposingBorders(givenRectangle))
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

bool IsotheticPolygon::containsPoint(const Point &givenPoint) const
{
	if (!boundingBox.containsPoint(givenPoint))
	{
		return false;
	}

	// Short circuit checking if we find a positive
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (basicRectangle.containsPoint(givenPoint))
		{
			return true;
		}
	}

	return false;
}

bool IsotheticPolygon::disjoint(const IsotheticPolygon &givenPolygon) const
{
	bool rectanglesIntersect, opposingAlignment;
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		for (const Rectangle &givenBasicRectangle : givenPolygon.basicRectangles)
		{
			rectanglesIntersect = basicRectangle.intersectsRectangle(givenBasicRectangle);
			opposingAlignment = basicRectangle.alignedOpposingBorders(givenBasicRectangle);

			if (rectanglesIntersect && !opposingAlignment)
			{
				return false;
			}
		}
	}

	return true;
}

std::vector<Rectangle> IsotheticPolygon::intersection(const Rectangle &givenRectangle) const
{
	std::vector<Rectangle> v;

	for (const Rectangle &basicRectangle : basicRectangles)
	{
		Rectangle r = basicRectangle.intersection(givenRectangle);
		if (r != Rectangle::atInfinity)
		{
			v.push_back(r);
		}
	}

	return v;
}

void IsotheticPolygon::intersection(const IsotheticPolygon &constraintPolygon)
{
	Rectangle r;
	std::vector<Rectangle> v;

	for (const Rectangle &basicRectangle : basicRectangles)
	{
		for (const Rectangle &constraintRectangle : constraintPolygon.basicRectangles)
		{
			r = basicRectangle.intersection(constraintRectangle);
			if (r != Rectangle::atInfinity)
			{
				v.push_back(r);
			}
		}
	}
	basicRectangles.clear();
	basicRectangles.swap(v);
}

void IsotheticPolygon::increaseResolution(const Point &givenPoint, const Rectangle &clippingRectangle)
{
	// Fragment each of our constiuent rectangles based on the clippingRectangle. This may result in
	// no splitting of the constiuent rectangles and that's okay.
	std::vector<Rectangle> extraRectangles;

	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (!basicRectangle.intersectsRectangle(clippingRectangle))
		{
			extraRectangles.push_back(basicRectangle);
		}
		else
		{
			// Break the rectangle and add the fragments to extras
			for (const Rectangle &fragment : basicRectangle.fragmentRectangle(clippingRectangle))
			{
				// If fragmentation results in a line anywhere then reject it unless it was part of
				// the original or contains the point we are interested in
				if (fragment.area() == 0.0  && !fragment.containsPoint(givenPoint))
				{
					Rectangle originalLine = fragment.intersection(basicRectangle);
					if (originalLine != Rectangle::atInfinity)
					{
						extraRectangles.push_back(originalLine);
					}
				}
				else
				{
					extraRectangles.push_back(fragment);
				}
			}
		}
	}

	// The new bounding polygon is now entirely defined by the fragments in extraRectangles
	basicRectangles.clear();
	basicRectangles.swap(extraRectangles);
}

void IsotheticPolygon::increaseResolution(const Point &givenPoint, const IsotheticPolygon &clippingPolygon)
{
	// Quick exit
	if (!boundingBox.intersectsRectangle(clippingPolygon.boundingBox))
	{
		return;
	}

	for (const Rectangle &basicClippingRectangle : clippingPolygon.basicRectangles)
	{
		increaseResolution(givenPoint, basicClippingRectangle);
	}
}

void IsotheticPolygon::maxLimit(double limit, unsigned d)
{
	unsigned startingSize = basicRectangles.size();

	// Set the maximum dimension of all rectangles to be limit
	for (unsigned i = 0; i < startingSize; ++i)
	{
		basicRectangles[i].upperRight[d] = std::min(basicRectangles[i].upperRight[d], limit);
	}

	// Remove all rectangles whose minimum in d is greater than limit
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

    recomputeBoundingBox();
}

void IsotheticPolygon::minLimit(double limit, unsigned d)
{
	unsigned startingSize = basicRectangles.size();

	// Set the maximum dimension of all rectangles to be limit
	for (unsigned i = 0; i < startingSize; ++i)
	{
		basicRectangles[i].lowerLeft[d] = std::max(basicRectangles[i].lowerLeft[d], limit);
	}

	// Remove all rectangles whose minimum in d is greater than limit
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

    recomputeBoundingBox();
}

void IsotheticPolygon::merge(const IsotheticPolygon &mergePolygon)
{
	// Merge basic rectangles
	basicRectangles.reserve(basicRectangles.size() + mergePolygon.basicRectangles.size());
	for (const Rectangle &mergeRectangle : mergePolygon.basicRectangles)
	{
		basicRectangles.push_back(mergeRectangle);
	}

    recomputeBoundingBox();
}

void IsotheticPolygon::remove(unsigned basicRectangleIndex)
{
	// Remove basic rectangle
	basicRectangles[basicRectangleIndex] = basicRectangles[basicRectangles.size() - 1];
	basicRectangles.pop_back();

    recomputeBoundingBox();
}

void IsotheticPolygon::deduplicate()
{
	if (basicRectangles.size() == 1)
	{
		return;
	}

	std::vector<Rectangle> deduplicated;
	std::sort(basicRectangles.begin(), basicRectangles.end(), [](Rectangle &a, Rectangle &b){return a.lowerLeft[0] < b.lowerLeft[0];});

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

// TODO: Optimize maybe?
void IsotheticPolygon::refine()
{
	// Early exit
	if( basicRectangles.size() <= 1 ) {
		return;
	}
	assert( basicRectangles.size() > 0 );

    //FIXME: remove
    recomputeBoundingBox();
    Rectangle boundingBefore = boundingBox;

	unsigned rIndex;
	std::vector<Rectangle> rectangleSetRefined;

    IsotheticPolygon dup = *this;

	for( unsigned d = 0; d < dimensions; d++ ) {
		// Refine along d
		std::sort(basicRectangles.begin(), basicRectangles.end(), [d](Rectangle &a, Rectangle &b){return a.lowerLeft.orderedCompare(b.lowerLeft, d);});
		rIndex = 0;
        //std::cout << "Sorted along dimension: " << d << std::endl;
		for( unsigned i = 1; i < basicRectangles.size(); i++ ) {
            //std::cout << "rIndex: " << rIndex << " i " << i << std::endl;
			// If a rectangle is contained in another rectangle get rid of. This implicts means
			// duplicates and zero-area rectangles located entirely on the border of another rectangle
			// will be removed. Other rectanlges which are all in a "row" in dimension d are merged
			// if (rectangleRefs[rIndex].get().containsRectangle(rectangleRefs[i].get()))
            //std::cout << "rIndex:" << basicRectangles[rIndex] <<
            //    std::endl;
            //std::cout << "i: " << basicRectangles[i] << std::endl;
			if (basicRectangles[rIndex].containsRectangle(basicRectangles[i]))
			{
                //std::cout << "rIndex contained i!" << std::endl;
				continue;
			}
			else if (basicRectangles[i].containsRectangle(basicRectangles[rIndex]))
			{
                //std::cout << "i contained rIndex!" << std::endl;
				rIndex = i;
			}
			else if (basicRectangles[rIndex].alignedForMerging(basicRectangles[i]))
			{
                //std::cout << "Aligned for merging!" << std::endl;
				basicRectangles[rIndex].expand(basicRectangles[i]);
                //std::cout << "Merged." << std::endl;
			}
			else
			{
                //std::cout << "Nothing, keep going." << std::endl;
				rectangleSetRefined.push_back(basicRectangles[rIndex]);
				rIndex = i;
			}
		}
		rectangleSetRefined.push_back(basicRectangles[rIndex]);
		basicRectangles.swap(rectangleSetRefined);
		rectangleSetRefined.clear();
	}
    recomputeBoundingBox();
    assert( boundingBefore == boundingBox );


    if( dup != *this ) {
        std::cout << "Rectangles before refinement: " << dup
            << std::endl;
        std::cout << "Rectangles after refinement: " << *this
            << std::endl;
    }

	assert(basicRectangles.size() > 0);
}

void IsotheticPolygon::recomputeBoundingBox()
{
    assert( basicRectangles.size() >= 0 );
	if (basicRectangles.size() == 0)
	{
		boundingBox = Rectangle::atInfinity;
	}
	else
	{
		boundingBox = basicRectangles.at(0);
		for (Rectangle &basicRectangle : basicRectangles)
		{
			boundingBox.expand(basicRectangle);
		}
	}
}

bool IsotheticPolygon::exists() const
{
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (basicRectangle.lowerLeft == basicRectangle.upperRight)
		{
			return false;
		}
	}

	return basicRectangles.size() > 0;
}

bool IsotheticPolygon::valid() const
{
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (!(basicRectangle.lowerLeft <= basicRectangle.upperRight))
		{
			return false;
		}
	}

	return true;
}

bool IsotheticPolygon::unique() const
{
	for (unsigned i = 0; i < basicRectangles.size(); ++i)
	{
		for (unsigned j = 0; j < basicRectangles.size(); ++j)
		{
			if (i != j && basicRectangles[i] == basicRectangles[j])
			{
				return false;
			}
		}
	}

	return true;
}

bool IsotheticPolygon::lineFree() const
{
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (basicRectangle.area() == 0.0)
		{
			return false;
		}
	}

	return true;
}

bool IsotheticPolygon::infFree() const
{
	for (const Rectangle &basicRectangle : basicRectangles)
	{
		if (basicRectangle == Rectangle::atInfinity)
		{
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

unsigned compute_sizeof_inline_unbounded_polygon( unsigned num_rects ) {
    return sizeof(InlineUnboundedIsotheticPolygon) +
        (num_rects-1)*sizeof(Rectangle);
}

bool operator==(const InlineBoundedIsotheticPolygon &lhs, const
        InlineBoundedIsotheticPolygon &rhs) {
    return lhs.rectangle_count_ == rhs.rectangle_count_ and
        lhs.basicRectangles == rhs.basicRectangles;
}
bool operator!=(const InlineBoundedIsotheticPolygon &lhs, const
        InlineBoundedIsotheticPolygon &rhs) {
    return !(lhs == rhs);
}

