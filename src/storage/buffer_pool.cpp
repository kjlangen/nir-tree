#include <storage/buffer_pool.h>
#include <storage/page.h>
#include <memory>
#include <list>
#include <cstring>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

buffer_pool::buffer_pool( size_t pool_size_bytes ) {

    max_mem_pages_ = pool_size_bytes / PAGE_SIZE;
    if( pool_size_bytes % PAGE_SIZE != 0 ) {
        max_mem_pages_++;
    }

    backing_file_name_ = "backing_tree_file.db";
    clock_hand_pos_ = 0;
    highest_allocated_page_id_ = 0;
}

void buffer_pool::initialize() {
    
    backing_file_fd_ = open( backing_file_name_.c_str(), O_CREAT | O_RDWR,
            S_IRUSR | S_IWUSR );

    assert( backing_file_fd_ != -1 );

    // Step 1: Read every data page we have, load 'em into memory
    size_t existing_page_count = 0;
    size_t file_offset = 0;
    for( ;; ) {
        std::unique_ptr<page> page_ptr = std::make_unique<page>();
        int rc = read( backing_file_fd_, (char *) page_ptr.get(), PAGE_SIZE );
        if( rc == PAGE_SIZE ) {
            std::cout << "Read page " << OFFSET_TO_PAGE_ID( file_offset
                    ) << " from disk" << std::endl;
            std::cout << "It has ID: " << page_ptr->header_.page_id_ <<
                std::endl;

            page *raw_page_ptr = page_ptr.get();

            // Construct metadata and record it 
            allocated_pages_.emplace_back( std::move(
                            page_ptr ) );
            page_index_.insert( { raw_page_ptr->header_.page_id_,
                    raw_page_ptr } ); 

            // Increment offsets
            file_offset += PAGE_SIZE;
            existing_page_count++;

            if( existing_page_count == max_mem_pages_ ) {
                // If we are out of memory to use, then the rest will need to be
                // read later.
                break;
            }
        } else {
            // Destroys page_ptr
            break;
        }
    }

    off_t rc = lseek( backing_file_fd_, file_offset, SEEK_SET );
    assert( rc == (off_t) file_offset );

    // Create any more backing file data that we need
    for( size_t i = existing_page_count; i < max_mem_pages_; i++ ) { 
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

    struct stat stat_buffer;
    int fstat_ret = fstat( backing_file_fd_, &stat_buffer );
    assert( fstat_ret == 0 );
    assert( stat_buffer.st_size % PAGE_SIZE == 0 );
    highest_allocated_page_id_ = (stat_buffer.st_size / PAGE_SIZE ) - 1;
    std::cout << "Created " << max_mem_pages_ << " pages." << std::endl;
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

    std::cout << "Oh shoot, I don't have page_id: " << page_id <<
        std::endl;

    // Step 2: It is not, so obtain a page
    // Will evict an old page if necessary
    page *page_ptr = obtain_clean_page();

    // No free pages available.
    if( page_ptr == nullptr ) {
        return nullptr;
    }

    // Step 3: Read in the contents of the page
    std::cout << "Seeking to offset: " << PAGE_ID_TO_OFFSET( page_id )
        << std::endl;
    off_t seek_ret = lseek( backing_file_fd_, PAGE_ID_TO_OFFSET(
                page_id ), SEEK_SET );
    assert( seek_ret == (off_t) PAGE_ID_TO_OFFSET( page_id ) );
    int read_ret = read( backing_file_fd_, (char *) page_ptr,
            PAGE_SIZE );
    assert( read_ret == PAGE_SIZE );
    std::cout << "Read in Page ID: " << page_ptr->header_.page_id_ << std::endl;
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
    std::cout << "Writing back page: " << page_ptr->header_.page_id_ <<
        std::endl;
    size_t file_offset = PAGE_ID_TO_OFFSET( page_ptr->header_.page_id_ );
    off_t seek_ret = lseek( backing_file_fd_, file_offset, SEEK_SET );
    std::cout << "Got seek_ret: " << seek_ret << " for offset " <<
        file_offset << std::endl;
    assert( seek_ret == (off_t) file_offset );
    int write_ret = write( backing_file_fd_, (char *) page_ptr,
            PAGE_SIZE );
    assert( write_ret == PAGE_SIZE );
}

page *buffer_pool::obtain_clean_page() {
    // Are there any free pages?
    if( !freelist_.empty() ) {
        std::unique_ptr<page> page_ptr = std::move( freelist_.front() );
        freelist_.pop_front();
        page *raw_page_ptr = page_ptr.get();
        allocated_pages_.emplace_back( std::move(page_ptr) );
        std::cout << "I had some left over pages in the freelist." <<
            std::endl;
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
