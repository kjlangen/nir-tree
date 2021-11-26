#include <catch2/catch.hpp>
#include <util/geometry.h>
#include <storage/tree_node_allocator.h>
#include <storage/page.h>
#include <unistd.h>
#include <cmath>
#include <cfloat>

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
	REQUIRE(r1.computeExpansionArea(p1) == Approx(45.0));

	// Test set two, expansion area for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	REQUIRE(r2.computeExpansionArea(r3) == Approx(18.0));

	// Test set three, expansion area for a point on a rectangle boundary
	Rectangle r4 = Rectangle(0.0, 0.0, 4.0, 4.0);
	Point p2 = Point(4.0, 4.0);
    // Requires expansion to 4.0+eps on x and y-axes
    double error = nextafter(4.0, DBL_MAX);
    error = error*error - 16.0;
	REQUIRE(r4.computeExpansionArea(p2) == error); 

	Rectangle r4_2 = Rectangle(0.0, 0.0, 4.0,
            nextafter(4.0, DBL_MAX));
    // Requires expansion to 4.0+eps from 4.0 on x axis
    // y-axis diff is 4.0
	REQUIRE(r4_2.computeExpansionArea(p2) ==
            (nextafter(4.0,DBL_MAX)-4.0)*4.0 ); 

	// Test set four, expansion area for a rectangle partially inside another rectangle
	Rectangle r5 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r6 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	REQUIRE(r5.computeExpansionArea(r6) == Approx(26.0));

	// Test set five, expansion area for a rectangle wholly inside another rectangle
	Rectangle r7 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r8 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	REQUIRE(r7.computeExpansionArea(r8) == Approx(-1.0));
}

TEST_CASE("Geometry: testRectangleExpansion")
{
	// Test set one, expansion for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	r1.expand(p1);
    double area = (nextafter( 9.0, DBL_MAX ) - 2.0) * (nextafter( 15.0,
                DBL_MAX ) - 8.0);
    REQUIRE( r1.area() == area );
	REQUIRE(r1.area() == Approx( 49.0 ).epsilon(0.1) );
	REQUIRE(r1.lowerLeft == Point(2.0, 8.0));
	REQUIRE(r1.upperRight == Point(nextafter( 9.0, DBL_MAX ),
                nextafter(15.0, DBL_MAX)));

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
	REQUIRE(r4.area() == Approx(16.0));
	REQUIRE(r4.lowerLeft == Point(0.0, 0.0));
	REQUIRE(r4.upperRight == Point(nextafter(4.0, DBL_MAX), 4.0));

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
	Rectangle r2 = Rectangle(7877.0, -3.452, 898989.89, 9889.0);
	REQUIRE(r1.alignedForMerging(r2));

	// Test set two, general case
	r1 = Rectangle(-1.578, -3.452, 7878.78, 9898.98);
	r2 = Rectangle(-42.34, -27.8, 1212.12, -3.452);
	REQUIRE(r1.alignedOpposingBorders(r2));

	// Test set three, unaligned case
	r1 = Rectangle(-2.2, -3.7, 4.4, 6.7);
	r2 = Rectangle(4.4, -5.0, 7.1, 6.7);
	REQUIRE(!r1.alignedForMerging(r2));

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
	REQUIRE(r1.intersectsRectangle(r2));

	// Test set two, side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	REQUIRE(r3.intersectsRectangle(r4));

	// Test set three, corner on corner intersection
	Rectangle r5 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r6 = Rectangle(48.8, 47.9, 72.8, 61.6);
	REQUIRE(!r5.intersectsRectangle(r6));

	// Test set four, side on side intersection
	Rectangle r7 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r8 = Rectangle(-56.4, -8.0, -19.2, 5.0);
	REQUIRE(!r7.intersectsRectangle(r8));

	// Test set five, no intersection
	Rectangle r9 = Rectangle(-23.5, -40.0, -3.0, -2.0);
	Rectangle r10 = Rectangle(1.0, 1.0, 3.0, 3.0);
	REQUIRE(!r9.intersectsRectangle(r10));
}


TEST_CASE("Geometry: testRectanglePointContainment1")
{
	// Test set one, zero area rectangle contains its corner
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	Point p1 = Point(0.0, 0.0);
	REQUIRE(not r1.containsPoint(p1));

	// Test set two, rectangle contains its lower left
	Rectangle r2 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p2 = Point(-1.0, -1.0);
	REQUIRE(r2.containsPoint(p2));

	// Test set three, rectangle contains its upper right
	Rectangle r3 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p3 = Point(3.0, 3.0);
	REQUIRE(not r3.containsPoint(p3));

	// Test set four, rectangle does not contain point
	Rectangle r4 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p4 = Point(217.3, 527.7);
	REQUIRE(not r4.containsPoint(p4));

	// Test set five, general case
	Rectangle r5 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p5 = Point(3.0, 1.0);
	REQUIRE(not r5.containsPoint(p5));

	Rectangle r5_2 = Rectangle(-1.0, -1.0, nextafter(3.0,DBL_MAX), 3.0);
	REQUIRE(r5_2.containsPoint(p5));
}

TEST_CASE("Geometry: testRectanglePointContainment2")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(0.0, 0.0, 7.0, 7.0);
	Point p1 = Point(0.000000000000001, 0.000000000000001);
	REQUIRE(r1.containsPoint(p1));

	// Test set two, rectangle does not contain point
	r1 = Rectangle(-4.0, -4.0, 27.0, 27.0);
	p1 = Point(-8.0, 9.0);
	REQUIRE(!r1.containsPoint(p1));
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
	r1 = Rectangle(0.0, 0.0, nextafter(12.3,DBL_MAX), nextafter(13.4,
                DBL_MAX));
	r2 = Rectangle(12.3, 13.4, 19.7, 82.0);
	REQUIRE(r1.intersection(r2) == Rectangle(12.3, 13.4,
                nextafter(12.3,DBL_MAX), nextafter(13.4, DBL_MAX)));

	// Test set three, no intersection
	r1 = Rectangle(0.0, 0.0, 12.3, 13.4);
	r2 = Rectangle(78.9, 56.4, 99.9, 99.756);
	REQUIRE(r1.intersection(r2) == Rectangle::atInfinity);

	// Test set four, side on side intersection
	r1 = Rectangle(0.0, 0.0, nextafter(12.3, DBL_MAX), nextafter(13.4,
                DBL_MAX));
	r2 = Rectangle(-12.3, 0.0, nextafter(0.0, DBL_MAX), nextafter(13.4,
                DBL_MAX ) );
	REQUIRE(r1.intersection(r2) == Rectangle(0.0, 0.0, nextafter(0.0,
                    DBL_MAX), nextafter(13.4,DBL_MAX)));

    // These do not intersect, they are beside each other.
	r1 = Rectangle(0.0, 0.0, 12.3, 13.4);
	r2 = Rectangle(-12.3, 0.0, 0.0, 13.4);
	REQUIRE(r1.intersection(r2) == Rectangle::atInfinity );

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

TEST_CASE("Geometry: testPolygonArea")
{
	// Test case one, general case
	IsotheticPolygon p1(Rectangle(0.0, 0.0, 1.0, 1.0));
	p1.basicRectangles.push_back(Rectangle(1.0, 0.0, 2.0, 1.0));
	p1.basicRectangles.push_back(Rectangle(2.0, 0.0, 3.0, 1.0));
	p1.basicRectangles.push_back(Rectangle(3.0, 0.0, 4.0, 1.0));
	p1.basicRectangles.push_back(Rectangle(4.0, 0.0, 5.0, 1.0));

	REQUIRE(p1.area() == 5.0);

	// Test case two, polygon composed of a single rectangle
	IsotheticPolygon p2(Rectangle(7.3, -1.0, 9.3, 3.0));
	REQUIRE(trunc(p2.area()) == 8.0);

	// Test case three, polygon with zero area
	IsotheticPolygon p3;

	REQUIRE(p3.area() == 0.0);
}

