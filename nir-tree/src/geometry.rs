#![allow(non_snake_case)]
#![allow(dead_code)]
#![allow(unused_variables)]

use std::cmp::max;
use std::cmp::min;


// A 2-D point
#[derive(Debug)]
pub struct Point
{
	pub x: u64,
	pub y: u64,
}

// A 2-D rectangle
#[derive(Debug)]
pub struct Rectangle
{
	pub lowerLeft: Point,
	pub upperRight: Point,
	pub area: u64,
}

impl Rectangle
{
	// Constructor
	pub fn new(xLower: u64, yLower: u64, xUpper: u64, yUpper: u64, area: u64) -> Rectangle
	{
		let lowerLeft = Point{x: xLower, y: yLower};
		let upperRight = Point{x: xUpper, y: yUpper};

		return Rectangle{lowerLeft, upperRight, area};
	}

	// Compute the smallest Rectangle needed to fit in the new given rectangle. Also give the area
	// to be added to accomplish this
	pub fn computeExpansionArea(&self, requestedRectangle: &Rectangle) -> (Rectangle, u64)
	{
		// Compute min x and min y
		let minX = min(self.lowerLeft.x, requestedRectangle.lowerLeft.x);
		let minY = min(self.lowerLeft.y, requestedRectangle.lowerLeft.y);
		// This defines the lowerLeft of our expanded/unchanged Rectangle
		let lowerLeft = Point{x: minX, y: minY};

		// Compute max x and max y
		let maxX = max(self.upperRight.x, requestedRectangle.upperRight.x);
		let maxY = max(self.upperRight.y, requestedRectangle.upperRight.y);
		// This defines the upperRight of our expanded/unchanged Rectangle
		let upperRight = Point{x: maxX, y: maxY};

		// Compute the area of the new rectangle
		let area = (maxX - minX) * (maxY - minY);
		// Make a new rectangle
		let expandedRectangle = Rectangle{lowerLeft: lowerLeft, upperRight: upperRight, area: area};
		
		// Return the calculated metrics, i.e. the two Points and the relative area
		return (expandedRectangle, area - self.area);
	}

	// Check if the requested Rectangle is fully contained, just intersecting a little, or totally
	// disjoint from us
	pub fn intersectsRectangle(&self, requestedRectangle: &Rectangle) -> bool
	{
		let intervalX = self.lowerLeft.x <= requestedRectangle.upperRight.x;
		let intervalXPrime = requestedRectangle.lowerLeft.x <= self.upperRight.x;
		let intervalY = self.lowerLeft.y <= requestedRectangle.upperRight.y;
		let intervalYPrime = requestedRectangle.lowerLeft.y <= self.upperRight.y;

		println!("[intersectsRectangle] Data is self = {:?} and requested = {:?}", self, requestedRectangle);
		println!("[intersectsRectangle] Containment information is ({}, {}, {}, {})", intervalX, intervalXPrime, intervalY, intervalYPrime);

		if intervalX && intervalXPrime && intervalY && intervalYPrime
		{
			return true;
		}

		return false;
	}

	// Check if the requested Point is contained in us
	pub fn containsPoint(&self, requestedPoint: &Point) -> bool
	{
		let inXRange = self.lowerLeft.x <= requestedPoint.x && requestedPoint.x <= self.upperRight.x;
		let inYRange = self.lowerLeft.y <= requestedPoint.y && requestedPoint.y <= self.upperRight.y;

		return inXRange && inYRange;
	}
}


