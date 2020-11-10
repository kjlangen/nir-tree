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

		void registerPoint(Point &point, Colour colour);
		void registerRectangle(Rectangle &boundingBox, Colour colour);
		void registerRectangleArray(std::vector<Rectangle> &boundingBoxes);

		void finalize(std::string &printId, unsigned level=0);

		void printToBMP(rplustree::Node *root);
		void printToBMP(nirtree::Node *root);
};

#endif
