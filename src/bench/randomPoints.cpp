#include <bench/randomPoints.h>

Point *generateUniform(unsigned benchmarkSize, unsigned seed)
{
	// Setup random generators
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<double> pointDist(0.0, 1.0);
	std::cout << "Uniformly distributing points between positions (0.0, 0.0) and (1.0, 1.0)." << std::endl;

	// Initialize points
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Create the point
		for (unsigned d = 0; d < dimensions; ++d)
		{
			points[i][d] = pointDist(generator);
		}

		// Print the new point
		// std::cout << "Point[" << i << "] " << points[i] << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	return points;
}

Point *generateSkewed(unsigned benchmarkSize, unsigned seed, double skewFactor)
{
	// Setup random generators
	std::default_random_engine generator(seed);
	std::uniform_real_distribution<double> pointDist(0.0, 1.0);
	std::cout << "Skewing points between positions (0.0, 0.0) and (1.0, 1.0)." << std::endl;

	// Initialize points
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Create the point
		for (unsigned d = 0; d < dimensions; ++d)
		{
			points[i][d] = pointDist(generator);
		}
		points[i][dimensions - 1] = pow(points[i][dimensions - 1], skewFactor);

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
	std::uniform_real_distribution<double> pointDist(0.0, 1.0);
	std::normal_distribution<double> clusterDist(0.0, 0.001);
	std::cout << "Clustering points between positions (0.0, 0.0) and (1.0, 1.0)." << std::endl;

	// Initialize cluster centres
	Point *centres = new Point[clusterCount];
	std::cout << "Beginning initialization of " << clusterCount << " clusters..." << std::endl;
	for (unsigned i = 0; i < clusterCount; ++i)
	{
		// Create the point
		for (unsigned d = 0; d < dimensions; ++d)
		{
			centres[i][d] = pointDist(generator);
		}

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
			for (unsigned d = 0; d < dimensions; ++d)
			{
				points[i][d] = centres[j][d] + clusterDist(generator);
			}

			// Print the new point
			// std::cout << "Point[" << i << "] " << points[i] << std::endl;

			// Move to the next point
			i++;
		}
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	return points;
}

Point *generateCalifornia()
{
	// Dataset is pre-generated and requires 4 dimensions
	assert(dimensions == 4);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea02");
	char *buffer = new char[sizeof(double)];

	// Initialize points
	Point *points = new Point[CaliforniaSize];
	std::cout << "Beginning initialization of " << CaliforniaSize << " points..." << std::endl;
	for (unsigned i = 0; i < CaliforniaSize; ++i)
	{
		file.read(buffer, sizeof(double));
		points[i][0] = *((double *)buffer);

		file.read(buffer, sizeof(double));
		points[i][2] = *((double *)buffer);

		file.read(buffer, sizeof(double));
		points[i][1] = *((double *)buffer);

		file.read(buffer, sizeof(double));
		points[i][3] = *((double *)buffer);
	}
	std::cout << "Finished initialization of " << CaliforniaSize << " points." << std::endl;

	// Cleanup
	file.close();

	return points;
}

Point *generateBiological()
{
	// Dataset is pre-generated and requires 3 dimensions
	assert(dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea03");
	char *buffer = new char[sizeof(double)];

	// Initialize points
	Point *points = new Point[BiologicalSize];
	std::cout << "Beginning initialization of " << BiologicalSize << " points..." << std::endl;
	assert(dimensions == 3);
	for (unsigned i = 0; i < BiologicalSize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			file.read(buffer, sizeof(double));
			points[i][d] = *((double *)buffer);
		}
	}
	std::cout << "Finished initialization of " << BiologicalSize << " points." << std::endl;

	// Cleanup
	file.close();

	return points;
}

Rectangle *generateCaliRectangles()
{
	// TODO: Generate reasonable rectangles or interpret query files for rea02

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[1];
	std::cout << "Beginning initialization of 1 california roll rectangle..." << std::endl;
	rectangles[0] = Rectangle();
	std::cout << "Finished initialization of 1 california roll rectangle. Fix this." << std::endl;

	return rectangles;
}

Rectangle *generateBioRectangles()
{
	// TODO: Generate reasonable rectangles or interpret query files for rea03

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[1];
	std::cout << "Beginning initialization of 1 biological warfare rectangle..." << std::endl;
	rectangles[0] = Rectangle();
	std::cout << "Finished initialization of 1 biological warfare rectangle. Fix this." << std::endl;

	return rectangles;
}

