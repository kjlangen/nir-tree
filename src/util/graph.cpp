#include <util/graph.h>

void Graph::weightSubtree(float *weights, TaggedRectangle *vTagged, unsigned lBound, unsigned rBound)
{
	if (lBound == rBound)
	{
		// Leaf => lBound == rBound == root
		weights[lBound] = vTagged[lBound].r.upperRight.x;
	}
	else
	{
		unsigned root = (rBound + lBound) / 2;
		weights[root] = vTagged[root].r.upperRight.x;

		// Recurse left
		if (root != lBound)
		{
			weightSubtree(weights, vTagged, lBound, root - 1);
			weights[root] = std::fmax(weights[root], weights[(root - 1) / 2]);
		}

		// Recurse right
		if (root != rBound)
		{
			weightSubtree(weights, vTagged, root + 1, rBound);
			weights[root] = std::fmax(weights[root], weights[(rBound + root + 1) / 2]);
		}
	}
}

void Graph::querySubtree(TaggedRectangle queryRectangle, float *weights, TaggedRectangle *vTagged, unsigned lBound, unsigned rBound)
{
	// Compute the root
	// unsigned root = (rBound + lBound) / 2;
	unsigned *lstack = new unsigned[vertices];
	unsigned *rstack = new unsigned[vertices];
	unsigned topOfStack = 1;
	unsigned root;

	// Prime the stack
	lstack[0] = lBound;
	rstack[0] = rBound;

	// DFS for intersections
	for (;topOfStack != 0;)
	{
		// std::cout << "topOfStack = " << topOfStack << std::endl;
		lBound = lstack[topOfStack - 1];
		rBound = rstack[topOfStack - 1];
		root = (lBound + rBound) / 2;
		topOfStack--;
		// std::cout << "lBound = " << lBound << std::endl;
		// std::cout << "rBound = " << rBound << std::endl;
		// std::cout << "root = " << root << std::endl;
		// std::cout << "topOfStack = " << topOfStack << std::endl;

		// Check against root
		// std::cout << "DFS1" << std::endl;
		if (queryRectangle.r.intersectsRectangle(vTagged[root].r) && queryRectangle.r != vTagged[root].r)
		{
			// std::cout << "DFS2" << std::endl;
			// std::cout << "queryRectangle.tag = " << queryRectangle.tag << std::endl;
			// std::cout << "vTagged[root].tag = " << vTagged[root].tag << std::endl;
			// std::cout << "vertices = " << vertices << std::endl;
			// std::cout << "comp0 = " << (queryRectangle.tag * vertices + vTagged[root].tag <= vertices) << std::endl;
			// std::cout << "comp1 = " << (vTagged[root].tag * vertices + queryRectangle.tag <= vertices) << std::endl;			
			assert((queryRectangle.tag * vertices + vTagged[root].tag) <= vertices * vertices);
			assert((vTagged[root].tag * vertices + queryRectangle.tag) <= vertices * vertices);
			g[queryRectangle.tag * vertices + vTagged[root].tag] = true;
			g[vTagged[root].tag * vertices + queryRectangle.tag] = true;
			// std::cout << "DFS2.1" << std::endl;
		}

		// std::cout << "DFS3" << std::endl;
		if (lBound == rBound)
		{
			continue;
		}

		// Might intersect things on the left
		// std::cout << "DFS4" << std::endl;
		if (root != lBound && queryRectangle.r.lowerLeft.x <= vTagged[root].r.lowerLeft.x)
		{
			// std::cout << "DFS5" << std::endl;
			// Go Left
			lstack[topOfStack] = lBound;
			rstack[topOfStack] = root - 1;
			++topOfStack;
			// querySubtree(queryRectangle, weights, vTagged, lBound, root - 1);
		}

		// Might intersect things on the right
		// std::cout << "DFS6" << std::endl;
		if (root != rBound && queryRectangle.r.upperRight.x >= vTagged[root].r.lowerLeft.x && queryRectangle.r.lowerLeft.x <= weights[root])
		{
			// std::cout << "DFS7" << std::endl;
			// Go Right
			lstack[topOfStack] = root + 1;
			rstack[topOfStack] = rBound;
			++topOfStack;
			// querySubtree(queryRectangle, weights, vTagged, root + 1, rBound);
		}
	}
}

