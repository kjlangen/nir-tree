#include<nirtree/pencilPrinter.h>

namespace nirtree
{
	PencilPrinter::PencilPrinter()
	{
		long unsigned now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		generator.seed(now);
	}

	PencilPrinter::~PencilPrinter()
	{
	}

	std::string PencilPrinter::pencilColourGenerator()
	{
		std::uniform_int_distribution<unsigned> symbolDist(0, 15);

		char symbols[] = "0123456789ABCDEF";
		std::string colour(6, '0');

		for (unsigned i = 0; i < colour.size(); ++i)
		{
			colour[i] = symbols[symbolDist(generator)];
		}

		return colour;
	}

	std::string PencilPrinter::pencilIdGenerator()
	{
		std::uniform_int_distribution<unsigned> symbolDist(0, 15);

		char symbols[] = "0123456789abcdef";
		std::string id(32, '0');

		for (unsigned i = 0; i < id.size(); ++i)
		{
			id[i] = symbols[symbolDist(generator)];
		}

		return id;
	}

	void PencilPrinter::fillPencilRectangle(ctemplate::TemplateDictionary* currentPage, Rectangle &r, std::string &polygonColour)
	{
		ctemplate::TemplateDictionary *currentRectangle = currentPage->AddSectionDictionary("RECTANGLES");
		currentRectangle->SetValue("SHAPEID", pencilIdGenerator());
		currentRectangle->SetFormattedValue("LOWERLEFTX", "%u", (unsigned)r.lowerLeft.x);
		currentRectangle->SetFormattedValue("LOWERLEFTY", "%u", (unsigned)r.lowerLeft.y);
		currentRectangle->SetFormattedValue("WIDTH", "%u", (unsigned)(r.upperRight.x - r.lowerLeft.x));
		currentRectangle->SetFormattedValue("HEIGHT", "%u", (unsigned)(r.upperRight.y - r.lowerLeft.y));
		currentRectangle->SetValue("FILLCOLOUR", polygonColour);
		currentRectangle->SetValue("TEXTCONTENT", "");
		currentRectangle->SetValue("RECTANGLEID", pencilIdGenerator());
		currentRectangle->SetValue("FILTERID", pencilIdGenerator());
		currentRectangle->SetValue("TEXTID", pencilIdGenerator());
		currentRectangle->SetValue("MISCID", pencilIdGenerator());
	}

	void PencilPrinter::fillPencilPage(ctemplate::TemplateDictionary* currentPage, std::string &pageId, unsigned currentLevel)
	{
		currentPage->SetValue("PAGEID", pageId);
		currentPage->SetFormattedValue("PAGELEVEL", "%u", currentLevel);
		currentPage->SetFormattedValue("PAGEWIDTH", "%u", WIDTHDEFAULT);
		currentPage->SetFormattedValue("PAGEHEIGHT", "%u", HEIGHTDEFAULT);
	}

	void PencilPrinter::fillPencilDocument(ctemplate::TemplateDictionary *document, std::string &pageId)
	{
		document->SetValue("ACTIVEID", pageId);
		document->AddSectionDictionary("PAGES")->SetValue("PAGEID", pageId);
	}

	void PencilPrinter::finalizePencilPage(ctemplate::TemplateDictionary *currentPage, std::string &pageId, std::string &printDirectory)
	{
		// Make the page template into a string
		std::string page;
		ctemplate::ExpandTemplate("src/templates/page.tpl", ctemplate::DO_NOT_STRIP, currentPage, &page);

		// Dump the string to a file
		std::ofstream pageFile;
		pageFile.open(printDirectory + "/page_" + pageId + ".xml");
		pageFile << page << std::endl;
		pageFile.close();
	}

	void PencilPrinter::finalizePencilDocument(ctemplate::TemplateDictionary *document, std::string &printDirectory)
	{
		// Make the document template into a string
		std::string doc;
		ctemplate::ExpandTemplate("src/templates/document.tpl", ctemplate::DO_NOT_STRIP, document, &doc);

		// Dump the string to a file
		std::ofstream documentFile;
		documentFile.open(printDirectory + "/content.xml");
		documentFile << doc << std::endl;
		documentFile.close();

		// Record print id
		std::cout << "Print Id: " <<  printDirectory << std::endl;
	}

