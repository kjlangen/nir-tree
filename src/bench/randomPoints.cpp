#include <bench/randomPoints.h>
#include <unistd.h>
#include <algorithm>

unsigned BenchTypeClasses::Uniform::size = 10000;
unsigned BenchTypeClasses::Uniform::dimensions = dimensions;
unsigned BenchTypeClasses::Uniform::seed = 3141;

static void fileGoodOrDie(std::fstream &file)
{
	if (!file.good())
	{
		std::cout << "Could not read from file: " << std::endl;
		exit(1);
	}
}

template <typename T>
PointGenerator<T>::PointGenerator(BenchTag::DistributionGenerated) :
	benchmarkSize(T::size), offset(0)
{
}

template <typename T>
PointGenerator<T>::PointGenerator(BenchTag::FileBackedReadAll) :
	benchmarkSize(T::size), backingFile(T::fileName), offset(0)
{
}

template <typename T>
PointGenerator<T>::PointGenerator(BenchTag::FileBackedReadChunksAtATime) :
    benchmarkSize(T::size), backingFile(T::fileName), offset(0)
{
}


template <typename T>
void PointGenerator<T>::reset(BenchTag::DistributionGenerated)
{
	offset = 0;
}

template <typename T>
void PointGenerator<T>::reset(BenchTag::FileBackedReadAll)
{
	offset = 0;
}

template <typename T>
void PointGenerator<T>::reset(BenchTag::FileBackedReadChunksAtATime)
{
	offset = 0;
	backingFile.seekg(0);
}

template <typename T>
void PointGenerator<T>::reset()
{
    reset(BenchDetail::getBenchTag<T>{});
}

template <>
std::optional<Point> PointGenerator<BenchTypeClasses::Uniform>::nextPoint(BenchTag::DistributionGenerated)
{
	// We produce all of the points at once and shove them in the buffer.
	if (pointBuffer.empty())
	{
		std::default_random_engine generator(BenchTypeClasses::Uniform::seed);
		std::uniform_real_distribution<double> pointDist(0.0, 1.0);
		pointBuffer.reserve(benchmarkSize);
		for (unsigned i = 0; i < benchmarkSize; i++)
		{
			Point p;
			for (unsigned d = 0; d < BenchTypeClasses::Uniform::dimensions; d++)
			{
				p[d] = pointDist(generator);
			}
			pointBuffer.push_back(std::move(p));
		}
	} // fall through
	if (offset < pointBuffer.size())
	{
		return pointBuffer[offset++];
	}
	return std::nullopt;
}

template <typename T>
std::optional<Point> PointGenerator<T>::nextPoint(BenchTag::FileBackedReadAll)
{
	if (pointBuffer.empty())
	{
		// We produce all of the points at once, shove them into the buffer.
		fileGoodOrDie(backingFile);

		// Initialize points
		pointBuffer.reserve(benchmarkSize);
		for (unsigned i = 0; i < benchmarkSize; ++i)
		{
			Point p;
			for (unsigned d = 0; d < T::dimensions; ++d)
			{
				fileGoodOrDie(backingFile);
				double dbl;
				backingFile >> dbl;
				p[d] = dbl;
			}
			pointBuffer.push_back(std::move(p));
		}
	}

	if (offset < pointBuffer.size())
	{
		return pointBuffer[offset++];
	}
	return std::nullopt;
}

template <typename T>
std::optional<Point> PointGenerator<T>::nextPoint(BenchTag::FileBackedReadChunksAtATime)
{
	if (offset >= benchmarkSize)
	{
		return std::nullopt;
	}
	if (pointBuffer.empty())
	{
		// Fill it with 10k entries
		pointBuffer.resize(10000);
	}
	if (offset % 10000 == 0)
	{
		// Time to read 10k more things
		// Do the fstream song and dance

		// Initialize points
		for (unsigned i = 0; i < 10000 and offset+i < benchmarkSize; ++i)
		{
			Point p;
			for (unsigned d = 0; d < T::dimensions; ++d)
			{
				fileGoodOrDie(backingFile);
				double dbl;
				backingFile >> dbl;
				p[d] = dbl;
			}
			pointBuffer[i] = p;
		}

	}

	return pointBuffer[offset++ % 10000];
}

template <typename T>
std::optional<Point> PointGenerator<T>::nextPoint()
{
	return nextPoint(BenchDetail::getBenchTag<T>{});
}