Graph::Graph(const unsigned n)
{
	vertices = n;
	g = new bool[n * n];
	std::memset(g, false, n * n);
}

// TODO: Remove this comment
// Lambda example: [](Point a, Point b){return a.x < b.x;}

Graph::Graph(std::vector<Rectangle> &v)
{
	vertices = v.size();
	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
	g = new bool[vertices * vertices];
	std::memset(g, false, vertices * vertices);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	std::cout << "Allocate & clear = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;
	// std::cout << "Ok 0" << std::endl;

	// Tag
	begin = std::chrono::high_resolution_clock::now();
	TaggedRectangle *vTagged = new TaggedRectangle[vertices];
	// std::cout << "Ok 1" << std::endl;
	for (unsigned i = 0; i < vertices; ++i)
	{
		vTagged[i] = {v[i], i};
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Tag = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;
	// std::cout << "Ok 2" << std::endl;

	// Sort
	begin = std::chrono::high_resolution_clock::now();
	std::sort(vTagged, vTagged + sizeof(vTagged)/sizeof(vTagged[0]), [](TaggedRectangle a, TaggedRectangle b){return a.r.lowerLeft.x < b.r.lowerLeft.x;});
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Sort = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;
	// std::cout << "Ok 3" << std::endl;

	// Weight
	// std::cout << "Graph constructor weighting interval tree." << std::endl;
	begin = std::chrono::high_resolution_clock::now();
	float *weights = new float[vertices];
	// std::cout << "Ok 4" << std::endl;
	weightSubtree(weights, vTagged, 0, vertices - 1);
	// std::cout << "Ok 5" << std::endl;
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Weight = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;

	// Query & Record
	// std::cout << "Graph constructor querying interval tree." << std::endl;
	begin = std::chrono::high_resolution_clock::now();
	for (unsigned i = 0; i < vertices; ++i)
	{
		querySubtree(vTagged[i], weights, vTagged, 0, vertices - 1);
	}
	end = std::chrono::high_resolution_clock::now();
	std::cout << "Query = " << std::chrono::duration_cast<std::chrono::duration<double>>(end - begin).count() << "s" << std::endl;
	// std::cout << "Graph construction complete!" << std::endl;
}

Graph::Graph(std::vector<IsotheticPolygon> &v)
{
	// vertices = v.size();
	// g = new bool[v.size() * v.size()];
	// std::memset(g, false, vertices * vertices);
	// unsigned basicRectanglesCount = 0;

	// // Tag
	// std::vector<TaggedRectangle> vTagged(vertices);
	// for (unsigned i = 0; i < vertices; ++i)
	// {
	// 	for (auto basicRectangle : v[i].basicRectangles)
	// 	{
	// 		vTagged.push_back({basicRectangle, i});
	// 		++basicRectanglesCount;
	// 	}
	// }

	// // Sort
	// std::sort(vTagged.begin(), vTagged.end(), [](TaggedRectangle a, TaggedRectangle b){return a.r.lowerLeft.x < b.r.lowerLeft.x;});

	// // Weight
	// // std::cout << "Graph constructor weighting interval tree." << std::endl;
	// std::vector<float> weights(basicRectanglesCount, 0.0);
	// weightSubtree(weights, vTagged, 0, basicRectanglesCount - 1);

	// // Query & Record
	// // std::cout << "Graph constructor querying interval tree." << std::endl;
	// for (unsigned i = 0; i < basicRectanglesCount; ++i)
	// {
	// 	querySubtree(vTagged[i], weights, vTagged, 0, basicRectanglesCount - 1);
	// }
	// std::cout << "Graph construction complete!" << std::endl;
}

Graph::~Graph()
{
	delete [] g;
}

bool *Graph::operator[](unsigned i)
{
	return &g[i * vertices];
}