TEST_CASE("Geometry: testPolygonExpansionArea")
{
	Rectangle r1(0.0, 1.0, 1.0, 5.0);
	Rectangle r2(2.0, 1.0, 3.0, 5.0);
	Rectangle r3(3.0, 0.0, 4.0, 3.0);
	Rectangle r4(3.0, 2.0, 5.0, 3.0);
	Rectangle r5(5.0, 2.0, 6.0, 5.0);

	IsotheticPolygon ip1(r1);
	ip1.basicRectangles.push_back(r2);
	ip1.basicRectangles.push_back(r3);
	ip1.basicRectangles.push_back(r4);
	ip1.basicRectangles.push_back(r5);
	ip1.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

    /*
	// Test set one, general case
	Point p1(5.5, 5.5);
	IsotheticPolygon::OptimalExpansion expansion = ip1.computeExpansionArea(p1);

	REQUIRE(expansion.index == 4);
	REQUIRE(expansion.area == (nextafter(5.5, DBL_MAX)-5.5) + 0.5 );

	// Test set two, point contained
	Point p2(3.5, 2.3);
	expansion = ip1.computeExpansionArea(p2);

	REQUIRE(expansion.index == 0);
	REQUIRE(expansion.area == -1.0);
    */

	// Test set three, point causes same expansion area in two rectangles
    // But moving top point incurs epsilon past overhead, so second
    // point is better.
	Point p3(1.5, 0.5);
	auto expansion = ip1.computeExpansionArea(p3);

	REQUIRE(expansion.index == 1);
	REQUIRE(expansion.area == 2.75);
}

TEST_CASE("Geometry: testPolygonExpand")
{
	Rectangle r1(0.0, 1.0, 1.0, 5.0);
	Rectangle r2(2.0, 1.0, 3.0, 5.0);
	Rectangle r3(3.0, 0.0, 4.0, 3.0);
	Rectangle r4(3.0, 2.0, 5.0, 3.0);
	Rectangle r5(5.0, 2.0, 6.0, 5.0);

	IsotheticPolygon reference(r1);
	reference.basicRectangles.push_back(r2);
	reference.basicRectangles.push_back(r3);
	reference.basicRectangles.push_back(r4);
	reference.basicRectangles.push_back(r5);
	reference.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	// Test set one, general case with pre-computed area
	Point p1(5.5, 5.5);
	IsotheticPolygon ip1(reference);
	IsotheticPolygon ip2(reference);
	ip1.expand(p1, ip1.computeExpansionArea(p1)); // with pre-computed area
	ip2.expand(p1); // without pre-computed area

	REQUIRE(ip1.basicRectangles[4] != r5);
	REQUIRE(ip2.basicRectangles[4] != r5);
	REQUIRE(ip1.basicRectangles[4] == Rectangle(5.0, 2.0, 6.0,
                nextafter(5.5, DBL_MAX)));
	REQUIRE(ip2.basicRectangles[4] == Rectangle(5.0, 2.0, 6.0,
                nextafter(5.5, DBL_MAX)));
	REQUIRE(ip1.basicRectangles[4] == ip2.basicRectangles[4]);
	REQUIRE(ip1.basicRectangles[4].area() == Approx(r5.area() + 0.5));
	REQUIRE(ip2.basicRectangles[4].area() == Approx(r5.area() + 0.5));
	REQUIRE(ip1.basicRectangles[4].area() == ip2.basicRectangles[4].area());

	// Test set two, point contained
	Point p2(3.5, 2.3);
	IsotheticPolygon ip3(reference);
	IsotheticPolygon ip4(reference);
	ip3.expand(p2, ip3.computeExpansionArea(p2)); // with pre-computed area
	ip4.expand(p2); // without pre-computed area

	REQUIRE(ip3.basicRectangles[2] == r3);
	REQUIRE(ip3.basicRectangles[3] == r4);
	REQUIRE(ip4.basicRectangles[2] == r3);
	REQUIRE(ip4.basicRectangles[3] == r4);
	REQUIRE(ip3.basicRectangles[2] == ip4.basicRectangles[2]);
	REQUIRE(ip3.basicRectangles[3] == ip4.basicRectangles[3]);

	// Test set three, point causes same expansion area in two rectangles
	Point p3(1.5, 0.5);
	IsotheticPolygon ip5(reference);
	IsotheticPolygon ip6(reference);
	ip5.expand(p3, ip5.computeExpansionArea(p3));
	ip6.expand(p3);

	REQUIRE(ip5.basicRectangles[1] != r2);
	REQUIRE(ip6.basicRectangles[1] != r2);
	REQUIRE(ip5.basicRectangles[1] == Rectangle(1.5, 0.5, 3.0, 5.0));
	REQUIRE(ip6.basicRectangles[1] == Rectangle(1.5, 0.5, 3.0, 5.0));
	REQUIRE(ip5.basicRectangles[1] == ip6.basicRectangles[1]);
	REQUIRE(ip5.basicRectangles[1].area() == Approx(r1.area() +
                2.75).epsilon(0.1));
	REQUIRE(ip6.basicRectangles[1].area() == Approx(r1.area() +
                2.75).epsilon(0.1));
	REQUIRE(ip5.basicRectangles[1].area() == ip6.basicRectangles[1].area());
}

TEST_CASE("Geometry: testPolygonContainsPoint")
{
	Rectangle r1(0.0, 1.0, 1.0, 5.0);
	Rectangle r2(2.0, 0.0, 3.0, 5.0);
	Rectangle r3(3.0, 0.0, 4.0, 3.0);
	Rectangle r4(3.0, 2.0, 5.0, 3.0);
	Rectangle r5(5.0, 2.0, 6.0, 5.0);

	IsotheticPolygon ip1(r1);
	ip1.basicRectangles.push_back(r2);
	ip1.basicRectangles.push_back(r3);
	ip1.basicRectangles.push_back(r4);
	ip1.basicRectangles.push_back(r5);
	ip1.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	// Test set one, general case
	Point p1(2.5, 4.75);

	REQUIRE(ip1.containsPoint(p1));

	// Test set two, point not contained
	Point p2(-1.0, 1.0);

	REQUIRE(!ip1.containsPoint(p2));

	// Test set three, point on border
	Point p3(4.0, 1.83);
	REQUIRE(not ip1.containsPoint(p3));
    Point p3_2(nextafter(4.0,-DBL_MAX),1.83);
	REQUIRE(ip1.containsPoint(p3_2));

	// Test set four, point contained in three
	Point p4(3.0, 2.0);

	REQUIRE(ip1.containsPoint(p4));

	// Test set five, point on corner
	Point p5(6.0, 5.0);

	REQUIRE(not ip1.containsPoint(p5));
    
	REQUIRE(ip1.containsPoint(Point::closest_smaller_point( p5 )));
}

