#include <bench/randomPoints.h>

void randomPoints(Index& spatialIndex, unsigned benchmarkSize, unsigned logFrequency, unsigned seed)
{
	// Setup random generators
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<float> pointDist(0.0, 200000.0);
	std::cout << "Uniformly distributing points between positions (0.0, 0.0) and (20000000.0, 20000000.0)." << std::endl;

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
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Create the point
		points[i] = Point(pointDist(generator), pointDist(generator));

		// Compute the checksum directly
		directSum += (unsigned)points[i].x;
		directSum += (unsigned)points[i].y;

		// Print the new point
		std::cout << "Point[" << i << "] " << points[i] << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	// Initialize search rectangles
	Rectangle *searchRectangles = new Rectangle[16];
	unsigned blockSize = benchmarkSize / 16;
	std::cout << "Beginning initialization of 16 rectangles..." << std::endl;
	for (unsigned i = 0; i < 16; ++i)
	{
		// Create the rectangle
		searchRectangles[i] = Rectangle(i * blockSize, i * blockSize, i * blockSize + blockSize, i * blockSize + blockSize);

		// Print the search rectangle
		std::cout << "searchRectangles[" << i << "] " << searchRectangles[i] << std::endl;
	}
	std::cout << "Finished initialization of 16 rectangles." << std::endl;

	// Insert points and time their insertion
	std::cout << "Beginning insertion of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Insert
		std::cout << "Point[" << i << "] ";
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex.insert(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeInserts += delta.count();
		totalInserts += 1;
		std::cout << "inserted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished insertion of " << benchmarkSize << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Search for points and time their retrieval
	std::cout << "Beginning search for " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Search
		std::cout << "Point[" << i << "] ";
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex.search(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeSearches += delta.count();
		totalSearches += 1;
		std::cout << "queried. " << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished search for " << benchmarkSize << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Search for rectangles
	std::cout << "Beginning search for 16 rectangles..." << std::endl;
	for (unsigned i = 0; i < 16; ++i)
	{
		// Search
		std::cout << "searchRectangles[" << i << "] ";
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		std::vector<Point> v = spatialIndex.search(searchRectangles[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeRangeSearches += delta.count();
		totalRangeSearches += 1;
		std::cout << "queried. " << delta.count() << " s" << std::endl;

		// Validate points returned in the search
		for (unsigned j = 0; j < v.size(); ++j)
		{
			assert(searchRectangles[i].containsPoint(v[j]));
		}
		std::cout << v.size() << " points verified." << std::endl;
	}
	std::cout << "Finished searching for 16 rectangles." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Delete points and time their deletion
	std::cout << "Beginning deletion of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Delete
		std::cout << "Point[" << i << "] ";
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex.remove(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeDeletes += delta.count();
		totalDeletes += 1;
		std::cout << "deleted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished deletion of " << benchmarkSize << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex.checksum() == 0);
	std::cout << "Checksum OK." << std::endl;

	// Statistics
	std::cout << "Total time to insert: " << totalTimeInserts << "s" << std::endl;
	std::cout << "Avg time to insert: " << totalTimeInserts / totalInserts << "s" << std::endl;
	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;
	std::cout << "Total time to range search: " << totalTimeRangeSearches << "s" << std::endl;
	std::cout << "Avg time to range search: " << totalTimeRangeSearches / totalRangeSearches << "s" << std::endl;
	std::cout << "Total time to delete: " << totalTimeDeletes << "s" << std::endl;
	std::cout << "Avg time to delete: " << totalTimeDeletes / totalDeletes << "s" << std::endl;
}
