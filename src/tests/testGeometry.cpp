#include <catch2/catch.hpp>
#include <util/geometry.h>

TEST_CASE("Geometry: testPointEquality")
{
	// Test set one
	Point p1 = Point(0.0, 0.0);
	Point p2 = Point(0.0, 0.0);
	REQUIRE(p1 == p2);

	// Test set two
	Point p3 = Point(1.0, 1.0);
	Point p4 = Point(1.0, 1.0);
	REQUIRE(p3 == p4);

	// Test set three
	Point p5 = Point(-1.0, -1.0);
	Point p6 = Point(-1.0, -1.0);
	REQUIRE(p5 == p6);

	// Test set four
	Point p7 = Point(2.0, 1.0);
	Point p8 = Point(-2.0, -1.0);
	REQUIRE(p7 != p8);

	// Test set five
	Point p9 = Point(2.0, 1.0);
	Point p10 = Point(1.0, 2.0);
	REQUIRE(p9 != p10);
}

TEST_CASE("Geometry: testRectangleArea")
{
	// Test set one
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	REQUIRE(r1.area() == 0.0);

	// Test set two
	Rectangle r2 = Rectangle(0.0, 0.0, 1.0, 1.0);
	REQUIRE(r2.area() == 1.0);

	// Test set three
	Rectangle r3 = Rectangle(-5.0, -3.0, 13.0, 11.0);
	REQUIRE(r3.area() == 252.0);

	// Test set four, floating point math is super busted so it's not quite 259.86 as it should be
	Rectangle r4 = Rectangle(-5.0, -3.0, 13.3, 11.2);
	REQUIRE(r4.area() == 259.8599853515625);
}

TEST_CASE("Geometry: testRectangleComputeExpansionArea")
{
	// Test computing expansion area for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	REQUIRE(r1.computeExpansionArea(p1) == 45.0);

	// Test computing expansion area for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	REQUIRE(r2.computeExpansionArea(r3) == 18.0);

	// Test computing expansion area for a rectangle partially inside another rectangle
	Rectangle r4 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r5 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	REQUIRE(r4.computeExpansionArea(r5) == 26.0);

	// Test computing expansion area for a rectangle wholly inside another rectangle
	Rectangle r6 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r7 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	REQUIRE(r6.computeExpansionArea(r7) == 0.0);
}

TEST_CASE("Geometry: testRectangleExpansion")
{
	// Test computing expansion for a point
	Rectangle r1 = Rectangle(2.0, 8.0, 4.0, 10.0);
	Point p1 = Point(9.0, 15.0);
	r1.expand(p1);
	REQUIRE(r1.area() == 49.0);
	REQUIRE(r1.lowerLeft == Point(2.0, 8.0));
	REQUIRE(r1.upperRight == Point(9.0, 15.0));

	// Test computing expansion for a rectangle
	Rectangle r2 = Rectangle(4.0, -2.5, 5.0, 1.5);
	Rectangle r3 = Rectangle(1.0, 1.0, 5.0, 3.0);
	r2.expand(r3);
	REQUIRE(r2.area() == 22.0);
	REQUIRE(r2.lowerLeft == Point(1.0, -2.5));
	REQUIRE(r2.upperRight == Point(5.0, 3.0));

	// Test computing expansion area for a rectangle partially inside another rectangle
	Rectangle r4 = Rectangle(-9.0, 4.0, 1.0, 8.0);
	Rectangle r5 = Rectangle(-10.0, 2.0, -6.0, 6.0);
	r4.expand(r5);
	REQUIRE(r4.area() == 66.0);
	REQUIRE(r4.lowerLeft == Point(-10.0, 2.0));
	REQUIRE(r4.upperRight == Point(1.0, 8.0));

	// Test computing expansion area for a rectangle wholly inside another rectangle
	Rectangle r6 = Rectangle(-4.0, -3.0, 3.0, 5.0);
	Rectangle r7 = Rectangle(-2.5, -1.0, 0.5, 2.0);
	r6.expand(r7);
	REQUIRE(r6.area() == 56.0);
	REQUIRE(r6.lowerLeft == Point(-4.0, -3.0));
	REQUIRE(r6.upperRight == Point(3.0, 5.0));
}

TEST_CASE("Geometry: testRectangleIntersection")
{
	// Test corner on corner intersection
	Rectangle r1 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r2 = Rectangle(46.0, 34.8, 72.8, 61.6);
	REQUIRE(r1.intersectsRectangle(r2));

	// Test side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	REQUIRE(r3.intersectsRectangle(r4));
}

