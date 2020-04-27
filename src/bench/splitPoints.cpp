#include <bench/splitPoints.h>

void splitPoints()
{
	// Setup random generators
	std::default_random_engine generator;
	std::uniform_real_distribution<float> pointDist(0.0, 29.0);
	std::cout << "Uniformly distributing points between positions (0.0, 0.0) and (20000000.0, 20000000.0)." << std::endl;

	// Setup checksums
	// unsigned directSum = 0;

	// Setup statistics
	// double totalTimeInserts = 0.0;
	// double totalTimeSearches = 0.0;
	// double totalTimeRangeSearches = 0.0;
	// double totalTimeDeletes = 0.0;
	// double totalInserts = 0.0;
	// double totalSearches = 0.0;
	// double totalRangeSearches = 0.0;
	// double totalDeletes = 0.0;

	// Set benchmark size
	unsigned benchmarkSize = 1000;

	// Initialize everything for R-Tree
	Point *points = new Point[benchmarkSize];
	rtree::Node *cluster = new rtree::Node();
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Create the point
		points[i] = Point(pointDist(generator), pointDist(generator));

		// Compute the checksum directly
		// directSum += (unsigned)points[i].x;
		// directSum += (unsigned)points[i].y;

		// Print the new point
		std::cout << "Points[" << i << "] "; points[i].print(); std::cout << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	std::cout << "Adding points to R-Tree node..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		cluster->data.push_back(points[i]);
	}
	std::cout << "Finished adding points to R-Tree node." << std::endl;

	// Initialize everything for for NIR-Tree
	IsotheticPolygon ip = IsotheticPolygon();
	ip.basicRectangles.push_back(Rectangle(0.0, 7.0, 5.0, 10.0));
	ip.basicRectangles.push_back(Rectangle(5.0, 5.0, 8.0, 12.0));
	ip.basicRectangles.push_back(Rectangle(8.0, 10.0, 13.0, 14.0));
	ip.basicRectangles.push_back(Rectangle(4.0, 0.0, 10.0, 5.0));
	ip.basicRectangles.push_back(Rectangle(10.0, 0.0, 13.0, 10.0));
	// Translated up 14.0 units
	ip.basicRectangles.push_back(Rectangle(0.0, 14.0 + 7.0, 5.0, 14.0 + 10.0));
	ip.basicRectangles.push_back(Rectangle(5.0, 14.0 + 5.0, 8.0, 14.0 + 12.0));
	ip.basicRectangles.push_back(Rectangle(8.0, 14.0 + 10.0, 13.0, 14.0 + 14.0));
	ip.basicRectangles.push_back(Rectangle(4.0, 14.0 + 0.0, 10.0, 14.0 + 5.0));
	ip.basicRectangles.push_back(Rectangle(10.0, 14.0 + 0.0, 13.0, 14.0 + 10.0));
	// Translated right 13.0 units
	ip.basicRectangles.push_back(Rectangle(13.0 + 0.0, 7.0, 13.0 + 5.0, 10.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 5.0, 5.0, 13.0 + 8.0, 12.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 8.0, 10.0, 13.0 + 13.0, 14.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 4.0, 0.0, 13.0 + 10.0, 5.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 10.0, 0.0, 13.0 + 13.0, 10.0));
	// Translated up 14.0 units and right 13.0 units
	ip.basicRectangles.push_back(Rectangle(13.0 + 0.0, 14.0 + 7.0, 13.0 + 5.0, 14.0 + 10.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 5.0, 14.0 + 5.0, 13.0 + 8.0, 14.0 + 12.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 8.0, 14.0 + 10.0, 13.0 +  13.0, 14.0 + 14.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 4.0, 14.0 + 0.0, 13.0 + 10.0, 14.0 + 5.0));
	ip.basicRectangles.push_back(Rectangle(13.0 + 10.0, 14.0 + 0.0, 13.0 + 13.0, 14.0 + 10.0));

	Point *pointsPrime = new Point[benchmarkSize];
	nirtree::Node *root = new nirtree::Node();
	nirtree::Node *clusterPrime = new nirtree::Node(1, 1, root);
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize;)
	{
		// Create the point
		float p1 = pointDist(generator);
		float p2 = pointDist(generator);
		std::cout << "Generator produced: " << p1 << ", " << p2 << std::endl;
		pointsPrime[i] = Point(p1, p2);

		// Compute the checksum directly
		// directSum += (unsigned)points[i].x;
		// directSum += (unsigned)points[i].y;

		// Print the new point
		std::cout << "PointsPrime[" << i << "] "; pointsPrime[i].print(); std::cout << std::endl;

		// Keep the point and advance only if it is within the isothetic polygon
		if (ip.containsPoint(pointsPrime[i]))
		{
			std::cout << "ip contianed point => advancing i" << std::endl;
			++i;
		}
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	std::cout << "Adding points to NIR-Tree node..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		clusterPrime->data.push_back(points[i]);
	}
	std::cout << "Finished adding points to NIR-Tree node." << std::endl;

	root->children.push_back(clusterPrime);
	root->boundingBoxes.push_back(ip);

	// Split R-Tree Node
	auto begin = std::chrono::high_resolution_clock::now();
	cluster->splitNode(Point(12.0, 5.0));
	auto end = std::chrono::high_resolution_clock::now();

	// Split NIR-Tree Node
	auto beginPrime = std::chrono::high_resolution_clock::now();
	clusterPrime->splitNode(Point(12.0, 5.0));
	auto endPrime = std::chrono::high_resolution_clock::now();

	// Statistics
	std::cout << "Time to split R-Tree node: " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "us" << std::endl;
	std::cout << "Time to split NIR-Tree node: " << std::chrono::duration_cast<std::chrono::microseconds>(endPrime - beginPrime).count() << "us" << std::endl;
	// std::cout << "Total time to insert: " << totalTimeInserts << "s" << std::endl;
	// std::cout << "Avg time to insert: " << totalTimeInserts / totalInserts << "s" << std::endl;
	// std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	// std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;
	// std::cout << "Total time to range search: " << totalTimeRangeSearches << "s" << std::endl;
	// std::cout << "Avg time to range search: " << totalTimeRangeSearches / totalRangeSearches << "s" << std::endl;
	// std::cout << "Total time to delete: " << totalTimeDeletes << "s" << std::endl;
	// std::cout << "Avg time to delete: " << totalTimeDeletes / totalDeletes << "s" << std::endl;
}
