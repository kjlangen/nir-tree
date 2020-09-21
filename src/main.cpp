#include <iostream>
#include <spatialindex/SpatialIndex.h>
#include <rtree/rtree.h>
#include <nirtree/nirtree.h>
#include <rplustree/rPlusTree.h>
#include <nirtree/pencilPrinter.h>
#include <util/geometry.h>
#include <bench/randomSquares.h>
#include <bench/randomDisjointSquares.h>
#include <bench/randomPoints.h>
#include <bench/splitPoints.h>
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

void testLibSpatialIndex()
{
	// Fiddle with the basic shapes a bit
	double testPtCoords[] = {16.1, 17.8};
	std::cout << testPtCoords << std::endl;
	SpatialIndex::Point *testPt = new SpatialIndex::Point(testPtCoords, 2);
	SpatialIndex::Point *newTestPt = testPt->clone();
	std::cout << testPt << std::endl;
	std::cout << newTestPt << std::endl;

	double testRegionLowCoords[] = {0.0, 0.0};
	double testRegionHighCoords[] = {20.0, 20.0};
	SpatialIndex::Region *testRegion = new SpatialIndex::Region(testRegionLowCoords, testRegionHighCoords, 2);
	bool collisionCheck = testPt->intersectsShape(*testRegion);
	std::cout << "Test point intersects test region? " << collisionCheck << std::endl;

	delete testPt;
	delete newTestPt;
	delete testRegion;

	// Fiddle with the RTree a bit
	SpatialIndex::IStorageManager *memStore = SpatialIndex::StorageManager::createNewMemoryStorageManager();
	SpatialIndex::id_type pirateId = 0;
	SpatialIndex::ISpatialIndex *pirateTree = SpatialIndex::RTree::createNewRTree(*memStore, 0.5, 4, 4, 2, SpatialIndex::RTree::RV_RSTAR, pirateId);

	// Make two data points with appropriate shapes

	// Data + Shape + Id
	uint8_t r1Data[] = {8};
	double r1LowCoords[] = {2.0, 2.0};
	double r1HighCoords[] = {4.0, 5.0};
	SpatialIndex::Region *r1 = new SpatialIndex::Region(r1LowCoords, r1HighCoords, 2);
	SpatialIndex::id_type r1Id = 0;

	// Data + Shape + Id
	uint8_t r2Data[] = {200};
	double r2LowCoords[] = {10.0, 18.0};
	double r2HighCoords[] = {12.0, 20.0};
	SpatialIndex::Region *r2 = new SpatialIndex::Region(r2LowCoords, r2HighCoords, 2);
	SpatialIndex::id_type r2Id = 1;

	std::cout << "First rounds of inserts" << std::endl;

	// Insert r1
	pirateTree->insertData(1, r1Data, *r1, r1Id);
	// Insert r2
	pirateTree->insertData(1, r2Data, *r2, r2Id);

	std::cout << "RTree is: " << std::endl << *pirateTree << std::endl;

	// Add three more 1x1 squares
	uint8_t r3Data[] = {104};
	double r3LowCoords[] = {13.0, 1.0};
	double r3HighCoords[] = {14.0, 2.0};
	SpatialIndex::Region *r3 = new SpatialIndex::Region(r3LowCoords, r3HighCoords, 2);
	SpatialIndex::id_type r3Id = 2;

	uint8_t r4Data[] = {27};
	double r4LowCoords[] = {16.0, 4.0};
	double r4HighCoords[] = {17.0, 5.0};
	SpatialIndex::Region *r4 = new SpatialIndex::Region(r4LowCoords, r4HighCoords, 2);
	SpatialIndex::id_type r4Id = 3;

	uint8_t r5Data[] = {32};
	double r5LowCoords[] = {18.0, 6.0};
	double r5HighCoords[] = {19.0, 7.0};
	SpatialIndex::Region *r5 = new SpatialIndex::Region(r5LowCoords, r5HighCoords, 2);
	SpatialIndex::id_type r5Id = 4;

	std::cout << "Second round of inserts" << std::endl;

	// Insert r3
	pirateTree->insertData(1, r3Data, *r3, r3Id);
	// Insert r4
	pirateTree->insertData(1, r4Data, *r4, r4Id);
	// Insert r5
	pirateTree->insertData(1, r5Data, *r5, r5Id);

	std::cout << "RTree is: " << std::endl << *pirateTree << std::endl;

	std::cout << "First round of deletes" << std::endl;

	// Delete r1
	pirateTree->deleteData(*r1, r1Id);
	// Delete r2
	pirateTree->deleteData(*r2, r2Id);

	std::cout << "RTree is: " << std::endl << *pirateTree << std::endl;

	std::cout << "Second round of deletes" << std::endl;

	// Delete r3
	pirateTree->deleteData(*r3, r3Id);
	// Delete r4
	pirateTree->deleteData(*r4, r4Id);
	// Delete r5
	pirateTree->deleteData(*r5, r5Id);

	std::cout << "RTree is: " << std::endl << *pirateTree << std::endl;

	delete pirateTree;
}

