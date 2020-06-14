#include <iostream>
#include <unistd.h>
#include <spatialindex/SpatialIndex.h>
#include <rtree/node.h>
#include <util/geometry.h>
#include <bench/randomSquares.h>
#include <bench/randomPoints.h>

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

void testRTree()
{
	// Unit test RTree
	testBoundingBox();
	testUpdateBoundingBox();
	testRemoveChild();
	testRemoveData();
	testChooseLeaf();
	testFindLeaf();
	testSplitNode();
	testAdjustTree();
	testCondenseTree();
	testSearch();
	testInsert();
	testRemove();

	std::cout << "RTree tested." << std::endl;
}

int main(int argc, char *argv[])
{
	/* getopt example
	while ((c = getopt (argc, argv, "abc:")) != -1)
	{
		switch (c)
		{
			case 'a':
				aflag = 1;
				break;
			case 'b':
				bflag = 1;
				break;
			case 'c':
				cvalue = optarg;
				break;
			case '?':
				if (optopt == 'c')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint (optopt))
					fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				return 1;
			default:
				abort();
		}
	}
    */

	// Argument parsing and switching
	bool runUnitTests = false;
	bool runBenchMark = false;
	// string benchmark;
	// string structure;

	for (int option = getopt(argc, argv, "ub"); option != -1; option = getopt(argc, argv, "ub"))
	{
		switch (option)
		{
			case 'u':
				runUnitTests = true;
				break;
			case 'b':
				runBenchMark = true;
				break;
			default:
				std::cout << "Options are:\n\t-u for unit tests\n\t-b for benchmarks" << std::endl;
				return 0;
		}
	}


	// First unit tests are run and then benchmarks are run
	if (runUnitTests)
	{
		testRTree();
	}

	if (runBenchMark)
	{
		RTree rTree(750, 1500);
		randomPoints(rTree, 2000000);
	}

	if (!runUnitTests && !runBenchMark)
	{
		std::cout << "No option selected, exiting." << std::endl;
	}

	return 0;
}
