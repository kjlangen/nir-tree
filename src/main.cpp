#include <iostream>
#include <spatialindex/SpatialIndex.h>

#include <rtree/rtree.h>
#include <bench/randomPoints.h>
#include <rplustree/rPlusTree.h>
#include <nirtree/nirtree.h>

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

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

enum TreeType { NONE = 0, R_PLUS_TREE, R_STAR_TREE, NIR_TREE };

void parameters(TreeType type, int a, int b, int s, int l) {
	std::string treeTypes[] = {"NONE", "R_PLUS_TREE", "R_STAR_TREE", "NIR_TREE"};
	std::cout << "### TEST PARAMETERS ###" << std::endl;
	std::cout << "  tree = " << treeTypes[type] << std::endl;
	std::cout << "  a = " << a << "; b = " << b << std::endl;
	std::cout << "  s = " << s << "; l = " << l << std::endl;
	std::cout << "### ### ### ### ### ###" << std::endl;
}

int main(int argc, char *argv[])
{
	Catch::Session session;
#ifdef UNIT_TESTING
	return session.run(argc, argv);
#else
	// process command line options
	int option, a = 750, b = 1500, s = 10000, l = 1000;
	TreeType type = NONE;
	while ((option = getopt(argc, argv, "t:a:b:s:l:")) != -1)
	{
		switch (option) {
			case 't': // t: tree type
			{
				type = (TreeType)atoi(optarg);
				break;
			}
			case 'a': // a: min branch factor
			{
				a = atoi(optarg);
				break;
			}
			case 'b': // b: max branch factor
			{
				b = atoi(optarg);
				break;
			}
			case 's': // s: benchmark size
			{
				s = atoi(optarg);
				break;
			}
			case 'l': // l: log frequency
			{
				l = atoi(optarg);
				break;
			}
			default:
			{
				std::cout << "Unknown option. Exiting." << std::endl;
				return 1;
			}
		}
	}

	// print test parameters
	parameters(type, a, b, s, l);

	// run benchmarking
	switch (type) {
		case R_PLUS_TREE:
		{
			rplustree::Tree rPlusTree(a, b);
			randomPoints(rPlusTree, s, l);
			break;
		}
		case R_STAR_TREE:
		{
			rtree::RTree rTree(a, b);
			randomPoints(rTree, s, l);
			break;
		}
		case NIR_TREE:
		{
			nirtree::NIRTree nirTree(a, b);
			randomPoints(nirTree, s, l);
			break;
		}
		default:
		{
			std::cout << "Tree not selected. Exiting." << std::endl;
			return 1;
		}
	}

	return 0;
#endif
}
