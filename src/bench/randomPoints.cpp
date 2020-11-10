#include <bench/randomPoints.h>

Point *generateUniform(unsigned benchmarkSize, unsigned seed)
{
	// Setup random generators
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> pointDist(0.0, 1.0);
	std::cout << "Uniformly distributing points between positions (0.0, 0.0) and (1.0, 1.0)." << std::endl;

	// Initialize points
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Create the point
		points[i] = Point(pointDist(generator), pointDist(generator));

		// Print the new point
		// std::cout << "Point[" << i << "] " << points[i] << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	return points;
}

Point *generateSkewed(unsigned benchmarkSize, unsigned seed, float skewFactor)
{
	// Setup random generators
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> pointDist(0.0, 1.0);
	std::cout << "Skewing points between positions (0.0, 0.0) and (1.0, 1.0)." << std::endl;

	// Initialize points
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Create the point
		points[i] = Point(pointDist(generator), pow(pointDist(generator), skewFactor));

		// Print the new point
		// std::cout << "Point[" << i << "] " << points[i] << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	return points;
}

Point *generateClustered(unsigned benchmarkSize, unsigned seed, unsigned clusterCount)
{
	// Setup random generators
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> pointDist(0.0, 1.0);
	std::normal_distribution<float> clusterDist(0.0, 0.001);
	std::cout << "Clustering points between positions (0.0, 0.0) and (1.0, 1.0)." << std::endl;

	// Initialize cluster centres
	Point *centres = new Point[clusterCount];
	std::cout << "Beginning initialization of " << clusterCount << " clusters..." << std::endl;
	for (unsigned i = 0; i < clusterCount; ++i)
	{
		// Create the point
		centres[i] = Point(pointDist(generator), pointDist(generator));

		// Print the centre
		// std::cout << "Centre[" << i << "] " << centres[i] << std::endl;
	}
	std::cout << "Finished initialization of " << clusterCount << " clusters." << std::endl;

	// Initialize points
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize;)
	{
		for (unsigned j = 0; j < clusterCount && i < benchmarkSize; ++j)
		{
			// Create the point
			points[i] = Point(centres[j].x + clusterDist(generator), centres[j].y + clusterDist(generator));

			// Print the new point
			// std::cout << "Point[" << i << "] " << points[i] << std::endl;

			// Move to the next point
			i++;
		}
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	return points;
}

Rectangle *generateRectangles(unsigned benchmarkSize, unsigned seed, unsigned rectanglesSize)
{
	std::default_random_engine generator(seed + benchmarkSize);
	std::uniform_real_distribution<float> pointDist(0.0, 1.0);

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[rectanglesSize];
	std::cout << "Begnning initialization of " << rectanglesSize << " rectangles..." << std::endl;
	for (unsigned i = 0; i < rectanglesSize; ++i)
	{
		// Generate a new point and then create a square from it that covers 5% of the total area
		Point randomPoint = Point(pointDist(generator), pointDist(generator));
		Point randomPointDelta = Point(randomPoint.x + 0.05, randomPoint.y + 0.05);

		rectangles[i] = Rectangle(randomPoint, randomPointDelta);
	}
	std::cout << "Finished initialization of " << rectanglesSize << " rectangles..." << std::endl;

	return rectangles;
}

void randomPoints(Index& spatialIndex, BenchType bench, unsigned benchmarkSize, unsigned seed, unsigned logFrequency)
{
	// Setup checksums
	unsigned directSum = 0;

	// Setup statistics
	double totalTimeInserts = 0.0;
	double totalTimeSearches = 0.0;
	double totalTimeRangeSearches = 0.0;
	double totalTimeDeletes = 0.0;
	double totalInserts = 0.0;
	double totalSearches = 0.0;
	double totalRangeSearches = 0.0;
	double totalDeletes = 0.0;

	// Initialize points
	Point *points;
	switch(bench)
	{
		case UNIFORM:
			points = generateUniform(benchmarkSize, seed);
			break;
		case SKEW:
			points = generateSkewed(benchmarkSize, seed, 9.0);
			break;
		case CLUSTER:
			points = generateClustered(benchmarkSize, seed, 20);
			break;
		default:
			assert(false);
	}

	// Initialize search rectangles
	unsigned rectanglesSize = 5000;
	Rectangle *searchRectangles = generateRectangles(benchmarkSize, seed, rectanglesSize);

	// Insert points and time their insertion
	std::cout << "Beginning insertion of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Compute the checksum directly
		directSum += (unsigned)points[i].x;
		directSum += (unsigned)points[i].y;

		// Insert
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex.insert(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeInserts += delta.count();
		totalInserts += 1;
		// std::cout << "Point[" << i << "] inserted. " << delta.count() << "s" << std::endl;
	}
	std::cout << "Finished insertion of " << benchmarkSize << " points." << std::endl;

	// Gather statistics
	spatialIndex.stat();
	std::cout << "Statistics OK." << std::endl;

	// Visualize the tree
	spatialIndex.print();
	std::cout << "Visualization OK." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Search for points and time their retrieval
	std::cout << "Beginning search for " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		assert(spatialIndex.search(points[i])[0] == points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeSearches += delta.count();
		totalSearches += 1;
		// std::cout << "Point[" << i << "] queried. " << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished search for " << benchmarkSize << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Search for rectangles
	std::cout << "Beginning search for " << rectanglesSize << " rectangles..." << std::endl;
	for (unsigned i = 0; i < rectanglesSize; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		std::vector<Point> v = spatialIndex.search(searchRectangles[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeRangeSearches += delta.count();
		totalRangeSearches += 1;
		// std::cout << "searchRectangles[" << i << "] queried. " << delta.count() << " s" << std::endl;

		// Validate points returned in the search
		for (unsigned j = 0; j < v.size(); ++j)
		{
			assert(searchRectangles[i].containsPoint(v[j]));
		}
		// std::cout << v.size() << " points verified." << std::endl;
	}
	std::cout << "Finished searching for " << rectanglesSize << " rectangles." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Delete points and time their deletion
	std::cout << "Beginning deletion of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Delete
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex.remove(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeDeletes += delta.count();
		totalDeletes += 1;
		// std::cout << "Point[" << i << "] deleted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished deletion of " << benchmarkSize << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == 0);
	std::cout << "Checksum OK." << std::endl;

	// Timing Statistics
	std::cout << "Total time to insert: " << totalTimeInserts << "s" << std::endl;
	std::cout << "Avg time to insert: " << totalTimeInserts / totalInserts << "s" << std::endl;
	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;
	std::cout << "Total time to range search: " << totalTimeRangeSearches << "s" << std::endl;
	std::cout << "Avg time to range search: " << totalTimeRangeSearches / totalRangeSearches << "s" << std::endl;
	std::cout << "Total time to delete: " << totalTimeDeletes << "s" << std::endl;
	std::cout << "Avg time to delete: " << totalTimeDeletes / totalDeletes << "s" << std::endl;
}
