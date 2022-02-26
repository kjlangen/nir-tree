#include <bench/randomPoints.h>
#include <nirtreedisk/node.h>
#include <nirtreedisk/nirtreedisk.h>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <random>
#include <catch2/catch.hpp>
#include <bulk_load.h>


nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> * generate_tree(unsigned size, unsigned branch_factor) {
    std::vector<Point> all_points;
    std::optional<Point> next;

    BenchTypeClasses::Uniform::size = size;
    BenchTypeClasses::Uniform::dimensions = dimensions;
    BenchTypeClasses::Uniform::seed = 0;
    PointGenerator<BenchTypeClasses::Uniform> points;
    while( (next = points.nextPoint() )) {
        all_points.push_back( next.value() );
    }
    double bulk_load_pct = 1.0;
    uint64_t cut_off_bulk_load = std::floor(bulk_load_pct*all_points.size());

    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    40960UL*130000UL, "bulkloaded_tree_test.txt" );
    auto tree_ptr =
        (nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>
         *) tree;
    std::vector<Point> overflow;
    uint64_t max_depth =
        std::floor(log(size)/log(branch_factor));
    tree_node_handle root = quad_tree_style_load( tree, all_points.begin(), all_points.begin() + cut_off_bulk_load, (unsigned) branch_factor, (unsigned) 0, (unsigned) max_depth, nullptr, overflow );
    tree->root = root;

    REQUIRE(overflow.size() == 0);

    for( Point p : all_points ) {
        std::vector<Point> out = tree->search(p);
        REQUIRE(out.size() == 1);
    }

    return tree;
}


TEST_CASE("gen_tree: test_generation") {
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> * tree = generate_tree(81, 9);
}
