#include <bench/randomSquares.h>

// To generate synthetic 3D datasets we distribute spatial boxes with each side of uniform
// random length (between 0 and 1) in a constant space of 1000 space units in each of the three
// dimensions. We use three different distributions, namely uniform, Gaussian (μ = 500, σ = 250)
// and clustered to generate datasets containing from 160K to 960K objects and from
// 1.6 to 9.6 million objects. The clustered distribution uniformly randomly chooses up to 100
// locations in 3D space around which the objects are distributed with a
// Gaussian distribution (μ = 0, σ = 220).

class DefaultVisitor : public SpatialIndex::IVisitor
{
	public:
	size_t m_indexIO{0};
	size_t m_leafIO{0};

    DefaultVisitor() = default;

	void visitNode(const SpatialIndex::INode& n) override
	{
		if (n.isLeaf()) m_leafIO++;
		else m_indexIO++;
	}

	void visitData(const SpatialIndex::IData& d) override
	{
		SpatialIndex::IShape* pS;
		d.getShape(&pS);
			// do something.
		delete pS;

		// data should be an array of characters representing a Region as a string.
		uint8_t* pData = nullptr;
		uint32_t cLen = 0;
		d.getData(cLen, &pData);
		// do something.
		//string s = reinterpret_cast<char*>(pData);
		//cout << s << endl;
		delete[] pData;

		std::cout << d.getIdentifier() << std::endl;
			// the ID of this data entry is an answer to the query. I will just print it to stdout.
	}

	void visitData(std::vector<const SpatialIndex::IData*>& v) override
	{
		std::cout << v[0]->getIdentifier() << " " << v[1]->getIdentifier() << std::endl;
	}
};

void Benchmark::benchmarkRandomSquares()
{
	// Setup random generators
	std::default_random_engine generator;
	std::uniform_real_distribution<double> pointDist(0.0, 999.0);
	std::uniform_real_distribution<double> sideDist(0.0, 1.0);
	std::cout << "Uniformly distributing squares between positions (0,0) and (999, 999)." << std::endl;
	std::cout << "Unifromly sizing squares between 0 and 1." << std::endl;

	// Setup squares
	// Number of squares
	int benchmarkSize = 5000000;
	// Initialize squares
	SpatialIndex::Region **squares = new SpatialIndex::Region *[benchmarkSize];
	std::cout << "Beginning initialization of " << benchmarkSize << " squares..." << std::endl;
	for (int i = 0; i < benchmarkSize; ++i)
	{
		double rLowCoords[] = {pointDist(generator), pointDist(generator)};
		double sideLength = sideDist(generator);
		double rHighCoords[] = {rLowCoords[0] + sideLength, rLowCoords[1] + sideLength};
		squares[i] = new SpatialIndex::Region(rLowCoords, rHighCoords, 2);

		std::cout << "Square[" << (int)i << "] = " << *squares[i] << std::endl;
	}
	std::cout << "Finished initialization of " << benchmarkSize << " squares." << std::endl;

	// Setup statistics
	double totalTimeInserts = 0.0;
	double totalTimeSearches = 0.0;
	double totalInserts = 0.0;
	double totalSearches = 0.0;

	// Setup the RTree
	SpatialIndex::IStorageManager *memStore = SpatialIndex::StorageManager::createNewMemoryStorageManager();
	SpatialIndex::id_type benchmarkTreeId = 4;
	SpatialIndex::ISpatialIndex *benchmarkTree = SpatialIndex::RTree::createNewRTree(*memStore, 0.5, 1000, 1000, 2, SpatialIndex::RTree::RV_RSTAR, benchmarkTreeId);
	SpatialIndex::IVisitor *defaultVisitor = new DefaultVisitor();

	// Insert squares and time their insertion
	std::cout << "Beginning insertion of " << benchmarkSize << " squares..." << std::endl;
	for (SpatialIndex::id_type i = 0; i < benchmarkSize; ++i)
	{
		// Insert
		uint8_t benchmarkData[] = {42};
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		benchmarkTree->insertData(1, benchmarkData, *squares[i], i);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeInserts += delta.count();
		totalInserts += 1;
		std::cout << "Square[" << i << "] inserted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished insertion of " << benchmarkSize << " squares." << std::endl;

	// Search for squares and time their retrieval
	std::cout << "Beginning search for " << benchmarkSize << " squares..." << std::endl;
	for (SpatialIndex::id_type i = 0; i < benchmarkSize; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		benchmarkTree->intersectsWithQuery(*squares[i], *defaultVisitor);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeSearches += delta.count();
		totalSearches += 1;
		std::cout << "Square[" << i << "] queried." << delta.count() << " s" << std::endl;
	}
	std::cout << "Finished searching for " << benchmarkSize << " squares." << std::endl;

	// Statistics
	std::cout << "Total time to insert: " << totalTimeInserts << "s" << std::endl;
	std::cout << "Avg time to insert: " << totalTimeInserts / totalInserts << "s" << std::endl;
	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;
	std::cout << "RTree is: " << std::endl << *benchmarkTree << std::endl;
}
