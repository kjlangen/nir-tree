#include <bench/randomPoints.h>

void randomPoints()
{
	// Setup random generators
	std::default_random_engine generator;
	// std::uniform_real_distribution<double> pointDist(0.0, 20000000.0);
	std::uniform_int_distribution<unsigned> decimalDist(0, 100000);
	std::uniform_int_distribution<unsigned> wholeDist(0, 20000001);
	// std::uniform_real_distribution<double> pointDist(-20000000.0, 0.0);
	std::cout << "Uniformly distributing points between positions (-20 000 000, -20 000 000) and (20 000 000, 20 000 000)." << std::endl;

	// Setup points
	// Number of points
	unsigned benchmarkSize = 10000; //5000000;
	std::cout << "Checkpoint" << std::endl;

	// Initialize points
	Point *points = new Point[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		double xGen = static_cast<double>(decimalDist(generator));
		std::cout << "xGen: " << xGen << std::endl;
		double x = xGen * 0.00001;
		std::cout << "xDecimal: " << x << std::endl;
		xGen = static_cast<double>(wholeDist(generator));
		std::cout << "xGen: " << xGen << std::endl;
		x += xGen;
		std::cout << "xWhole+Decimal: " << x << std::endl;
		double yGen = static_cast<double>(decimalDist(generator));
		std::cout << "yGen: " << yGen << std::endl;
		double y = yGen * 0.00001;
		std::cout << "yDecimal: " << y << std::endl;
		yGen = static_cast<double>(wholeDist(generator));
		std::cout << "yGen: " << yGen << std::endl; 
		y += yGen;
		std::cout << "yWhole+Decimal: " << y << std::endl;
		points[i] = Point(x, y);

		std::cout << "Point[" << i << "] "; points[i].print(); std::cout << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " points." << std::endl;

	// Setup statistics
	double totalTimeInserts = 0.0;
	double totalTimeSearches = 0.0;
	double totalTimeDeletes = 0.0;
	double totalInserts = 0.0;
	double totalSearches = 0.0;
	double totalDeletes = 0.0;

	// Setup the RTree
	RTree *rtree = new RTree(750, 1500);

	// Insert points and time their insertion
	std::cout << "Beginning insertion of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Insert
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		rtree->insert(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeInserts += delta.count();
		totalInserts += 1;
		std::cout << "Point[" << i << "] "; points[i].print(); std::cout << " inserted." << delta.count() << " s" << std::endl;
	}
	// for (unsigned i = 20; i < benchmarkSize; ++i)
	// {
	// 	// Insert
	// 	std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
	// 	rtree->insert(points[i]);
	// 	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
	// 	std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
	// 	totalTimeInserts += delta.count();
	// 	totalInserts += 1;
	// 	std::cout << "Point[" << i << "] "; points[i].print(); std::cout << " inserted." << delta.count() << " s" << std::endl;
	// 	Rectangle searchRectangle = Rectangle(points[i], 0.0, 0.0);
	// 	if (rtree->search(searchRectangle).size() == 0)
	// 	{
	// 		rtree->print();
	// 		assert(false);
	// 	}
	// }
	// std::cout << "Finished insertion of " << benchmarkSize << " points." << std::endl;

	// Search for points and time their retrieval
	std::cout << "Beginning search for " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Search
		Rectangle searchRectangle = Rectangle(points[i], 0.0, 0.0);
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		if (rtree->search(searchRectangle).size() == 0)
		{
			rtree->print();
			assert(false);
		}
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeSearches += delta.count();
		totalSearches += 1;
		std::cout << "Point[" << i << "] queried." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished searching for " << benchmarkSize << " points." << std::endl;

	// Delete points and time their deletion
	std::cout << "Beginning deletion of " << benchmarkSize << " points..." << std::endl;
	for (unsigned i = 0; i < benchmarkSize; ++i)
	{
		// Insert
		// Rectangle searchRectangle = Rectangle(points[i], 0.0, 0.0);
		// assert(rtree->search(searchRectangle).size() == 1);
		std::cout << "Point[" << i << "] "; points[i].print();
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		rtree->remove(points[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeDeletes += delta.count();
		totalDeletes += 1;
		std::cout << " deleted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished deletion of " << benchmarkSize << " points." << std::endl;	

	// Statistics
	std::cout << "Total time to insert: " << totalTimeInserts << "s" << std::endl;
	std::cout << "Avg time to insert: " << totalTimeInserts / totalInserts << "s" << std::endl;
	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;
	std::cout << "Total time to delete: " << totalTimeDeletes << "s" << std::endl;
	std::cout << "Avg time to delete: " << totalTimeDeletes / totalSearches << "s" << std::endl;
}
