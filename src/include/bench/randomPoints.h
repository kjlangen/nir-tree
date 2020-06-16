#ifndef __RANDOMPOINTS__
#define __RANDOMPOINTS__
#include <iostream>
#include <random>
#include <chrono>
#include <rtree/rtree.h>
#include <index/index.h>

void randomPoints(Index& spatialIndex, unsigned benchmarkSize = 2000000);

#endif