TEST_CASE("Geometry: testStrictRectangleIntersection")
{
	// Test corner on corner intersection
	Rectangle r1 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r2 = Rectangle(46.0, 34.8, 72.8, 61.6);
	REQUIRE(r1.strictIntersectsRectangle(r2));

	// Test side on side intersection
	Rectangle r3 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r4 = Rectangle(-9.2, -2.0, 0.8, 2.0);
	REQUIRE(r3.strictIntersectsRectangle(r4));

	// Test corner on corner intersection
	Rectangle r5 = Rectangle(37.6, 36.1, 48.8, 47.9);
	Rectangle r6 = Rectangle(48.8, 47.9, 72.8, 61.6);
	REQUIRE(!r5.strictIntersectsRectangle(r6));

	// Test side on side intersection
	Rectangle r7 = Rectangle(-19.2, -0.3, 10.8, 3.7);
	Rectangle r8 = Rectangle(-56.4, -8.0, -19.2, 5.0);
	REQUIRE(!r7.strictIntersectsRectangle(r8));
}

TEST_CASE("Geometry: testRectanglePointContainment")
{
	// Test set one
	Rectangle r1 = Rectangle(0.0, 0.0, 0.0, 0.0);
	Point p1 = Point(0.0, 0.0);
	REQUIRE(r1.containsPoint(p1));

	// Test set two
	Rectangle r2 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p2 = Point(-1.0, -1.0);
	REQUIRE(r2.containsPoint(p2));

	// Test set three
	Rectangle r3 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p3 = Point(217.3, 527.7);
	REQUIRE(!r3.containsPoint(p3));

	// Test set four
	Rectangle r4 = Rectangle(-1.0, -1.0, 3.0, 3.0);
	Point p4 = Point(3.0, 1.0);
	REQUIRE(r4.containsPoint(p4));
}

TEST_CASE("Geometry: testRectangleFragmentation")
{
	// Test set one
	Rectangle r1 = Rectangle(1.0, 1.0, 5.0, 5.0);
	Rectangle r2 = Rectangle(4.0, 4.0, 8.0, 8.0);
	std::vector<Rectangle> v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == Rectangle(1.0, 1.0, 5.0, 4.0));
	REQUIRE(v[1] == Rectangle(1.0, 4.0, 4.0, 5.0));

	// Test set two
	r1 = Rectangle(0.0, 0.0, 4.0, 4.0);
	r2 = Rectangle(1.0, -3.0, 6.0, 1.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == Rectangle(0.0, 1.0, 4.0, 4.0));
	REQUIRE(v[1] == Rectangle(0.0, 0.0, 1.0, 1.0));

	// Test set three
	r1 = Rectangle(-7.0, -5.0, 0.0, 0.0);
	r2 = Rectangle(-8.0, -8.0, -4.0, -3.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 2);
	REQUIRE(v[0] == Rectangle(-7.0, -3.0, 0.0, 0.0));
	REQUIRE(v[1] == Rectangle(-4.0, -5.0, 0.0, -3.0));

	// Test set four
	r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	r2 = Rectangle(3.0, 3.0, 7.0, 7.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 4);
	REQUIRE(v[0] == Rectangle(0.0, 0.0, 4.0, 3.0));
	REQUIRE(v[1] == Rectangle(0.0, 3.0, 3.0, 8.0));
	REQUIRE(v[2] == Rectangle(0.0, 7.0, 4.0, 8.0));
	REQUIRE(v[3] == Rectangle(0.0, 0.0, 3.0, 7.0));

	// Test set five
	r1 = Rectangle(0.0, 0.0, 4.0, 8.0);
	r2 = Rectangle(-3.0, 3.0, 1.0, 7.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 4);
	REQUIRE(v[0] == Rectangle(0.0, 7.0, 4.0, 8.0));
	REQUIRE(v[1] == Rectangle(1.0, 0.0, 4.0, 7.0));
	REQUIRE(v[2] == Rectangle(0.0, 0.0, 4.0, 3.0));
	REQUIRE(v[3] == Rectangle(1.0, 3.0, 4.0, 8.0));

	// Test set six
	r1 = Rectangle(0.0, 0.0, 8.0, 4.0);
	r2 = Rectangle(1.0, 2.0, 5.0, 6.0);
	v = r1.fragmentRectangle(r2);
	REQUIRE(v.size() == 4);
	REQUIRE(v[0] == Rectangle(0.0, 0.0, 8.0, 2.0));
	REQUIRE(v[1] == Rectangle(5.0, 2.0, 8.0, 4.0));
	REQUIRE(v[2] == Rectangle(0.0, 0.0, 8.0, 2.0));
	REQUIRE(v[3] == Rectangle(0.0, 2.0, 1.0, 4.0));
}