	void PencilPrinter::printToPencil(std::vector<Rectangle> &rectangles)
	{
		// Print directory
		std::string printDirectory = "prints/" + pencilIdGenerator();
		mkdir(printDirectory.c_str(), 0777);

		// File templates
		ctemplate::TemplateDictionary *currentPage = new ctemplate::TemplateDictionary("page");
		ctemplate::TemplateDictionary document("document");

		// Template data
		std::string pageId = pencilIdGenerator();

		// Print the tree one level-page at a time
		for (unsigned i = 0; i < rectangles.size(); ++i)
		{
			std::string rectangleColour = pencilColourGenerator();
			fillPencilRectangle(currentPage, rectangles[i], rectangleColour);
		}

		fillPencilPage(currentPage, pageId, 0);

		// Make the page a file
		finalizePencilPage(currentPage, pageId, printDirectory);

		// Record this page in the document
		fillPencilDocument(&document, pageId);

		// Finish the document
		finalizePencilDocument(&document, printDirectory);
	}

	void PencilPrinter::printToPencil(std::vector<IsotheticPolygon> &polygons)
	{
		// Print directory
		std::string printDirectory = "prints/" + pencilIdGenerator();
		mkdir(printDirectory.c_str(), 0777);

		// File templates
		ctemplate::TemplateDictionary *currentPage = new ctemplate::TemplateDictionary("page");
		ctemplate::TemplateDictionary document("document");

		// Template data
		std::string pageId = pencilIdGenerator();

		// Print the tree one level-page at a time
		for (unsigned i = 0; i < polygons.size(); ++i)
		{
			std::string polygonColour = pencilColourGenerator();

			for (unsigned j = 0; j < polygons[i].basicRectangles.size(); ++j)
			{
				fillPencilRectangle(currentPage, polygons[i].basicRectangles[j], polygonColour);
			}
		}

		fillPencilPage(currentPage, pageId, 0);

		// Make the page a file
		finalizePencilPage(currentPage, pageId, printDirectory);

		// Record this page in the document
		fillPencilDocument(&document, pageId);

		// Finish the document
		finalizePencilDocument(&document, printDirectory);
	}

	void PencilPrinter::printToPencil(Node *root)
	{
		// Print directory
		std::string printDirectory = "prints/" + pencilIdGenerator();
		mkdir(printDirectory.c_str(), 0777);

		// File templates
		ctemplate::TemplateDictionary *currentPage = new ctemplate::TemplateDictionary("page");
		ctemplate::TemplateDictionary document("document");

		// Template data
		std::string pageId;

		// BFS variables
		unsigned currentLevel = 0;
		std::pair<Node *, unsigned> currentContext;
		std::queue<std::pair<Node *, unsigned>> explorationQ;

		// Prime the Q
		explorationQ.push(std::pair<Node *, unsigned>(root, 0));

		// Print the tree one level-page at a time
		for (;explorationQ.size();)
		{
			currentContext = explorationQ.front();

			if (currentContext.second != currentLevel)
			{
				// Fill in the page
				pageId = pencilIdGenerator();
				fillPencilPage(currentPage, pageId, currentLevel);

				// Make the page a file
				finalizePencilPage(currentPage, pageId, printDirectory);

				// Record this page in the document
				fillPencilDocument(&document, pageId);

				// Setup new level-page
				delete currentPage;
				currentPage = new ctemplate::TemplateDictionary("page");
				currentLevel++;
			}

			// Add all of our children's isothetic bounding polygons to the level-page
			for (unsigned i = 0; i < currentContext.first->boundingBoxes.size(); ++i)
			{
				std::string boundingBoxColour = pencilColourGenerator();

				for (unsigned j = 0; j < currentContext.first->boundingBoxes[i].basicRectangles.size(); ++j)
				{
					fillPencilRectangle(currentPage, currentContext.first->boundingBoxes[i].basicRectangles[j], boundingBoxColour);
				}
			}

			for (unsigned i = 0; i < currentContext.first->children.size(); ++i)
			{
				explorationQ.push(std::pair<Node *, unsigned>(currentContext.first->children[i], currentLevel + 1));
			}

			explorationQ.pop();
		}

		// Finish the last level-page representing the leaves
		pageId = pencilIdGenerator();
		fillPencilPage(currentPage, pageId, currentLevel);

		// Make the page a file
		finalizePencilPage(currentPage, pageId, printDirectory);

		// Record this page in the document
		fillPencilDocument(&document, pageId);

		// Finish the document
		finalizePencilDocument(&document, printDirectory);
	}

	void testPencilIdGeneration()
	{
		// This only tests n out of n^2 pairs but still provides a reasonably high level
		// of confidence that duplicate id's are not being generated.
		PencilPrinter printer;

		for (int i = 0; i < 4000; ++i)
		{
			assert(printer.pencilIdGenerator() != printer.pencilIdGenerator());
		}
	}