#[test]
fn RectangleExpansionAreaTest()
{
	// The existing Rectangle
	let baseRectangle = Rectangle::new(3, 3, 6, 6, 9);
	// Fully contained Rectangle
	let containedRectangle = Rectangle::new(4, 4, 5, 5, 1);
	// Intersecting corners Rectangles
	let intersectingCornerRectangle = Rectangle::new(4, 5, 7, 7, 6);
	// Intersecting side Rectangles
	let intersectingSideRectangle = Rectangle::new(4, 2, 5, 4, 2);
	// Disjoint Rectangles
	let disjointRectangle = Rectangle::new(7, 7, 8, 8, 1);

	// Test expansion when one Rectangle is inside the other
	let mut expansionMetrics = baseRectangle.computeExpansionArea(&containedRectangle);
	assert!(expansionMetrics.0.lowerLeft.x == baseRectangle.lowerLeft.x);
	assert!(expansionMetrics.0.lowerLeft.y == baseRectangle.lowerLeft.y);
	assert!(expansionMetrics.1 == 0);

	// Test expansion when the Rectangles intersect
	expansionMetrics = baseRectangle.computeExpansionArea(&intersectingCornerRectangle);
	assert!(expansionMetrics.0.upperRight.x == intersectingCornerRectangle.upperRight.x);
	assert!(expansionMetrics.0.upperRight.y == intersectingCornerRectangle.upperRight.y);
	assert!(expansionMetrics.1 == 7);
	expansionMetrics = baseRectangle.computeExpansionArea(&intersectingSideRectangle);
	assert!(expansionMetrics.0.lowerLeft.x == 3);
	assert!(expansionMetrics.0.lowerLeft.y == 2);
	assert!(expansionMetrics.1 == 3);

	// Test expansion when one Rectangle is totally disjoint from the other
	expansionMetrics = baseRectangle.computeExpansionArea(&disjointRectangle);
	assert!(expansionMetrics.0.upperRight.x == disjointRectangle.upperRight.x);
	assert!(expansionMetrics.0.upperRight.y == disjointRectangle.upperRight.y);
	assert!(expansionMetrics.1 == 16);
}

#[test]
fn RectangleIntersectionTest()
{
	// Base Rectangle
	let baseRectangle = Rectangle::new(2, 2, 4, 4, 4);
	// Fully Contained Rectangle
	let containedRectangle = Rectangle::new(2, 2, 4, 4, 4);
	// Lower Left Quadrant Contained
	let lowerLeftQuadrantRectangle = Rectangle::new(3, 3, 5, 5, 4);
	// Lower Right Quadrant Contained
	let lowerRightQuadrantRectangle = Rectangle::new(1, 3, 3, 5, 4);
	// Upper Left Quadrant Contained
	let upperLeftQuadrantRectangle = Rectangle::new(3, 1, 5, 3, 4);
	// Upper Right Quadrant Contained
	let upperRightQuadrantRectangle = Rectangle::new(1, 1, 3, 3, 4);
	// Uncontained Rectangle
	let uncontainedRectangle = Rectangle::new(6, 6, 8, 8, 4);

	// Test full containment
	assert!(baseRectangle.intersectsRectangle(&containedRectangle));
	// Test lower left point in base rectangle
	assert!(baseRectangle.intersectsRectangle(&lowerLeftQuadrantRectangle));
	// Test no point in rectangle but corners still intersect
	assert!(baseRectangle.intersectsRectangle(&lowerRightQuadrantRectangle));
	assert!(baseRectangle.intersectsRectangle(&upperLeftQuadrantRectangle));
	// Test upper right point in base rectangle
	assert!(baseRectangle.intersectsRectangle(&upperRightQuadrantRectangle));
	// Test rectangles don't intersect
	assert!(!baseRectangle.intersectsRectangle(&uncontainedRectangle));
}

#[test]
fn PointIntersectionTest()
{
	// Base Rectangle
	let baseRectangle = Rectangle::new(2, 2, 4, 4, 4);
	// Point inside the Rectangle
	let insidePoint = Point{x: 3, y: 3};
	// Point outside the Rectangle
	let outsidePoint = Point{x: 5, y: 5};
	// Point on a side of the Rectangle
	let borderPoint = Point{x: 2, y: 3};

	// Test Point in Rectangle
	assert!(baseRectangle.containsPoint(&insidePoint) == true);
	// Test Point not in Rectangle
	assert!(baseRectangle.containsPoint(&outsidePoint) == false);
	// Test Point on border of Rectangle
	assert!(baseRectangle.containsPoint(&borderPoint) == true);
}
