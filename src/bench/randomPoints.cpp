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
	std::cout << "Initialization OK." << std::endl;

	return points;
}

Point *generateBits()
{
	// Dataset is pre-generated and requires 2 or 3 dimensions
	assert(dimensions == 2 || dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = dimensions == 2 ? "/home/kjlangen/nir-tree/data/bit02" : "/home/kjlangen/nir-tree/data/bit03";
	file.open(dataPath.c_str());
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize points
	Point *points = new Point[BitDataSize];
	std::cout << "Beginning initialization of " << BitDataSize << " points..." << std::endl;
	for (unsigned i = 0; i < BitDataSize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			file.read(buffer, sizeof(double));
			file.sync();
			points[i][d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return points;
}

Point *generateHaze()
{
	// Dataset is pre-generated and requires 2 or 3 dimensions
	assert(dimensions == 2 || dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = dimensions == 2 ? "/home/kjlangen/nir-tree/data/pha02" : "/home/kjlangen/nir-tree/data/pha03";
	file.open(dataPath.c_str());
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize points
	Point *points = new Point[HazeDataSize];
	std::cout << "Beginning initialization of " << HazeDataSize << " points..." << std::endl;
	for (unsigned i = 0; i < HazeDataSize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			file.read(buffer, sizeof(double));
			file.sync();
			points[i][d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return points;
}

Point *generateCalifornia()
{
	// Dataset is pre-generated and requires 2 dimensions
	assert(dimensions == 2);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea02");
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize points
	Point *points = new Point[CaliforniaDataSize];
	std::cout << "Beginning initialization of " << CaliforniaDataSize << " points..." << std::endl;
	for (unsigned i = 0; i < CaliforniaDataSize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			file.sync();
			points[i][d] = *doubleBuffer;
			file.read(buffer, sizeof(double));
			file.sync();
			points[i][d] = (points[i][d] + *doubleBuffer) / 2;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

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
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize points
	Point *points = new Point[BiologicalDataSize];
	std::cout << "Beginning initialization of " << BiologicalDataSize << " points..." << std::endl;
	for (unsigned i = 0; i < BiologicalDataSize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			file.read(buffer, sizeof(double));
			file.sync();
			points[i][d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return points;
}

Point *generateForest()
{
	// Dataset is pre-generated and requires 5 dimensions
	assert(dimensions == 5);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea05");
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize points
	Point *points = new Point[ForestDataSize];
	std::cout << "Beginning initialization of " << ForestDataSize << " points..." << std::endl;
	for (unsigned i = 0; i < ForestDataSize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			file.read(buffer, sizeof(double));
			file.sync();
			points[i][d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return points;
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
	std::cout << "Initialization OK." << std::endl;

	return rectangles;
}

Rectangle *generateBitRectangles()
{
	// Query set is pre-generated and requires 2 or 3 dimensions
	assert(dimensions == 2 || dimensions == 3);

	// Setup file reader and double buffef
	std::fstream file;
	std::string queryPath = dimensions == 2 ? "/home/kjlangen/nir-tree/bit02.2" : "/home/kjlangen/nir-tree/bit03.2";
	file.open(queryPath.c_str());
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[BitQuerySize];
	std::cout << "Beginning initialization of " << BitQuerySize << " computer rectangles..." << std::endl;
	for (unsigned i = 0; i < BitQuerySize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			rectangles[i].lowerLeft[d] = *doubleBuffer;
			file.read(buffer, sizeof(double));
			rectangles[i].upperRight[d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

Rectangle *generateHazeRectangles()
{
	// Query set is pre-generated and requires 2 or 3 dimensions
	assert(dimensions == 2 || dimensions == 3);

	// Setup file reader and double buffef
	std::fstream file;
	std::string queryPath = dimensions == 2 ? "/home/kjlangen/nir-tree/pha02.2" : "/home/kjlangen/nir-tree/pha03.2";
	file.open(queryPath.c_str());
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[HazeQuerySize];
	std::cout << "Beginning initialization of " << HazeQuerySize << " hazy rectangles..." << std::endl;
	for (unsigned i = 0; i < HazeQuerySize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			rectangles[i].lowerLeft[d] = *doubleBuffer;
			file.read(buffer, sizeof(double));
			rectangles[i].upperRight[d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

Rectangle *generateCaliRectangles()
{
	// Query set is pre-generated and requires 2 dimensions
	assert(dimensions == 2);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea02.2");
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[CaliforniaQuerySize];
	std::cout << "Beginning initialization of " << CaliforniaQuerySize << " california roll rectangles..." << std::endl;
	for (unsigned i = 0; i < CaliforniaQuerySize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			rectangles[i].lowerLeft[d] = *doubleBuffer;
			file.read(buffer, sizeof(double));
			rectangles[i].upperRight[d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

Rectangle *generateBioRectangles()
{
	// Query set is pre-generated and requires 3 dimensions
	assert(dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea03.2");
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[BiologicalQuerySize];
	std::cout << "Beginning initialization of " << BiologicalQuerySize << " biological warfare rectangles..." << std::endl;
	for (unsigned i = 0; i < BiologicalQuerySize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			rectangles[i].lowerLeft[d] = *doubleBuffer;
			file.read(buffer, sizeof(double));
			rectangles[i].upperRight[d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

Rectangle *generateForestRectangles()
{
	// Query set is pre-generated and requires 5 dimensions
	assert(dimensions == 5);

	// Setup file reader and double buffer
	std::fstream file;
	file.open("/home/kjlangen/nir-tree/data/rea05.2");
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
	Rectangle *rectangles = new Rectangle[ForestQuerySize];
	std::cout << "Beginning initialization of " << ForestQuerySize << " forest fire rectangles..." << std::endl;
	for (unsigned i = 0; i < ForestQuerySize; ++i)
	{
		for (unsigned d = 0; d < dimensions; ++d)
		{
			file.read(buffer, sizeof(double));
			rectangles[i].lowerLeft[d] = *doubleBuffer;
			file.read(buffer, sizeof(double));
			rectangles[i].upperRight[d] = *doubleBuffer;
		}
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

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
	else if (configU["tree"] == R_STAR_TREE)
	{
		spatialIndex = new rstartree::RStarTree(configU["minfanout"], configU["maxfanout"]);
	}
	else if (configU["tree"] == NIR_TREE)
	{
		spatialIndex = new nirtree::NIRTree(configU["minfanout"], configU["maxfanout"]);
	}
	else
	{
		return;
	}

	// Initialize points
	Point *points;
	if (configU["distribution"] == UNIFORM)
	{
		points = generateUniform(configU["size"], configU["seed"]);
	}
	else if (configU["distribution"] == SKEW)
	{
		configU["size"] = BitDataSize;
		points = generateBits();
	}
	else if (configU["distribution"] == CLUSTER)
	{
		configU["size"] = HazeDataSize;
		points = generateHaze();
	}
	else if (configU["distribution"] == CALIFORNIA)
	{
		configU["size"] = CaliforniaDataSize;
		points = generateCalifornia();
	}
	else if (configU["distribution"] == BIOLOGICAL)
	{
		configU["size"] = BiologicalDataSize;
		points = generateBiological();
	}
	else if (configU["distribution"] == FOREST)
	{
		configU["size"] = ForestDataSize;
		points = generateForest();
	}
	else
	{
		return;
	}

	// Initialize search rectangles
	Rectangle *searchRectangles;
	if (configU["distribution"] == UNIFORM)
	{
		searchRectangles = generateRectangles(configU["size"], configU["seed"], configU["rectanglescount"]);
	}
	else if (configU["distribution"] == SKEW)
	{
		configU["rectanglescount"] = BitQuerySize;
		searchRectangles = generateBitRectangles();
	}
	else if (configU["distribution"] == CLUSTER)
	{
		configU["rectanglescount"] = HazeQuerySize;
		searchRectangles = generateHazeRectangles();
	}
	else if (configU["distribution"] == CALIFORNIA)
	{
		configU["rectanglescount"] = CaliforniaQuerySize;
		searchRectangles = generateCaliRectangles();
	}
	else if (configU["distribution"] == BIOLOGICAL)
	{
		configU["rectanglescount"] = BiologicalQuerySize;
		searchRectangles = generateBioRectangles();
	}
	else if (configU["distribution"] == FOREST)
	{
		configU["rectanglescount"] = ForestQuerySize;
		searchRectangles = generateForestRectangles();
	}
	else
	{
		return;
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
	std::cout << "Insertion OK." << std::endl;

	// Visualize the tree
	if (configU["visualization"])
	{
		spatialIndex->visualize();
		std::cout << "Visualization OK." << std::endl;
	}

	// Validate checksum
	if (spatialIndex->checksum() != directSum)
	{
		exit(1);
	}
	std::cout << "Checksum OK." << std::endl;

	// Validate tree
	if (!spatialIndex->validate())
	{
		exit(1);
	}
	std::cout << "Validation OK." << std::endl;

	// Search for points and time their retrieval
	std::cout << "Beginning search for " << configU["size"] << " points..." << std::endl;
	for (unsigned i = 0; i < configU["size"]; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		if (spatialIndex->search(points[i])[0] != points[i])
		{
			exit(1);
		}
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeSearches += delta.count();
		totalSearches += 1;
		// std::cout << "Point[" << i << "] queried. " << delta.count() << " s" << std::endl;
	}
	std::cout << "Search OK." << std::endl;

	// Validate checksum
	if (spatialIndex->checksum() != directSum)
	{
		exit(1);
	}
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

#ifndef NDEBUG
		// Validate points returned in the search
		for (unsigned j = 0; j < v.size(); ++j)
		{
			assert(searchRectangles[i].containsPoint(v[j]));
		}
		// std::cout << v.size() << " points verified." << std::endl;
#endif
	}
	std::cout << "Range search OK." << std::endl;

	// Gather statistics
	STATEXEC(spatialIndex->stat());
	STATEXEC(std::cout << "Statistics OK." << std::endl);

	// Validate checksum
	if (spatialIndex->checksum() != directSum)
	{
		exit(1);
	}
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
	std::cout << "Deletion OK." << std::endl;

	// Validate checksum
	if (spatialIndex->checksum() != 0)
	{
		exit(1);
	}
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
