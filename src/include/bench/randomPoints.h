#ifndef __RANDOMPOINTS__
#define __RANDOMPOINTS__

#include <map>
#include <cmath>
#include <chrono>
#include <random>
#include <string>
#include <iostream>
#include <index/index.h>
#include <rtree/rtree.h>
#include <rplustree/rplustree.h>
#include <nirtree/nirtree.h>

enum BenchType {UNIFORM, SKEW, CLUSTER};
enum TreeType {R_TREE, R_PLUS_TREE, NIR_TREE};

Point *generateUniform(unsigned benchmarkSize, unsigned seed);
Point *generateSkewed(unsigned benchmarkSize, unsigned seed, float skewFactor);
Point *generateClustered(unsigned benchmarkSize, unsigned seed, unsigned clusterCount);

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, float> &configF);

#endif
