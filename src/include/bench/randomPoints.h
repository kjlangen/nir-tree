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

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configF);

namespace BenchTag {
    struct DistributionGenerated{};
    struct FileBackedReadAll{};
    struct FileBackedReadChunksAtATime{};
    struct Error{};
};

namespace xxBenchType {

    class Benchmark {};

    class Uniform : public Benchmark {
    public:
        static constexpr unsigned size = 0;

        static std::string getFileName() {
            throw std::runtime_error( "Uniform distribution not backed by file." );
        }
    };

    class Skew : public Benchmark {
    public:
        static constexpr unsigned size = BitDataSize;
        static constexpr unsigned querySize = BitQuerySize;
        static std::string getFileName() {
            return "/hdd1/nir-tree/data/bits02";
        }

    };

    class California : public Benchmark {
    public:
        static constexpr unsigned size = CaliforniaDataSize;
        static constexpr unsigned querySize = CaliforniaQuerySize;
        static std::string getFileName() {
            return "/hdd1/nir-tree/data/california";
        }
    };



    class Biological: public Benchmark {
    public:
        static constexpr unsigned size = BiologicalDataSize;
        static constexpr unsigned querySize = BiologicalQuerySize;

        static std::string getFileName() {
            return "/hdd1/nir-tree/data/biological";
        }
    };

    class Forest : public Benchmark {
    public:
        static constexpr unsigned size = ForestDataSize;
        static constexpr unsigned querySize = ForestQuerySize;

        static std::string getFileName() {
            return "/hdd1/nir-tree/data/forest";
        }
    };

    class Canada : public Benchmark {
    public:
        static constexpr unsigned size = CanadaDataSize;
        static constexpr unsigned querySize = 0;

        static std::string getFileName() {
            return "/hdd1/nir-tree/data/canada";
        }
    };

    class MicrosoftBuildings : public Benchmark {
    public:
        static constexpr unsigned size = MicrosoftBuildingsDataSize;
        static constexpr unsigned querySize = 0;

        static std::string getFileName() {
            return "/hdd1/nir-tree/data/microsoft";
        }
    };
};


namespace BenchDetail {

    template <typename T>
    struct getBenchTag : BenchTag::Error{};

    template <>
    struct getBenchTag<xxBenchType::Uniform> : BenchTag::DistributionGenerated{};

    template <>
    struct getBenchTag<xxBenchType::Skew> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<xxBenchType::California> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<xxBenchType::Biological> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<xxBenchType::Forest> : BenchTag::FileBackedReadAll{};

    template <>
    struct getBenchTag<xxBenchType::Canada> : BenchTag::FileBackedReadChunksAtATime{};

    template <>
    struct getBenchTag<xxBenchType::MicrosoftBuildings> : BenchTag::FileBackedReadChunksAtATime{};

}

template <typename T>
class PointGenerator {
public:

    static_assert( std::is_base_of<xxBenchType::Benchmark, T>::value and not std::is_same<T,xxBenchType::Benchmark>::value, "PointGenerator must take a Benchmark subclass" );

    static_assert( std::is_base_of<BenchTag::DistributionGenerated, BenchDetail::getBenchTag<xxBenchType::Uniform>>::value );

    // You may only create a PointGenerator with a seed if it is distribution-generated
    // Rebind typename U because T is instantiated at this point, need to get typename back
    template <typename U = T>
    PointGenerator( unsigned benchmarkSize, unsigned seed, typename std::enable_if<std::is_base_of<BenchTag::DistributionGenerated, BenchDetail::getBenchTag<U>>::value>::type* = 0 ) :
        benchmarkSize( T::size ), seed( seed ), offset( 0 )
    {
    }

    // You may only create PointGenerator with a backing file if the benchmark is backed by a file
    template <typename U = T>
    PointGenerator( unsigned benchmarkSize, std::fstream &&backingFile, typename std::enable_if<std::is_base_of<BenchTag::FileBackedReadAll, BenchDetail::getBenchTag<U>>::value or std::is_base_of<BenchTag::FileBackedReadChunksAtATime, BenchDetail::getBenchTag<U>>::value>::type* = 0 ):
        benchmarkSize( T::size ), backingFile( std::move( backingFile ) ), offset( 0 )
    {
    }

    std::optional<Point> nextPoint();
    void reset();
private:

    void reset( BenchTag::DistributionGenerated );
    void reset( BenchTag::FileBackedReadAll );
    void reset( BenchTag::FileBackedReadChunksAtATime );
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
