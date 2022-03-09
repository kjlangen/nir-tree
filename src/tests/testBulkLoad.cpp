#include <bench/randomPoints.h>
#include <nirtreedisk/node.h>
#include <nirtreedisk/nirtreedisk.h>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <random>
#include <catch2/catch.hpp>
#include <bulk_load.h>
#include <string>
#include <stdio.h>


std::vector<Point> generate_points( unsigned size ) {
    std::vector<Point> all_points;
    all_points.reserve( size );
    std::optional<Point> next;

    BenchTypeClasses::Uniform::size = size;
    BenchTypeClasses::Uniform::dimensions = dimensions;
    BenchTypeClasses::Uniform::seed = 0;
    PointGenerator<BenchTypeClasses::Uniform> points;
    while( (next = points.nextPoint() )) {
        all_points.push_back( next.value() );
    }

    return all_points;
}

void generate_tree(nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree, std::vector<Point> &all_points, unsigned branch_factor) {
    double bulk_load_pct = 1.0;
    unsigned size = (unsigned) all_points.size();
    uint64_t cut_off_bulk_load = std::floor(bulk_load_pct*all_points.size());

    uint64_t max_depth =
        std::floor(log(size)/log(branch_factor));
    auto ret = quad_tree_style_load( tree,
            all_points.begin(), all_points.begin() + cut_off_bulk_load,
            (unsigned) branch_factor, (unsigned) 0, (unsigned)
            max_depth, nullptr );
    tree->root = ret.first;
}

TEST_CASE("gen_tree: test_uniform_generation") {
    unlink( "bulkloaded_tree_test.txt" );
    std::string file_name = "bulkloaded_tree_test.txt";
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    40960UL*20000UL, file_name );
    unsigned size = 81;
    std::vector<Point> points = generate_points(size);
    generate_tree(tree, points, 9);

    for( Point p : points ) {
        std::vector<Point> out = tree->search(p);
        REQUIRE(out.size() == 1);
    }
    delete tree;
    unlink( "bulkloaded_tree_test.txt" );
}

TEST_CASE("gen_tree: test_non_perfect_depth") {
    unlink( "test_non_perfect_depth.txt" );
    std::string file_name = "test_non_perfect_depth.txt";
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    4096UL*20, file_name );
    unsigned size = 1203;
    std::vector<Point> points = generate_points(size);
    generate_tree(tree, points, 9);

    for( Point p : points ) {
        std::vector<Point> out = tree->search(p);
        REQUIRE(out.size() == 1);
    }
    delete tree;
    unlink( "test_non_perfect_depth.txt" );

}

TEST_CASE("gen_tree: test_same_y") {
    unlink( "test_same_y.txt" );
    std::string file_name = "test_same_y.txt";
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    4096UL*200, file_name);
    unsigned size = 10;
    std::vector<Point> points;
    for(unsigned i = 0; i < size; i++) {
        Point p;
        p[0] = (double) i;
        p[1] = 0;
        points.push_back(std::move(p));
    }
    generate_tree(tree, points, 9);

    for( Point p : points ) {
        std::vector<Point> out = tree->search(p);
        REQUIRE(out.size() == 1);
    }
    delete tree;
    unlink( "test_same_y.txt" );
}

TEST_CASE("gen_tree: test_same_x") {
    unlink( "test_same_x.txt" );
    std::string file_name = "test_same_x.txt";
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    4096UL*200, file_name);
    unsigned size = 81;
    std::vector<Point> points;
    for(unsigned i = 0; i < size; i++) {
        Point p;
        p[1] = (double) i;
        p[0] = 0;
        points.push_back(std::move(p));
    }
    generate_tree(tree, points, 9);

    for( Point p : points ) {
        std::vector<Point> out = tree->search(p);
        REQUIRE(out.size() == 1);
    }
    delete tree;
    unlink( "test_same_x.txt" );
}

TEST_CASE("gen_tree: underfill_tree") {
    unlink( "underfill_tree.txt" );
    std::string file_name = "underfill_tree.txt";
    nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy> *tree =  new
            nirtreedisk::NIRTreeDisk<5,9,nirtreedisk::ExperimentalStrategy>(
                    4096UL*200UL, file_name);
    unsigned size = 8;
    std::vector<Point> points = generate_points(size);
    generate_tree(tree, points, 9);

    for( Point p : points ) {
        std::vector<Point> out = tree->search(p);
        REQUIRE(out.size() == 1);
    }
    delete tree;
    unlink( "underfill_tree.txt" );
}

