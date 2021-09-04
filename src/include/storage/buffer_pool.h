#pragma once

#include <storage/page.h>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class buffer_pool {
public:
    buffer_pool( size_t pool_size_bytes );
    void initialize();

    page *get_page( size_t page_id );
    page *create_new_page();

    void writeback_page( size_t page_id );
    void pin_page( page *page_ptr );
    void unpin_page( page *page_ptr );

    inline size_t get_in_memory_page_count() { return max_mem_pages_; }
    inline size_t get_highest_allocated_page_id() { return
        highest_allocated_page_id_; }
    inline std::string &get_backing_file_name() { return
        backing_file_name_; }

    inline bool is_page_in_memory( size_t page_id ) {
        return page_index_.find( page_id ) != page_index_.end();
    }

protected:
    page *obtain_clean_page();
    void evict( std::unique_ptr<page> &page );
    void writeback_page( page *page_ptr );

    size_t max_mem_pages_;
    std::list<std::unique_ptr<page>> freelist_;
    std::vector<std::unique_ptr<page>> allocated_pages_;
    std::string backing_file_name_;

    // These pointers come from the allocated_pages_ vector unique_ptrs.
    // I can't use references to those unique_ptrs because if the vector
    // gets resized then my references are dangling.
    std::unordered_map<size_t, page *> page_index_;
    size_t clock_hand_pos_;
    int backing_file_fd_;
    size_t highest_allocated_page_id_;
};
