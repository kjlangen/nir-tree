#include <catch2/catch.hpp>
#include <rstartree/rstartree.h>
#include <rstartree/node.h>
#include <storage/tree_node_allocator.h>
#include <storage/page.h>
#include <unistd.h>


TEST_CASE( "Tree Node Allocator: Single RStarTree Node" ) {

    tree_node_allocator allocator( 10 * PAGE_SIZE );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    std::pair<rstartree::Node *, tree_node_ptr> alloc_data =
        allocator.create_new_tree_node<rstartree::Node>();

    REQUIRE( alloc_data.first != nullptr );
    REQUIRE( alloc_data.second.page_id_ == 0 );
    REQUIRE( alloc_data.second.offset_ == sizeof(page_header) + 0 );
}


TEST_CASE( "Tree Node Allocator: Overflow one Page" ) {
    size_t node_size = sizeof( rstartree::Node );
    tree_node_allocator allocator( 10 * PAGE_SIZE );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    size_t start_offset = sizeof(page_header);

    for( size_t i = 0; i < PAGE_DATA_SIZE / node_size; i++ ) {
        std::pair<rstartree::Node *, tree_node_ptr> alloc_data =
            allocator.create_new_tree_node<rstartree::Node>();
        REQUIRE( alloc_data.first != nullptr );
        REQUIRE( alloc_data.second.page_id_ == 0 );
        REQUIRE( alloc_data.second.offset_== start_offset + i *
                node_size );
    }
    std::pair<rstartree::Node *, tree_node_ptr> alloc_data =
            allocator.create_new_tree_node<rstartree::Node>();
    REQUIRE( alloc_data.first != nullptr );
    REQUIRE( alloc_data.second.page_id_ == 1 );
    REQUIRE( alloc_data.second.offset_== start_offset );
}
