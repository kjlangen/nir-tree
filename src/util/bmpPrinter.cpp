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

bool BMPPrinter::whitePixel(const unsigned x, const unsigned y)
{
	const char white = 255;
	const unsigned index = 3 * x + 3 * y * xDimension;

	return (colourBytes[index + 0] == white) &&	(colourBytes[index + 1] == white) && (colourBytes[index + 2] == white);
}

void BMPPrinter::registerPoint(Point &point, Colour colour)
{
	DPRINT1("registerPoint");

	double xScale = (double) xDimension;
	double yScale = (double) yDimension;

	DPRINT4("scaling x and y by ", xScale, " and ", yScale);

	double xTransform = point[0];
	double yTransform = point[1];

	unsigned xp = (unsigned) round(xTransform * xScale);
	// Rounding may cause the pixel to be outside the array so cap xp at the maximum array index
	xp = std::min(xp, xDimension - 1);

	unsigned yp = (unsigned) round(yTransform * yScale);
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

void BMPPrinter::registerQuadrants(Point &point, Rectangle limits, Colour colour)
{
	DPRINT1("registerQuadtrants");

	double xScale = (double) xDimension;
	double yScale = (double) yDimension;

	// Transform the point
	DPRINT4("scaling x and y by ", xScale, " and ", yScale);

	double xTransform = point[0];
	double yTransform = point[1];

	unsigned xp = (unsigned) round(xTransform * xScale);
	// Rounding may cause the pixel to be outside the array so cap xp at the maximum array index
	xp = std::min(xp, xDimension - 1);

	unsigned yp = (unsigned) round(yTransform * yScale);
	// Rounding may cause the pixel to be outside the array so cap yp at the maximum array index
	yp = std::min(yp, yDimension - 1);

	DPRINT2("uncast point = ", point);
	DPRINT4("cast point = ", xp, ", ", yp);

	// Transform the rectangle
	unsigned limitsLowerX = (unsigned) round(limits.lowerLeft[0] * xScale);
	limitsLowerX = std::min(limitsLowerX, xDimension - 1);
	
	unsigned limitsLowerY = (unsigned) round(limits.lowerLeft[1] * yScale);
	limitsLowerY = std::min(limitsLowerY, yDimension - 1);
	
	unsigned limitsUpperX = (unsigned) round(limits.upperRight[0] * xScale);
	limitsUpperX = std::min(limitsUpperX, xDimension - 1);
	
	unsigned limitsUpperY = (unsigned) round(limits.upperRight[1] * yScale);
	limitsUpperY = std::min(limitsUpperY, yDimension - 1);

	DPRINT1("filling in pixels");

	// Up/Down
	for (unsigned i = limitsLowerY; i <= limitsUpperY; ++i)
	{
		colourBytes[3 * xp + 3 * i * xDimension + 0] = colour.r;
		colourBytes[3 * xp + 3 * i * xDimension + 1] = colour.g;
		colourBytes[3 * xp + 3 * i * xDimension + 2] = colour.b;
	}

	// Left/Right
	for (unsigned i = limitsLowerX; i <= limitsUpperX; ++i)
	{
		colourBytes[3 * i + 3 * yp * xDimension + 0] = colour.r;
		colourBytes[3 * i + 3 * yp * xDimension + 1] = colour.g;
		colourBytes[3 * i + 3 * yp * xDimension + 2] = colour.b;
	}

	DPRINT1("registerQuadtrants finished");
}

void BMPPrinter::registerRectangle(Rectangle &boundingBox, Colour colour)
{
	DPRINT1("registerRectangle");

	double xScale = (double) xDimension;
	double yScale = (double) yDimension;

	DPRINT4("scaling x and y by ", xScale, " and ", yScale);

	double xTransform = boundingBox.lowerLeft[0];
	double yTransform = boundingBox.lowerLeft[1];

	unsigned xLower = (unsigned) round(xTransform * xScale);
	unsigned yLower = (unsigned) round(yTransform * yScale);

	xTransform = boundingBox.upperRight[0];
	yTransform = boundingBox.upperRight[1];

	unsigned xUpper = (unsigned) round(xTransform * xScale);
	// Rounding may cause the pixel to be outside the array so cap xUpper at the maximum array index
	xUpper = std::min(xUpper, xDimension - 1);
	unsigned yUpper = (unsigned) round(yTransform * yScale);
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

void BMPPrinter::printToBMP(rtree::Node *root)
{
	DPRINT1("printToBMP");

	// Print directory
	std::string id = bmpIdGenerator();
	DPRINT2("id = ", id);

	// BFS variables
	unsigned currentLevel = 0;
	std::pair<rtree::Node *, unsigned> currentContext;
	std::queue<std::pair<rtree::Node *, unsigned>> explorationQ;
	DPRINT1("BFS variables initialized");

	// Prime the Q
	explorationQ.push(std::pair<rtree::Node *, unsigned>(root, 0));
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

		DPRINT3("cycling through ", currentContext.first->boundingBoxes.size(), " branches");
		// Add all of our children's bounding boxes to this level's image
		for (unsigned i = 0; i < currentContext.first->boundingBoxes.size(); ++i)
		{
			registerRectangle(currentContext.first->boundingBoxes[i], bmpColourGenerator());
			explorationQ.push(std::pair<rtree::Node *, unsigned>(currentContext.first->children[i], currentLevel + 1));
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

void BMPPrinter::printToBMP(rstartree::Node *root)
{
	DPRINT1("printToBMP");

	// Print directory
	std::string id = bmpIdGenerator();
	DPRINT2("id = ", id);

	// BFS variables
	unsigned currentLevel = 0;
	std::pair<rstartree::Node *, unsigned> currentContext;
	std::queue<std::pair<rstartree::Node *, unsigned>> explorationQ;
	DPRINT1("BFS variables initialized");

	// Prime the Q
	explorationQ.push(std::pair<rstartree::Node *, unsigned>(root, 0));
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

		DPRINT3("cycling through ", currentContext.first->entries.size(), " branches");
		// Add all of our children's bounding boxes to this level's image
		for (unsigned i = 0; i < currentContext.first->entries.size(); ++i)
		{
			if (std::holds_alternative<rstartree::Node::Branch>(currentContext.first->entries[i]))
			{
				registerRectangle(std::get<rstartree::Node::Branch>(currentContext.first->entries[i]).boundingBox, bmpColourGenerator());
				explorationQ.push(std::pair<rstartree::Node *, unsigned>(std::get<rstartree::Node::Branch>(currentContext.first->entries[i]).child, currentLevel + 1));
			}
			else if (std::holds_alternative<Point>(currentContext.first->entries[i]))
			{
				registerPoint(std::get<Point>(currentContext.first->entries[i]), {0, 0, 0});
			}
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

void BMPPrinter::printToBMP(revisedrstartree::Node *root)
{
	DPRINT1("printToBMP");

	// Print directory
	std::string id = bmpIdGenerator();
	DPRINT2("id = ", id);

	// BFS variables
	unsigned currentLevel = 0;
	std::pair<revisedrstartree::Node *, unsigned> currentContext;
	std::queue<std::pair<revisedrstartree::Node *, unsigned>> explorationQ;
	DPRINT1("BFS variables initialized");

	// Prime the Q
	explorationQ.push(std::pair<revisedrstartree::Node *, unsigned>(root, 0));
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
			explorationQ.push(std::pair<revisedrstartree::Node *, unsigned>(currentContext.first->branches[i].child, currentLevel + 1));
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

void BMPPrinter::quadtreeHelper(quadtree::Node *node, Rectangle limits)
{
	registerQuadrants(node->data, limits, bmpColourGenerator());

	// Lower Left
	if (node->branches[0] != nullptr)
	{
		Rectangle subLimits = limits;
		subLimits.upperRight = node->data;
		quadtreeHelper(node->branches[0], subLimits);
	}

	// Upper Left
	if (node->branches[2] != nullptr)
	{
		Rectangle subLimits = limits;
		subLimits.upperRight[0] = node->data[0];
		subLimits.lowerLeft[1] = node->data[1];
		quadtreeHelper(node->branches[2], subLimits);
	}

	// Upper Right
	if (node->branches[3] != nullptr)
	{
		Rectangle subLimits = limits;
		subLimits.lowerLeft = node->data;
		quadtreeHelper(node->branches[3], subLimits);
	}

	// Lower Right
	if (node->branches[1] != nullptr)
	{
		Rectangle subLimits = limits;
		subLimits.upperRight[1] = node->data[1];
		subLimits.lowerLeft[0] = node->data[0];
		quadtreeHelper(node->branches[1], subLimits);
	}
}

void BMPPrinter::printToBMP(quadtree::Node *root)
{
	DPRINT1("printToBMP");

	// Print directory
	std::string id = bmpIdGenerator();
	DPRINT2("id = ", id);

	Rectangle limits;
	limits.lowerLeft[0] = 0;
	limits.lowerLeft[1] = 0;
	limits.upperRight[0] = xDimension - 1;
	limits.upperRight[1] = yDimension - 1;

	quadtreeHelper(root, limits);

	// Finalize the image
	finalize(id, 0);
}
