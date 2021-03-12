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
#include <optional>

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
const unsigned MicrosoftBuildingsDataSize = 752704741;

enum BenchType {UNIFORM, SKEW, CLUSTER, CALIFORNIA, BIOLOGICAL, FOREST, CANADA, MICROSOFTBUILDINGS};
enum TreeType {R_TREE, R_PLUS_TREE, R_STAR_TREE, NIR_TREE};

Point *generateUniform(unsigned benchmarkSize, unsigned seed);
Point *generateBits();
Point *generateHaze();
Point *generateCalifornia();
Point *generateBiological();
Point *generateForest();
Point *generateCanada();
Point *generateMicrosoftBuildings();

Rectangle *generateRectangles(unsigned benchmarkSize, unsigned seed, unsigned rectanglesSize);
Rectangle *generateBitRectangles();
Rectangle *generateHazeRectangles();
Rectangle *generateCaliRectangles();
Rectangle *generateBioRectangles();
Rectangle *generateForestRectangles();

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configF);

namespace BenchTag {
    struct DistributionGenerated{};
    struct FileBackedReadAll{};
    struct FileBackedReadChunksAtATime{};
    struct Error{};
};


namespace BenchDetail {

    template <BenchType bt>
    struct getBenchTag : BenchTag::Error{};

    template <>
    struct getBenchTag<UNIFORM> : BenchTag::DistributionGenerated{};

    // Sort of, but it also reads crap from a file
    template <>
    struct getBenchTag<SKEW> : BenchTag::DistributionGenerated{};

    template <>
    struct getBenchTag<CALIFORNIA> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<BIOLOGICAL> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<FOREST> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<CANADA> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<MICROSOFTBUILDINGS> : BenchTag::FileBackedReadAll{};

}

template <BenchType bt>
class PointGenerator {
public:

    PointGenerator( unsigned benchmarkSize, unsigned seed ) :
        benchmarkSize( benchmarkSize ), seed( seed ), offset( 0 )
    {
    }

    PointGenerator( unsigned benchmarkSize, std::fstream &&backingFile ):
        benchmarkSize( benchmarkSize ), backingFile( std::move( backingFile ) ), offset( 0 )
    {
    }

    std::optional<Point> nextPoint();
    void reset();
private:

    void reset( BenchTag::DistributionGenerated );
    void reset( BenchTag::FileBackedReadAll );
    std::optional<Point> nextPoint( BenchTag::DistributionGenerated );
    std::optional<Point> nextPoint( BenchTag::FileBackedReadAll );
    std::optional<Point> nextPoint( BenchTag::FileBackedReadChunksAtATime );

    // Class members

    unsigned benchmarkSize;
    unsigned seed;
    std::fstream backingFile;
    unsigned offset;

    std::vector<Point> pointBuffer;
};

#endif