TEST_CASE("Geometry: testPolygonDisjoint")
{
	Rectangle r1(0.0, 1.0, 1.0, 5.0);
	Rectangle r2(2.0, 1.0, 3.0, 5.0);
	Rectangle r3(3.0, 0.0, 4.0, 3.0);
	Rectangle r4(3.0, 2.0, 5.0, 3.0);
	Rectangle r5(5.0, 2.0, 6.0, 5.0);

	IsotheticPolygon reference(r1);
	reference.basicRectangles.push_back(r2);
	reference.basicRectangles.push_back(r3);
	reference.basicRectangles.push_back(r4);
	reference.basicRectangles.push_back(r5);
	reference.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	// Test set one, polygons disjoint
	IsotheticPolygon ip1(Rectangle(7.0, 1.0, 9.0, 2.0));
	ip1.basicRectangles.push_back(Rectangle(8.0, 2.0, 9.0, 3.5));
	ip1.basicRectangles.push_back(Rectangle(9.0, 1.0, 10.0, 4.0));
	ip1.basicRectangles.push_back(Rectangle(9.0, 4.0, 11.5, 4.5));

	REQUIRE(ip1.disjoint(reference));

	// Test set two, polygons not disjoint
	IsotheticPolygon ip2(Rectangle(0.0, 1.0, 2.0, 2.0));
	ip2.basicRectangles.push_back(Rectangle(1.0, 2.0, 2.0, 3.5));
	ip2.basicRectangles.push_back(Rectangle(2.0, 1.0, 3.0, 4.0));
	ip2.basicRectangles.push_back(Rectangle(2.0, 4.0, 4.5, 4.5));

	REQUIRE(!ip2.disjoint(reference));

	// Test set three, polygons lay edge-on-edge
	IsotheticPolygon ip3(Rectangle(2.0, 5.0, 4.0, 6.0));
	ip3.basicRectangles.push_back(Rectangle(3.0, 6.0, 4.0, 7.5));
	ip3.basicRectangles.push_back(Rectangle(4.0, 5.0, 5.0, 8.0));
	ip3.basicRectangles.push_back(Rectangle(4.0, 8.0, 6.5, 8.5));

	REQUIRE(ip3.disjoint(reference));

	// Test set four, polygons lay corner-on-corner
	IsotheticPolygon ip4(Rectangle(Rectangle(-4.5, 2.0, -2.5, 3.0)));
	ip4.basicRectangles.push_back(Rectangle(Rectangle(-3.5, 3.0, -2.5, 4.5)));
	ip4.basicRectangles.push_back(Rectangle(Rectangle(-2.5, 2.0, -1.5, 5.0)));
	ip4.basicRectangles.push_back(Rectangle(Rectangle(-2.5, 5.0, 0.0, 5.5)));

	REQUIRE(ip4.disjoint(reference));
}

TEST_CASE("Geometry: testPolygonIntersection")
{
	// Test set one, general case rectangle
	IsotheticPolygon ip1(Rectangle(0.0, 1.0, 1.0, 5.0));
	ip1.basicRectangles.push_back(Rectangle(2.0, 1.0, 3.0, 5.0));
	ip1.basicRectangles.push_back(Rectangle(3.0, 0.0, 4.0, 3.0));
	ip1.basicRectangles.push_back(Rectangle(3.0, 2.0, 5.0, 3.0));
	ip1.basicRectangles.push_back(Rectangle(5.0, 2.0, 6.0, 5.0));
	ip1.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	Rectangle r1 = Rectangle(0.0, 3.0, 6.0, 4.0);
    // Intersects rectangle 0
    // Does not intersect rectagle 1
    // Intersects rectangle 2
    // Does not intersect rectangle 3
    // Intersects rectangle 4

	double geometricIntersectionArea = ip1.computeIntersectionArea(r1);
	std::vector<Rectangle> geometricIntersection = ip1.intersection(r1);

	REQUIRE(geometricIntersectionArea == 3.0);
	REQUIRE(geometricIntersection.size() == 3);
	REQUIRE(geometricIntersection[0] == Rectangle(0.0, 3.0, 1.0, 4.0));
	REQUIRE(geometricIntersection[1] == Rectangle(2.0, 3.0, 3.0, 4.0));
	REQUIRE(geometricIntersection[2] == Rectangle(5.0, 3.0, 6.0, 4.0));

	// Test set one, a general complicated intersection
	IsotheticPolygon ip2(Rectangle(0.0, 1.0, 1.0, 5.0));
	ip2.basicRectangles.push_back(Rectangle(2.0, 1.0, 3.0, 5.0));
	ip2.basicRectangles.push_back(Rectangle(3.0, 0.0, 4.0, 3.0));
	ip2.basicRectangles.push_back(Rectangle(3.0, 2.0, 5.0, 3.0));
	ip2.basicRectangles.push_back(Rectangle(5.0, 2.0, 6.0, 5.0));
	ip2.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	IsotheticPolygon ip3(Rectangle(0.0, 1.0, 2.0, 2.0));
	ip3.basicRectangles.push_back(Rectangle(1.0, 2.0, 2.0, 3.5));
	ip3.basicRectangles.push_back(Rectangle(2.0, 1.0, 3.0, 4.0));
	ip3.basicRectangles.push_back(Rectangle(2.0, 4.0, 4.5, 4.5));
	ip3.boundingBox = Rectangle(0.0, 1.0, 4.5, 4.5);

	ip2.intersection(ip3);

	REQUIRE(ip2.basicRectangles.size() == 3);
	REQUIRE(ip2.basicRectangles[0] == Rectangle(0.0, 1.0, 1.0, 2.0));
	REQUIRE(ip2.basicRectangles[1] == Rectangle(2.0, 1.0, 3.0, 4.0));
	REQUIRE(ip2.basicRectangles[2] == Rectangle(2.0, 4.0, 3.0, 4.5));
}

TEST_CASE("Geometry: testPolygonIncreaseResolution")
{
	Rectangle r1 = Rectangle(0.0, 1.0, 1.0, 5.0);
	Rectangle r2 = Rectangle(2.0, 0.0, 3.0, 5.0);
	Rectangle r3 = Rectangle(3.0, 0.0, 4.0, 3.0);
	Rectangle r4 = Rectangle(3.0, 2.0, 5.0, 3.0);
	Rectangle r5 = Rectangle(5.0, 2.0, 6.0, 5.0);

	IsotheticPolygon reference(r1);
	reference.basicRectangles.push_back(r2);
	reference.basicRectangles.push_back(r3);
	reference.basicRectangles.push_back(r4);
	reference.basicRectangles.push_back(r5);
	reference.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	// Test set one, general and minimum intersection of the reference
	IsotheticPolygon ip1(reference);
	IsotheticPolygon ip2(Rectangle(0.0, 0.0, 1.0, 1.5));
	ip2.basicRectangles.push_back(Rectangle(1.0, 1.5, 2.5, 3.0));

	ip1.increaseResolution(Point::atInfinity, ip2);

	REQUIRE(ip1.basicRectangles.size() == 7);
	REQUIRE(ip1.basicRectangles[0] == Rectangle(0.0, 1.5, 1.0, 5.0));
	REQUIRE(ip1.basicRectangles[1] == Rectangle(2.5, 0.0, 3.0, 5.0));
	REQUIRE(ip1.basicRectangles[2] == Rectangle(2.0, 3.0, 2.5, 5.0));
	REQUIRE(ip1.basicRectangles[3] == Rectangle(2.0, 0.0, 2.5, 1.5));
	REQUIRE(ip1.basicRectangles[4] == r3);
	REQUIRE(ip1.basicRectangles[5] == r4);
	REQUIRE(ip1.basicRectangles[6] == r5);

	// Test set two, limited and maximum intersection of the reference
	IsotheticPolygon ip3(reference);
	IsotheticPolygon ip4(Rectangle(4.25, 0.5, 5.0, 2.5));
	ip4.basicRectangles.push_back(Rectangle(3.25, 0.5, 3.75, 1.5));

	ip3.increaseResolution(Point::atInfinity, ip4);

	REQUIRE(ip3.basicRectangles.size() == 9);
	REQUIRE(ip3.basicRectangles[0] == r1);
	REQUIRE(ip3.basicRectangles[1] == r2);
	REQUIRE(ip3.basicRectangles[2] == Rectangle(3.75, 0.0, 4.0, 3.0));
	REQUIRE(ip3.basicRectangles[3] == Rectangle(3.0, 0.0, 3.25, 3.0));
	REQUIRE(ip3.basicRectangles[4] == Rectangle(3.25, 1.5, 3.75, 3.0));
	REQUIRE(ip3.basicRectangles[5] == Rectangle(3.25, 0.0, 3.75, 0.5));
	REQUIRE(ip3.basicRectangles[6] == Rectangle(3.0, 2.0, 4.25, 3.0));
	REQUIRE(ip3.basicRectangles[7] == Rectangle(4.25, 2.5, 5.0, 3.0));
	REQUIRE(ip3.basicRectangles[8] == r5);
}

