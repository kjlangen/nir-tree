#include <util/graph.h>

// TODO: Remove this comment
// Lambda example: [](Point a, Point b){return a.x < b.x;}
// std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
// std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
// std::cout << "Allocate & clear = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

Graph::Graph(const unsigned n)
{
	// Make the appropriately sized graph
	vertices = n;
	g = new std::vector<unsigned>[vertices];
	explored = new bool[vertices];
}

Graph::Graph(std::vector<Rectangle> &v)
{
	// Make the appropriately sized graph
	vertices = v.size();
	g = new std::vector<unsigned>[vertices];
	explored = new bool[vertices];

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
	std::unordered_set<unsigned> activeIndices(vertices);

	// Sweep
	for (unsigned i = 0; i < vertices * 2; ++i)
	{
		// "Active" a rectangle if we have hit it's lower left corner. "Deactivate" a rectangle if
		// we have hit it's upper right corner
		if (schedule[i].start)
		{
			DPRINT3(schedule[i].index, "\tstart\t", schedule[i].coord);
			// Check if the newly "active" rectangle intersects any other "active" rectangles
			for (unsigned index : activeIndices)
			{
				if (v[schedule[i].index].intersectsRectangle(v[index]))
				{
					DPRINT6("inserting edge schedule[", i, "].index = ", schedule[i].index, " ----- activeIndex = ", index);
					g[schedule[i].index].push_back(index);
					g[index].push_back(schedule[i].index);
				}
			}

			// Activation
			activeIndices.insert(schedule[i].index);
		}
		else
		{
			DPRINT3(schedule[i].index, "\tend\t", schedule[i].coord);
			activeIndices.erase(schedule[i].index);
		}
	}
}

Graph::Graph(std::vector<IsotheticPolygon> &p)
{
	// Record the size of the graph
	vertices = p.size();
	g = new std::vector<unsigned>[vertices];
	explored = new bool[vertices];

	DPRINT2("initialization of vertices = ", vertices);

	// Place all possible schedule points in a list
	SchedulePoint schedule[vertices * 2];
	for (unsigned i = 0; i < vertices; ++i)
	{
		Rectangle pBB = p[i].boundingBox();
		schedule[i].coord = pBB.lowerLeft.x;
		schedule[i].index = i;
		schedule[i].start = true;

		schedule[vertices * 2 - i - 1].coord = pBB.upperRight.x;
		schedule[vertices * 2 - i - 1].index = i;
		schedule[vertices * 2 - i - 1].start = false;
	}

	// Sort to create a sweep schedule
	std::sort(schedule, &schedule[vertices * 2 - 1], [](SchedulePoint a, SchedulePoint b){return a.coord == b.coord ? a.start && !b.start : a.coord < b.coord;});

	// Allocate space to record which rectangles are "active" at the current SchedulePoint
	std::unordered_set<unsigned> activeIndices(vertices);

	// Sweep
	for (unsigned i = 0; i < vertices * 2; ++i)
	{
		// "Active" a rectangle if we have hit it's lower left corner. "Deactivate" a rectangle if
		// we have hit it's upper right corner
		if (schedule[i].start)
		{
			DPRINT3(schedule[i].index, "\tstart\t", schedule[i].coord);
			// Check if the newly "active" rectangle intersects any other "active" rectangles
			for (unsigned index : activeIndices)
			{
				if (p[schedule[i].index].intersectsPolygon(p[index]))
				{
					DPRINT6("inserting edge schedule[", i, "].index = ", schedule[i].index, " ----- activeIndex = ", index);
					g[schedule[i].index].push_back(index);
					g[index].push_back(schedule[i].index);
				}
			}

			// Activation
			activeIndices.insert(schedule[i].index);
		}
		else
		{
			DPRINT3(schedule[i].index, "\tend\t", schedule[i].coord);
			activeIndices.erase(schedule[i].index);
		}
	}
}

Graph::~Graph()
{
	for (unsigned i = 0; i < vertices; ++i)
	{
		g[i].clear();
	}
	delete [] g;

	delete [] explored;
}

bool Graph::contiguous()
{
	// BFS variables
	unsigned currentVertex;
	std::queue<unsigned> explorationQ;
	std::memset(explored, false, vertices);
	unsigned explorationCount = 0;

	DPRINT2("vertices = ", vertices);

	// BFS, counting number of visited vertices which if the corresponding geometry is contigous
	// will be equal to vertices
	explorationQ.push(vertices / 2);
	for (;explorationQ.size();)
	{
		currentVertex = explorationQ.front();
		DPRINT2("currentVertex = ", currentVertex);
		explorationQ.pop();

		if (explored[currentVertex])
		{
			continue;
		}

		explored[currentVertex] = true;
		++explorationCount;

		// Go to all of currentVertex's neighbours
		for (auto neighbouringVertex : g[currentVertex])
		{
			if (!explored[neighbouringVertex])
			{
				explorationQ.push(neighbouringVertex);
			}
		}
	}

	return explorationCount == vertices;
}