enum TreeType { R_TREE, R_PLUS_TREE, NIR_TREE };

void parameters(TreeType type, BenchType bench, unsigned a, unsigned b, unsigned n, unsigned l, unsigned s) {
	std::string treeTypes[] = {"R_TREE", "R_PLUS_TREE", "NIR_TREE"};
	std::string benchTypes[] = { "UNIFORM", "SKEW", "CLUSTER" };
	std::cout << "### TEST PARAMETERS ###" << std::endl;
	std::cout << "  tree = " << treeTypes[type] << std::endl;
	std::cout << "  bench = " << benchTypes[bench] << std::endl;
	std::cout << "  a = " << a << "; b = " << b << std::endl;
	std::cout << "  n = " << n << "; l = " << l << std::endl;
	std::cout << "  s = " << s << ";" << std::endl;
	std::cout << "### ### ### ### ### ###" << std::endl;
}

int main(int argc, char *argv[])
{
	Catch::Session session;
#ifdef UNIT_TESTING
	return session.run(argc, argv);
#else
	// Process command line options
	int option, a = 250, b = 500, n = 10000, l = 1000, s = 3141;
	TreeType type = NIR_TREE;
	BenchType bench = UNIFORM;
	while ((option = getopt(argc, argv, "t:m:a:b:n:l:s:")) != -1)
	{
		switch (option)
		{
			case 't': // Tree type
			{
				type = (TreeType)atoi(optarg);
				break;
			}
			case 'm': // Benchmark type
			{
				bench = (BenchType)atoi(optarg);
				break;
			}
			case 'a': // Minimum branch factor
			{
				a = atoi(optarg);
				break;
			}
			case 'b': // Maximum branch factor
			{
				b = atoi(optarg);
				break;	
			}
			case 'n': // Benchmark size
			{
				n = atoi(optarg);
				break;
			}
			case 'l': // Log frequency
			{
				l = atoi(optarg);
				break;
			}
			case 's': // Benchmark seed
			{
				s = atoi(optarg);
				break;
			}
			default:
			{
				std::cout << "Bad option. Exiting." << std::endl;
				return 1;
			}
		}
	}

	// Print test parameters
	parameters(type, bench, a, b, n, l, s);

	// Run benchmarking
	switch (type) {
		case R_TREE:
		{
			rtree::RTree rt(a, b);
			randomPoints(rt, bench, n, s, l);
			break;
		}
		case R_PLUS_TREE:
		{
			rplustree::RPlusTree rpt(a, b);
			randomPoints(rpt, bench, n, s, l);
			break;
		}
		case NIR_TREE:
		{
			nirtree::NIRTree nt(a, b);
			randomPoints(nt, bench, n, s, l);
			break;
		}
		default:
		{
			assert(false);
		}
	}
#endif
}