TEST_CASE("Geometry: testPolygonMinMaxLimit")
{
	Rectangle r1 = Rectangle(0.0, 1.0, 1.0, 5.0);
	Rectangle r2 = Rectangle(2.0, 0.0, 3.0, 5.0);
	Rectangle r3 = Rectangle(3.0, 0.0, 4.0, 3.0);
	Rectangle r4 = Rectangle(3.0, 2.0, 5.0, 3.0);
	Rectangle r5 = Rectangle(5.0, 2.0, 6.0, 5.0);

	IsotheticPolygon reference(r1);
	reference.basicRectangles.push_back(r2);
	reference.basicRectangles.push_back(r3);
	reference.basicRectangles.push_back(r4);
	reference.basicRectangles.push_back(r5);
	reference.boundingBox = Rectangle(0.0, 0.0, 6.0, 5.0);

	// Test set one, min limit in empty space
	IsotheticPolygon ip1(reference);
	ip1.minLimit(1.5, 0);

	REQUIRE(ip1.basicRectangles.size() == 4);
	REQUIRE(ip1.basicRectangles[0] == r5);
	REQUIRE(ip1.basicRectangles[1] == r2);
	REQUIRE(ip1.basicRectangles[2] == r3);
	REQUIRE(ip1.basicRectangles[3] == r4);

	// Test set two, max limit in empty space
	IsotheticPolygon ip2(reference);
	ip2.maxLimit(1.5, 0);

	REQUIRE(ip2.basicRectangles.size() == 1);
	REQUIRE(ip2.basicRectangles[0] == r1);

	// Test set three, min limit cuts some rectangles
	IsotheticPolygon ip3(reference);
	ip3.minLimit(3.5, 1);

	REQUIRE(ip3.basicRectangles.size() == 3);
	REQUIRE(ip3.basicRectangles[0] == Rectangle(0.0, 3.5, 1.0, 5.0));
	REQUIRE(ip3.basicRectangles[1] == Rectangle(2.0, 3.5, 3.0, 5.0));
	REQUIRE(ip3.basicRectangles[2] == Rectangle(5.0, 3.5, 6.0, 5.0));

	// Test set four, max limit cuts some rectangles
	IsotheticPolygon ip4(reference);
	ip4.maxLimit(3.5, 1);

	REQUIRE(ip4.basicRectangles.size() == 5);
	REQUIRE(ip4.basicRectangles[0] == Rectangle(0.0, 1.0, 1.0, 3.5));
	REQUIRE(ip4.basicRectangles[1] == Rectangle(2.0, 0.0, 3.0, 3.5));
	REQUIRE(ip4.basicRectangles[2] == r3);
	REQUIRE(ip4.basicRectangles[3] == r4);
	REQUIRE(ip4.basicRectangles[4] == Rectangle(5.0, 2.0, 6.0, 3.5));

	// Test set five, min limit cuts all rectangles
	IsotheticPolygon ip5(reference);
	ip5.minLimit(2.5, 1);

	REQUIRE(ip5.basicRectangles.size() == 5);
	REQUIRE(ip5.basicRectangles[0] == Rectangle(0.0, 2.5, 1.0, 5.0));
	REQUIRE(ip5.basicRectangles[1] == Rectangle(2.0, 2.5, 3.0, 5.0));
	REQUIRE(ip5.basicRectangles[2] == Rectangle(3.0, 2.5, 4.0, 3.0));
	REQUIRE(ip5.basicRectangles[3] == Rectangle(3.0, 2.5, 5.0, 3.0));
	REQUIRE(ip5.basicRectangles[4] == Rectangle(5.0, 2.5, 6.0, 5.0));

	// Test set six, max limit cuts all rectangles
	IsotheticPolygon ip6(reference);
	ip6.maxLimit(2.5, 1);

	REQUIRE(ip6.basicRectangles.size() == 5);
	REQUIRE(ip6.basicRectangles[0] == Rectangle(0.0, 1.0, 1.0, 2.5));
	REQUIRE(ip6.basicRectangles[1] == Rectangle(2.0, 0.0, 3.0, 2.5));
	REQUIRE(ip6.basicRectangles[2] == Rectangle(3.0, 0.0, 4.0, 2.5));
	REQUIRE(ip6.basicRectangles[3] == Rectangle(3.0, 2.0, 5.0, 2.5));
	REQUIRE(ip6.basicRectangles[4] == Rectangle(5.0, 2.0, 6.0, 2.5));

	// Test set seven, min limit along border
	IsotheticPolygon ip7(reference);
	ip7.minLimit(4.0, 0);

	REQUIRE(ip7.basicRectangles.size() == 3);
	REQUIRE(ip7.basicRectangles[0] == r5);
	REQUIRE(ip7.basicRectangles[1] == Rectangle(4.0, 2.0, 5.0, 3.0));
	REQUIRE(ip7.basicRectangles[2] == Rectangle(4.0, 0.0, 4.0, 3.0));

	// Test set eight, max limit along border
	IsotheticPolygon ip8(reference);
	ip8.maxLimit(4.0, 0);

	REQUIRE(ip8.basicRectangles.size() == 4);
	REQUIRE(ip8.basicRectangles[0] == r1);
	REQUIRE(ip8.basicRectangles[1] == r2);
	REQUIRE(ip8.basicRectangles[2] == r3);
	REQUIRE(ip8.basicRectangles[3] == Rectangle(3.0, 2.0, 4.0, 3.0));
}

TEST_CASE("Geometry: testPolygonMerge")
{
	Rectangle r1 = Rectangle(70.1, 80.2, 80.2, 90.3);
	Rectangle r2 = Rectangle(60.0, 60.0, 70.1, 80.2);
	Rectangle r3 = Rectangle(100.3, 200.4, 105.3, 205.4);
	Rectangle r4 = Rectangle(1.0, 1.0, 2.0, 2.0);
	Rectangle r5 = Rectangle(2.0, 1.0, 3.0, 2.0);

	// Test case one, general case
	IsotheticPolygon p1(r1);
	p1.basicRectangles.push_back(r2);

	IsotheticPolygon p2(r3);
	p2.basicRectangles.push_back(r4);
	p2.basicRectangles.push_back(r5);

	p1.merge(p2);

	REQUIRE(p1.basicRectangles.size() == 5);
	REQUIRE(p2.basicRectangles.size() == 3);
	REQUIRE(p1.basicRectangles[0] == r1);
	REQUIRE(p1.basicRectangles[1] == r2);
	REQUIRE(p1.basicRectangles[2] == r3);
	REQUIRE(p1.basicRectangles[3] == r4);
	REQUIRE(p1.basicRectangles[4] == r5);

	// Test case two, merge non-existant poly and existant poly
	IsotheticPolygon p3;
	IsotheticPolygon p4(r4);

	p3.merge(p4);

	REQUIRE(p3.basicRectangles.size() == 1);
	REQUIRE(p3.basicRectangles[0] == r4);

	// Test case three, merge existant poly and non-existant poly
	IsotheticPolygon p5;
	IsotheticPolygon p6(r2);

	p6.merge(p5);

	REQUIRE(p6.basicRectangles.size() == 1);
	REQUIRE(p6.basicRectangles[0] == r2);
}

