#include <catch2/catch.hpp>
#include <util/geometry.h>

TEST_CASE("Geometry: testPointEquality")
{
	// Test set one, zero case
	Point p1 = Point(0.0, 0.0);
	Point p2 = Point(0.0, 0.0);
	REQUIRE(p1 == p2);

	// Test set two, general case
	Point p3 = Point(1.0, 1.0);
	Point p4 = Point(1.0, 1.0);
	REQUIRE(p3 == p4);

	// Test set three, negative numbers
	Point p5 = Point(-1.0, -1.0);
	Point p6 = Point(-1.0, -1.0);
	REQUIRE(p5 == p6);

	// Test set four, mixed numbers
	Point p7 = Point(2.0, 1.0);
	Point p8 = Point(-2.0, -1.0);
	REQUIRE(p7 != p8);

	// Test set five, logical negation
	Point p9 = Point(2.0, 1.0);
	Point p10 = Point(1.0, 2.0);
	REQUIRE(p9 != p10);

	// Test set six, small numbers and logical negation
	Point p11 = Point(0.000000000078, -0.0000000000000087);
	Point p12 = Point(23456234500000000, 96598568000000000);
	REQUIRE(p11 != p12);
}

TEST_CASE("Geometry: testPointSubtraction")
{
	// Test set one, general case
	Point p1 = Point(3.0, 3.0);
	Point p2 = Point(7.0, 9.0);
	REQUIRE((p1 - p2)[0] == -4.0);
	REQUIRE(p1 - p2 == Point(-4.0, -6.0));

	// Test set two, operator chaining
	Point p3 = Point(3.0, 3.0);
	Point p4 = Point(7.0, 9.0);
	REQUIRE(p3 - p4 - p4 == Point(-11.0, -15.0));

	// Test set three, subtraction reflexively
	Point p5 = Point(3.0, 1.0);
	REQUIRE(p5 - p5 == Point::atOrigin);

	// Test set four, subtract negative numbers
	Point p6 = Point(-89.0, -798.0);
	Point p7 = Point(-3246, -3465.0);
	REQUIRE(p6 - p7 == Point(3157.0, 2667.0));

	// Test set five, subtract mixed numbers
	Point p8 = Point(-93.0, 74.0);
	Point p9 = Point(5.0, -3.0);
	REQUIRE(p8 - p9 == Point(-98.0, 77.0));

	// Test set six, zero identity
	Point p10 = Point::atOrigin;
	REQUIRE(p10 - p10 == Point::atOrigin);

	// Test set seven, limits
	Point p11 = Point::atNegInfinity;
	Point p12 = Point::atInfinity;
	REQUIRE(p11 - p12 == Point::atNegInfinity);
}

TEST_CASE("Geometry: testPointSubtractionEquals")
{
	// Test set one, general case
	Point p1 = Point(3.0, 3.0);
	Point p2 = Point(7.0, 9.0);
	p1 -= p2;
	REQUIRE(p1 == Point(-4.0, -6.0));

	// Test set two, operator chaining
	Point p3 = Point(3.0, 3.0);
	Point p4 = Point(7.0, 9.0);
	p3 -= p4 -= p4;
	REQUIRE(p3 == Point(3.0, 3.0));

	// Test set three, subtraction reflexively
	Point p5 = Point(3.0, 1.0);
	p5 -= p5;
	REQUIRE(p5 == Point::atOrigin);

	// Test set four, subtract negative numbers
	Point p6 = Point(-89.0, -798.0);
	Point p7 = Point(-3246, -3465.0);
	p6 -= p7;
	REQUIRE(p6 == Point(3157.0, 2667.0));

	// Test set five, subtract mixed numbers
	Point p8 = Point(-93.0, 74.0);
	Point p9 = Point(5.0, -3.0);
	p8 -= p9;
	REQUIRE(p8 == Point(-98.0, 77.0));

	// Test set six, zero identity
	Point p10 = Point::atOrigin;
	p10 -= p10;
	REQUIRE(p10 == Point::atOrigin);

	// Test set seven, limits
	Point p11 = Point::atNegInfinity;
	Point p12 = Point::atInfinity;
	p11 -= p12;
	REQUIRE(p11 == Point::atNegInfinity);
}

