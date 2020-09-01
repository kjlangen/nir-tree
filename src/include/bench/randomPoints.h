#ifndef __RANDOMPOINTS__
#define __RANDOMPOINTS__

#include <iostream>
#include <random>
#include <chrono>
#include <index/index.h>
#include <rtree/rtree.h>
#include <nirtree/nirtree.h>

void randomPoints(Index& spatialIndex, unsigned benchmarkSize, unsigned logFrequency, unsigned seed);

#endif