	void testSimplePrintToPencil()
	{
		Node *rootLeaf = new Node(4, 4);
		rootLeaf->boundingBoxes.push_back(IsotheticPolygon(Rectangle(20.0, 20.0, 27.0, 27.0)));
		rootLeaf->boundingBoxes.push_back(IsotheticPolygon(Rectangle(40.0, 20.0, 47.0, 27.0)));
		rootLeaf->boundingBoxes.push_back(IsotheticPolygon(Rectangle(20.0, 40.0, 27.0, 47.0)));
		rootLeaf->boundingBoxes.push_back(IsotheticPolygon(Rectangle(40.0, 40.0, 47.0, 47.0)));

		PencilPrinter printer;
		printer.printToPencil(rootLeaf);
	}

	void testRectanglesPrintToPencil()
	{
		std::vector<Rectangle> rectangles;
		rectangles.push_back(Rectangle(0.0, 30.0, 20.0, 70.0));
		rectangles.push_back(Rectangle(20.0, 60.0, 30.0, 70.0));
		rectangles.push_back(Rectangle(20.0, 50.0, 40.0, 60.0));
		rectangles.push_back(Rectangle(30.0, 40.0, 60.0, 50.0));
		rectangles.push_back(Rectangle(30.0, 10.0, 40.0, 40.0));
		rectangles.push_back(Rectangle(40.0, 20.0, 50.0, 30.0));
		rectangles.push_back(Rectangle(20.0, 0.0, 60.0, 10.0));
		rectangles.push_back(Rectangle(70.0, 40.0, 80.0, 70.0));

		PencilPrinter printer;
		printer.printToPencil(rectangles);
	}

	void testPolygonsPrintToPencil()
	{
		std::vector<IsotheticPolygon> polygons;
		
		IsotheticPolygon p0;
		p0.basicRectangles.push_back(Rectangle(100.0, 600.0, 400.0, 700.0));
		p0.basicRectangles.push_back(Rectangle(300.0, 700.0, 400.0, 900.0));
		polygons.push_back(p0);

		IsotheticPolygon p1;
		p1.basicRectangles.push_back(Rectangle(500.0, 500.0, 600.0, 800.0));
		p1.basicRectangles.push_back(Rectangle(600.0, 400.0, 800.0, 600.0));
		polygons.push_back(p1);

		IsotheticPolygon p2;
		p2.basicRectangles.push_back(Rectangle(1200.0, 900.0, 1300.0, 1100.0));
		p2.basicRectangles.push_back(Rectangle(1300.0, 800.0, 1400.0, 1000.0));
		polygons.push_back(p2);

		IsotheticPolygon p3;
		p3.basicRectangles.push_back(Rectangle(1200.0, 600.0, 1600.0, 700.0));
		p3.basicRectangles.push_back(Rectangle(1400.0, 700.0, 1500.0, 800.0));
		p3.basicRectangles.push_back(Rectangle(1400.0, 500.0, 1500.0, 600.0));
		polygons.push_back(p3);

		PencilPrinter printer;
		printer.printToPencil(polygons);
	}

