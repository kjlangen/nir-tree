#pragma once 

#include <storage/buffer_pool.h>
#include <storage/page.h>
#include <cassert>

typedef struct tree_node_ptr {
    tree_node_ptr( size_t page_id, size_t offset ) :
        page_id_(page_id), offset_( offset ) {}

    size_t page_id_;
    size_t offset_;
} tree_node_ptr;

class tree_node_allocator {
public:
    tree_node_allocator( size_t memory_budget );

    inline void initialize() {
        buffer_pool_.initialize();
    }

    inline std::string get_backing_file_name() {
        return buffer_pool_.get_backing_file_name();
    }
    // We want to pack all the tree types into something that fits
    // nicely in these.
    // TreeNodePtr = <PageId, offset>
    template <typename T>
    std::pair<T *, tree_node_ptr> create_new_tree_node() {
        page *page_ptr = get_page_to_alloc_on( sizeof( T ) );
        assert( page_ptr != nullptr );

        size_t offset_into_page = (PAGE_SIZE - space_left_in_cur_page_);
        T *obj_ptr = (T *) (page_ptr->data_ + offset_into_page);
        space_left_in_cur_page_ -= sizeof( T );

        tree_node_ptr meta_ptr( page_ptr->header_.page_id_,
                offset_into_page );

        
        return std::make_pair( obj_ptr, std::move(meta_ptr) );
    }

    //Convert TreeNodePtr into data ptr.

protected:
    page *get_page_to_alloc_on( size_t object_size );

    buffer_pool buffer_pool_;
    size_t space_left_in_cur_page_;
    size_t cur_page_;
};