TEST_CASE("Geometry: testPointAddition")
{
	// Test set one, general case
	Point p1 = Point(3.0, 3.0);
	Point p2 = Point(7.0, 9.0);
	REQUIRE(p1 + p2 == Point(10.0, 12.0));

	// Test set two, operator chaining
	Point p3 = Point(3.0, 3.0);
	Point p4 = Point(7.0, 9.0);
	REQUIRE(p3 + p4 + p4 == Point(17.0, 21.0));

	// Test set three, addition reflexively
	Point p5 = Point(3.0, 1.0);
	REQUIRE(p5 + p5 == Point(6.0, 2.0));

	// Test set four, adding negative numbers
	Point p6 = Point(-89.0, -798.0);
	Point p7 = Point(-3246, -3465.0);
	REQUIRE(p6 + p7 == Point(-3335.0, -4263.0));

	// Test set five, adding mixed numbers
	Point p8 = Point(-93.0, 74.0);
	Point p9 = Point(5.0, -3.0);
	REQUIRE(p8 + p9 == Point(-88.0, 71.0));

	// Test set six, zero identity
	Point p10 = Point::atOrigin;
	REQUIRE(p10 + p10 == Point::atOrigin);

	// Test set seven, limits
	Point p11 = Point::atInfinity;
	REQUIRE(p11 + p11 == Point::atInfinity);

	// Test set eight, limits
	Point p12 = Point::atNegInfinity;
	REQUIRE(p12 + p12 == Point::atNegInfinity);
}

TEST_CASE("Geometry: testPointAdditionEquals")
{
	// Test set one, general case
	Point p1 = Point(3.0, 3.0);
	Point p2 = Point(7.0, 9.0);
	p1 += p2;
	REQUIRE(p1 == Point(10.0, 12.0));

	// Test set two, operator chaining
	Point p3 = Point(3.0, 3.0);
	Point p4 = Point(7.0, 9.0);
	p3 += p4 += p4;
	REQUIRE(p3 == Point(17.0, 21.0));

	// Test set three, addition reflexively
	Point p5 = Point(3.0, 1.0);
	p5 += p5;
	REQUIRE(p5 == Point(6.0, 2.0));

	// Test set four, adding negative numbers
	Point p6 = Point(-89.0, -798.0);
	Point p7 = Point(-3246, -3465.0);
	p6 += p7;
	REQUIRE(p6 == Point(-3335.0, -4263.0));

	// Test set five, adding mixed numbers
	Point p8 = Point(-93.0, 74.0);
	Point p9 = Point(5.0, -3.0);
	p8 += p9;
	REQUIRE(p8 == Point(-88.0, 71.0));

	// Test set six, zero identity
	Point p10 = Point::atOrigin;
	p10 += p10;
	REQUIRE(p10 == Point::atOrigin);

	// Test set seven, limits
	Point p11 = Point::atInfinity;
	p11 += p11;
	REQUIRE(p11 == Point::atInfinity);

	// Test set eight, limits
	Point p12 = Point::atNegInfinity;
	p12 += p12;
	REQUIRE(p12 == Point::atNegInfinity);
}

TEST_CASE("Geometry: testPointMultiplication")
{
	// Test set one, general case
	Point p1 = Point(3.0, 3.0);
	Point p2 = Point(7.0, 9.0);
	REQUIRE(p1 * p2 == Point(21.0, 27.0));

	// Test set two, operator chaining
	Point p3 = Point(3.0, 3.0);
	Point p4 = Point(7.0, 9.0);
	REQUIRE(p3 * p4 * p4 == Point(147.0, 243.0));

	// Test set three, squaring
	Point p5 = Point(3.0, 1.0);
	REQUIRE(p5 * p5 == Point(9.0, 1.0));

	// Test set four, multiplying negative numbers
	Point p6 = Point(-89.0, -798.0);
	Point p7 = Point(-3246, -3465.0);
	REQUIRE(p6 * p7 == Point(288894.0, 2765070.0));

	// Test set five, multiplying mixed numbers
	Point p8 = Point(-93.0, 74.0);
	Point p9 = Point(5.0, -3.0);
	REQUIRE(p8 * p9 == Point(-465.0, -222.0));

	// Test set six, zero identity
	Point p10 = Point::atOrigin;
	REQUIRE(p10 * p10 == Point::atOrigin);

	// Test set seven, limits
	Point p11 = Point::atInfinity;
	REQUIRE(p11 * p11 == Point::atInfinity);

	// Test set eight, limits
	Point p12 = Point::atNegInfinity;
	REQUIRE(p12 * p12 == Point::atInfinity);
}