TEST_CASE("Geometry: testPolygonRemove")
{
	Rectangle r1 = Rectangle(70.1, 80.2, 80.2, 90.3);
	Rectangle r2 = Rectangle(60.0, 60.0, 70.1, 80.2);
	Rectangle r3 = Rectangle(100.3, 200.4, 105.3, 205.4);
	Rectangle r4 = Rectangle(1.0, 1.0, 2.0, 2.0);
	Rectangle r5 = Rectangle(2.0, 1.0, 3.0, 2.0);

	// Test set one, general case
	IsotheticPolygon p1(r1);
	p1.basicRectangles.push_back(r2);
	p1.basicRectangles.push_back(r3);
	p1.basicRectangles.push_back(r4);

	p1.remove(2);

	REQUIRE(p1.basicRectangles.size() == 3);
	REQUIRE(p1.basicRectangles[0] == r1);
	REQUIRE(p1.basicRectangles[1] == r2);
	REQUIRE(p1.basicRectangles[2] == r4);

	// Test set two, repeated removal
	IsotheticPolygon p2(r1);
	p2.basicRectangles.push_back(r2);
	p2.basicRectangles.push_back(r3);
	p2.basicRectangles.push_back(r4);
	p2.basicRectangles.push_back(r5);

	p2.remove(1);

	REQUIRE(p2.basicRectangles.size() == 4);
	REQUIRE(p2.basicRectangles[0] == r1);
	REQUIRE(p2.basicRectangles[1] == r5);
	REQUIRE(p2.basicRectangles[2] == r3);
	REQUIRE(p2.basicRectangles[3] == r4);

	p2.remove(1);

	REQUIRE(p2.basicRectangles.size() == 3);
	REQUIRE(p2.basicRectangles[0] == r1);
	REQUIRE(p2.basicRectangles[1] == r4);
	REQUIRE(p2.basicRectangles[2] == r3);

	p2.remove(2);

	REQUIRE(p2.basicRectangles.size() == 2);
	REQUIRE(p2.basicRectangles[0] == r1);
	REQUIRE(p2.basicRectangles[1] == r4);
}

TEST_CASE("Geometry: testPolygonShrink")
{
	Rectangle r1 = Rectangle(0.0, 1.0, nextafter(1.0, DBL_MAX), nextafter(5.0, DBL_MAX));
	Rectangle r2 = Rectangle(2.0, 0.0, nextafter(3.0, DBL_MAX),
            nextafter(5.0, DBL_MAX));
	Rectangle r3 = Rectangle(3.0, 0.0, nextafter(4.0, DBL_MAX),
            nextafter(3.0, DBL_MAX));
	Rectangle r4 = Rectangle(3.0, 2.0, nextafter(5.0, DBL_MAX),
            nextafter(3.0, DBL_MAX));
	Rectangle r5 = Rectangle(5.0, 2.0, nextafter(6.0, DBL_MAX),
            nextafter(5.0, DBL_MAX));

	IsotheticPolygon reference(r1);
	reference.basicRectangles.push_back(r2);
	reference.basicRectangles.push_back(r3);
	reference.basicRectangles.push_back(r4);
	reference.basicRectangles.push_back(r5);
	reference.boundingBox = Rectangle(0.0, 0.0, nextafter(6.0, DBL_MAX),
            nextafter(5.0, DBL_MAX));

	// Test set one, one rectangle shrinks
	std::vector<Point> pinPoints1;
	pinPoints1.push_back(Point(1.0, 1.0));
	pinPoints1.push_back(Point(0.0, 5.0));
	pinPoints1.push_back(Point(2.3, 1.7));
	pinPoints1.push_back(Point(3.0, 0.0));
	pinPoints1.push_back(Point(3.0, 4.0));
	pinPoints1.push_back(Point(4.0, 0.0));
	pinPoints1.push_back(Point(3.0, 3.0));
	pinPoints1.push_back(Point(5.0, 2.0));
	pinPoints1.push_back(Point(6.0, 5.0));

	IsotheticPolygon ip1(reference);
	ip1.shrink(pinPoints1);

	REQUIRE(ip1.basicRectangles.size() == 5);
	REQUIRE(ip1.basicRectangles[0] == r1);
	REQUIRE(ip1.basicRectangles[1] == Rectangle(2.3, 0.0, nextafter(3.0,
                    DBL_MAX), nextafter(4.0, DBL_MAX)));
	REQUIRE(ip1.basicRectangles[2] == r3);
	REQUIRE(ip1.basicRectangles[3] == r4);
	REQUIRE(ip1.basicRectangles[4] == r5);

	// Test set two, no shrinkage
	std::vector<Point> pinPoints2;
	pinPoints2.push_back(Point(0.0, 1.0));
	pinPoints2.push_back(Point(1.0, 5.0));
	pinPoints2.push_back(Point(2.0, 0.0));
	pinPoints2.push_back(Point(3.0, 5.0));
	pinPoints2.push_back(Point(3.0, 0.0));
	pinPoints2.push_back(Point(4.0, 3.0));
	pinPoints2.push_back(Point(3.0, 2.0));
	pinPoints2.push_back(Point(5.0, 3.0));
	pinPoints2.push_back(Point(5.0, 2.0));
	pinPoints2.push_back(Point(6.0, 5.0));

	IsotheticPolygon ip2(reference);
	ip2.shrink(pinPoints2);

	REQUIRE(ip2.basicRectangles.size() == 5);
	REQUIRE(ip2.basicRectangles[0] == r1);
	REQUIRE(ip2.basicRectangles[1] == r2);
	REQUIRE(ip2.basicRectangles[2] == r3);
	REQUIRE(ip2.basicRectangles[3] == r4);
	REQUIRE(ip2.basicRectangles[4] == r5);

	// Test set three, all rectangles shrink
	std::vector<Point> pinPoints3;
	pinPoints3.push_back(Point(0.0, 2.5));
	pinPoints3.push_back(Point(1.0, 4.0));
	pinPoints3.push_back(Point(2.5, 1.0));
	pinPoints3.push_back(Point(2.75, 2.75));
	pinPoints3.push_back(Point(3.5, 2.5));
	pinPoints3.push_back(Point(3.75, 1.0));
	pinPoints3.push_back(Point(5.0, 3.0));
	pinPoints3.push_back(Point(5.75, 4.0));

	IsotheticPolygon ip3(reference);
	ip3.shrink(pinPoints3);

	REQUIRE(ip3.basicRectangles.size() == 5);
	REQUIRE(ip3.basicRectangles[0] == Rectangle(0.0, 2.5, nextafter(1.0,
                    DBL_MAX), nextafter(4.0, DBL_MAX)));
	REQUIRE(ip3.basicRectangles[1] == Rectangle(2.5, 1.0,
                nextafter(2.75, DBL_MAX), nextafter(2.75, DBL_MAX)));
	REQUIRE(ip3.basicRectangles[2] == Rectangle(3.5, 1.0,
                nextafter(3.75, DBL_MAX), nextafter(2.5, DBL_MAX)));
	REQUIRE(ip3.basicRectangles[3] == Rectangle(3.5, 2.5, nextafter(5.0,
                    DBL_MAX), nextafter(3.0, DBL_MAX)));
	REQUIRE(ip3.basicRectangles[4] == Rectangle(5.0, 3.0,
                nextafter(5.75, DBL_MAX),
                nextafter(4.0, DBL_MAX)));

	// Test set four, some rectangles are eliminated
	std::vector<Point> pinPoints4;
	pinPoints4.push_back(Point(0.0, 2.5));
	pinPoints4.push_back(Point(1.0, 4.0));
	pinPoints4.push_back(Point(3.5, 2.5));
	pinPoints4.push_back(Point(3.75, 1.0));
	pinPoints4.push_back(Point(5.0, 3.0));
	pinPoints4.push_back(Point(5.75, 4.0));

	IsotheticPolygon ip4(reference);
	ip4.shrink(pinPoints4);

	REQUIRE(ip4.basicRectangles.size() == 4);
	REQUIRE(ip4.basicRectangles[0] == Rectangle(0.0, 2.5, nextafter(1.0,
                    DBL_MAX), nextafter(4.0, DBL_MAX)));
	REQUIRE(ip4.basicRectangles[1] == Rectangle(3.5, 1.0,
                nextafter(3.75, DBL_MAX), nextafter(2.5, DBL_MAX)));
	REQUIRE(ip4.basicRectangles[2] == Rectangle(3.5, 2.5, nextafter(5.0,
                    DBL_MAX), nextafter(3.0, DBL_MAX)));
	REQUIRE(ip4.basicRectangles[3] == Rectangle(5.0, 3.0,
                nextafter(5.75, DBL_MAX),
                nextafter(4.0, DBL_MAX)));
}

