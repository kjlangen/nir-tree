#include <algorithm>
#include <memory>
#include <list>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <thread>


#include <storage/buffer_pool.h>
#include <storage/page.h>


buffer_pool::buffer_pool( size_t pool_size_bytes, std::string
        backing_file_name ) {

    max_mem_pages_ = pool_size_bytes / PAGE_SIZE;
    if( pool_size_bytes % PAGE_SIZE != 0 ) {
        max_mem_pages_++;
    }

    backing_file_name_ = backing_file_name;
    clock_hand_pos_ = 0;
    highest_allocated_page_id_ = 0;
}


buffer_pool::~buffer_pool() {
    // Teardown, write back everypage
    for( auto &page_ptr : allocated_pages_ ) {
        evict( page_ptr );
    }
}

void buffer_pool::initialize() {
    
    backing_file_fd_ = open( backing_file_name_.c_str(), O_CREAT | O_RDWR | O_DIRECT,
            S_IRUSR | S_IWUSR );

    assert( backing_file_fd_ != -1 );

    std::cout << "Going to create " << max_mem_pages_ << " pages of size: " << PAGE_SIZE << std::endl;

    // Step 1: Read every data page we have, load 'em into memory
    size_t file_offset = 0;
    for( ;; ) {
        std::unique_ptr<page> page_ptr = std::make_unique<page>();
        int rc = read( backing_file_fd_, (char *) page_ptr.get(), PAGE_SIZE );
        if( rc == PAGE_SIZE ) {
            page *raw_page_ptr = page_ptr.get();

            // Construct metadata and record it 
            allocated_pages_.emplace_back( std::move(
                            page_ptr ) );
            page_index_.insert( { raw_page_ptr->header_.page_id_,
                    raw_page_ptr } ); 

            // Increment offsets
            file_offset += PAGE_SIZE;
            existing_page_count_++;

            if( existing_page_count_ == max_mem_pages_ ) {
                // If we are out of memory to use, then the rest will need to be
                // read later.
                break;
            }
        } else {
            // Destroys page_ptr
            break;
        }
    }

    struct stat stat_buffer;
    int fstat_ret = fstat( backing_file_fd_, &stat_buffer );
    assert( fstat_ret == 0 );
    assert( stat_buffer.st_size % PAGE_SIZE == 0 );
    if( stat_buffer.st_size > 0 ) {
        existing_page_count_ = (stat_buffer.st_size / PAGE_SIZE ) - 1;
    } else {
        existing_page_count_ = 0;
    }

    // We have enough pages. Break.
    if( existing_page_count_ >= max_mem_pages_ ) {
        highest_allocated_page_id_ = existing_page_count_;
        return;
    }

    off_t rc = lseek( backing_file_fd_, file_offset, SEEK_SET );
    assert( rc == (off_t) file_offset );

    // Create any more backing file data that we need
    for( size_t i = existing_page_count_; i < max_mem_pages_; i++ ) { 
        std::unique_ptr<page> page_ptr = std::make_unique<page>();
        page_ptr->header_.page_id_ = OFFSET_TO_PAGE_ID( file_offset );
        page_ptr->header_.pin_count_ = 0;
        page_ptr->header_.clock_active_ = false;
        memset( page_ptr->data_, '\0', sizeof( page_ptr->data_ ) );
        writeback_page( page_ptr.get() );

        // Push these into the freelist
        freelist_.push_back( std::move( page_ptr ) );

        file_offset += PAGE_SIZE;
    }

    highest_allocated_page_id_ = max_mem_pages_-1;
}

