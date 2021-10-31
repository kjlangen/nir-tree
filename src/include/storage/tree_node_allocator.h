#pragma once 

#include <storage/buffer_pool.h>
#include <storage/page.h>
#include <cassert>
#include <optional>
#include <iostream>
#include <string>
#include <list>

template <typename T>
class pinned_node_ptr {
public:

    pinned_node_ptr( buffer_pool &pool, T *obj_ptr, page *page_ptr ) :
        pool_( pool ), obj_ptr_( obj_ptr ), page_ptr_( page_ptr ) {
            if( page_ptr != nullptr ) {
                pool_.pin_page( page_ptr_ );
            } }

    pinned_node_ptr( const pinned_node_ptr &other ) :
        pool_( other.pool_ ), obj_ptr_( other.obj_ptr_ ),
        page_ptr_( other.page_ptr_ ){
            if( page_ptr_ != nullptr ) {
                pool_.pin_page( page_ptr_ );
            }
        
    }

    ~pinned_node_ptr() {
        if( page_ptr_ != nullptr ) {
            pool_.unpin_page( page_ptr_ );
        }
    }

    pinned_node_ptr &operator=( pinned_node_ptr other ) {
        // Unpin old
        if( page_ptr_ != nullptr ) {
            pool_.unpin_page( page_ptr_ );
        }
        // Set new
        obj_ptr_ = other.obj_ptr_;
        page_ptr_ = other.page_ptr_;

        // Pin new
        if( page_ptr_ != nullptr ) {
            pool_.pin_page( page_ptr_ );
        }
        return *this;
    }

    bool operator==( std::nullptr_t ptr ) const {
        return obj_ptr_ == ptr; 
    }

    bool operator!=( std::nullptr_t ptr ) const {
        return !(obj_ptr_ == ptr); 
    }


    bool operator==( const pinned_node_ptr &other ) const {
        return obj_ptr_ == other.obj_ptr_;
    }

    bool operator!=( const pinned_node_ptr &other ) const {
        return !(obj_ptr_ == other.obj_ptr_);
    }


    constexpr T& operator*() const {
        return *obj_ptr_;
    }

    constexpr T *operator->() const {
        return obj_ptr_;
    }

private:
    buffer_pool &pool_;
    T *obj_ptr_;
    page *page_ptr_;

};

class tree_node_handle {
public:
    tree_node_handle( size_t page_id, size_t offset ) :
        page_location_( std::in_place, page_id, offset ) {}

    tree_node_handle() {
        page_location_= std::nullopt;
    }

    tree_node_handle( std::nullptr_t ) {
        page_location_= std::nullopt;
    }


    operator bool() const {
        return page_location_.has_value();
    }

    bool operator==( const tree_node_handle &other ) const {
        return page_location_ == other.page_location_;
    }

    bool operator!=( const tree_node_handle &other ) const {
        return !(*this == other);
    }


    bool operator==( const std::nullptr_t ) const {
        return not page_location_.has_value();
    }

    tree_node_handle &operator=( const tree_node_handle &other ) =
        default;

    tree_node_handle &operator=( const std::nullptr_t ) { 
        page_location_.reset();
        return *this;
    }

    struct page_location {
        page_location( size_t page_id, size_t offset ) :
            page_id_( page_id ),
            offset_( offset ) {}
        size_t page_id_;
        size_t offset_;
        bool operator==( const page_location &other ) const {
            return page_id_ == other.page_id_ and offset_ ==
                other.offset_;
        }
    };

    inline size_t get_page_id() {
        return page_location_.value().page_id_;
    }

    inline size_t get_offset() {
        return page_location_.value().offset_;
    }

    friend std::ostream& operator<<(std::ostream &os, const
            tree_node_handle &handle ) {
        if( handle.page_location_.has_value() ) {
            os << "{ PageID: " << handle.page_location_.value().page_id_
                << ", Offset: " << handle.page_location_.value().offset_ << "}" << std::endl;
        } else {
            os << "{ nullptr }" << std::endl;
        }
        return os;
    }

private:

    std::optional<page_location> page_location_;
};

static_assert( std::is_trivially_copyable<tree_node_handle>::value );

class tree_node_allocator {
public:
    tree_node_allocator( size_t memory_budget,
            std::string backing_file );

    inline void initialize() {
        buffer_pool_.initialize();
    }

    inline std::string get_backing_file_name() {
        return buffer_pool_.get_backing_file_name();
    }

    template <typename T>
    std::pair<pinned_node_ptr<T>, tree_node_handle> create_new_tree_node() {
        return create_new_tree_node<T>( sizeof( T ) );
    }

    template <typename T>
    std::pair<pinned_node_ptr<T>, tree_node_handle>
    create_new_tree_node( size_t node_size ) {
        assert( node_size <= PAGE_DATA_SIZE );

        for( auto iter = free_list_.begin(); iter != free_list_.end();
                iter++ ) {
            auto &alloc_location = *iter;
            if( alloc_location.second < node_size ) {
                continue;
            }

            size_t remainder = alloc_location.second - node_size;
            free_list_.erase( iter );
            page *page_ptr = buffer_pool_.get_page(
                alloc_location.first.get_page_id() );
            T *obj_ptr = (T *) (page_ptr->data_ +
                    alloc_location.first.get_offset() );

            // sizeof inline unbounded polygon with
            // MAX_RECTANGLE_COUNT+1
            // Can't use that symbol here because it would be recursive
            // includes. So instead I static assert it in that file and
            // use the constant here.
            if( remainder >= 272 ) {
                size_t new_offset = alloc_location.first.get_offset() +
                    node_size;
                tree_node_handle split_handle(
                        alloc_location.first.get_page_id(), new_offset
                        );
                free_list_.push_back( std::make_pair( split_handle,
                            remainder ) );
            }

            return std::make_pair( pinned_node_ptr( buffer_pool_,
                        obj_ptr, page_ptr ), alloc_location.first );


        }

        // Fall through, need new location
        page *page_ptr = get_page_to_alloc_on( node_size );
        if( page_ptr == nullptr ) {
            return std::make_pair( pinned_node_ptr( buffer_pool_,
                        static_cast<T *>( nullptr ), static_cast<page *>(
                            nullptr ) ), tree_node_handle() );
        }

        size_t offset_into_page = (PAGE_DATA_SIZE - space_left_in_cur_page_);
        T *obj_ptr = (T *) (page_ptr->data_ + offset_into_page);
        space_left_in_cur_page_ -= node_size;

        tree_node_handle meta_ptr( page_ptr->header_.page_id_,
                offset_into_page );
        
        return std::make_pair( pinned_node_ptr( buffer_pool_, obj_ptr,
                    page_ptr ), std::move(meta_ptr) );
    }

    void free( tree_node_handle handle, size_t alloc_size ) {
        free_list_.push_back( std::make_pair( handle, alloc_size ) );
    }

    template <typename T>
    pinned_node_ptr<T> get_tree_node( tree_node_handle node_ptr ) {
        page *page_ptr = buffer_pool_.get_page( node_ptr.get_page_id() );
        assert( page_ptr != nullptr );
        T *obj_ptr = (T *) (page_ptr->data_ + node_ptr.get_offset() );
        return pinned_node_ptr( buffer_pool_, obj_ptr, page_ptr );
    }

    buffer_pool buffer_pool_;

protected:
    page *get_page_to_alloc_on( size_t object_size );

    size_t space_left_in_cur_page_;
    size_t cur_page_;
    std::list<std::pair<tree_node_handle,size_t>> free_list_;
};