Rectangle *generateRectangles(unsigned benchmarkSize, unsigned seed, unsigned rectanglesSize)
{
	std::default_random_engine generator(seed + benchmarkSize);
	std::uniform_real_distribution<double> pointDist(0.0, 1.0);

	// Initialize rectangles
	Point ll;
	Point ur;
	Rectangle *rectangles = new Rectangle[rectanglesSize];
	std::cout << "Begnning initialization of " << rectanglesSize << " rectangles..." << std::endl;
	for (unsigned i = 0; i < rectanglesSize; ++i)
	{
		// Generate a new point and then create a square from it that covers 5% of the total area
		for (unsigned d = 0; d < dimensions; ++d)
		{
			ll[d] = pointDist(generator);
			ur[d] = ll[d] + 0.05;
		}

		rectangles[i] = Rectangle(ll, ur);
	}
	std::cout << "Finished initialization of " << rectanglesSize << " rectangles..." << std::endl;

	return rectangles;
}

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configD)
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

	// Initialize the index
	Index *spatialIndex;
	if (configU["tree"] == R_TREE)
	{
		spatialIndex = new rtree::RTree(configU["minfanout"], configU["maxfanout"]);
	}
	else if (configU["tree"] == R_PLUS_TREE)
	{
		spatialIndex = new rplustree::RPlusTree(configU["minfanout"], configU["maxfanout"]);
	}
	else if (configU["tree"] == NIR_TREE)
	{
		spatialIndex = new nirtree::NIRTree(configU["minfanout"], configU["maxfanout"]);
	}
	else
	{
		assert(false);
	}

	// Initialize points
	Point *points;
	if(configU["distribution"] == UNIFORM)
	{
		points = generateUniform(configU["size"], configU["seed"]);
	}
	else if (configU["distribution"] == SKEW)
	{
		points = generateSkewed(configU["size"], configU["seed"], (double) configD["skewfactor"]);
	}
	else if (configU["distribution"] == CLUSTER)
	{
		points = generateClustered(configU["size"], configU["seed"], configU["clustercount"]);
	}
	else if (configU["distribution"] == CALIFORNIA)
	{
		configU["size"] = CaliforniaSize;
		points = generateCalifornia();
	}
	else if (configU["distribution"] == BIOLOGICAL)
	{
		configU["size"] = BiologicalSize;
		points = generateBiological();
	}
	else
	{
		assert(false);
	}

	// Initialize search rectangles
	Rectangle *searchRectangles;
	if (configU["distribution"] == CALIFORNIA)
	{
		configU["rectanglescount"] = 0; // ??
		searchRectangles = generateCaliRectangles();
	}
	else if (configU["distribution"] == BIOLOGICAL)
	{
		configU["rectanglescount"] = 0;
		searchRectangles = generateBioRectangles();
	}
	else
	{
		searchRectangles = generateRectangles(configU["size"], configU["seed"], configU["rectanglescount"]);
	}

	// Insert points and time their insertion
	std::cout << "Beginning insertion of " << configU["size"] << " points..." << std::endl;
	for (unsigned i = 0; i < configU["size"]; ++i)
	{
		// Compute the checksum directly
		for (unsigned d = 0; d < dimensions; ++d)
		{
			directSum += (unsigned)points[i][d];
		}

		// Insert
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex->insert(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeInserts += delta.count();
		totalInserts += 1;
		// std::cout << "Point[" << i << "] inserted. " << delta.count() << "s" << std::endl;
	}
	std::cout << "Finished insertion of " << configU["size"] << " points." << std::endl;

	// Gather statistics
	spatialIndex->stat();
	std::cout << "Statistics OK." << std::endl;

	// Visualize the tree
	// spatialIndex->visualize();
	// std::cout << "Visualization OK." << std::endl;

	// Validate checksum
	assert(spatialIndex->checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Validate tree
	// assert(spatialIndex->validate());
	// std::cout << "Validation OK." << std::endl;

	// Search for points and time their retrieval
	std::cout << "Beginning search for " << configU["size"] << " points..." << std::endl;
	for (unsigned i = 0; i < configU["size"]; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		assert(spatialIndex->search(points[i])[0] == points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeSearches += delta.count();
		totalSearches += 1;
		// std::cout << "Point[" << i << "] queried. " << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished search for " << configU["size"] << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex->checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Search for rectangles
	std::cout << "Beginning search for " << configU["rectanglescount"] << " rectangles..." << std::endl;
	for (unsigned i = 0; i < configU["rectanglescount"]; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		std::vector<Point> v = spatialIndex->search(searchRectangles[i]);
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
	std::cout << "Finished searching for " << configU["rectanglescount"] << " rectangles." << std::endl;

	// Validate checksum
	assert(spatialIndex->checksum() == directSum);
	std::cout << "Checksum OK." << std::endl;

	// Delete points and time their deletion
	std::cout << "Beginning deletion of " << configU["size"] << " points..." << std::endl;
	for (unsigned i = 0; i < configU["size"]; ++i)
	{
		// Delete
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex->remove(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeDeletes += delta.count();
		totalDeletes += 1;
		// std::cout << "Point[" << i << "] deleted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished deletion of " << configU["size"] << " points." << std::endl;

	// Validate checksum
	assert(spatialIndex->checksum() == 0);
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

	// Cleanup
	delete spatialIndex;
	delete [] points;
	delete [] searchRectangles;
}
