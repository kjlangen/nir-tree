#include <util/graph.h>

Graph::Graph(const unsigned n)
{
	vertices = n;
	g = new bool[n * n];
	std::memset(g, false, n * n);
	removedVertices = 0;
	gRemoved = new bool[n];
	std::memset(gRemoved, false, n);
}

// TODO: Remove this comment
// Lambda example: [](Point a, Point b){return a.x < b.x;}
// std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
// std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
// std::cout << "Allocate & clear = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

Graph::Graph(std::vector<Rectangle> &v)
{
	// Record the size of the graph
	vertices = v.size();

	// Sort the rectangles for (I think) better intersection performance
	// std::sort(v.begin(), v.end(), [](Rectangle a, Rectangle b){return a.lowerLeft.x < b.lowerLeft.x;});

	// Build the graph
	g = new bool[vertices * vertices];
	std::memset(g, false, vertices * vertices);
	for (unsigned i = 0; i < vertices; ++i)
	{
		for (unsigned j = 0; j < i; ++j)
		{
			if (v[i].intersectsRectangle(v[j]))
			{
				g[i * vertices + j] = true;
				g[j * vertices + i] = true;
			}
		}
	}

	removedVertices = 0;

	gRemoved = new bool[vertices];
	std::memset(gRemoved, false, vertices);
}

Graph::Graph(std::vector<IsotheticPolygon> &v)
{
	removedVertices = 0;

	// Record the size of the graph
	vertices = v.size();

	// Build the graph
	g = new bool[vertices * vertices];
	std::memset(g, false, vertices * vertices);
	for (unsigned i = 0; i < vertices; ++i)
	{
		for (unsigned j = 0; j < i; ++j)
		{
			if (v[i].intersectsPolygon(v[j]))
			{
				g[i * vertices + j] = true;
				g[j * vertices + i] = true;
			}
		}
	}

	removedVertices = 0;

	gRemoved = new bool[vertices];
	std::memset(gRemoved, false, vertices);
}

Graph::~Graph()
{
	delete [] g;
}


bool Graph::contiguous()
{
	// BFS variables
	unsigned root; for (root = 0; gRemoved[root]; ++root){}
	unsigned currentVertex;
	std::queue<unsigned> explorationQ;
	bool explored[vertices];
	std::memset(explored, false, vertices);
	unsigned explorationCount = 0;

	// BFS, counting number of visited vertices which if the corresponding geometry is contigous
	// will be equal to vertices
	explorationQ.push(root);
	for (;explorationQ.size();)
	{
		currentVertex = explorationQ.front();
		explorationQ.pop();

		if (explored[currentVertex])
		{
			continue;
		}

		explored[currentVertex] = true;
		++explorationCount;

		// Go to all of currentVertex's neighbours
		for (unsigned neighbouringVertex = 0; neighbouringVertex < vertices; ++neighbouringVertex)
		{
			if (g[currentVertex * vertices + neighbouringVertex] && !explored[neighbouringVertex])
			{
				explorationQ.push(neighbouringVertex);
			}
		}
	}

	return explorationCount == vertices - removedVertices;
}

bool Graph::contiguous(unsigned skipVertex)
{
	// BFS variables
	unsigned root; for (root = 0; gRemoved[root] || root == skipVertex; ++root){}
	unsigned currentVertex;
	std::queue<unsigned> explorationQ;
	bool explored[vertices];
	std::memset(explored, false, vertices);
	unsigned explorationCount = 0;

	// BFS, counting number of visited vertices which if the corresponding geometry is contigous
	// will be equal to vertices
	explorationQ.push(root);
	for (;explorationQ.size();)
	{
		currentVertex = explorationQ.front();
		explorationQ.pop();

		if (explored[currentVertex])
		{
			continue;
		}

		explored[currentVertex] = true;
		++explorationCount;

		// Go to all of currentVertex's neighbours
		for (unsigned neighbouringVertex = 0; neighbouringVertex < vertices; ++neighbouringVertex)
		{
			if (neighbouringVertex != skipVertex && g[currentVertex * vertices + neighbouringVertex] && !explored[neighbouringVertex])
			{
				explorationQ.push(neighbouringVertex);
			}
		}
	}

	return explorationCount == vertices - 1 - removedVertices;
}

unsigned Graph::components(unsigned *labels)
{
	// BFS variables
	unsigned root;
	unsigned currentVertex;
	bool explored[vertices];
	std::queue<unsigned> explorationQ;

	unsigned componentNumber = 0;
	std::memset(labels, 0, vertices * sizeof(unsigned));
	std::memset(explored, false, vertices);

	// Loop until all components have been labeled
	for (unsigned labelledSoFar = 0; labelledSoFar != vertices - removedVertices;)
	{
		// std::cout << "A" << std::endl;
		// Increase the number of components
		++componentNumber;

		// Find the next unlabeled component
		for (root = 0; labels[root] != 0 || gRemoved[root]; ++root)
		{
			// std::cout << "B" << std::endl;
		}

		// std::cout << "root = " << root << std::endl;

		// Propagate labels to that component
		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			// std::cout << "C" << std::endl;
			currentVertex = explorationQ.front();
			explorationQ.pop();

			if (explored[currentVertex])
			{
				continue;
			}

			explored[currentVertex] = true;
			labels[currentVertex] = componentNumber;
			++labelledSoFar;

			// Go to all of currentVertex's neighbours
			for (unsigned neighbouringVertex = 0; neighbouringVertex < vertices; ++neighbouringVertex)
			{
				if (g[currentVertex * vertices + neighbouringVertex] && !explored[neighbouringVertex])
				{
					// std::cout << "Pushing " << neighbouringVertex << " from " << currentVertex << std::endl;
					explorationQ.push(neighbouringVertex);
				}
			}

			// std::cout << "Node " << currentVertex << " finished" << std::endl;
		}
	}

	return componentNumber;
}

void Graph::remove(unsigned givenVertex)
{
	std::memset(&g[givenVertex * vertices], false, vertices);
	for (unsigned i = 0; i < vertices; ++i)
	{
		g[i * vertices + givenVertex] = false;
	}

	++removedVertices;
	gRemoved[givenVertex] = true;
}

bool *Graph::operator[](unsigned i)
{
	return &g[i * vertices];
}