page *buffer_pool::get_page( size_t page_id ) {

    if( page_id > highest_allocated_page_id_ ) {
        return nullptr;
    }

    // Step 1: Determine if this page is already in memory
    auto search = page_index_.find( page_id );
    if( search != page_index_.end() ) {
        page *page_ptr = search->second;
        page_ptr->header_.clock_active_ = true;
        return page_ptr;
    }

    //std::this_thread::sleep_for(std::chrono::milliseconds(20));
    //std::cout << "Page miss." << std::endl;

    // Step 2: It is not, so obtain a page
    // Will evict an old page if necessary
    page *page_ptr = obtain_clean_page();

    // No free pages available.
    if( page_ptr == nullptr ) {
        std::cout << "No pages available." << std::endl;
        return nullptr;
    }

    // Step 3: Read in the contents of the page
    off_t seek_ret = lseek( backing_file_fd_, PAGE_ID_TO_OFFSET(
                page_id ), SEEK_SET );
    assert( seek_ret == (off_t) PAGE_ID_TO_OFFSET( page_id ) );
    int rd_thus_far = 0;
    while( rd_thus_far < PAGE_SIZE ) {
        int read_ret = read( backing_file_fd_, (char *) page_ptr + rd_thus_far,
                PAGE_SIZE - rd_thus_far );
        if( read_ret == -1 ) {
            std::cout << "Error on read: " << errno << std::endl;
            abort();
        }
        rd_thus_far += read_ret;
    }
    assert( page_ptr->header_.page_id_ == page_id );

    // Step 4: Put the page into the page_index (obtain_clean_page puts it into
    // allocated_pages_)
    page_index_.insert( { page_id, page_ptr } );
    return page_ptr;
}

void buffer_pool::pin_page( page *page_ptr ) {
    page_ptr->header_.pin_count_++;
}

void buffer_pool::unpin_page( page *page_ptr ) {
    page_ptr->header_.pin_count_--;
}


page *buffer_pool::create_new_page() {
    page *page_ptr = obtain_clean_page();
    if( page_ptr == nullptr ) {
        return nullptr;
    }

    memset( page_ptr->data_, '\0', sizeof(page_ptr->data_) );
    highest_allocated_page_id_++;

    // Set the header
    page_ptr->header_.page_id_ = highest_allocated_page_id_;
    page_ptr->header_.pin_count_ = 0;
    page_ptr->header_.clock_active_ = false;

    writeback_page( page_ptr );
    page_index_.insert( { highest_allocated_page_id_, page_ptr } );
    return page_ptr;
}

void buffer_pool::writeback_page( size_t page_id ) {
    page *page_ptr = get_page( page_id );
    if( page_ptr != nullptr ) {
        writeback_page( page_ptr );
    }
}

void buffer_pool::writeback_page( page *page_ptr ) {
    // Seek to the right offset in the file and flush it out
    size_t file_offset = PAGE_ID_TO_OFFSET( page_ptr->header_.page_id_ );
    off_t seek_ret = lseek( backing_file_fd_, file_offset, SEEK_SET );
    assert( seek_ret == (off_t) file_offset );
    int wr_thus_far = 0;
    while( wr_thus_far < PAGE_SIZE ) {
        int write_ret = write( backing_file_fd_, (char *) page_ptr + wr_thus_far,
                PAGE_SIZE - wr_thus_far );
        if( write_ret == -1 ) {
            std::cout << "Error on write: " << errno << std::endl;
            abort();
        }
        wr_thus_far += write_ret;
    }
}

page *buffer_pool::obtain_clean_page() {
    // Are there any free pages?
    if( !freelist_.empty() ) {
        std::unique_ptr<page> page_ptr = std::move( freelist_.front() );
        freelist_.pop_front();
        page *raw_page_ptr = page_ptr.get();
        allocated_pages_.emplace_back( std::move(page_ptr) );
        return raw_page_ptr;
    }

    size_t orig_clock_hand_pos_ = clock_hand_pos_;
    bool looped_over_everything_once = false;

    // Use clock
    do {
        std::unique_ptr<page> &page = allocated_pages_[ clock_hand_pos_ ];
        // Move hand past
        clock_hand_pos_ = ( clock_hand_pos_ + 1 ) %
            allocated_pages_.size();

        if( not page->header_.clock_active_ and not
                (page->header_.pin_count_
                    > 0) ) { 
            // Evict the page
            page->header_.clock_active_ = true;
            evict( page );
            return page.get();
        }
        // Unset
        page->header_.clock_active_ = false;
        if( orig_clock_hand_pos_ == clock_hand_pos_ ) {
            // We should have unset all the in_use bits and found
            // something, every page is pinned
            if( looped_over_everything_once ) {
                break;
            }
            looped_over_everything_once = true;
        }
    } while( true );

    // No free pages
    std::cout << "No Free pages, its all pinned!" << std::endl;
    return nullptr;
}

void buffer_pool::evict( std::unique_ptr<page> &page ) {
    writeback_page( page.get() );
    page_index_.erase( page->header_.page_id_ );
}

void buffer_pool::writeback_all_pages() {
    for( auto entry : page_index_ ) {
        page *page_ptr = entry.second;
        writeback_page( page_ptr );
    }
}