static std::vector<Rectangle> generateRectangles(unsigned benchmarkSize, unsigned seed, unsigned rectanglesSize)
{
	std::default_random_engine generator(seed + benchmarkSize);
	std::uniform_real_distribution<double> pointDist(0.0, 1.0);

	// Initialize rectangles
	Point ll;
	Point ur;
    std::vector<Rectangle> rectangles;
    rectangles.reserve( rectanglesSize );
	// Compute the dimensions-th root of a percentage that will give rectangles that in expectation return 1000 points
	double requiredPercentage = 1500.0 / (double) benchmarkSize;
	double root = std::pow(requiredPercentage, 1.0 / (double) dimensions);
	std::cout << "Begnning initialization of " << rectanglesSize << " rectangles with " << requiredPercentage << "% and " << root << "..." << std::endl;
	for (unsigned i = 0; i < rectanglesSize; ++i)
	{
        Rectangle rect;
		// Generate a new point and then create a square from it that covers 5% of the total area
		for (unsigned d = 0; d < dimensions; ++d)
		{
			ll[d] = pointDist(generator);
			ur[d] = ll[d] + root;
		}

		rectangles.push_back( Rectangle(ll, ur) );
	}
	std::cout << "Initialization OK." << std::endl;

	return rectangles;
}

static std::vector<Rectangle> generateBitRectangles()
{
	// Query set is pre-generated and requires 2 or 3 dimensions
	assert(dimensions == 2 || dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = dimensions == 2 ? "/home/bjglasbe/Documents/code/nir-tree/data/bit02.2" : "/home/bjglasbe/Documents/code/nir-tree/data/bit03.2";
	file.open(dataPath.c_str());
	fileGoodOrDie(file);
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
    rectangles.reserve( BitQuerySize );
	std::cout << "Beginning initialization of " << BitQuerySize << " computer rectangles..." << std::endl;
	for (unsigned i = 0; i < BitQuerySize; ++i)
	{
        Rectangle rect;
		for (unsigned d = 0; d < dimensions; ++d)
		{
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.lowerLeft[d] = *doubleBuffer;
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.upperRight[d] = *doubleBuffer;
		}
        rectangles.push_back( rect );
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

static std::vector<Rectangle> generateHazeRectangles()
{
	// Query set is pre-generated and requires 2 or 3 dimensions
	assert(dimensions == 2 || dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = dimensions == 2 ? "/home/bjglasbe/Documents/code/nir-tree/data/pha02.2" : "/home/bjglasbe/Documents/code/nir-tree/data/pha03.2";
	file.open(dataPath.c_str());
	fileGoodOrDie(file);
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
    rectangles.reserve( HazeQuerySize );
	std::cout << "Beginning initialization of " << HazeQuerySize << " hazy rectangles..." << std::endl;
	for (unsigned i = 0; i < HazeQuerySize; ++i)
	{
        Rectangle rect;
		for (unsigned d = 0; d < dimensions; ++d)
		{
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.lowerLeft[d] = *doubleBuffer;
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.upperRight[d] = *doubleBuffer;
		}
        rectangles.push_back( rect );
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

static std::vector<Rectangle> generateCaliRectangles()
{
	// Query set is pre-generated and requires 2 dimensions
	assert(dimensions == 2);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = "/home/bjglasbe/Documents/code/nir-tree/data/rea02.2";
	file.open(dataPath);
	fileGoodOrDie(file);
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
    rectangles.reserve(CaliforniaQuerySize);
	std::cout << "Beginning initialization of " << CaliforniaQuerySize << " california roll rectangles..." << std::endl;
	for (unsigned i = 0; i < CaliforniaQuerySize; ++i)
	{
        Rectangle loc_rect;

		for (unsigned d = 0; d < dimensions; ++d)
		{
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			loc_rect.lowerLeft[d] = *doubleBuffer;
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			loc_rect.upperRight[d] = *doubleBuffer;
		}
        rectangles.push_back( loc_rect );
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

static std::vector<Rectangle> generateBioRectangles()
{
	// Query set is pre-generated and requires 3 dimensions
	assert(dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = "/home/bjglasbe/Documents/code/nir-tree/data/rea03.2";
	file.open(dataPath);
	fileGoodOrDie(file);
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
    rectangles.reserve( BiologicalQuerySize );
	std::cout << "Beginning initialization of " << BiologicalQuerySize << " biological warfare rectangles..." << std::endl;
	for (unsigned i = 0; i < BiologicalQuerySize; ++i)
	{
        Rectangle rect;
		for (unsigned d = 0; d < dimensions; ++d)
		{
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.lowerLeft[d] = *doubleBuffer;
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.upperRight[d] = *doubleBuffer;
		}
        rectangles.push_back( rect );
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

static std::vector<Rectangle> generateForestRectangles()
{
	// Query set is pre-generated and requires 5 dimensions
	assert(dimensions == 5);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = "/home/bjglasbe/Documents/code/nir-tree/data/rea05.2";
	file.open(dataPath);
	fileGoodOrDie(file);
	char *buffer = new char[sizeof(double)];
	memset(buffer, 0, sizeof(double));
	double *doubleBuffer = (double *)buffer;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
	rectangles.reserve( ForestQuerySize );
	std::cout << "Beginning initialization of " << ForestQuerySize << " forest fire rectangles..." << std::endl;
	for (unsigned i = 0; i < ForestQuerySize; ++i)
	{
        Rectangle rect;
		for (unsigned d = 0; d < dimensions; ++d)
		{
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.lowerLeft[d] = *doubleBuffer;
			fileGoodOrDie(file);
			file.read(buffer, sizeof(double));
			rect.upperRight[d] = *doubleBuffer;
		}

        rectangles.push_back( rect );
	}
	std::cout << "Initialization OK." << std::endl;

	// Cleanup
	file.close();
	delete [] buffer;

	return rectangles;
}

static std::vector<Rectangle> generateCanadaRectangles()
{
	// Query set is pre-generated and requires 2 dimensions
	assert(dimensions == 2);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = "/home/bjglasbe/Documents/code/nir-tree/data/canadaQ";
	file.open(dataPath);
	fileGoodOrDie(file);
	double bufferX, bufferY;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
	rectangles.reserve(CanadaQuerySize);
	std::cout << "Beginning initialization of " << CanadaQuerySize << " maple leaf rectangles..." << std::endl;
	for (unsigned i = 0; i < CanadaQuerySize; ++i)
	{
        Rectangle rect;
		// Read in lower left
		file >> bufferX;
		file >> bufferY;
		rect.lowerLeft[0] = bufferX;
		rect.lowerLeft[1] = bufferY;

		// Read in upper right
		file >> bufferX;
		file >> bufferY;
		rect.upperRight[0] = bufferX;
		rect.upperRight[1] = bufferY;

        rectangles.push_back( rect );
	}

	// Cleanup
	file.close();

	return rectangles;
}

static std::vector<Rectangle> generateGaiaRectangles()
{
	// Query set is pre-generated and requires 3 dimensions
	assert(dimensions == 3);

	// Setup file reader and double buffer
	std::fstream file;
	std::string dataPath = "/home/bjglasbe/Documents/code/nir-tree/data/gaiaQ";
	file.open(dataPath);
	fileGoodOrDie(file);
	double buffer;

	// Initialize rectangles
    std::vector<Rectangle> rectangles;
	rectangles.reserve( GaiaQuerySize );
	std::cout << "Beginning initialization of " << GaiaQuerySize << " starry rectangles..." << std::endl;
	for (unsigned i = 0; i < GaiaQuerySize; ++i)
	{
        Rectangle rect;
		// Read in lower left
		file >> buffer;
		rect.lowerLeft[0] = buffer;
		file >> buffer;
		rect.lowerLeft[1] = buffer;
		file >> buffer;
		rect.lowerLeft[2] = buffer;

		// Read in upper right
		file >> buffer;
		rect.upperRight[0] = buffer;
		file >> buffer;
		rect.upperRight[1] = buffer;
		file >> buffer;
		rect.upperRight[2] = buffer;

        rectangles.push_back( rect );
	}

	// Cleanup
	file.close();

	return rectangles;
}

static bool is_already_loaded(
    std::map<std::string, unsigned> &configU,
    Index *spatial_index
) {
    if( configU["tree"] == NIR_TREE ) {
        
        nirtreedisk::NIRTreeDisk<12,25,nirtreedisk::LineMinimizeDownsplits>
            *tree =
            (nirtreedisk::NIRTreeDisk<12,25,nirtreedisk::LineMinimizeDownsplits> *) spatial_index;

        size_t existing_page_count = tree->node_allocator_->buffer_pool_.get_preexisting_page_count();
        if( existing_page_count > 0 ) {
            return true;
        }
    } else if( configU["tree"] == R_PLUS_TREE ) {
        rplustreedisk::RPlusTreeDisk<12,25> *tree =
            (rplustreedisk::RPlusTreeDisk<12,25> *) spatial_index;
        size_t existing_page_count = tree->node_allocator_.buffer_pool_.get_preexisting_page_count();
        if( existing_page_count > 0 ) {
            return true;
        }
    } else if( configU["tree"] == R_STAR_TREE ) {
        rstartreedisk::RStarTreeDisk<12,25> *tree =
            (rstartreedisk::RStarTreeDisk<12,25> *) spatial_index;
        size_t existing_page_count = tree->node_allocator_->buffer_pool_.get_preexisting_page_count();
        if( existing_page_count > 0 ) {
            return true;
        }
    } else if( configU["tree"] == R_TREE ) {
        rtreedisk::RTreeDisk<3,6> *tree =
            (rtreedisk::RTreeDisk<3,6> *) spatial_index;
        size_t existing_page_count = tree->node_allocator_.buffer_pool_.get_preexisting_page_count();
        if( existing_page_count > 0 ) {
            return true;
        }
    }
    return false;
}

template <class T>
void repack_tree( T *tree_ptr, std::string &new_file_name,
        tree_node_handle (*repack_func)( tree_node_handle, tree_node_allocator *,
            tree_node_allocator * ) ) {

    auto new_file_allocator = std::make_unique<tree_node_allocator>(
            40960 * 13000,
            new_file_name );

    new_file_allocator->initialize();
    std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
    auto repacked_handle = repack_func( tree_ptr->root,
            tree_ptr->node_allocator_.get(), new_file_allocator.get() );
    tree_ptr->root = repacked_handle;

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);

    std::cout << "Repacking done in: " << delta.count() << "s" <<
        std::endl;

    new_file_allocator->dump_free_list();

    // This evicts all the old pages, which is painful.
    tree_ptr->node_allocator_ = std::move( new_file_allocator );
}

template <typename T>
static void runBench(PointGenerator<T> &pointGen, std::map<std::string, unsigned> &configU, std::map<std::string, double> &configD)
{
	std::cout << "Running benchmark." << std::endl;

	// Setup checksums
	unsigned directSum = 0;

	// Setup statistics
	double totalTimeInserts = 0.0;
	double totalTimeSearches = 0.0;
	double totalTimeRangeSearches = 0.0;
	double totalTimeDeletes = 0.0;
	unsigned totalInserts = 0;
	unsigned totalSearches = 0;
	double totalRangeSearches = 0.0;
	unsigned totalDeletes = 0.0;

	// Initialize the index
	Index *spatialIndex;
	if (configU["tree"] == R_TREE)
	{
		//spatialIndex = new rtree::RTree(configU["minfanout"], configU["maxfanout"]);
		spatialIndex = new rtreedisk::RTreeDisk<3,6>( 4096 * 13000,
                "rtreediskbacked_california.txt" );
	}
	else if (configU["tree"] == R_PLUS_TREE)
	{
        spatialIndex = new
            rplustreedisk::RPlusTreeDisk<12,25>(4096*13000,
                "rplustreediskbacked_california.txt" );
		//spatialIndex = new rplustree::RPlusTree(configU["minfanout"], configU["maxfanout"]);
	}
	else if (configU["tree"] == R_STAR_TREE)
	{
		//spatialIndex = new rstartree::RStarTree(configU["minfanout"], configU["maxfanout"]);
		spatialIndex = new rstartreedisk::RStarTreeDisk<12,25>( 4096 *
                13000, "rstardiskbacked_california.txt" );
	}
	else if (configU["tree"] == NIR_TREE)
	{
		//spatialIndex = new nirtree::NIRTree(configU["minfanout"], configU["maxfanout"]);
		//spatialIndex = new nirtree::NIRTree(12,25);
		spatialIndex = new
            nirtreedisk::NIRTreeDisk<12,25,nirtreedisk::LineMinimizeDownsplits>(
                40960*13000, 
                /*"repacked_nirtree.txt"*/
                "nirdiskbacked_california.txt");
	}
	else if (configU["tree"] == QUAD_TREE)
	{
		spatialIndex = new quadtree::QuadTree();
	}
	else if (configU["tree"] == REVISED_R_STAR_TREE)
	{
		spatialIndex = new revisedrstartree::RevisedRStarTree(configU["minfanout"], configU["maxfanout"]);
	}
	else
	{
		std::cout << "Unknown tree selected. Exiting." << std::endl;
		return;
	}

	// Initialize search rectangles
    std::vector<Rectangle> searchRectangles;
	if (configU["distribution"] == UNIFORM)
	{
		searchRectangles = generateRectangles(configU["size"], configU["seed"], configU["rectanglescount"]);
	}
	else if (configU["distribution"] == SKEW)
	{
		configU["rectanglescount"] = BitQuerySize;
		searchRectangles = generateBitRectangles();
	}
	else if (configU["distribution"] == CLUSTER)
	{
		configU["rectanglescount"] = HazeQuerySize;
		searchRectangles = generateHazeRectangles();
	}
	else if (configU["distribution"] == CALIFORNIA)
	{
		configU["rectanglescount"] = CaliforniaQuerySize;
		searchRectangles = generateCaliRectangles();
	}
	else if (configU["distribution"] == BIOLOGICAL)
	{
		configU["rectanglescount"] = BiologicalQuerySize;
		searchRectangles = generateBioRectangles();
	}
	else if (configU["distribution"] == FOREST)
	{
		configU["rectanglescount"] = ForestQuerySize;
		searchRectangles = generateForestRectangles();
	}
	else if (configU["distribution"] == CANADA)
	{
		configU["rectanglescount"] = CanadaQuerySize;
		searchRectangles = generateCanadaRectangles();
	}
	else if (configU["distribution"] == GAIA)
	{
		configU["rectanglescount"] = GaiaQuerySize;
		searchRectangles = generateGaiaRectangles();
	}
	else
	{
		return;
	}

    std::optional<Point> nextPoint;

    if( not is_already_loaded( configU, spatialIndex ) ) {
        // If we read stuff from disk and don't need to reinsert, skip this.
        // Insert points and time their insertion
        std::cout << "Inserting Points." << std::endl;
        while((nextPoint = pointGen.nextPoint()) /* Intentional = and not == */)
        {
            // Compute the checksum directly
            for (unsigned d = 0; d < dimensions; ++d)
            {
                directSum += (unsigned) nextPoint.value()[d];
            }

            // Insert
            std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
            spatialIndex->insert(nextPoint.value());
            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
            totalTimeInserts += delta.count();
            totalInserts += 1;

            if( totalInserts % 10000 == 0 ) {
                std::cout << "Point[" << totalInserts << "] inserted. " << delta.count() << "s" << std::endl;
            }
            std::cout << "Insert OK." << std::endl;

            // std::cout << "Point[" << totalInserts << "] inserted. " << delta.count() << "s" << std::endl;
        }
        std::cout << "Insertion OK." << std::endl;

        // Validate checksum
        if (spatialIndex->checksum() != directSum)
        {
            std::cout << "Bad Checksum!" << std::endl;
            exit(1);
        }
        std::cout << "Checksum OK." << std::endl;


        if( configU["tree"] == NIR_TREE ) {
            std::cout << "Repacking..." << std::endl;
            auto tree_ptr =
                (nirtreedisk::NIRTreeDisk<12,25,nirtreedisk::LineMinimizeDownsplits> *) spatialIndex;
            std::string fname = "repacked_nirtree.txt";
            repack_tree( tree_ptr, fname,
                    nirtreedisk::repack_subtree<12,25,nirtreedisk::LineMinimizeDownsplits>  );
        } else if( configU["tree"] == R_STAR_TREE ) {
            std::cout << "Repacking..." << std::endl;
            auto tree_ptr = (rstartreedisk::RStarTreeDisk<12,25> *) spatialIndex;
            std::string fname = "repacked_rstar.txt";
            repack_tree( tree_ptr, fname, rstartreedisk::repack_subtree<12,25> );
        }

        spatialIndex->write_metadata();

    }

	// Validate tree
	//spatialIndex->validate();
	//std::cout << "Validation OK." << std::endl;

	// Search for points and time their retrieval
	std::cout << "Beginning search." << std::endl;
	pointGen.reset();

    std::mt19937 g;
    g.seed(0);

    std::shuffle( pointGen.pointBuffer.begin(),
            pointGen.pointBuffer.end(), g );

#if 0
	while((nextPoint = pointGen.nextPoint()) /* Intentional = not == */)
	{
		// Search
		Point &p = nextPoint.value();
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        if (spatialIndex->search(p)[0] != p)
        {
            exit(1);
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
        totalTimeSearches += delta.count();
        totalSearches += 1;

        if( totalSearches % 10000 == 0 ) {
		    std::cout << "Point[" << totalSearches << "] queried. " << delta.count() << " s" << std::endl;
        }

        if( totalSearches >= 300 ) {
            break;
        }
	}
	std::cout << "Search OK." << std::endl;

	// Validate checksum
    /*
	if (spatialIndex->checksum() != directSum)
	{
		std::cout << "Bad Checksum!" << std::endl;
		exit(1);
	}
	std::cout << "Checksum OK." << std::endl;
    */

#endif
    std::shuffle( searchRectangles.begin(), searchRectangles.end(), g );

#if 1
	// Search for rectangles
	unsigned rangeSearchChecksum = 0;
	std::cout << "Beginning search for " << configU["rectanglescount"] << " rectangles..." << std::endl;
	for (unsigned i = 0; i < configU["rectanglescount"]; ++i)
	{
		// Search
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		std::vector<Point> v = spatialIndex->search(searchRectangles[i]);
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeRangeSearches += delta.count();
		totalRangeSearches += 1;
		rangeSearchChecksum += v.size();
		// std::cout << "searchRectangles[" << i << "] queried. " << delta.count() << " s" << std::endl;
		// std::cout << "searchRectangles[" << i << "] returned " << v.size() << " points" << std::endl;

	}
	std::cout << "Range search OK. Checksum = " << rangeSearchChecksum << std::endl;

#endif
	// Gather statistics

#ifdef STAT
	spatialIndex->stat();
	std::cout << "Statistics OK." << std::endl;
#endif

	// Validate checksum
    /*
	if (spatialIndex->checksum() != directSum)
	{
		std::cout << "Bad Checksum!" << std::endl;
		exit(1);
	}
	std::cout << "Checksum OK." << std::endl;
    */

    /*
	// Delete points and time their deletion
	std::cout << "Beginning deletion." << std::endl;
	pointGen.reset();
	while((nextPoint = pointGen.nextPoint()))
	{
		// Delete
		std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
		spatialIndex->remove(nextPoint.value());
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
		totalTimeDeletes += delta.count();
		totalDeletes += 1;
		// std::cout << "Point[" << i << "] deleted." << delta.count() << " s" << std::endl;
	}
	std::cout << "Deletion OK." << std::endl;
    */

	// Timing Statistics
	std::cout << "Total time to insert: " << totalTimeInserts << "s" << std::endl;
	std::cout << "Avg time to insert: " << totalTimeInserts / (double) totalInserts << "s" << std::endl;
	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;
	std::cout << "Total time to range search: " << totalTimeRangeSearches << "s" << std::endl;
	std::cout << "Avg time to range search: " << totalTimeRangeSearches / totalRangeSearches << "s" << std::endl;
	std::cout << "Total time to delete: " << totalTimeDeletes << "s" << std::endl;
	std::cout << "Avg time to delete: " << totalTimeDeletes / (double) totalDeletes << "s" << std::endl;

    spatialIndex->write_metadata();

    std::cout << "Metadata written." << std::endl;
	// Cleanup
	//delete spatialIndex;
}

void randomPoints(std::map<std::string, unsigned> &configU, std::map<std::string, double> &configD)
{
	switch (configU["distribution"])
	{
		case UNIFORM:
		{
			BenchTypeClasses::Uniform::size = configU["size"];
			BenchTypeClasses::Uniform::dimensions = dimensions;
			BenchTypeClasses::Uniform::seed = configU["seed"];
			PointGenerator<BenchTypeClasses::Uniform> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case SKEW:
		{
			PointGenerator<BenchTypeClasses::Skew> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case CALIFORNIA:
		{
			PointGenerator<BenchTypeClasses::California> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case BIOLOGICAL:
		{
			PointGenerator<BenchTypeClasses::Biological> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case FOREST:
		{
			PointGenerator<BenchTypeClasses::Forest> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case CANADA:
		{
			PointGenerator<BenchTypeClasses::Canada> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case GAIA:
		{
			PointGenerator<BenchTypeClasses::Gaia> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
		case MICROSOFTBUILDINGS:
		{
			PointGenerator<BenchTypeClasses::MicrosoftBuildings> pointGen;
			runBench(pointGen, configU, configD);
			break;
		}
	}
}
