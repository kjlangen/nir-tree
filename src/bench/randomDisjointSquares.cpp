#include <bench/randomDisjointSquares.h>

// To generate synthetic 3D datasets we distribute spatial boxes with each side of uniform
// random length (between 0 and 1) in a constant space of 1000 space units in each of the three
// dimensions. We use three different distributions, namely uniform, Gaussian (μ = 500, σ = 250)
// and clustered to generate datasets containing from 160K to 960K objects and from
// 1.6 to 9.6 million objects. The clustered distribution uniformly randomly chooses up to 100
// locations in 3D space around which the objects are distributed with a
// Gaussian distribution (μ = 0, σ = 220).

void randomDisjointSquares()
{
	// Setup random generators
	std::default_random_engine generator;
	std::uniform_int_distribution<unsigned> rectDist(0, 1);
	std::cout << "Uniformly distributing squares between positions (0,0) and (999, 999)." << std::endl;

	// Setup statistics
	std::chrono::duration<double> deltaNIR;
	std::chrono::duration<double> deltaR;

	// Setup NIR-Tree node
	nirtree::Node *r = new nirtree::Node(3, 3);
	nirtree::Node *n = new nirtree::Node(0, 1000000, r);
	r->boundingBoxes.push_back(IsotheticPolygon(Rectangle(0.0, 0.0, 1010.0, 1010.0)));
	r->children.push_back(n);

	// Setup R-Tree node
	rtree::Node *rr = new rtree::Node(3, 3);
	rtree::Node *nn = new rtree::Node(0, 1000000, rr);
	rr->boundingBoxes.push_back(Rectangle(0.0, 0.0, 900.0, 900.0));
	rr->children.push_back(nn);

	// Initialize squares
	std::cout << "Beginning initialization of squares..." << std::endl;
	for (unsigned i = 0; i < 40; ++i)
	{
		for (unsigned j = 0; j < 40; ++j)
		{
			if (i != 20 && j != 20 && rectDist(generator))
			{
				float llx = (float)(i * 10);
				float lly = (float)(j * 10);
				float urx = llx + 10.0;
				float ury = lly + 10.0;

				nirtree::Node *c = new nirtree::Node(3, 3);
				n->boundingBoxes.push_back(IsotheticPolygon(Rectangle(llx, lly, urx, ury)));
				n->children.push_back(c);

				rtree::Node *cc = new rtree::Node(3, 3);
				nn->boundingBoxes.push_back(Rectangle(llx, lly, urx, ury));
				nn->children.push_back(cc);
			}
		}
	}
	std::cout << "Finished initialization of squares." << std::endl;

	nirtree::PencilPrinter p;
	p.printToPencil(n);

	nirtree::Node *f = new nirtree::Node(3, 3);
	rtree::Node *ff = new rtree::Node(3, 3);
	ff->data.push_back(Point(200.0, 200.0));
	ff->data.push_back(Point(210.0, 210.0));

	std::cout << "Beginning split of NIR node..." << std::endl;
	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
	// n->splitNode(f, IsotheticPolygon(Rectangle(200.0, 200.0, 210.0, 210.0)));
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	deltaNIR = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
	std::cout << "Finished split of NIR node." << std::endl;

	std::cout << "Beginning split of R node..." << std::endl;
	begin = std::chrono::high_resolution_clock::now();
	nn->splitNode(ff);
	end = std::chrono::high_resolution_clock::now();
	deltaR = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
	std::cout << "Finished split of R node." << std::endl;

	// Statistics
	std::cout << "Time to split NIR: " << deltaNIR.count() << "s" << std::endl;
	std::cout << "Time to split R: " << deltaR.count() << "s" << std::endl;
}
