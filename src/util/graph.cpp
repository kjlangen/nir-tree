#include <util/graph.h>

// TODO: Remove this comment
// Lambda example: [](Point a, Point b){return a.x < b.x;}
// std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
// std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
// std::cout << "Allocate & clear = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

Graph::Graph(const unsigned n)
{
	// Allocate the graph
	vertices = n;
	g = new bool[n * n];
	std::memset(g, false, n * n);

	// Allocate the set of removed vertices
	removedVertices = 0;
	gRemoved = new bool[n];
	std::memset(gRemoved, false, n);
}

Graph::Graph(std::vector<Rectangle> &v)
{
	// Record the size of the graph
	vertices = v.size();

	// Allocate the graph
	g = new bool[vertices * vertices];
	std::memset(g, false, vertices * vertices);

	// Place all possible schedule points in a list
	SchedulePoint schedule[vertices * 2];
	for (unsigned i = 0; i < vertices; ++i)
	{
		schedule[i].coord = v[i].lowerLeft.x;
		schedule[i].index = i;
		schedule[i].start = true;

		schedule[vertices * 2 - i - 1].coord = v[i].upperRight.x;
		schedule[vertices * 2 - i - 1].index = i;
		schedule[vertices * 2 - i - 1].start = false;
	}

	// Sort to create a sweep schedule
	std::sort(schedule, &schedule[vertices * 2 - 1], [](SchedulePoint a, SchedulePoint b){return a.coord == b.coord ? a.start && !b.start : a.coord < b.coord;});

	// Allocate space to record which rectangles are "active" at the current SchedulePoint
	unsigned active[vertices];
	unsigned activeEnd = 0;

	// Sweep
	for (unsigned i = 0; i < vertices * 2; ++i)
	{
		// "Active" a rectangle if we have hit it's lower left corner. "Deactivate" a rectangle if
		// we have hit it's upper right corner
		if (schedule[i].start)
		{
			// Check if the newly "active" rectangle intersects any other "active" rectangles
			for (unsigned j = 0; j < activeEnd; ++j)
			{
				if (v[schedule[i].index].intersectsRectangle(v[j]))
				{
					g[schedule[i].index * vertices + j] = true;
					g[j * vertices + schedule[i].index] = true;
				}
			}

			// Activation
			active[activeEnd] = schedule[i].index;
			++activeEnd;
		}
		else
		{
			for (unsigned j = 0; j < activeEnd; ++j)
			{
				// Find the rectangle in the active set
				if (active[j] == schedule[i].index)
				{
					// Deactivation
					active[j] = active[activeEnd - 1];
					--activeEnd;
					break;
				}
			}
		}
	}

	// Allocate the set of removed vertices
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
		DPRINT1("A");
		// Increase the number of components
		++componentNumber;

		// Find the next unlabeled component
		for (root = 0; labels[root] != 0 || gRemoved[root]; ++root) { DPRINT1("B"); }

		DPRINT2("root = ", root);

		// Propagate labels to that component
		explorationQ.push(root);
		for (;explorationQ.size();)
		{
			DPRINT1("C");
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
					DPRINT4("Pushing ", neighbouringVertex, " from ", currentVertex);
					explorationQ.push(neighbouringVertex);
				}
			}

			DPRINT3("Node ", currentVertex, " finished");
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
