#pragma once

#include "page.h"
#include <fstream>
#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

typedef struct page_metadata { 
    page_metadata( bool clock_active, std::unique_ptr<page> page_ptr ) :
        clock_active_(clock_active), page_ptr_( std::move( page_ptr ) ) {}

    bool clock_active_;
    std::unique_ptr<page> page_ptr_;
} page_metadata;

class buffer_pool {
public:
    buffer_pool( size_t pool_size_bytes );
    void initialize();
    std::unique_ptr<page> &get_page( size_t page_id );
private:
    std::unique_ptr<page> &alloc_page();
    void evict( page_metadata &meta );

    size_t num_pages_to_alloc_;
    std::list<std::unique_ptr<page>> freelist_;

    // This probably needs to be a hashmap so I can find out if the page
    // is allocated quickly on search

    std::unordered_map<size_t, std::unique_ptr<page> &> page_index_;

    std::vector<page_metadata> allocated_pages_;
    size_t clock_hand_pos_;

    std::fstream backing_file_;
};