TEST_CASE("Geometry: testPolygonRefine")
{
	// Test set one, general case
	Rectangle r1 = Rectangle(-2.0, 0.0, 4.0, 4.0);
	Rectangle r2 = Rectangle(-5.0, 0.0, 0.0, 4.0);
	Rectangle r3 = Rectangle(3.0, 0.0, 7.0, 4.0);
	Rectangle r4 = Rectangle(7.0, 0.0, 9.0, 7.0);

	IsotheticPolygon p1;
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);
	p1.basicRectangles.push_back(r4);
	p1.basicRectangles.push_back(r3);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 2);
	REQUIRE(p1.basicRectangles[0] == Rectangle(-5.0, 0.0, 7.0, 4.0));
	REQUIRE(p1.basicRectangles[1] == Rectangle(7.0, 0.0, 9.0, 7.0));

	// Test set two, ducks in a row
	r1 = Rectangle(-4.0, -1.0, 0.1, 3.0);
	r2 = Rectangle(0.0, -1.0, 4.1, 3.0);
	r3 = Rectangle(4.0, -1.0, 8.0, 3.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r3);
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 1);
	REQUIRE(p1.basicRectangles[0] == Rectangle(-4.0, -1.0, 8.0, 3.0));

	// Test set three, edge case
	r1 = Rectangle(0.0, 0.0, 5.0, 4.0);
	r2 = Rectangle(3.0, 0.0, 9.0, 0.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 2);
	REQUIRE(p1.basicRectangles[0] == Rectangle(0.0, 0.0, 5.0, 4.0));
	REQUIRE(p1.basicRectangles[1] == Rectangle(3.0, 0.0, 9.0, 0.0));

	// Test set four, edge case
	r1 = Rectangle(1.0, 1.0, 7.0, 4.0);
	r2 = Rectangle(1.0, 1.0, 7.0, 1.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r2);
	p1.basicRectangles.push_back(r1);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 1);
	REQUIRE(p1.basicRectangles[0] == Rectangle(1.0, 1.0, 7.0, 4.0));

	// Test set five, corner case
	r1 = Rectangle(4.0, 4.0, 5.0, 7.0);
	r2 = Rectangle(0.0, 0.0, 4.0, 4.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 2);
	REQUIRE(p1.basicRectangles[0] == Rectangle(0.0, 0.0, 4.0, 4.0));
	REQUIRE(p1.basicRectangles[1] == Rectangle(4.0, 4.0, 5.0, 7.0));

	// Test set six, square with hole
	r1 = Rectangle(2.0, 4.0, 4.0, 6.0);
	r2 = Rectangle(4.0, 0.0, 6.0, 6.0);
	r3 = Rectangle(2.0, 0.0, 4.0, 2.0);
	r4 = Rectangle(0.0, 0.0, 2.0, 6.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);
	p1.basicRectangles.push_back(r3);
	p1.basicRectangles.push_back(r4);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 4);
	REQUIRE(p1.basicRectangles[0] == r4);
	REQUIRE(p1.basicRectangles[1] == r3);
	REQUIRE(p1.basicRectangles[2] == r2);
	REQUIRE(p1.basicRectangles[3] == r1);

	// Test set seven, no touchy
	r1 = Rectangle(0.0, 0.0, 2.0, 4.0);
	r2 = Rectangle(4.0, 0.0, 6.0, 4.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 2);
	REQUIRE(p1.basicRectangles[0] == r1);
	REQUIRE(p1.basicRectangles[1] == r2);

	// Test set eight, H formation
	r1 = Rectangle(2.0, 2.0, 3.0, 3.0);
	r2 = Rectangle(0.0, 0.0, 2.0, 4.0);
	r3 = Rectangle(3.0, 0.0, 5.0, 4.0);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);
	p1.basicRectangles.push_back(r3);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 3);
	REQUIRE(p1.basicRectangles[0] == r2);
	REQUIRE(p1.basicRectangles[1] == r3);
	REQUIRE(p1.basicRectangles[2] == r1);

	// Test set nine, combinanble in X then in Y
	r1 = Rectangle(0.0, 1.9, 3.0, 4.0);
	r2 = Rectangle(0.0, 1.0, 6.0, 2.0);
	r3 = Rectangle(2.9, 1.9, 6.0, 4.0);
	r4 = Rectangle(0.0, 0.0, 6.0, 1.1);

	p1.basicRectangles.clear();
	p1.basicRectangles.push_back(r1);
	p1.basicRectangles.push_back(r2);
	p1.basicRectangles.push_back(r3);
	p1.basicRectangles.push_back(r4);

	p1.refine();
	REQUIRE(p1.basicRectangles.size() == 1);
	REQUIRE(p1.basicRectangles[0] == Rectangle(0.0, 0.0, 6.0, 4.0));
}

TEST_CASE( "Geometry: InlineBoundedIsotheticPolygon materialization" ) {
    InlineBoundedIsotheticPolygon inline_poly;
    Point one_one(1,1);
    Point two_two(2,2);
    Rectangle one_two_rect( one_one, two_two );
    IsotheticPolygon loc_poly;
    loc_poly.basicRectangles = { one_two_rect, one_two_rect,
        one_two_rect, one_two_rect, one_two_rect };
    loc_poly.recomputeBoundingBox();
    inline_poly.push_polygon_to_disk( loc_poly );
    IsotheticPolygon materialized_rect =
        inline_poly.materialize_polygon();
    REQUIRE( materialized_rect.basicRectangles.size() == 5 );
    for( const auto &rect : materialized_rect.basicRectangles ) {
        REQUIRE( rect == one_two_rect );
    }

    auto iter1 = inline_poly.begin();
    auto iter2 = materialized_rect.basicRectangles.begin();
    for( int i = 0; i < 5; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly.end() );
    REQUIRE( iter2 == materialized_rect.basicRectangles.end() );
    REQUIRE( inline_poly.get_summary_rectangle() == one_two_rect );
}

TEST_CASE( "Geometry: InlineBoundedIsotheticPolygon smaller, materialization" ) {
    InlineBoundedIsotheticPolygon inline_poly;
    Point one_one(1,1);
    Point two_two(2,2);
    Rectangle one_two_rect( one_one, two_two );
    IsotheticPolygon loc_poly;
    loc_poly.basicRectangles = { one_two_rect, one_two_rect,
        one_two_rect };
    loc_poly.recomputeBoundingBox();
    inline_poly.push_polygon_to_disk( loc_poly );
    IsotheticPolygon materialized_rect =
        inline_poly.materialize_polygon();
    REQUIRE( materialized_rect.basicRectangles.size() == 3 );
    for( const auto &rect : materialized_rect.basicRectangles ) {
        REQUIRE( rect == one_two_rect );
    }

    auto iter1 = inline_poly.begin();
    auto iter2 = materialized_rect.basicRectangles.begin();
    for( int i = 0; i < 3; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly.end() );
    REQUIRE( iter2 == materialized_rect.basicRectangles.end() );
    REQUIRE( inline_poly.get_summary_rectangle() == one_two_rect );
}

