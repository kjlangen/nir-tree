#include <iostream>
#include <spatialindex/SpatialIndex.h>
#include <bench/randomSquares.h>
#include <util/geometry.h>
#include <cassert>

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

void testGeometryStuff()
{
	// Test set one
	std::cout << "Test Set 1" << std::endl;
	Rectangle r1 = Rectangle(3.0, 3.0, 2.0, 2.0);
	Rectangle r2 = Rectangle(6.0, 6.0, 2.0, 2.0);

	r1.print();
	r2.print();

	std::vector<Rectangle> v = r1.splitRectangle(r2);

	assert(v.size() == 2);
	assert(v[0].radiusX == 0.5);
	assert(v[0].radiusY == 1.5);
	assert(v[1].radiusX == 1.5);
	assert(v[1].radiusY == 2.0);
	v[0].print();
	v[1].print();

	// Test set two
	std::cout << "Test Set 2" << std::endl;
	r1 = Rectangle(2.0, 2.0, 2.0, 2.0);
	r2 = Rectangle(3.5, -0.5, 2.5, 1.5);

	r1.print();
	r2.print();

	v = r1.splitRectangle(r2);

	assert(v.size() == 2);
	assert(v[0].radiusX == 1.5);
	assert(v[0].radiusY == 1.5);
	assert(v[1].radiusX == 0.5);
	assert(v[1].radiusY == 2.0);
	v[0].print();
	v[1].print();

	// Test set three
	std::cout << "Test Set 3" << std::endl;
	r1 = Rectangle(-3.5, -2.5, 3.5, 2.5);
	r2 = Rectangle(-6, -5.5, 2, 2.5);

	r1.print();
	r2.print();

	v = r1.splitRectangle(r2);

	assert(v.size() == 2);
	assert(v[0].radiusX == 1.5);
	assert(v[0].radiusY == 1.5);
	assert(v[1].radiusX == 2.0);
	assert(v[1].radiusY == 2.5);
	v[0].print();
	v[1].print();

	// Test set four
	std::cout << "Test Set 4" << std::endl;
	r1 = Rectangle(2.0, 4.0, 2.0, 4.0);
	r2 = Rectangle(5.0, 5.0, 2.0, 2.0);

	r1.print();
	r2.print();

	v = r1.splitRectangle(r2);

	assert(v.size() == 3);
	assert(v[0].radiusX == 2.0);
	assert(v[0].radiusY == 0.5);
	assert(v[1].radiusX == 1.5);
	assert(v[1].radiusY == 2.0);
	assert(v[2].radiusX == 2.0);
	assert(v[2].radiusY == 1.5);
	v[0].print();
	v[1].print();
	v[2].print();

	// Test set five
	std::cout << "Test Set 5" << std::endl;
	r1 = Rectangle(2.0, 4.0, 2.0, 4.0);
	r2 = Rectangle(-1.0, 5.0, 2.0, 2.0);

	r1.print();
	r2.print();

	v = r1.splitRectangle(r2);

	assert(v.size() == 3);
	assert(v[0].radiusX == 2.0);
	assert(v[0].radiusY == 0.5);
	assert(v[1].radiusX == 1.5);
	assert(v[1].radiusY == 2.0);
	assert(v[2].radiusX == 2.0);
	assert(v[2].radiusY == 1.5);
	v[0].print();
	v[1].print();
	v[2].print();

	// Test set six
	std::cout << "Test Set 6" << std::endl;
	r1 = Rectangle(4.0, 2.0, 4.0, 2.0);
	r2 = Rectangle(3.0, 4.0, 2.0, 2.0);

	r1.print();
	r2.print();

	v = r1.splitRectangle(r2);

	assert(v.size() == 3);
	assert(v[0].radiusX == 0.5);
	assert(v[0].radiusY == 2.0);
	assert(v[1].radiusX == 2.0);
	assert(v[1].radiusY == 1.0);
	assert(v[2].radiusX == 1.5);
	assert(v[2].radiusY == 2.0);
	v[0].print();
	v[1].print();
	v[2].print();
}

int main(int argc, char const *argv[])
{
	// Argument parsing and switching goes here

	// Fiddle around with the RTree as provided by spatial index
	testLibSpatialIndex();

	// Fiddle around with the geometry objects as provided by util
	testGeometryStuff();

	// Now we assume the randomSquares benchmark is requested
	// Benchmark::benchmarkRandomSquares();

	return 0;
}
