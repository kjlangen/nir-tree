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
#include <nirtreedisk/nirtreedisk.h>
#include <quadtree/quadtree.h>
#include <revisedrstartree/revisedrstartree.h>
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
const unsigned CanadaQuerySize = 5000;
const unsigned GaiaDataSize = 18084053;
const unsigned GaiaQuerySize = 5000;
const unsigned MicrosoftBuildingsDataSize = 752704741;

enum BenchType {UNIFORM, SKEW, CLUSTER, CALIFORNIA, BIOLOGICAL, FOREST, CANADA, GAIA, MICROSOFTBUILDINGS};
enum TreeType {R_TREE, R_PLUS_TREE, R_STAR_TREE, NIR_TREE, QUAD_TREE, REVISED_R_STAR_TREE};

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configD);

// Tags defining how the benchmark is generated
namespace BenchTag
{
	struct DistributionGenerated {};
	struct FileBackedReadAll {};
	struct FileBackedReadChunksAtATime {};
	struct Error {};
};

// Classes for each benchmark with their relevant constants
namespace BenchTypeClasses
{
	class Benchmark {};

	class Uniform : public Benchmark
	{
		public:
			static unsigned size;
			static unsigned dimensions;
			static unsigned seed;
			static constexpr char fileName[] = "";
	};

	class Skew : public Benchmark
	{
		public:
			static constexpr unsigned size = BitDataSize;
			static constexpr unsigned querySize = BitQuerySize;
			static constexpr unsigned dimensions = 2;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/bits02";

	};

	class California : public Benchmark
	{
		public:
			static constexpr unsigned size = CaliforniaDataSize;
			static constexpr unsigned querySize = CaliforniaQuerySize;
			static constexpr unsigned dimensions = 2;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/california";
	};

	class Biological: public Benchmark
	{
		public:
			static constexpr unsigned size = BiologicalDataSize;
			static constexpr unsigned querySize = BiologicalQuerySize;
			static constexpr unsigned dimensions = 3;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/biological";
	};

	class Forest : public Benchmark
	{
		public:
			static constexpr unsigned size = ForestDataSize;
			static constexpr unsigned querySize = ForestQuerySize;
			static constexpr unsigned dimensions = 5;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/forest";
	};

	class Canada : public Benchmark
	{
		public:
			static constexpr unsigned size = CanadaDataSize;
			static constexpr unsigned querySize = CanadaQuerySize;
			static constexpr unsigned dimensions = 2;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/canada";
	};

	class Gaia : public Benchmark
	{
		public:
			static constexpr unsigned size = GaiaDataSize;
			static constexpr unsigned querySize = GaiaQuerySize;
			static constexpr unsigned dimensions = 3;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/gaia";
	};

	class MicrosoftBuildings : public Benchmark
	{
		public:
			static constexpr unsigned size = MicrosoftBuildingsDataSize;
			static constexpr unsigned querySize = 0;
			static constexpr unsigned dimensions = 2;
			static constexpr char fileName[] = "/home/bjglasbe/Documents/code/nir-tree/data/microsoftbuildings";
	};
};


// Mappings from each benchmark type to its tag
namespace BenchDetail
{
	template <typename T>
	struct getBenchTag : BenchTag::Error {};

	template <>
	struct getBenchTag<BenchTypeClasses::Uniform> : BenchTag::DistributionGenerated {};

	template <>
	struct getBenchTag<BenchTypeClasses::Skew> : BenchTag::FileBackedReadAll {};

	template <>
	struct getBenchTag<BenchTypeClasses::California> : BenchTag::FileBackedReadAll {};

	template <>
	struct getBenchTag<BenchTypeClasses::Biological> : BenchTag::FileBackedReadAll {};

	template <>
	struct getBenchTag<BenchTypeClasses::Forest> : BenchTag::FileBackedReadAll {};

	template <>
	struct getBenchTag<BenchTypeClasses::Canada> : BenchTag::FileBackedReadAll {};

	template <>
	struct getBenchTag<BenchTypeClasses::Gaia> : BenchTag::FileBackedReadAll {};

	template <>
	struct getBenchTag<BenchTypeClasses::MicrosoftBuildings> : BenchTag::FileBackedReadChunksAtATime {};

}

template <typename T>
class PointGenerator
{
	private:
		PointGenerator(BenchTag::DistributionGenerated);
		PointGenerator(BenchTag::FileBackedReadAll);
		PointGenerator(BenchTag::FileBackedReadChunksAtATime);

		void reset(BenchTag::DistributionGenerated);
		void reset(BenchTag::FileBackedReadAll);
		void reset(BenchTag::FileBackedReadChunksAtATime);
		std::optional<Point> nextPoint(BenchTag::DistributionGenerated);
		std::optional<Point> nextPoint(BenchTag::FileBackedReadAll);
		std::optional<Point> nextPoint(BenchTag::FileBackedReadChunksAtATime);

		// Class members
		unsigned benchmarkSize;
		unsigned seed;
		std::fstream backingFile;
		unsigned offset;

		std::vector<Point> pointBuffer;

	public:
		static_assert(std::is_base_of<BenchTypeClasses::Benchmark, T>::value && 
			!std::is_same<T,BenchTypeClasses::Benchmark>::value, "PointGenerator must take a Benchmark subclass");

		PointGenerator() : PointGenerator(BenchDetail::getBenchTag<T>{})
		{
			if (T::dimensions != 0 && T::dimensions != DIM)
			{
				throw std::runtime_error( "Wrong number of dimensions configured." );
			}
		}

		std::optional<Point> nextPoint();
		void reset();
};
#endif