TEST_CASE("Geometry: testPointMultiplicationEquals")
{
	// Test set one, general case
	Point p1 = Point(3.0, 3.0);
	Point p2 = Point(7.0, 9.0);
	p1 *= p2;
	REQUIRE(p1 == Point(21.0, 27.0));

	// Test set two, operator chaining
	Point p3 = Point(3.0, 3.0);
	Point p4 = Point(7.0, 9.0);
	p3 *= p4 *= p4;
	REQUIRE(p3 == Point(147.0, 243.0));

	// Test set three, squaring
	Point p5 = Point(3.0, 1.0);
	p5 *= p5;
	REQUIRE(p5 == Point(9.0, 1.0));

	// Test set four, multiplying negative numbers
	Point p6 = Point(-89.0, -798.0);
	Point p7 = Point(-3246, -3465.0);
	p6 *= p7;
	REQUIRE(p6 == Point(288894.0, 2765070.0));

	// Test set five, multiplying mixed numbers
	Point p8 = Point(-93.0, 74.0);
	Point p9 = Point(5.0, -3.0);
	p8 *= p9;
	REQUIRE(p8 == Point(-465.0, -222.0));

	// Test set six, zero identity
	Point p10 = Point::atOrigin;
	p10 *= p10;
	REQUIRE(p10 == Point::atOrigin);

	// Test set seven, limits
	Point p11 = Point::atInfinity;
	p11 *= p11;
	REQUIRE(p11 == Point::atInfinity);

	// Test set eight, limits
	Point p12 = Point::atNegInfinity;
	p12 *= p12;
	REQUIRE(p12 == Point::atInfinity);
}

TEST_CASE("Geometry: testPointScalarMultiplication")
{
	// Test set one, general case
	Point p1 = Point(4.0, 5.1);
	double s1 = 9.0;
	REQUIRE(p1 * s1 == Point(36.0, 45.9));

	// Test set two, operator chaining
	Point p2 = Point(3.0, 4.1);
	double s2 = 7.5;
	REQUIRE(p2 * s2 * s2 == Point(168.75, 230.62499999999997158));

	// Test set three, squaring
	Point p3 = Point(3.0, 3.0);
	double s3 = 3.0;
	REQUIRE(p3 * s3 == Point(9.0, 9.0));

	// Test set four, multiplying negative numbers
	Point p4 = Point(-89.0, -798.0);
	double s4 = -3246.0;
	REQUIRE(p4 * s4 == Point(288894.0, 2590308.0));

	// Test set five, multiplying mixed numbers
	Point p5 = Point(-93.0, 74.0);
	double s5 = 5.0;
	REQUIRE(p5 * s5 == Point(-465.0, 370.0));

	// Test set six, zero identity
	Point p6 = Point::atOrigin;
	double s6 = 0.0;
	REQUIRE(p6 * s6 == Point::atOrigin);

	// Test set seven, limits
	Point p7 = Point::atInfinity;
	double s7 = p7[0];
	REQUIRE(p7 * s7 == Point::atInfinity);

	// Test set eight, limits
	Point p8 = Point::atNegInfinity;
	double s8 = p8[0];
	REQUIRE(p8 * s8 == Point::atInfinity);
}

TEST_CASE("Geometry: testPointScalarMultiplicationEquals")
{
	// Test set one, general case
	Point p1 = Point(4.0, 5.1);
	double s1 = 9.0;
	p1 *= s1;
	REQUIRE(p1 == Point(36.0, 45.9));

	// Test set two, operator chaining
	Point p2 = Point(3.0, 4.1);
	double s2 = 7.5;
	p2 *= s2 * s2;
	REQUIRE(p2 == Point(168.75, 230.62499999999997158));

	// Test set three, squaring
	Point p3 = Point(3.0, 3.0);
	double s3 = 3.0;
	p3 *= s3;
	REQUIRE(p3 == Point(9.0, 9.0));

	// Test set four, multiplying negative numbers
	Point p4 = Point(-89.0, -798.0);
	double s4 = -3246.0;
	p4 *= s4;
	REQUIRE(p4 == Point(288894.0, 2590308.0));

	// Test set five, multiplying mixed numbers
	Point p5 = Point(-93.0, 74.0);
	double s5 = 5.0;
	p5 *= s5;
	REQUIRE(p5 == Point(-465.0, 370.0));

	// Test set six, zero identity
	Point p6 = Point::atOrigin;
	double s6 = 0.0;
	p6 *= s6;
	REQUIRE(p6 == Point::atOrigin);

	// Test set seven, limits
	Point p7 = Point::atInfinity;
	double s7 = p7[0];
	p7 * s7;
	REQUIRE(p7 == Point::atInfinity);

	// Test set eight, limits
	Point p8 = Point::atNegInfinity;
	double s8 = p8[0];
	p8 *= s8;
	REQUIRE(p8 == Point::atInfinity);
}

