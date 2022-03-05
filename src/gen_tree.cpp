#include <bulk_load.h>


void generate_tree( std::map<std::string, unsigned> &configU ) {

    std::string backing_file = "bulkloaded_tree.txt";
    unlink( backing_file.c_str() );

    std::vector<Point> all_points;
    std::optional<Point> next;

    if( configU["distribution"] == CALIFORNIA ) {
        PointGenerator<BenchTypeClasses::California> points;
        while( (next = points.nextPoint() )) {
            all_points.push_back( next.value() );
        }
    } else if( configU["distribution"] == UNIFORM ) {
        BenchTypeClasses::Uniform::size = configU["size"];
        BenchTypeClasses::Uniform::dimensions = dimensions;
        BenchTypeClasses::Uniform::seed = 0;
        PointGenerator<BenchTypeClasses::Uniform> points;
        while( (next = points.nextPoint() )) {
            all_points.push_back( next.value() );
        }
    }

    double bulk_load_pct = 1.0;
    uint64_t cut_off_bulk_load = std::floor(bulk_load_pct*all_points.size());
    std::cout << "Bulk loading " << cut_off_bulk_load << " points." << std::endl;
    std::cout << "Sequential Inserting " << all_points.size() - cut_off_bulk_load << " points." << std::endl;

    Index *spatialIndex;
    if( configU["tree"] == NIR_TREE ) {
        // FIXME: one of the main slow downs of bulk loading in the NIR-Tree is going to be that
        // it looks for compression opportunities duringthe bulk load. This makes no sense because
        // we are guaranteed that each generated rectangle is disjoint.
        nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    40960UL*130000UL, backing_file );
        std::cout << "Bulk Loading..." << std::endl;
        std::cout << "Creating tree with " << 40960UL *130000UL << "bytes" << std::endl;
        bulk_load_tree( tree, configU, all_points.begin(), all_points.begin() + cut_off_bulk_load, 9 );
        std::cout << "Created NIRTree." << std::endl;
        /*
        if( !tree->validate() ) {
            std::cout << "Tree Validation Failed" << std::endl;
        }
        */
        spatialIndex = tree;
    } else if( configU["tree"] == R_STAR_TREE ) {
        rstartreedisk::RStarTreeDisk<5,9> *tree = new rstartreedisk::RStarTreeDisk<5,9>(
                    40960UL*130000UL, backing_file );
        std::cout << "Bulk Loading..." << std::endl;
        bulk_load_tree( tree, configU, all_points.begin(), all_points.begin() + cut_off_bulk_load, 9 );
        std::cout << "Created R*Tree" << std::endl;
        spatialIndex = tree;
    } else {
        abort();
    }

    std::mt19937 g;
    g.seed(0);

    std::shuffle( all_points.begin(), all_points.end(), g );

    unsigned totalSearches  = 0;
	double totalTimeSearches = 0.0;
    for( Point p : all_points ) {
        std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
        std::vector<Point> out = spatialIndex->search(p);
        if( out.size() != 1 ) {
            std::cout << "Could not find " << p << std::endl;
            abort();
        }
        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> delta = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
        totalTimeSearches += delta.count();
        totalSearches += 1;
        if( totalSearches >= 300 ) {
            break;
        }
    }

    spatialIndex->stat();

	std::cout << "Total time to search: " << totalTimeSearches << "s" << std::endl;
	std::cout << "Avg time to search: " << totalTimeSearches / totalSearches << "s" << std::endl;


    return;
}



int main( int argc, char **argv ) {

    std::map<std::string, unsigned> configU;
    configU.emplace( "tree", NIR_TREE );
    configU.emplace( "distribution", CALIFORNIA);

    int option;

    while( (option = getopt(argc,argv, "t:m:n:")) != -1 ) {
        switch( option ) {
            case 't': {
                configU["tree"] = (TreeType)atoi(optarg);
                break;
            }
            case 'm': {
                configU["distribution"] = (BenchType)atoi(optarg);
                break;
            }
            case 'n': {
                configU["size"] = atoi(optarg);
                break;
            }
        }
    }

    generate_tree( configU );

    return 0;
}
