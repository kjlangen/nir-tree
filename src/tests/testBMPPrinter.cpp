#include <catch2/catch.hpp>
#include <util/bmpPrinter.h>
#include <nirtree/nirtree.h>

TEST_CASE("BMPPrinter: testPrintSingleRectangle")
{
	BMPPrinter p(1920, 1080);

	Rectangle r(0.1, 0.1, 0.4, 0.4);
	Rectangle s(0.5, 0.5, 0.6, 0.6);

	p.registerRectangle(r, p.bmpColourGenerator());
	p.registerRectangle(s, p.bmpColourGenerator());

	std::string printId = p.bmpIdGenerator();

	p.finalize(printId, 1);
}

TEST_CASE("BMPPrinter: testPrintTree")
{
    nirtree::NIRTree tree(25,50);
	nirtree::Node *root = tree.root;
	nirtree::Node *branchA = new nirtree::Node(tree, 25, 50, root);
	nirtree::Node *branchB = new nirtree::Node(tree, 25, 50, root);

	root->branches.push_back({branchA, IsotheticPolygon(Rectangle(0.1, 0.1, 0.4, 0.4))});
	root->branches.push_back({branchB, IsotheticPolygon(Rectangle(0.5, 0.5, 0.6, 0.6))});

	BMPPrinter p(1920, 1080);

	p.printToBMP(root);
}
