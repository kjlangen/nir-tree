#ifndef __NIRTREE__
#define __NIRTREE__
#include <cassert>
#include <vector>
#include <stack>
#include <queue>
#include <string>
#include <random>
#include <iostream>
#include <fstream>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctemplate/template.h>
#include <util/geometry.h>
#include <nirtree/node.h>

namespace nirtree
{
	// For now we will work with a NIRTree that only stores points
	class NIRTree
	{
		private:
			std::default_random_engine generator;
			const unsigned WIDTHDEFAULT = 10000;
			const unsigned HEIGHTDEFAULT = 10000;
			const unsigned COLOURDEFAULT = 0x0000ff;

		public:
			Node *root;

			// Constructors and destructors
			NIRTree(unsigned minBranchFactor, unsigned maxBranchFactor);
			NIRTree(Node *root);
			~NIRTree();

			// Datastructure interface
			std::vector<Point> exhaustiveSearch(Point requestedPoint);
			std::vector<Point> search(Point requestedPoint);
			std::vector<Point> search(Rectangle requestedRectangle);
			void insert(Point givenPoint);
			// void remove(Point givenPoint);

			// Miscellaneous
			unsigned checksum();
			void print();

			// Pencil Printing
			std::string pencilColourGenerator();
			std::string pencilIdGenerator();
			void fillPencilRectangle(ctemplate::TemplateDictionary* currentPage, Rectangle &r, std::string &polygonColour);
			void fillPencilPage(ctemplate::TemplateDictionary* currentPage, std::string &pageId, unsigned currentLevel);
			void fillPencilDocument(ctemplate::TemplateDictionary *document, std::string &pageId);
			void fillPencilDocument();
			void finalizePencilPage(ctemplate::TemplateDictionary *currentPage, std::string &pageId, std::string &printDirectory);
			void finalizePencilDocument(ctemplate::TemplateDictionary *document, std::string &printDirectory);
			void printToPencil();
	};

	void testSimpleSearch();
	void testSimpleInsert();
	void expandRootTest();
	void testPencilIdGeneration();
	void testSimplePrintToPencil();
	void testMultiLevelPrintToPencil();
	void testComplexGeometryPrintToPencil();
}

#endif