bool Graph::contiguous(unsigned skipVertex)
{
	// BFS variables
	unsigned currentVertex;
	std::queue<unsigned> explorationQ;
	std::memset(explored, false, vertices);
	unsigned explorationCount = 0;

	// BFS, counting number of visited vertices which if the corresponding geometry is contigous
	// will be equal to vertices
	explorationQ.push(vertices / 2);
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
		for (auto neighbouringVertex : g[currentVertex])
		{
			if (neighbouringVertex != skipVertex && !explored[neighbouringVertex])
			{
				explorationQ.push(neighbouringVertex);
			}
		}
	}

	return explorationCount == vertices - 1;
}

bool *Graph::findBalancedSeparator(unsigned *weights)
{
	// Allocate a tree
	std::vector<unsigned> t[vertices];

	unsigned currentVertex;
	std::queue<unsigned> explorationQ;
	std::memset(explored, false, vertices);

	bool connected[vertices]; // Connection labels so we don't double connect vertices
	std::memset(connected, false, vertices);

	// Stacks to propogate weights
	std::stack<unsigned> weightStack;
	std::stack<unsigned> parentStack;

	// Build the tree
	explorationQ.push(vertices / 2);
	for (;explorationQ.size();)
	{
		currentVertex = explorationQ.front();
		explorationQ.pop();

		if (explored[currentVertex])
		{
			continue;
		}

		explored[currentVertex] = true;

		// Go to all of currentVertex's neighbours
		for (auto neighbouringVertex : g[currentVertex])
		{
			if (!explored[neighbouringVertex])
			{
				if (!connected[neighbouringVertex])
				{
					DPRINT2("Connecting ", neighbouringVertex);
					t[currentVertex].push_back(neighbouringVertex);
					t[neighbouringVertex].push_back(currentVertex);

					connected[neighbouringVertex] = true;

					weightStack.push(neighbouringVertex);
					parentStack.push(currentVertex);
				}

				explorationQ.push(neighbouringVertex);
			}
		}
	}

	for (unsigned i = 0; i < vertices; ++i)
	{
		DPRINT4("weights[", i, "] = ", weights[i]);
	}

	DPRINT2("parentStack.size() = ", parentStack.size());
	DPRINT2("weightStack.size() = ", weightStack.size());

	// Weight the tree so we can quickly find a separator
	for (;!parentStack.empty();)
	{
		DPRINT4("parentStack.top = ", parentStack.top(), " weightStack.top = ", weightStack.top());
		weights[parentStack.top()] += weights[weightStack.top()];
		weightStack.pop();
		parentStack.pop();
	}

	for (unsigned i = 0; i < vertices; ++i)
	{
		DPRINT4("weights[", i, "] = ", weights[i]);
	}

	// Find a separator within the tree
	DPRINT1("Finding a separator...");
	unsigned delta = std::numeric_limits<unsigned>::max();
	unsigned root = vertices / 2;
	unsigned subtreeRoot = 0;
	unsigned subtreeParent = 0;

	std::memset(explored, false, vertices);

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

		// Go to all of currentVertex's neighbours
		for (auto neighbouringVertex : t[currentVertex])
		{
			if (!explored[neighbouringVertex])
			{
				unsigned componentTwoWeight = weights[neighbouringVertex];
				unsigned componentOneWeight = weights[root] - componentTwoWeight;
				unsigned comparisonDelta = componentOneWeight > componentTwoWeight ? componentOneWeight - componentTwoWeight : componentTwoWeight - componentOneWeight;

				if (comparisonDelta < delta)
				{
					delta = comparisonDelta;
					subtreeRoot = neighbouringVertex;
					subtreeParent = currentVertex;
				}

				explorationQ.push(neighbouringVertex);
			}
		}
	}

	// Split along separator
	DPRINT1("Splitting along separator...");
	bool *switchboard = new bool[vertices];

	t[subtreeRoot].erase(std::find(t[subtreeRoot].begin(), t[subtreeRoot].end(), subtreeParent));
	t[subtreeParent].erase(std::find(t[subtreeParent].begin(), t[subtreeParent].end(), subtreeRoot));

	DPRINT1("Clearing switchboard and exploration labels");
	std::memset(switchboard, false, vertices);
	std::memset(explored, false, vertices);

	switchboard[subtreeRoot] = true;

	DPRINT1("BFSing for separator");
	explorationQ.push(subtreeRoot);
	for (;explorationQ.size();)
	{
		currentVertex = explorationQ.front();
		explorationQ.pop();

		if (explored[currentVertex])
		{
			continue;
		}

		explored[currentVertex] = true;

		// Go to all of currentVertex's neighbours
		for (auto neighbouringVertex : t[currentVertex])
		{
			if (!explored[neighbouringVertex])
			{
				switchboard[neighbouringVertex] = true;
				explorationQ.push(neighbouringVertex);
			}
		}
	}

	return switchboard;
}
