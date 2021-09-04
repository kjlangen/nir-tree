#include <storage/tree_node_allocator.h>
#include <storage/buffer_pool.h>
#include <storage/page.h>
#include <limits>
#include <cassert>
#include <iostream>

tree_node_allocator::tree_node_allocator( size_t memory_budget ) :
    buffer_pool_( memory_budget ),
    space_left_in_cur_page_( PAGE_DATA_SIZE ),
    cur_page_( std::numeric_limits<size_t>::max() ) {
}

page *tree_node_allocator::get_page_to_alloc_on( size_t object_size ) {
    if( cur_page_ == std::numeric_limits<size_t>::max() ) {
        cur_page_ = 0;
        page *page_ptr = buffer_pool_.get_page( 0 );
        assert( object_size <= space_left_in_cur_page_ );
        return page_ptr;
    }
    if( object_size <= space_left_in_cur_page_ ) {
        return buffer_pool_.get_page( cur_page_ );
    }
    cur_page_++;
    space_left_in_cur_page_ = PAGE_DATA_SIZE;

    // We may have pre-allocated a whole bunch of pages.
    std::cout << "Getting new page!" << std::endl;
    page *page_ptr = buffer_pool_.get_page( cur_page_ );
    if( page_ptr != nullptr ) {
        return page_ptr;
    }

    std::cout << "Creating new page!" << std::endl;
    return buffer_pool_.create_new_page();
}
