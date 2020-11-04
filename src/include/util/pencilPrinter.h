#ifndef __PENCILPRINTER__
#define __PENCILPRINTER__

#include <string>
#include <random>
#include <stack>
#include <queue>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctemplate/template.h>
#include <nirtree/node.h>
#include <rplustree/node.h>

class PencilPrinter
{
	private:
		std::default_random_engine generator;
		const unsigned WIDTHDEFAULT = 10000;
		const unsigned HEIGHTDEFAULT = 10000;
		const unsigned COLOURDEFAULT = 0x0000ff;
	
	public:
		PencilPrinter();
		~PencilPrinter();

		// Generators
		std::string pencilColourGenerator();
		std::string pencilIdGenerator();

		// Template fillers
		void fillPencilPoint(ctemplate::TemplateDictionary* currentPage, Point &p);
		void fillPencilRectangle(ctemplate::TemplateDictionary* currentPage, Rectangle &r, std::string &polygonColour, unsigned rectangleId=0);
		void fillPencilPage(ctemplate::TemplateDictionary* currentPage, std::string &pageId, unsigned currentLevel);
		void fillPencilDocument(ctemplate::TemplateDictionary *document, std::string &pageId);
		void fillPencilDocument();

		// Template finalizers
		void finalizePencilPage(ctemplate::TemplateDictionary *currentPage, std::string &pageId, std::string &printDirectory);
		void finalizePencilDocument(ctemplate::TemplateDictionary *document, std::string &printDirectory);

		// Printers
		void printToPencil(std::vector<Point> &points);
		void printToPencil(std::vector<Rectangle> &rectangles);
		void printToPencil(std::vector<IsotheticPolygon> &polygons);
		void printToPencil(nirtree::Node *root);
		void printToPencil(rplustree::Node *root);
};

void testPencilIdGeneration();
void testSimplePrintToPencil();
void testPointsPrintToPencil();
void testRectanglesPrintToPencil();
void testPolygonsPrintToPencil();
void testMultiLevelPrintToPencil();
void testComplexGeometryPrintToPencil();

#endif
