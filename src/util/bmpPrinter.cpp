#include <util/bmpPrinter.h>

BMPPrinter::BMPPrinter(const unsigned xPixels, const unsigned yPixels)
{
	xDimension = xPixels;
	yDimension = yPixels;

	// Seed the rng machine
	long unsigned now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	generator.seed(now);

	// Allocate colourBytes
	colourBytes = new char[xPixels * yPixels * 3];
	memset(colourBytes, 255, xPixels * yPixels * 3);

	// Prepare header
	assert(sizeof(unsigned) == 4);
	assert(sizeof(short unsigned) == 2);
	unsigned computedSize = 0;

	memset(header, 0, 14);
	memset(infoHeader, 0, 40);

	header[0] = 'B', header[1] = 'M'; // Signature

	computedSize = 40 + 14 + xPixels * yPixels * 3;
	memcpy(&header[2], &computedSize, 4); // File size
	
	header[10] = 54; // Offset to data

	// Prepare infoHeader
	infoHeader[0] = 40; // Info header size

	computedSize = xPixels;
	memcpy(&infoHeader[4], &computedSize, 4); // Size of picture horizontally
	computedSize = yPixels;
	memcpy(&infoHeader[8], &computedSize, 4); // Size of picture vertically

	infoHeader[12] = 1; // Number of planes
	infoHeader[14] = 24; // Number of bits per pixel
}

BMPPrinter::~BMPPrinter()
{
	delete [] colourBytes;
}

BMPPrinter::Colour BMPPrinter::bmpColourGenerator()
{
	DPRINT1("bmpColourGenerator");

	std::uniform_int_distribution<unsigned> colourDist(0, 255);

	DPRINT1("bmpColourGenerator finished");

	return {(char) colourDist(generator), (char) colourDist(generator), (char) colourDist(generator)};
}

std::string BMPPrinter::bmpIdGenerator()
{
	DPRINT1("bmpIdGenerator");

	std::uniform_int_distribution<unsigned> symbolDist(0, 15);

	char symbols[] = "0123456789abcdef";
	std::string id(16, '0');

	for (unsigned i = 0; i < id.size(); ++i)
	{
		id[i] = symbols[symbolDist(generator)];
	}

	DPRINT1("bmpIdGenerator finished");

	return id;
}

void BMPPrinter::registerPoint(Point &point, Colour colour)
{
	DPRINT1("registerPoint");

	double xScale = (double) xDimension;
	double yScale = (double) yDimension;

	DPRINT4("scaling x and y by ", xScale, " and ", yScale);

	unsigned xp = (unsigned) round(point[0] * xScale);
	// Rounding may cause the pixel to be outside the array so cap xp at the maximum array index
	xp = std::min(xp, xDimension - 1);

	unsigned yp = (unsigned) round(point[1] * yScale);
	// Rounding may cause the pixel to be outside the array so cap yp at the maximum array index
	yp = std::min(yp, yDimension - 1);

	DPRINT2("uncast point = ", point);
	DPRINT4("cast point = ", xp, ", ", yp);

	DPRINT1("filling in pixels");
	colourBytes[3 * xp + 3 * yp * xDimension + 0] = colour.r;
	colourBytes[3 * xp + 3 * yp * xDimension + 1] = colour.g;
	colourBytes[3 * xp + 3 * yp * xDimension + 2] = colour.b;

	DPRINT1("registerPoint finished");
}

void BMPPrinter::registerRectangle(Rectangle &boundingBox, Colour colour)
{
	DPRINT1("registerRectangle");

	double xScale = (double) xDimension;
	double yScale = (double) yDimension;

	DPRINT4("scaling x and y by ", xScale, " and ", yScale);

	unsigned xLower = (unsigned) round(boundingBox.lowerLeft[0] * xScale);
	unsigned xUpper = (unsigned) round(boundingBox.upperRight[0] * xScale);
	// Rounding may cause the pixel to be outside the array so cap xUpper at the maximum array index
	xUpper = std::min(xUpper, xDimension - 1);

	unsigned yLower = (unsigned) round(boundingBox.lowerLeft[1] * yScale);
	unsigned yUpper = (unsigned) round(boundingBox.upperRight[1] * yScale);
	// Rounding may cause the pixel to be outside the array so cap yUpper at the maximum array index
	yUpper = std::min(yUpper, yDimension - 1);

	DPRINT2("uncast boundingBox = ", boundingBox);
	DPRINT8("cast boundingBox = ", xLower, ", ", yLower, ", ", xUpper, ", ", yUpper);

	DPRINT1("filling in pixels");
	for (unsigned x = xLower; xLower <= x && x <= xUpper; ++x)
	{
		for (unsigned y = yLower; yLower <= y && y <= yUpper; ++y)
		{
			colourBytes[3 * x + 3 * y * xDimension + 0] = colour.r;
			colourBytes[3 * x + 3 * y * xDimension + 1] = colour.g;
			colourBytes[3 * x + 3 * y * xDimension + 2] = colour.b;
		}
	}

	DPRINT1("registerRectangle finished");
}

