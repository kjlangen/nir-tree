#ifndef __BMPPRINTER__
#define __BMPPRINTER__

#include <cmath>
#include <string>
#include <random>
#include <chrono>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <util/debug.h>
#include <util/geometry.h>
#include <nirtree/node.h>
#include <rplustree/node.h>
#include <rtree/node.h>
#include <rstartree/node.h>
#include <revisedrstartree/node.h>
#include <quadtree/node.h>

class BMPPrinter
{
	char header[14];
	char infoHeader[40];
	char *colourBytes;

	unsigned xDimension, yDimension;

	std::default_random_engine generator;

	public:
		struct Colour
		{
			char r, g, b;
		};

		BMPPrinter(const unsigned xPixels, const unsigned yPixels);
		~BMPPrinter();

		Colour bmpColourGenerator();
		std::string bmpIdGenerator();

		bool whitePixel(const unsigned x, const unsigned y);
		void registerPoint(Point &point, Colour colour);
		void registerQuadrants(Point &point, Rectangle limits, Colour colour);
		void registerRectangle(Rectangle &boundingBox, Colour colour);
		void registerRectangleArray(std::vector<Rectangle> &boundingBoxes);
		void registerPolygon(IsotheticPolygon &polygon, Colour colour);

		void finalize(std::string &printId, unsigned level=0);

		void quadtreeHelper(quadtree::Node *node, Rectangle limits);

		void printToBMP(rtree::Node *root);
		void printToBMP(rplustree::Node *root);
		void printToBMP(rstartree::Node *root);
		void printToBMP(revisedrstartree::Node *root);
		void printToBMP(nirtree::Node *root);
		void printToBMP(quadtree::Node *root);
};

#endif