TEST_CASE("Geometry: testPointScalarDivisionEquals")
{
	// Test set one, general case
	Point p1 = Point(21.0, 28.7);
	double s1 = 7.0;
	p1 /= s1;
	REQUIRE(p1 == Point(3.0, 4.1));

	// Test set two, operator chaining
	Point p2 = Point(168.75, 230.625);
	double s2 = 7.5;
	p2 /= s2;
	p2 /= s2;
	REQUIRE(p2 == Point(3.0, 4.1));

	// Test set three, divide by onself
	Point p3 = Point(3.0, 3.0);
	double s3 = 3.0;
	p3 /= s3;
	REQUIRE(p3 == Point(1.0, 1.0));

	// Test set four, dividing negative numbers
	Point p4 = Point(-288894.0, -2590308.0);
	double s4 = -3246.0;
	p4 /= s4;
	REQUIRE(p4 == Point(89.0, 798.0));

	// Test set five, dviding mixed numbers
	Point p5 = Point(-465.0, 370.0);
	double s5 = 5.0;
	p5 /= s5;
	REQUIRE(p5 == Point(-93.0, 74.0));

	// Test set six, zero identity
	Point p6 = Point::atOrigin;
	double s6 = 342967.98765;
	p6 /= s6;
	REQUIRE(p6 == Point::atOrigin);

	// Test set seven, limits
	Point p7 = Point::atInfinity;
	double s7 = 342967.98765;
	p7 /= s7;
	REQUIRE(p7 == Point::atInfinity);

	// Test set eight, limits
	Point p8 = Point::atNegInfinity;
	double s8 = 342967.98765;
	p8 /= s8;
	REQUIRE(p8 == Point::atNegInfinity);
}

TEST_CASE("Geometry: testHammingMinimum")
{
	// Test set one, general case
	Point p1 = Point(20.0, 45.0);
	Point p2 = Point(30.0, 15.0);
	REQUIRE(p1 << p2 == Point(20.0, 15.0));

	// Test set two, dominance of first point
	Point p3 = Point(1.0, 2.78);
	Point p4 = Point(2.0, 3.0);
	REQUIRE(p3 << p4 == Point(1.0, 2.78));

	// Test set three, dominance of second point
	Point p5 = Point(7.346, 65.54623);
	Point p6 = Point(3.7534, 7.95);
	REQUIRE(p5 << p6 == Point(3.7534, 7.95));

	// Test set four, equivalence
	Point p7 = Point(8.0, 10.0);
	Point p8 = Point(8.0, 10.0);
	REQUIRE(p7 << p8 == Point(8.0, 10.0));

	// Test set five, reflexively
	Point p9 = Point(435.326, 9275.94327);
	REQUIRE(p9 << p9 == Point(435.326, 9275.94327));

	// Test set six, negative numbers
	Point p10 = Point(-23.0, -27.0);
	Point p11 = Point(-98.0, -6.0);
	REQUIRE(p10 << p11 == Point(-98.0, -27.0));

	// Test set seven, mixed numbers
	Point p12 = Point(-23.0, 27.0);
	Point p13 = Point(98.0, -6.0);
	REQUIRE(p12 << p13 == Point(-23.0, -6.0));

	// Test set eight, zero case
	Point p14 = Point::atOrigin;
	REQUIRE(p14 << p14 == Point::atOrigin);

	// Test set eight, mixed zero case
	Point p15 = Point::atOrigin;
	Point p16 = Point(12.0, -3.0);
	REQUIRE(p15 << p16 == Point(0.0, -3.0));

	// Test set nine, limits
	Point p17 = Point::atInfinity;
	Point p18 = Point::atNegInfinity;
	REQUIRE(p17 << p18 == Point::atNegInfinity);

	// Test set ten, limits
	Point p19 = Point::atOrigin;
	Point p20 = Point::atNegInfinity;
	REQUIRE(p19 << p20 == Point::atNegInfinity);

	// Test set eleven, limits
	Point p21 = Point::atInfinity;
	Point p22 = Point::atOrigin;
	REQUIRE(p21 << p22 == Point::atOrigin);
}