	void testMultiLevelPrintToPencil()
	{
		Node *quad0 = new Node(4, 4);
		quad0->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 150.0, 150.0)));
		quad0->children.push_back(new Node(4, 4));
		quad0->boundingBoxes.push_back(IsotheticPolygon(Rectangle(150.0, 0.0, 300.0, 150.0)));
		quad0->children.push_back(new Node(4, 4));
		quad0->boundingBoxes.push_back(IsotheticPolygon(Rectangle(150.0, 150.0, 300.0, 300.0)));
		quad0->children.push_back(new Node(4, 4));
		quad0->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 150.0, 150.0, 300.0)));
		quad0->children.push_back(new Node(4, 4));

		Node *quad1 = new Node(4, 4);
		quad1->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 0.0, 550.0, 150.0)));
		quad1->children.push_back(new Node(4, 4));
		quad1->boundingBoxes.push_back(IsotheticPolygon(Rectangle(550.0, 0.0, 700.0, 150.0)));
		quad1->children.push_back(new Node(4, 4));
		quad1->boundingBoxes.push_back(IsotheticPolygon(Rectangle(550.0, 150.0, 700.0, 300.0)));
		quad1->children.push_back(new Node(4, 4));
		quad1->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 150.0, 550.0, 300.0)));
		quad1->children.push_back(new Node(4, 4));

		Node *quad2 = new Node(4, 4);
		quad2->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 400.0, 550.0, 550.0)));
		quad2->children.push_back(new Node(4, 4));
		quad2->boundingBoxes.push_back(IsotheticPolygon(Rectangle(550.0, 400.0, 700.0, 550.0)));
		quad2->children.push_back(new Node(4, 4));
		quad2->boundingBoxes.push_back(IsotheticPolygon(Rectangle(550.0, 550.0, 700.0, 700.0)));
		quad2->children.push_back(new Node(4, 4));
		quad2->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 550.0, 550.0, 700.0)));
		quad2->children.push_back(new Node(4, 4));

		Node *quad3 = new Node(4, 4);
		quad3->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 400.0, 150.0, 550.0)));
		quad3->children.push_back(new Node(4, 4));
		quad3->boundingBoxes.push_back(IsotheticPolygon(Rectangle(150.0, 400.0, 300.0, 550.0)));
		quad3->children.push_back(new Node(4, 4));
		quad3->boundingBoxes.push_back(IsotheticPolygon(Rectangle(150.0, 550.0, 300.0, 700.0)));
		quad3->children.push_back(new Node(4, 4));
		quad3->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 550.0, 150.0, 700.0)));
		quad3->children.push_back(new Node(4, 4));

		Node *root = new Node(4, 4);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 300.0, 300.0)));
		root->children.push_back(quad0);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 0.0, 700.0, 300.0)));
		root->children.push_back(quad1);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(400.0, 400.0, 700.0, 700.0)));
		root->children.push_back(quad2);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 400.0, 300.0, 700.0)));
		root->children.push_back(quad3);

		PencilPrinter printer;
		printer.printToPencil(root);
	}

	void testComplexGeometryPrintToPencil()
	{
		Node *n0 = new Node(2, 2);
		n0->boundingBoxes.push_back(IsotheticPolygon());
		n0->boundingBoxes[0].basicRectangles.push_back(Rectangle(100.0, 120.0, 170.0, 150.0));
		n0->boundingBoxes.push_back(IsotheticPolygon());
		n0->boundingBoxes[1].basicRectangles.push_back(Rectangle(150.0, 100.0, 170.0, 120.0));
		n0->boundingBoxes[1].basicRectangles.push_back(Rectangle(170.0, 100.0, 180.0, 170.0));
		n0->boundingBoxes.push_back(IsotheticPolygon());
		n0->boundingBoxes[2].basicRectangles.push_back(Rectangle(150.0, 170.0, 180.0, 190.0));
		n0->boundingBoxes[2].basicRectangles.push_back(Rectangle(160.0, 150.0, 170.0, 170.0));

		Node *n1 = new Node(2, 2);
		n1->boundingBoxes.push_back(IsotheticPolygon());
		n1->boundingBoxes[0].basicRectangles.push_back(Rectangle(0.0, 30.0, 20.0, 70.0));
		n1->boundingBoxes[0].basicRectangles.push_back(Rectangle(20.0, 60.0, 30.0, 70.0));
		n1->boundingBoxes.push_back(IsotheticPolygon());
		n1->boundingBoxes[1].basicRectangles.push_back(Rectangle(20.0, 50.0, 40.0, 60.0));
		n1->boundingBoxes[1].basicRectangles.push_back(Rectangle(30.0, 40.0, 60.0, 50.0));
		n1->boundingBoxes.push_back(IsotheticPolygon());
		n1->boundingBoxes[2].basicRectangles.push_back(Rectangle(30.0, 10.0, 40.0, 40.0));
		n1->boundingBoxes[2].basicRectangles.push_back(Rectangle(40.0, 20.0, 50.0, 30.0));
		n1->boundingBoxes.push_back(IsotheticPolygon());
		n1->boundingBoxes[3].basicRectangles.push_back(Rectangle(20.0, 0.0, 60.0, 10.0));
		n1->boundingBoxes.push_back(IsotheticPolygon());
		n1->boundingBoxes[4].basicRectangles.push_back(Rectangle(70.0, 40.0, 80.0, 70.0));

		Node *n2 = new Node(2, 2);
		n2->boundingBoxes.push_back(IsotheticPolygon());
		n2->boundingBoxes[0].basicRectangles.push_back(Rectangle(150.0, 100.0, 180.0, 120.0));
		n2->boundingBoxes[0].basicRectangles.push_back(Rectangle(100.0, 120.0, 180.0, 190.0));
		n2->children.push_back(n0);
		n2->boundingBoxes.push_back(IsotheticPolygon());
		n2->boundingBoxes[1].basicRectangles.push_back(Rectangle(20.0, 0.0, 60.0, 30.0));
		n2->boundingBoxes[1].basicRectangles.push_back(Rectangle(0.0, 30.0, 80.0, 70.0));
		n2->children.push_back(n1);

		Node *root = new Node(2, 2);
		root->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 180.0, 190.0)));
		root->children.push_back(n2);

		PencilPrinter printer;
		printer.printToPencil(root);
	}
}