void BMPPrinter::registerRectangleArray(std::vector<Rectangle> &boundingBoxes)
{
	// TODO
}

void BMPPrinter::registerPolygon(IsotheticPolygon &polygon, Colour colour)
{
	for (unsigned i = 0; i < polygon.basicRectangles.size(); ++i)
	{
		registerRectangle(polygon.basicRectangles[i], colour);
	}
}

void BMPPrinter::finalize(std::string &printId, unsigned level)
{
	DPRINT1("finalize");

	// Ensure there is a directory to print to
	DPRINT1("creating directory");
	std::string printDirectory = "prints/" + printId;
	mkdir(printDirectory.c_str(), 0777);

	// Open the file
	DPRINT1("opening file");
	std::ofstream bmpFile;
	bmpFile.open("prints/" + printId + "/" + printId + "." + (char) (level + 48) + ".bmp");

	// Write headers
	DPRINT1("writing headers");
	bmpFile.write(header, 14);
	bmpFile.write(infoHeader, 40);

	// Write image data
	DPRINT1("writing image data");
	bmpFile.write(colourBytes, xDimension * yDimension * 3);

	// Close the file
	DPRINT1("closing file");
	bmpFile.close();

	DPRINT1("finalize finished");
}

void BMPPrinter::printToBMP(rplustree::Node *root)
{
	DPRINT1("printToBMP");

	// Print directory
	std::string id = bmpIdGenerator();
	DPRINT2("id = ", id);

	// BFS variables
	unsigned currentLevel = 0;
	std::pair<rplustree::Node *, unsigned> currentContext;
	std::queue<std::pair<rplustree::Node *, unsigned>> explorationQ;
	DPRINT1("BFS variables initialized");

	// Prime the Q
	explorationQ.push(std::pair<rplustree::Node *, unsigned>(root, 0));
	DPRINT1("explorationQ primed");

	// Print the tree one level-page at a time
	for (;!explorationQ.empty();)
	{
		DPRINT1("retreiving front of explorationQ");
		currentContext = explorationQ.front();
		explorationQ.pop();

		if (currentContext.second != currentLevel)
		{
			DPRINT2("moved to new level ", currentContext.second);
			// Finalize the previous layer and reset colourBytes
			finalize(id, currentLevel);
			DPRINT1("resetting colour bytes");
			memset(colourBytes, 255, xDimension * yDimension * 3);

			currentLevel++;
		}

		DPRINT3("cycling through ", currentContext.first->branches.size(), " branches");
		// Add all of our children's bounding boxes to this level's image
		for (unsigned i = 0; i < currentContext.first->branches.size(); ++i)
		{
			registerRectangle(currentContext.first->branches[i].boundingBox, bmpColourGenerator());
			explorationQ.push(std::pair<rplustree::Node *, unsigned>(currentContext.first->branches[i].child, currentLevel + 1));
		}

		DPRINT3("cycling through ", currentContext.first->data.size(), " data points");
		for (unsigned i = 0; i < currentContext.first->data.size(); ++i)
		{
			registerPoint(currentContext.first->data[i], {0, 0, 0});
		}
	}

	// Finalize the last level
	finalize(id, currentLevel);

	DPRINT1("printToBMP finished");
}

void BMPPrinter::printToBMP(nirtree::Node *root)
{
	DPRINT1("printToBMP");

	// Print directory
	std::string id = bmpIdGenerator();
	DPRINT2("id = ", id);

	// BFS variables
	unsigned currentLevel = 0;
	std::pair<nirtree::Node *, unsigned> currentContext;
	std::queue<std::pair<nirtree::Node *, unsigned>> explorationQ;
	DPRINT1("BFS variables initialized");

	// Prime the Q
	explorationQ.push(std::pair<nirtree::Node *, unsigned>(root, 0));
	DPRINT1("explorationQ primed");

	// Print the tree one level-page at a time
	for (;!explorationQ.empty();)
	{
		DPRINT1("retreiving front of explorationQ");
		currentContext = explorationQ.front();
		explorationQ.pop();

		if (currentContext.second != currentLevel)
		{
			DPRINT2("moved to new level ", currentContext.second);
			// Finalize the previous layer and reset colourBytes
			finalize(id, currentLevel);
			DPRINT1("resetting colour bytes");
			memset(colourBytes, 255, xDimension * yDimension * 3);

			currentLevel++;
		}

		DPRINT3("cycling through ", currentContext.first->branches.size(), " branches");
		// Add all of our children's bounding boxes to this level's image
		for (unsigned i = 0; i < currentContext.first->branches.size(); ++i)
		{
			registerPolygon(currentContext.first->branches[i].boundingPoly, bmpColourGenerator());
			explorationQ.push(std::pair<nirtree::Node *, unsigned>(currentContext.first->branches[i].child, currentLevel + 1));
		}

		DPRINT3("cycling through ", currentContext.first->data.size(), " data points");
		for (unsigned i = 0; i < currentContext.first->data.size(); ++i)
		{
			registerPoint(currentContext.first->data[i], {0, 0, 0});
		}
	}

	// Finalize the last level
	finalize(id, currentLevel);

	DPRINT1("printToBMP finished");
}