TEST_CASE("Geometry: testHammingMaximum")
{
	// Test set one, general case
	Point p1 = Point(20.0, 45.0);
	Point p2 = Point(30.0, 15.0);
	REQUIRE(p1 >> p2 == Point(30.0, 45.0));

	// Test set two, dominance of first point
	Point p3 = Point(2.0, 3.0);
	Point p4 = Point(1.0, 2.78);
	REQUIRE(p3 >> p4 == Point(2.0, 3.0));

	// Test set three, dominance of second point
	Point p5 = Point(3.7534, 7.95);
	Point p6 = Point(7.346, 65.54623);
	REQUIRE(p5 >> p6 == Point(7.346, 65.54623));

	// Test set four, equivalence
	Point p7 = Point(8.0, 10.0);
	Point p8 = Point(8.0, 10.0);
	REQUIRE(p7 >> p8 == Point(8.0, 10.0));

	// Test set five, reflexively
	Point p9 = Point(435.326, 9275.94327);
	REQUIRE(p9 >> p9 == Point(435.326, 9275.94327));

	// Test set six, negative numbers
	Point p10 = Point(-23.0, -27.0);
	Point p11 = Point(-98.0, -6.0);
	REQUIRE(p10 >> p11 == Point(-23.0, -6.0));

	// Test set seven, mixed numbers
	Point p12 = Point(-23.0, 27.0);
	Point p13 = Point(98.0, -6.0);
	REQUIRE(p12 >> p13 == Point(98.0, 27.0));

	// Test set eight, zero case
	Point p14 = Point::atOrigin;
	REQUIRE(p14 >> p14 == Point::atOrigin);

	// Test set eight, mixed zero case
	Point p15 = Point::atOrigin;
	Point p16 = Point(12.0, -3.0);
	REQUIRE(p15 >> p16 == Point(12.0, 0.0));

	// Test set nine, limits
	Point p17 = Point::atInfinity;
	Point p18 = Point::atNegInfinity;
	REQUIRE(p17 >> p18 == Point::atInfinity);

	// Test set ten, limits
	Point p19 = Point::atOrigin;
	Point p20 = Point::atNegInfinity;
	REQUIRE(p19 >> p20 == Point::atOrigin);

	// Test set eleven, limits
	Point p21 = Point::atInfinity;
	Point p22 = Point::atOrigin;
	REQUIRE(p21 >> p22 == Point::atInfinity);
}

TEST_CASE("Geometry: testRectangleArea")
{
	// Test set one, zero area
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	REQUIRE(r1.area() == 0.0);

	// Test set two, positive area
	Rectangle r2 = Rectangle(0.0, 0.0, 1.0, 1.0);
	REQUIRE(r2.area() == 1.0);

	// Test set three, negative corner
	Rectangle r3 = Rectangle(-5.0, -3.0, 13.0, 11.0);
	REQUIRE(r3.area() == 252.0);

	// Test set four
	Rectangle r4 = Rectangle(-5.0, -3.0, 13.3, 11.2);
	REQUIRE(r4.area() == 259.86);
}

TEST_CASE("Geometry: testRectangleIntersectionArea")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(0.0, 0.0, 314.0, 434.0);
	Rectangle r2 = Rectangle(-678.0, -100.0, 50.0, 50.0);
	REQUIRE(r1.computeIntersectionArea(r2) == 2500.0);

	// Test set two, zero case
	r1 = Rectangle(0.0, 0.0, 314.0, 434.0);
	r2 = Rectangle(-678.0, -100.0, 0.0, 0.0);
	REQUIRE(r1.computeIntersectionArea(r2) == 0.0);

	// Test set three, edge on edge case
	r1 = Rectangle(0.0, 0.0, 444.0, 222.0);
	r2 = Rectangle(444.0, 111.1, 494.4, 161.1);
	REQUIRE(r1.computeIntersectionArea(r2) == 0.0);

	// Test set four, reflexive case
	r1 = Rectangle(-10.0, -10.0, 10.0, 10.0);
	r2 = Rectangle(-10.0, -10.0, 10.0, 10.0);
	REQUIRE(r1.computeIntersectionArea(r2) == 400.0);
}

