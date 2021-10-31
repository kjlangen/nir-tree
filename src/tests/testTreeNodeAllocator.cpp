#include <catch2/catch.hpp>
#include <rstartree/rstartree.h>
#include <rstartree/node.h>
#include <storage/tree_node_allocator.h>
#include <storage/page.h>
#include <unistd.h>
#include <util/geometry.h>
#include <nirtreedisk/nirtreedisk.h>

TEST_CASE( "Tree Node Allocator: Single RStarTree Node" ) {

    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
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
    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
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
    tree_node_allocator allocator( 10 * PAGE_SIZE, "file_backing.db" );
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
    tree_node_allocator allocator( PAGE_SIZE, "file_backing.db" );
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

    allocator_tester( size_t memory_budget, std::string
            backing_file_name ) :
        tree_node_allocator( memory_budget, backing_file_name ) {}

    buffer_pool &get_buffer_pool() {
        return buffer_pool_;
    }

    size_t get_cur_page() {
        return cur_page_;
    }
    size_t get_space_left_in_cur_page() {
        return space_left_in_cur_page_;
    }
};

TEST_CASE( "Tree Node Allocator: Test pinned_node_ptr scope" ) {
    // 2 page allocator
    allocator_tester allocator( PAGE_SIZE *2, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    buffer_pool &bp = allocator.get_buffer_pool();
    tree_node_handle first_obj_handle;

    {
        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data
            = allocator.create_new_tree_node<size_t>();
        first_obj_handle = alloc_data.second;

        
        // There's only one page, so we know the ptr is on it.
        page *p = bp.get_page( 0 );
        REQUIRE( p->header_.pin_count_ == 1 );

        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data2
            = allocator.create_new_tree_node<size_t>();

        REQUIRE( p->header_.pin_count_ == 2 );
    }

    page *p = bp.get_page( 0 );
    REQUIRE( p->header_.pin_count_ == 0 );

    // Fill up one whole page
    for( size_t i = 0; i < ( PAGE_DATA_SIZE / sizeof( size_t ) ) - 2;
            i++ ) {
        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data
            = allocator.create_new_tree_node<size_t>();
    }

    // Should be nothing pinned!
    page *p1 = bp.get_page( 0 );
    REQUIRE( p1->header_.pin_count_ == 0 );

    // NOthing will be allocated on this yet, but because it is in the
    // freelist we can get it (since we prealloc'd 2 pages)
    page *p2 = bp.get_page( 1 );

    // Get first object
    pinned_node_ptr<size_t> first_obj_ptr = allocator.get_tree_node<size_t>(
            first_obj_handle );
    REQUIRE( p1->header_.pin_count_ == 1 );

    {
        std::pair<pinned_node_ptr<size_t>, tree_node_handle> alloc_data
            = allocator.create_new_tree_node<size_t>();
        p2 = bp.get_page( 1 );
        REQUIRE( p2 != nullptr );

        // Both pinned
        REQUIRE( p1->header_.pin_count_ == 1 );
        REQUIRE( p2->header_.pin_count_ == 1 );

        // Overwrite ptr
        first_obj_ptr = alloc_data.first;

        // P1 unpinned
        REQUIRE( p1->header_.pin_count_ == 0 );
        REQUIRE( p2->header_.pin_count_ == 2 );
        
        // Second ref falls out of scope
    }

    REQUIRE( p1->header_.pin_count_ == 0 );
    REQUIRE( p2->header_.pin_count_ == 1 );
}

TEST_CASE( "Tree Node Allocator: Test freelist perfect allocs" ) {

    allocator_tester allocator( PAGE_SIZE*2, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    for( unsigned i = 0; i < PAGE_DATA_SIZE/sizeof(size_t)+1; i++ ) {
        auto alloc_data = allocator.create_new_tree_node<size_t>();
        REQUIRE( allocator.get_space_left_in_cur_page() == PAGE_DATA_SIZE -
                sizeof(size_t) );
        allocator.free( alloc_data.second, sizeof(size_t) ); // perfect fit, reuse.
    }
    REQUIRE( allocator.get_cur_page() == 0 );
}

static_assert( sizeof(size_t) == 2 * sizeof(int) );
TEST_CASE( "Tree Node Allocator: Test freelist split allocs" ) {
    using NodeType =
        nirtreedisk::Node<3,7,nirtreedisk::LineMinimizeDownsplits>;
    allocator_tester allocator( PAGE_SIZE*2, "file_backing.db" );
    unlink( allocator.get_backing_file_name().c_str() );
    allocator.initialize();

    for( unsigned i = 0; i < (PAGE_DATA_SIZE/sizeof(NodeType))-1; i++ ) {
        auto alloc_data = allocator.create_new_tree_node<NodeType>();
    }
    auto alloc_data = allocator.create_new_tree_node<NodeType>();
    REQUIRE( allocator.get_cur_page() == 0 );
    allocator.free( alloc_data.second, sizeof(NodeType) );
    unsigned remaining_slots = ((PAGE_DATA_SIZE % sizeof(NodeType)) +
        sizeof(NodeType))/compute_sizeof_inline_unbounded_polygon(
        MAX_RECTANGLE_COUNT+1);
    REQUIRE( remaining_slots == 7 );
    for( unsigned i = 0; i < remaining_slots; i++ ) {
        auto alloc_data2 =
            allocator.create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                    compute_sizeof_inline_unbounded_polygon(
                        MAX_RECTANGLE_COUNT+1) );
        REQUIRE( allocator.get_cur_page() == 0 );
    }
    auto alloc_data2 =
            allocator.create_new_tree_node<InlineUnboundedIsotheticPolygon>(
                    compute_sizeof_inline_unbounded_polygon(
                        MAX_RECTANGLE_COUNT+1) );
    REQUIRE( allocator.get_cur_page() == 1 );

}