TEST_CASE( "Geometry: InlineBoundedIsotheticPolygon push to disk" ) {
    Point one_one(1,1);
    Point two_two(2,2);
    Rectangle one_two_rect( one_one, two_two );

    InlineBoundedIsotheticPolygon inline_poly;
    IsotheticPolygon local_poly;
    local_poly.basicRectangles = { one_two_rect, one_two_rect,
        one_two_rect };
    local_poly.recomputeBoundingBox();
    inline_poly.push_polygon_to_disk( local_poly );
    REQUIRE( inline_poly.get_rectangle_count() == 3 );
    auto iter1 = inline_poly.begin();
    auto iter2 = local_poly.basicRectangles.begin();
    for( int i = 0; i < 3; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly.end() );
    REQUIRE( iter2 == local_poly.basicRectangles.end() );
    REQUIRE( inline_poly.get_summary_rectangle() == one_two_rect );
}

class TestInlineUnboundedIsotheticPolygon : public InlineUnboundedIsotheticPolygon {
public:
    TestInlineUnboundedIsotheticPolygon(
            tree_node_allocator *allocator,
            unsigned max_rectangle_count_on_first_page
            ) : InlineUnboundedIsotheticPolygon( allocator,
                max_rectangle_count_on_first_page ) {}

    PageableIsotheticPolygon &get_poly_data() {
        return poly_data_;
    }

    void set_total_rectangles( unsigned total_rectangles ) {
        total_rectangle_count_ = total_rectangles;
    }

    void set_summary_rectangle( const Rectangle &rect ) {
        summary_rectangle_ = rect;
    }

};

TEST_CASE( "Geometry: Materialize single page big poly" ) {

    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    size_t max_rects_on_first_page = ((PAGE_DATA_SIZE -
            sizeof(TestInlineUnboundedIsotheticPolygon))/sizeof(Rectangle))
        + 1;

    auto alloc_data =
        allocator.create_new_tree_node<TestInlineUnboundedIsotheticPolygon>(
                PAGE_DATA_SIZE, NodeHandleType( 0 ) );
    new (&(*(alloc_data.first))) TestInlineUnboundedIsotheticPolygon(
            &allocator, max_rects_on_first_page );
    pinned_node_ptr<TestInlineUnboundedIsotheticPolygon> inline_poly = alloc_data.first;


    REQUIRE( sizeof(TestInlineUnboundedIsotheticPolygon) +
            (sizeof(Rectangle) * (max_rects_on_first_page)) +
            (sizeof(page_header)) > PAGE_SIZE );

    REQUIRE( sizeof(TestInlineUnboundedIsotheticPolygon) +
            (sizeof(Rectangle) * (max_rects_on_first_page-1)) +
            (sizeof(page_header)) <= PAGE_SIZE );

    PageableIsotheticPolygon &poly_data = inline_poly->get_poly_data();
    for( size_t i = 0; i < max_rects_on_first_page; i++ ) {

        poly_data.basicRectangles[i] = Rectangle(
                Point(i,i), Point(i,i) );
    }
    poly_data.rectangle_count_ = max_rects_on_first_page;
    inline_poly->set_total_rectangles( max_rects_on_first_page );
    inline_poly->set_summary_rectangle( Rectangle( Point(0,0),
                Point(max_rects_on_first_page-1,
                    max_rects_on_first_page-1)
                ) );

    IsotheticPolygon materialized_poly =
        inline_poly->materialize_polygon();
    REQUIRE( materialized_poly.basicRectangles.size() ==
            inline_poly->get_total_rectangle_count() );

    auto iter1 = inline_poly->begin();
    auto iter2 = materialized_poly.basicRectangles.begin();
    for( size_t i = 0; i < max_rects_on_first_page; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly->end() );
    REQUIRE( iter2 == materialized_poly.basicRectangles.end() );
    REQUIRE( inline_poly->get_summary_rectangle() == Rectangle( Point(0,0
                    ), Point( max_rects_on_first_page-1,
                        max_rects_on_first_page-1) ) );
    REQUIRE( materialized_poly.boundingBox ==
            inline_poly->get_summary_rectangle() );

}

TEST_CASE( "Geometry: Materialize multi-page big poly" ) {
    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    size_t max_rects_on_first_page = ((PAGE_DATA_SIZE -
            sizeof(TestInlineUnboundedIsotheticPolygon))/sizeof(Rectangle))
        + 1;

    auto alloc_data =
        allocator.create_new_tree_node<TestInlineUnboundedIsotheticPolygon>(
                PAGE_DATA_SIZE, NodeHandleType( 0 ) );
    new (&(*(alloc_data.first))) TestInlineUnboundedIsotheticPolygon(
            &allocator, max_rects_on_first_page );
    pinned_node_ptr<TestInlineUnboundedIsotheticPolygon> inline_poly = alloc_data.first;

    size_t max_rectangles_per_page =
        PageableIsotheticPolygon::get_max_rectangle_count_per_page();

    size_t total_rectangles = max_rects_on_first_page + 2 *
        max_rectangles_per_page - 5;

    PageableIsotheticPolygon &poly_data = inline_poly->get_poly_data();
    for( size_t i = 0; i < max_rects_on_first_page; i++ ) {
        poly_data.basicRectangles[i] = Rectangle(
                Point(i,i), Point(i,i) );
    }
    poly_data.rectangle_count_ = max_rects_on_first_page;
    inline_poly->set_total_rectangles( total_rectangles );
    inline_poly->set_summary_rectangle( Rectangle( Point(0,0),
                Point(max_rectangles_per_page-1,max_rectangles_per_page-1)));

    auto poly_alloc_data =
        allocator.create_new_tree_node<PageableIsotheticPolygon>(
                PAGE_DATA_SIZE, NodeHandleType( 0 ) );
    pinned_node_ptr<PageableIsotheticPolygon> pageable_poly =
        poly_alloc_data.first;
    new (&(*(poly_alloc_data.first))) PageableIsotheticPolygon();

    poly_data.next_ = poly_alloc_data.second;
    pageable_poly->rectangle_count_ = max_rectangles_per_page;
    for( size_t i = 0; i < max_rectangles_per_page; i++ ) {
        pageable_poly->basicRectangles[i] = Rectangle( Point(i,i),
                Point(i,i) );
    }

    poly_alloc_data =
        allocator.create_new_tree_node<PageableIsotheticPolygon>(
                PAGE_DATA_SIZE, NodeHandleType( 0 ) );
    new (&(*poly_alloc_data.first)) PageableIsotheticPolygon();
    pageable_poly->next_ = poly_alloc_data.second;

    pageable_poly = poly_alloc_data.first;
    pageable_poly->rectangle_count_ = max_rectangles_per_page-5;
    for( size_t i = 0; i < max_rectangles_per_page-5; i++ ) {
        pageable_poly->basicRectangles[i] = Rectangle( Point(i,i),
                Point(i,i) );
    }
    pageable_poly->next_ = tree_node_handle(nullptr);


    IsotheticPolygon materialized_poly =
        inline_poly->materialize_polygon();
    REQUIRE( materialized_poly.basicRectangles.size() ==
            inline_poly->get_total_rectangle_count() );

    auto iter1 = inline_poly->begin();
    auto iter2 = materialized_poly.basicRectangles.begin();
    for( size_t i = 0; i < max_rects_on_first_page + (2 *
            max_rectangles_per_page)-5; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly->end() );
    REQUIRE( iter2 == materialized_poly.basicRectangles.end() );
    REQUIRE( inline_poly->get_summary_rectangle() ==
            Rectangle(Point(0,0), Point(max_rectangles_per_page-1,
                    max_rectangles_per_page-1)) );

}