TEST_CASE("Geometry: testRectangleComputeExpansionArea")
{
	// Test set one, expansion area for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	REQUIRE(r1.computeExpansionArea(p1) == 45.0);

	// Test set two, expansion area for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	REQUIRE(r2.computeExpansionArea(r3) == 18.0);

	// Test set three, expansion area for a point on a rectangle boundary
	Rectangle r4 = Rectangle(0.0, 0.0, 4.0, 4.0);
	Point p2 = Point(4.0, 3.0);
	REQUIRE(r4.computeExpansionArea(p2) == 0.0);

	// Test set four, expansion area for a rectangle partially inside another rectangle
	Rectangle r5 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r6 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	REQUIRE(r5.computeExpansionArea(r6) == 26.0);

	// Test set five, expansion area for a rectangle wholly inside another rectangle
	Rectangle r7 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r8 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	REQUIRE(r7.computeExpansionArea(r8) == 0.0);
}

TEST_CASE("Geometry: testRectangleExpansion")
{
	// Test set one, expansion for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	r1.expand(p1);
	REQUIRE(r1.area() == 49.0);
	REQUIRE(r1.lowerLeft == Point(2.0, 8.0));
	REQUIRE(r1.upperRight == Point(9.0, 15.0));

	// Test set two, expansion for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	r2.expand(r3);
	REQUIRE(r2.area() == 22.0);
	REQUIRE(r2.lowerLeft == Point(1.0, -2.5));
	REQUIRE(r2.upperRight == Point(5.0, 3.0));

	// Test set three, expansion area for a point on a rectangle boundary
	Rectangle r4 = Rectangle(0.0, 0.0, 4.0, 4.0);
	Point p2 = Point(4.0, 3.0);
	r4.expand(p2);
	REQUIRE(r4.area() == 16.0);
	REQUIRE(r4.lowerLeft == Point(0.0, 0.0));
	REQUIRE(r4.upperRight == Point(4.0, 4.0));

	// Test set four, expansion area for a rectangle partially inside another rectangle
	Rectangle r5 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r6 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	r5.expand(r6);
	REQUIRE(r5.area() == 66.0);
	REQUIRE(r5.lowerLeft == Point(-10.0, 2.0));
	REQUIRE(r5.upperRight == Point(1.0, 8.0));

	// Test set five, expansion area for a rectangle wholly inside another rectangle
	Rectangle r7 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r8 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	r7.expand(r8);
	REQUIRE(r7.area() == 56.0);
	REQUIRE(r7.lowerLeft == Point(-4.0, -3.0));
	REQUIRE(r7.upperRight == Point(3.0, 5.0));
}

TEST_CASE("Geometry: testRectangleAlignment")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(-1.578, -3.452, 7878.0, 9889.0);
	Rectangle r2 = Rectangle(7878.0, -3.452, 898989.89, 9889.0);
	REQUIRE(r1.aligned(r2));

	// Test set two, general case
	r1 = Rectangle(-1.578, -3.452, 7878.78, 9898.98);
	r2 = Rectangle(-42.34, -27.8, 1212.12, -3.452);
	REQUIRE(r1.alignedOpposingBorders(r2));

	// Test set three, unaligned case
	r1 = Rectangle(-2.2, -3.7, 4.4, 6.7);
	r2 = Rectangle(4.4, -5.0, 7.1, 6.7);
	REQUIRE(!r1.aligned(r2));

	// Test set four, unaligned case
	r1 = Rectangle(-2.2, 4.4, 3.7, 6.9);
	r2 = Rectangle(5.0, -3.2, 7.0, -4.0);
	REQUIRE(!r1.alignedOpposingBorders(r2));
}

