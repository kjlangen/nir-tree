#ifndef __RANDOMPOINTS__
#define __RANDOMPOINTS__

#include <map>
#include <cmath>
#include <chrono>
#include <random>
#include <string>
#include <iostream>
#include <fstream>
#include <index/index.h>
#include <rtree/rtree.h>
#include <rplustree/rplustree.h>
#include <nirtree/nirtree.h>

const unsigned CaliforniaDataSize = 1888012;
const unsigned CaliforniaQuerySize = 5974;
const unsigned BiologicalDataSize = 11958999;
const unsigned BiologicalQuerySize = 37844;
const unsigned ForestDataSize = 581012;
const unsigned ForestQuerySize = 1838;

enum BenchType {UNIFORM, SKEW, CLUSTER, CALIFORNIA, BIOLOGICAL, FOREST};
enum TreeType {R_TREE, R_PLUS_TREE, NIR_TREE};

Point *generateUniform(unsigned benchmarkSize, unsigned seed);
Point *generateSkewed(unsigned benchmarkSize, unsigned seed, double skewFactor);
Point *generateClustered(unsigned benchmarkSize, unsigned seed, unsigned clusterCount);
Point *generateCalifornia();
Point *generateBiological();

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configF);

#endif
