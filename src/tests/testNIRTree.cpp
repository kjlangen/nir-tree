#include <catch2/catch.hpp>
#include <nirtree/nirtree.h>

TEST_CASE("NIRTree: testTightenPoints")
{
	// setup
	nirtree::Node parent, node;
	node.parent = &parent; // set parent relation
	IsotheticPolygon polygon;
	polygon.basicRectangles.push_back(Rectangle(0.0, 5.0, 5.0, 10.0));
	polygon.basicRectangles.push_back(Rectangle(5.0, 0.0, 10.0, 10.0));
	parent.children.push_back(&node);
	parent.boundingBoxes.push_back(polygon);

	// data
	node.data.push_back(Point(2.0, 8.0));
	node.data.push_back(Point(4.0, 6.0));
	node.data.push_back(Point(6.0, 1.0));
	node.data.push_back(Point(9.0, 9.0));

	// result
	REQUIRE(!node.tighten());  // false indicates no change
	REQUIRE(node.data.size() == 4);
	REQUIRE(parent.boundingBoxes.at(0).basicRectangles.size() == 2);

	// clear data, then tighten
	node.data.clear();
	REQUIRE(node.tighten());  // true indicates change
	REQUIRE(node.data.size() == 0);
	REQUIRE(parent.boundingBoxes.at(0).basicRectangles.size() == 0);
	REQUIRE(!node.tighten());  // false indicates no change
}

TEST_CASE("NIRTree: testTightenRectNoChange")
{
	// setup
	nirtree::Node parent, node;
	node.parent = &parent; // set parent relation
	IsotheticPolygon polygon;
	polygon.basicRectangles.push_back(Rectangle(0.0, 10.0, 5.0, 25.0));
	polygon.basicRectangles.push_back(Rectangle(5.0, 5.0, 10.0, 20.0));
	polygon.basicRectangles.push_back(Rectangle(10.0, 0.0, 15.0, 15.0));
	parent.children.push_back(&node);
	parent.boundingBoxes.push_back(polygon);

	// children
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 23.0, 2.0, 25.0)));
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(3.0, 18.0, 7.0, 20.0)));
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(3.0, 10.0, 9.0, 15.0)));
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(8.0, 5.0, 10.0, 8.0)));
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(12.0, 0.0, 15.0, 4.0)));

	// result
	REQUIRE(!node.tighten());  // false indicates no change
	REQUIRE(node.boundingBoxes.size() == 5);
	REQUIRE(parent.boundingBoxes.at(0).basicRectangles.size() == 3);
}

TEST_CASE("NIRTree: testTightenRectMultiple")
{
	// setup
	nirtree::Node parent, node;
	node.parent = &parent; // set parent relation
	IsotheticPolygon polygon;
	polygon.basicRectangles.push_back(Rectangle(0.0, 10.0, 5.0, 25.0));
	polygon.basicRectangles.push_back(Rectangle(5.0, 5.0, 10.0, 20.0));
	polygon.basicRectangles.push_back(Rectangle(10.0, 0.0, 15.0, 15.0));
	parent.children.push_back(&node);
	parent.boundingBoxes.push_back(polygon);

	// children
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(6.0, 18.0, 7.0, 20.0)));
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(7.0, 10.0, 9.0, 15.0)));
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(8.0, 5.0, 10.0, 8.0)));

	// result
	REQUIRE(node.tighten());  // true indicates change
	REQUIRE(node.boundingBoxes.size() == 3);
	REQUIRE(parent.boundingBoxes.at(0).basicRectangles.size() == 1);
	REQUIRE(!node.tighten());  // false indicates no change
}

TEST_CASE("NIRTree: testTightenRectCascade")
{
	// setup
	nirtree::Node parent, node;
	node.parent = &parent; // set parent relation
	IsotheticPolygon polygon;
	polygon.basicRectangles.push_back(Rectangle(0.0, 10.0, 5.0, 25.0));
	polygon.basicRectangles.push_back(Rectangle(5.0, 5.0, 10.0, 20.0));
	polygon.basicRectangles.push_back(Rectangle(10.0, 0.0, 15.0, 15.0));
	parent.children.push_back(&node);
	parent.boundingBoxes.push_back(polygon);

	// children
	node.boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 23.0, 2.0, 25.0)));

	// result
	REQUIRE(node.tighten());  // true indicates change
	REQUIRE(node.boundingBoxes.size() == 1);
	REQUIRE(parent.boundingBoxes.at(0).basicRectangles.size() == 1);
	REQUIRE(!node.tighten());  // false indicates no change
}
