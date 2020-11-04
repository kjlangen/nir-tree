#include <catch2/catch.hpp>
#include <nirtree/nirtree.h>

TEST_CASE("NIRTree: testPrefixConsistency")
{
	auto tree = nirtree::NIRTree(2, 3);

	tree.insert(Point(-14.5, -13.0));
	tree.print();
	REQUIRE(Point(-14.5, -13.0) == tree.search(Point(-14.5, -13.0)).front());

	tree.insert(Point(-14.0, -13.3));
	tree.print();
	REQUIRE(Point(-14.0, -13.3) == tree.search(Point(-14.0, -13.3)).front());

	tree.insert(Point(-12.5, -14.5));
	tree.print();
	REQUIRE(Point(-12.5, -14.5) == tree.search(Point(-12.5, -14.5)).front());

	tree.insert(Point(-15.0, -15.0));
	tree.print();
	REQUIRE(Point(-15.0, -15.0) == tree.search(Point(-15.0, -15.0)).front());

	tree.insert(Point(-10.0, -2.0));
	tree.print();
	REQUIRE(Point(-10.0, -2.0) == tree.search(Point(-10.0, -2.0)).front());

	tree.insert(Point(-11.0, -3.0));
	tree.print();
	REQUIRE(Point(-11.0, -3.0) == tree.search(Point(-11.0, -3.0)).front());

	tree.insert(Point(-9.0, -3.1));
	tree.print();
	REQUIRE(Point(-9.0, -3.1) == tree.search(Point(-9.0, -3.1)).front());

	tree.insert(Point(-12.5, -14.5));
	tree.print();
	REQUIRE(Point(-12.5, -14.5) == tree.search(Point(-12.5, -14.5)).front());

	tree.insert(Point(-7.0, -3.7));
	tree.print();
	REQUIRE(Point(-7.0, -3.7) == tree.search(Point(-7.0, -3.7)).front());

	tree.insert(Point(-10.1, -5.0));
	tree.print();
	REQUIRE(Point(-10.1, -5.0) == tree.search(Point(-10.1, -5.0)).front());

	tree.insert(Point(-12.0, -3.4));
	tree.print();
	REQUIRE(Point(-12.0, -3.4) == tree.search(Point(-12.0, -3.4)).front());
}
