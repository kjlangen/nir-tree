#ifndef __RANDOMPOINTS__
#define __RANDOMPOINTS__

#include <iostream>
#include <random>
#include <chrono>
#include <cmath>
#include <index/index.h>
#include <rtree/rtree.h>
#include <nirtree/nirtree.h>

enum BenchType { UNIFORM, SKEW, CLUSTER };

Point *generateUniform(unsigned benchmarkSize, unsigned seed);
Point *generateSkewed(unsigned benchmarkSize, unsigned seed, float skewFactor);
Point *generateClustered(unsigned benchmarkSize, unsigned seed, unsigned clusterCount);

void randomPoints(Index& spatialIndex, BenchType benchmark, unsigned benchmarkSize, unsigned logFrequency, unsigned seed);

#endif