TEST_CASE("Geometry: testRectangleIntersectionTest")
{
	// Test set one, corner on corner intersection
	Rectangle r1 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r2 = Rectangle(46.0, 34.8, 72.8, 61.6);
	REQUIRE(r1.intersectsRectangle(r2));

	// Test set two, side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	REQUIRE(r3.intersectsRectangle(r4));

	// Test set three, no intersection
	Rectangle r5 = Rectangle(-23.5, -40.0, -3.0, -2.0);
	Rectangle r6 = Rectangle(1.0, 1.0, 3.0, 3.0);
	REQUIRE(!r5.intersectsRectangle(r6));

	// Test set four, implicitly defined corners intersect
	Rectangle r7 = Rectangle(1.0, 4.0, 4.0, 7.0);
	Rectangle r8 = Rectangle(2.0, 1.0, 5.0, 5.0);
	REQUIRE(r7.intersectsRectangle(r8));
}

TEST_CASE("Geometry: testStrictRectangleIntersectionTest")
{
	// Test set one, corner on corner intersection
	Rectangle r1 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r2 = Rectangle(46.0, 34.8, 72.8, 61.6);
	REQUIRE(r1.strictIntersectsRectangle(r2));

	// Test set two, side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	REQUIRE(r3.strictIntersectsRectangle(r4));

	// Test set three, corner on corner intersection
	Rectangle r5 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r6 = Rectangle(48.8, 47.9, 72.8, 61.6);
	REQUIRE(!r5.strictIntersectsRectangle(r6));

	// Test set four, side on side intersection
	Rectangle r7 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r8 = Rectangle(-56.4, -8.0, -19.2, 5.0);
	REQUIRE(!r7.strictIntersectsRectangle(r8));

	// Test set five, no intersection
	Rectangle r9 = Rectangle(-23.5, -40.0, -3.0, -2.0);
	Rectangle r10 = Rectangle(1.0, 1.0, 3.0, 3.0);
	REQUIRE(!r9.strictIntersectsRectangle(r10));
}

TEST_CASE("Geometry: testBorderOnlyRectangleIntersectionTest")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(-78.9, -82.43, 5.5, 7.24);
	Rectangle r2 = Rectangle(-100.2, -234.3, -78.9, -20.3);
	REQUIRE(r1.borderOnlyIntersectsRectangle(r2));

	// Test set two, corner on corner intersection
	r1 = Rectangle(77.7, 77.7, 332.3, 443.2);
	r2 = Rectangle(323.3, 443.2, 1323.4, 1500.2);
	REQUIRE(r1.borderOnlyIntersectsRectangle(r2));

	// Test set three, no intersection
	r1 = Rectangle(15.5, 15.5, 49.4, 49.4);
	r2 = Rectangle(5.5, 5.5, 12.3, 12.4);
	REQUIRE(!r1.borderOnlyIntersectsRectangle(r2));
}

TEST_CASE("Geometry: testRectanglePointContainment")
{
	// Test set one, zero area rectangle contains its corner
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	Point p1 = Point(0.0, 0.0);
	REQUIRE(r1.containsPoint(p1));

	// Test set two, rectangle contains its lower left
	Rectangle r2 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p2 = Point(-1.0, -1.0);
	REQUIRE(r2.containsPoint(p2));

	// Test set three, rectangle contains its upper right
	Rectangle r3 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p3 = Point(3.0, 3.0);
	REQUIRE(r3.containsPoint(p3));

	// Test set four, rectangle does not contain point
	Rectangle r4 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p4 = Point(217.3, 527.7);
	REQUIRE(!r4.containsPoint(p4));

	// Test set five, general case
	Rectangle r5 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p5 = Point(3.0, 1.0);
	REQUIRE(r5.containsPoint(p5));
}

TEST_CASE("Geometry: testRectangleStrictPointContainment")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(0.0, 0.0, 7.0, 7.0);
	Point p1 = Point(0.000000000000001, 0.000000000000001);
	REQUIRE(r1.strictContainsPoint(p1));

	// Test set two, rectangle does not contain point
	r1 = Rectangle(-4.0, -4.0, 27.0, 27.0);
	p1 = Point(-8.0, 9.0);
	REQUIRE(!r1.strictContainsPoint(p1));
}

TEST_CASE("Geometry: testRectangleRectangleContainment")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(89.0, 89.0, 212.0, 212.0);
	Rectangle r2 = Rectangle(112.0, 122.0, 144.0, 156.0);
	REQUIRE(r1.containsRectangle(r2));

	// Test set two, non-containment case
	r1 = Rectangle(112.0, 122.0, 144.0, 156.0);
	r2 = Rectangle(7.0, 9.0, 14.0, 12.0);
	REQUIRE(!r1.containsRectangle(r2));

	// Test set three, partial-containment case
	r1 = Rectangle(-4.0, -6.0, 8.0, 12.0);
	r2 = Rectangle(0.0, 0.0, 20.0, 10.0);
	REQUIRE(!r1.containsRectangle(r2));
}

