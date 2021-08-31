#include "buffer_pool.h"
#include "page.h"
#include <memory>
#include <list>
#include <cstring>
#include <cassert>

buffer_pool::buffer_pool( size_t pool_size_bytes ) {
    num_pages_to_alloc_ = pool_size_bytes / PAGE_SIZE + 1;
    clock_hand_pos_ = 0;
}

void buffer_pool::initialize() {

    backing_file_.open( "backing_file.txt", std::fstream::in |
            std::fstream::out | std::fstream::binary );
    assert( not backing_file_.fail() );

    // Step 1: Read every data page we have, load 'em into memory
    size_t existing_page_count = 0;
    size_t file_offset = 0;
    while( not backing_file_.eof() ) {

        // Read the page
        std::unique_ptr<page> page_ptr = std::make_unique<page>();
        backing_file_.read( (char *) (page_ptr.get()), PAGE_SIZE ); 
        assert( not backing_file_.fail() );
        page_ptr->header_.page_id_= file_offset;

        // Construct metadata and record it 
        allocated_pages_.emplace_back( page_metadata( true, std::move(
                        page_ptr ) ) );
        std::unique_ptr<page> &p_ref = allocated_pages_.back().page_ptr_;
        page_index_.insert( { file_offset, p_ref } ); 
        // Increment offsets, keep going
        file_offset += PAGE_SIZE;
        existing_page_count++;

    }

    // Seek to back
    backing_file_.seekp( file_offset );

    // Create any more backing file data that we need
    for( size_t i = existing_page_count; i < num_pages_to_alloc_; i++ ) { 
        std::unique_ptr<page> page_ptr = std::make_unique<page>();
        page_ptr->header_.page_id_ = file_offset;
        memset( page_ptr->data_, '\0', sizeof( page_ptr->data_ ) );
        backing_file_.write( (char *) (page_ptr.get()), PAGE_SIZE );

        // Push these into the freelist
        freelist_.push_back( std::move( page_ptr ) );

        file_offset += PAGE_SIZE;
    }
}

std::unique_ptr<page> &buffer_pool::get_page( size_t page_id ) {
    // Step 1: Determine if this page is already in memory
    auto search = page_index_.find( page_id );
    if( search != page_index_.end() ) {
        // FIXME: second chance set clock bit?
        return search->second;
    }

    // Step 2: It is not, so alloc a page
    // Will evict an old page if necessary
    std::unique_ptr<page> &page_ptr = alloc_page();

    // Step 3: Read in the contents of the page
    backing_file_.seekg( PAGE_ID_TO_OFFSET( page_id ) );
    backing_file_.read( (char *) page_ptr.get(), PAGE_SIZE );
    assert( page_ptr->header_.page_id_ == page_id );

    // Step 4: Put the page into the page_index (alloc_page puts it into
    // allocated_pages_)
    page_index_.insert( { page_id, page_ptr } );
    return page_ptr;
}

std::unique_ptr<page> &buffer_pool::alloc_page() {
    // Are there any free pages?
    if( !freelist_.empty() ) {
        std::unique_ptr<page> page_ptr = std::move( freelist_.front() );
        freelist_.pop_front();
        allocated_pages_.emplace_back( page_metadata( true,
                    std::move(page_ptr) ) );
        return allocated_pages_.back().page_ptr_;
    }

    // Use clock
    for( ;; ) {
        page_metadata &meta = allocated_pages_[ clock_hand_pos_ ];
        // Move hand past
        clock_hand_pos_ = ( clock_hand_pos_ + 1 ) %
            allocated_pages_.size();

        if( not meta.clock_active_ ) { 
            // Write back the page
            evict( meta );
            meta.clock_active_ = true;
            return meta.page_ptr_;
        }
        // Unset
        meta.clock_active_ = false;
    }
}

void buffer_pool::evict( page_metadata &meta ) {
    // Seek to the right offset in the file and flush it out
    size_t file_offset = PAGE_ID_TO_OFFSET(
            meta.page_ptr_->header_.page_id_ );
    backing_file_.seekp( file_offset );
    backing_file_.write( (char *) (meta.page_ptr_.get()), PAGE_SIZE );

    page_index_.erase( meta.page_ptr_->header_.page_id_ );
}
