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

    std::pair<pinned_node_ptr<rstartree::Node>, tree_node_handle> alloc_data =
        allocator.create_new_tree_node<rstartree::Node>();

    REQUIRE( alloc_data.first != nullptr );
    REQUIRE( alloc_data.second.get_page_id() == 0 );
    REQUIRE( alloc_data.second.get_offset() == 0 );
}


TEST_CASE( "Tree Node Allocator: Overflow one Page" ) {
    size_t node_size = sizeof( rstartree::Node );
    tree_node_allocator allocator( 10 * PAGE_SIZE );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    for( size_t i = 0; i < PAGE_DATA_SIZE / node_size; i++ ) {
        std::pair<pinned_node_ptr<rstartree::Node>, tree_node_handle> alloc_data =
            allocator.create_new_tree_node<rstartree::Node>();
        REQUIRE( alloc_data.first != nullptr );
        REQUIRE( alloc_data.second.get_page_id() == 0 );
        REQUIRE( alloc_data.second.get_offset() ==  i * node_size );
    }
    std::pair<pinned_node_ptr<rstartree::Node>, tree_node_handle> alloc_data =
            allocator.create_new_tree_node<rstartree::Node>();
    REQUIRE( alloc_data.first != nullptr );
    REQUIRE( alloc_data.second.get_page_id() == 1 );
    REQUIRE( alloc_data.second.get_offset() == 0 );
}

TEST_CASE( "Tree Node Allocator: Convert TreeNodePtr to Raw Ptr" ) {
    tree_node_allocator allocator( 10 * PAGE_SIZE );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    std::pair<pinned_node_ptr<rstartree::Node>, tree_node_handle> alloc_data =
        allocator.create_new_tree_node<rstartree::Node>();
    pinned_node_ptr<rstartree::Node> output_ptr = alloc_data.first;
    REQUIRE( output_ptr != nullptr );
    REQUIRE( alloc_data.second.get_page_id() == 0 );
    REQUIRE( alloc_data.second.get_offset() == 0 );
    pinned_node_ptr<rstartree::Node> converted_ptr =
        allocator.get_tree_node<rstartree::Node>(
            alloc_data.second );
    REQUIRE( output_ptr == converted_ptr );

}

TEST_CASE( "Tree Node Allocator: Can Handle Paged Out data" ) {

    // Create a single page allocator
    tree_node_allocator allocator( PAGE_SIZE );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    // Size_t's to make it easier to understand, but it doesn't matter
    size_t node_size = sizeof( size_t );
    std::vector<tree_node_handle> allocated_ptrs;
    for( size_t i = 0; i < PAGE_DATA_SIZE / node_size; i++ ) {
        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data =
            allocator.create_new_tree_node<size_t>();
        pinned_node_ptr<size_t> sz_ptr = alloc_data.first;
        REQUIRE( sz_ptr != nullptr );
        REQUIRE( alloc_data.second.get_page_id() == 0 );
        REQUIRE( alloc_data.second.get_offset() == i *
                sizeof(size_t));
        *sz_ptr = i;
        allocated_ptrs.push_back( alloc_data.second );
    }

    // This will go on the next page, forcing a page out of the first
    allocator.create_new_tree_node<size_t>();

    // Walk over all the pointers in the first page, make sure the data
    // is preserved
    for( size_t i = 0; i < allocated_ptrs.size(); i++ ) {
        pinned_node_ptr<size_t> sz_ptr = allocator.get_tree_node<size_t>(
                allocated_ptrs[i] );
        REQUIRE( *sz_ptr == i );
    }
}

class allocator_tester : public tree_node_allocator {
public:

    allocator_tester( size_t memory_budget ) :
        tree_node_allocator( memory_budget ) {}

    buffer_pool &get_buffer_pool() {
        return buffer_pool_;
    }
};

TEST_CASE( "Tree Node Allocator: Test pinned_node_ptr scope" ) {
    // Create a single page allocator
    allocator_tester allocator( PAGE_SIZE );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    buffer_pool &bp = allocator.get_buffer_pool();
    {
        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data
            = allocator.create_new_tree_node<size_t>();

        
        // There's only one page, so we know the ptr is on it.
        page *p = bp.get_page( 0 );
        REQUIRE( p->header_.pin_count_ == 1 );

        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data2
            = allocator.create_new_tree_node<size_t>();

        REQUIRE( p->header_.pin_count_ == 2 );
    }

    page *p = bp.get_page( 0 );
    REQUIRE( p->header_.pin_count_ == 0 );


}