TEST_CASE( "Write single_page big poly to disk" ) {
    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    size_t max_rects_on_first_page = ((PAGE_DATA_SIZE -
            sizeof(InlineUnboundedIsotheticPolygon))/sizeof(Rectangle))
        + 1;

    IsotheticPolygon polygon;
    for( size_t i = 0; i < max_rects_on_first_page-5; i++ ) {
        polygon.basicRectangles.push_back( Rectangle( Point(i,i),
                    Point(i,i) ) );
    }
    polygon.recomputeBoundingBox();

    auto alloc_data = allocator.create_new_tree_node<InlineUnboundedIsotheticPolygon>(
            PAGE_DATA_SIZE, NodeHandleType( 0 ) );

    new (&(*(alloc_data.first))) InlineUnboundedIsotheticPolygon(
            &allocator, max_rects_on_first_page );

    auto inline_poly = alloc_data.first;

    inline_poly->push_polygon_to_disk( polygon );

    REQUIRE( inline_poly->get_total_rectangle_count() ==
            max_rects_on_first_page-5 );
    REQUIRE( inline_poly->get_cur_overflow_pages() == 0 );
    auto iter1 = inline_poly->begin();
    auto iter2 = polygon.basicRectangles.begin();
    for( size_t i = 0; i < max_rects_on_first_page-5; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly->end() );
    REQUIRE( iter2 == polygon.basicRectangles.end() );
    REQUIRE( polygon.boundingBox ==
            inline_poly->get_summary_rectangle() );
}

TEST_CASE( "Write multi-page big poly to disk" ) {
    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    size_t max_rects_on_first_page = ((PAGE_DATA_SIZE -
            sizeof(InlineUnboundedIsotheticPolygon))/sizeof(Rectangle))
        + 1;

    size_t max_rectangles_per_page =
        PageableIsotheticPolygon::get_max_rectangle_count_per_page();

    IsotheticPolygon polygon;
    for( size_t i = 0; i < max_rects_on_first_page +
            2*max_rectangles_per_page-5; i++ ) {
        polygon.basicRectangles.push_back( Rectangle( Point(i,i),
                    Point(i,i) ) );
    }
    polygon.recomputeBoundingBox();

    auto alloc_data = allocator.create_new_tree_node<InlineUnboundedIsotheticPolygon>(
            PAGE_DATA_SIZE, NodeHandleType( 0 ) );

    new (&(*(alloc_data.first))) InlineUnboundedIsotheticPolygon(
            &allocator, max_rects_on_first_page );

    auto inline_poly = alloc_data.first;

    inline_poly->push_polygon_to_disk( polygon );

    REQUIRE( inline_poly->get_total_rectangle_count() ==
            max_rects_on_first_page + 2 * max_rectangles_per_page - 5 );
    auto iter1 = inline_poly->begin();
    auto iter2 = polygon.basicRectangles.begin();
    for( size_t i = 0; i < max_rects_on_first_page + 2 *
            max_rectangles_per_page -5; i++ ) {
        REQUIRE( *iter1 == *iter2 );
        iter1++;
        iter2++;
    }
    REQUIRE( iter1 == inline_poly->end() );
    REQUIRE( iter2 == polygon.basicRectangles.end() );
    REQUIRE( inline_poly->get_summary_rectangle() == polygon.boundingBox
            );
}

TEST_CASE( "Test simplify real poly" ) {

    std::vector<Rectangle> rectangles;
    {
        Point lower(-115.57735750000000507, 32.851100999999999885);
        Point upper(-115.57423649999999782, 32.852118000000004372);
        Rectangle rect(lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.57423649999999782, 32.851100999999999885);
        Point upper(-115.56960850000000107, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56960850000000107, 32.851100999999999885);
        Point upper(-115.56886199999999576, 32.851477500000001442);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56886199999999576, 32.851100999999999885);
        Point upper(-115.56808849999998756, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56808849999998756, 32.851100999999999885);
        Point upper(-115.56640899999999306, 32.851454500000002668);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56640899999999306, 32.851100999999999885);
        Point upper(-115.56390999999999281, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56324599999999236, 32.851100999999999885);
        Point upper(-115.56324599999999236, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56199250000000234, 32.851100999999999885);
        Point upper(-115.56199250000000234, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );

    }
    {
        Point lower(-115.56139899999999443, 32.851100999999999885);
        Point upper(-115.56139899999999443, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );

    }
    {
        Point lower(-115.56960850000000107, 32.851511500000000865);
        Point upper(-115.56886199999999576, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56808849999998756, 32.851889999999997372);
        Point upper(-115.56640899999999306, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56390999999999281, 32.852047999999996364);
        Point upper(-115.56324599999999236, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56324599999999236, 32.852047999999996364);
        Point upper(-115.56199250000000234, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }
    {
        Point lower(-115.56199250000000234, 32.852047999999996364);
        Point upper(-115.56139899999999443, 32.852118000000004372);
        Rectangle rect( lower, upper );
        rectangles.push_back( rect );
    }

    IsotheticPolygon polygon;
    polygon.basicRectangles = rectangles;
    polygon.recomputeBoundingBox();

    IsotheticPolygon polygon2( polygon );
    polygon.refine();
    REQUIRE( polygon.basicRectangles.size() <
            polygon2.basicRectangles.size() );
    REQUIRE( polygon.boundingBox == polygon2.boundingBox );

    std::cout << compute_sizeof_inline_unbounded_polygon(
            MAX_RECTANGLE_COUNT + 1 ) << std::endl;
}

TEST_CASE("Test computeExpansionAreaForInlinePolygon: Bounded") {
	Point one_one(1,1);
    Point two_two(2,2);
	Point three_three(3,3);
    Rectangle one_two_rect( one_one, two_two );
	Rectangle two_three_rect( two_two, three_three );

    InlineBoundedIsotheticPolygon inline_poly;
    IsotheticPolygon local_poly;
    local_poly.basicRectangles = { one_two_rect, two_three_rect };
    local_poly.recomputeBoundingBox();
    inline_poly.push_polygon_to_disk( local_poly );
    REQUIRE( inline_poly.get_rectangle_count() == 2 );
    auto begin = inline_poly.begin();
	auto end = inline_poly.end();

	// TESTING THE QUICK EXIT
	Point contained(1.5, 1.5);

	auto contained_expansion =  computeExpansionArea<InlineBoundedIsotheticPolygon, Rectangle  *>(
		inline_poly, begin, end, contained
	);

	REQUIRE(contained_expansion.index == 0);
	REQUIRE(contained_expansion.area == -1);

	Point not_contained(0.5, 0.5);

	auto contained_expansion2 =  computeExpansionArea<InlineBoundedIsotheticPolygon, Rectangle  *>(
		inline_poly, begin, end, not_contained
	);

	REQUIRE(contained_expansion2.index == 0);
	REQUIRE(contained_expansion2.area == 1.25);

	Point far(6, 6);

	auto far_expansion =  computeExpansionArea<InlineBoundedIsotheticPolygon, Rectangle  *>(
		inline_poly, begin, end, far
	);

	REQUIRE(far_expansion.index == 1);
	REQUIRE((int)far_expansion.area == 15); // float comparison

}
