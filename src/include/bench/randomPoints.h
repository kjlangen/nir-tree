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
#include <rstartree/rstartree.h>
#include <nirtree/nirtree.h>

const unsigned BitDataSize = 60000;
const unsigned BitQuerySize = 3164;
const unsigned HazeDataSize = 65365;
const unsigned HazeQuerySize = 3244;
const unsigned CaliforniaDataSize = 1888012;
const unsigned CaliforniaQuerySize = 5974;
const unsigned BiologicalDataSize = 11958999;
const unsigned BiologicalQuerySize = 37844;
const unsigned ForestDataSize = 581012;
const unsigned ForestQuerySize = 1838;
const unsigned CanadaDataSize = 19371405;
const unsigned CanadaQuerySize = 0;

enum BenchType {UNIFORM, SKEW, CLUSTER, CALIFORNIA, BIOLOGICAL, FOREST, CANADA};
enum TreeType {R_TREE, R_PLUS_TREE, R_STAR_TREE, NIR_TREE};

Point *generateUniform(unsigned benchmarkSize, unsigned seed);
Point *generateBits();
Point *generateHaze();
Point *generateCalifornia();
Point *generateBiological();
Point *generateForest();
Point *generateCanada();

Rectangle *generateRectangles(unsigned benchmarkSize, unsigned seed, unsigned rectanglesSize);
Rectangle *generateBitRectangles();
Rectangle *generateHazeRectangles();
Rectangle *generateCaliRectangles();
Rectangle *generateBioRectangles();
Rectangle *generateForestRectangles();
Rectangle *generateCanadaRectangles();

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configF);

#endif