TEST_CASE("Geometry: testRectangleIntersection")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	Rectangle r2 = Rectangle(3.0, 3.0, 10.0, 7.0);
	REQUIRE(r1.intersection(r2) == Rectangle(3.0, 3.0, 4.0, 7.0));

	// Test set two, corner on corner intersection
	r1 = Rectangle(0.0, 0.0, 12.3, 13.4);
	r2 = Rectangle(12.3, 13.4, 19.7, 82.0);
	REQUIRE(r1.intersection(r2) == Rectangle::atInfinity);

	// Test set three, no intersection
	r1 = Rectangle(0.0, 0.0, 12.3, 13.4);
	r2 = Rectangle(78.9, 56.4, 99.9, 99.756);
	REQUIRE(r1.intersection(r2) == Rectangle::atInfinity);

	// Test set four, side on side intersection
	r1 = Rectangle(0.0, 0.0, 12.3, 13.4);
	r2 = Rectangle(-12.3, 0.0, 0.0, 13.4);
	REQUIRE(r1.intersection(r2) == Rectangle::atInfinity);
}

TEST_CASE("Geometry: testRectangleFragmentation")
{
	// Test set one, upper right corner cut out
	Rectangle r1 = Rectangle(1.0, 1.0, 5.0, 5.0);
	Rectangle r2 = Rectangle(4.0, 4.0, 8.0, 8.0);
	std::vector<Rectangle> v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == Rectangle(1.0, 1.0, 4.0, 5.0));
	REQUIRE(v[1] == Rectangle(4.0, 1.0, 5.0, 4.0));

	// Test set two, bottom right corner cut out
	r1 = Rectangle(0.0, 0.0, 4.0, 4.0);
	r2 = Rectangle(1.0, -3.0, 6.0, 1.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == Rectangle(0.0, 0.0, 1.0, 4.0));
	REQUIRE(v[1] == Rectangle(1.0, 1.0, 4.0, 4.0));

	// Test set three, bottom left corner cut out
	r1 = Rectangle(-7.0, -5.0, 0.0, 0.0);
	r2 = Rectangle(-8.0, -8.0, -4.0, -3.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == Rectangle(-4.0, -5.0, 0.0, 0.0));
	REQUIRE(v[1] == Rectangle(-7.0, -3.0, -4.0, 0.0));

	// Test set four, right side cut out
	r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	r2 = Rectangle(3.0, 3.0, 7.0, 7.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 3);
	REQUIRE(v[0] == Rectangle(0.0, 0.0, 3.0, 8.0));
	REQUIRE(v[1] == Rectangle(3.0, 7.0, 4.0, 8.0));
	REQUIRE(v[2] == Rectangle(3.0, 0.0, 4.0, 3.0));

	// Test set five, left side cut out
	r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	r2 = Rectangle(-3.0, 3.0, 1.0, 7.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 3);
	REQUIRE(v[0] == Rectangle(1.0, 0.0, 4.0, 8.0));
	REQUIRE(v[1] == Rectangle(0.0, 7.0, 1.0, 8.0));
	REQUIRE(v[2] == Rectangle(0.0, 0.0, 1.0, 3.0));

	// Test set six, top side cut out
	r1 = Rectangle(0.0, 0.0, 8.0, 4.0);
	r2 = Rectangle(1.0, 2.0, 5.0, 6.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 3);
	REQUIRE(v[0] == Rectangle(5.0, 0.0, 8.0, 4.0));
	REQUIRE(v[1] == Rectangle(0.0, 0.0, 1.0, 4.0));
	REQUIRE(v[2] == Rectangle(1.0, 0.0, 5.0, 2.0));

	// Test set seven, bottom side cut out
	r1 = Rectangle(0.0, 0.0, 8.0, 4.0);
	r2 = Rectangle(1.0, -3.0, 5.0, 1.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 3);
	REQUIRE(v[0] == Rectangle(5.0, 0.0, 8.0, 4.0));
	REQUIRE(v[1] == Rectangle(0.0, 0.0, 1.0, 4.0));
	REQUIRE(v[2] == Rectangle(1.0, 1.0, 5.0, 4.0));
}
