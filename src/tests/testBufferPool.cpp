#include <catch2/catch.hpp>
#include <storage/buffer_pool.h>
#include <storage/page.h>
#include <iostream>

#include <unistd.h>
#include <sys/stat.h>

TEST_CASE("Storage: Allocation Counts") {

    {
        size_t num_pages = 10;
        buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
        REQUIRE( bp.get_in_memory_page_count() == num_pages );
    }
    {
        size_t num_pages = 23;
        buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
        REQUIRE( bp.get_in_memory_page_count() == num_pages );
    }
    {
        buffer_pool bp( 1000000 /* 1 MB */, "file_backing.db"  );
        REQUIRE( bp.get_in_memory_page_count() == 245 /* pages */ );
    }

}

TEST_CASE("Storage: Create 10 pages") {

    size_t num_pages = 10;
    buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db"  );

    // Destroy existing data, if any
    unlink( bp.get_backing_file_name().c_str() );
    bp.initialize();

    // Should be 10 disk-backed pages.
    struct stat stat_buffer;
    int rc = stat( bp.get_backing_file_name().c_str(), &stat_buffer );
    assert( rc == 0 );
    REQUIRE( stat_buffer.st_size == (off_t) (PAGE_SIZE * num_pages) );

    for( size_t i = 0; i < num_pages; i++ ) {
        page *page_ptr = bp.get_page( i );
        REQUIRE( page_ptr->header_.page_id_ == i );
    }
    REQUIRE( bp.get_highest_allocated_page_id() == num_pages-1 );
}

TEST_CASE("Storage: Read 10 existing pages") {
    size_t num_pages = 10;

    //Set up 10 pages with specific data on them
    {
        buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
        // Destroy existing data, if any
        unlink( bp.get_backing_file_name().c_str() );
        bp.initialize();

        for( size_t i = 0; i < num_pages; i++ ) {
            page *page_ptr = bp.get_page( i );
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                int_data_ptr[write_count] = i;
            }
            bp.writeback_page( i );
        }
    }

    // Now read those pages, confirm we got the special data
    {
        buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
        bp.initialize();
        for( size_t i = 0; i < num_pages; i++ ) {
            page *page_ptr = bp.get_page(i);
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                REQUIRE( int_data_ptr[write_count] == i );
            }
        }
    }
}

TEST_CASE( "Storage: More pages than memory" ) {
    size_t num_pages = 10;

    //Set up 15 pages with specific data on them
    {
        buffer_pool bp( PAGE_SIZE * (num_pages+5), "file_backing.db" );
        // Destroy existing data, if any
        unlink( bp.get_backing_file_name().c_str() );
        bp.initialize();

        for( size_t i = 0; i < num_pages+5; i++ ) {
            page *page_ptr = bp.get_page( i );
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                int_data_ptr[write_count] = i;
            }
            bp.writeback_page( i );
        }
    }

    // Now set up a pool with only 10 pages, But check we can access all
    // 15 pages and that they have the right content
    {
        buffer_pool bp( PAGE_SIZE * (num_pages), "file_backing.db" );
        bp.initialize();
        for( size_t i = 0; i < num_pages+5; i++ ) {
            page *page_ptr = bp.get_page(i);
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                REQUIRE( int_data_ptr[write_count] == i );
            }
        }
    }
}

TEST_CASE( "Storage: More pages than memory (through new_page)" ) {
    size_t num_pages = 10;

    // Set up 10 pages
    {
        buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
        // Destroy existing data, if any
        unlink( bp.get_backing_file_name().c_str() );
        bp.initialize();

        for( size_t i = 0; i < num_pages; i++ ) {
            page *page_ptr = bp.create_new_page();
            REQUIRE( page_ptr->header_.page_id_ == num_pages + i );
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                int_data_ptr[write_count] = num_pages + i;
            }
            bp.writeback_page( num_pages + i );
        }

        for( size_t i = 0; i < num_pages; i++ ) {
            page *page_ptr = bp.get_page( i );
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                int_data_ptr[write_count] = i;
            }
            bp.writeback_page( i );
        }
        REQUIRE( bp.get_highest_allocated_page_id() == 2*num_pages-1);
    }

    // Now set up a pool with only 10 pages, But check we can access all
    // 15 pages and that they have the right content
    {
        buffer_pool bp( PAGE_SIZE * (num_pages), "file_backing.db" );
        bp.initialize();
        for( size_t i = 0; i < 2*num_pages; i++ ) {
            page *page_ptr = bp.get_page(i);
            size_t *int_data_ptr = (size_t *) page_ptr->data_;
            for( size_t write_count = 0; write_count <
                    (sizeof(page_ptr->data_)) /
                    sizeof(size_t); write_count++ ) {
                REQUIRE( int_data_ptr[write_count] == i );
            }
        }
        REQUIRE( bp.get_highest_allocated_page_id() == 2*num_pages-1);
    }
}

TEST_CASE( "Storage: Test Clock Page Replacement Strategy" ) {

    // Make 10 pages
    size_t num_pages = 10;

    buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
    unlink( bp.get_backing_file_name().c_str() );
    bp.initialize();

    for( size_t i = 0; i < num_pages; i++ ) {
        // Materialize the pages (otherwise they are on the freelist)
        bp.get_page( i );
    }

    bp.create_new_page(); //replace 0, create 10
    bp.get_page(1);
    bp.get_page(2);
    bp.create_new_page(); //replace 3, create 11
    bp.get_page(4);
    bp.get_page(5);
    bp.create_new_page(); //replace 6, create 12


    REQUIRE( not bp.is_page_in_memory( 0 ) );
    REQUIRE( bp.is_page_in_memory( 1 ) );
    REQUIRE( bp.is_page_in_memory( 2 ) );
    REQUIRE( not bp.is_page_in_memory( 3 ) );
    REQUIRE( bp.is_page_in_memory( 4 ) );
    REQUIRE( bp.is_page_in_memory( 5 ) );
    REQUIRE( not bp.is_page_in_memory( 6 ) );
    REQUIRE( bp.is_page_in_memory( 7 ) );
    REQUIRE( bp.is_page_in_memory( 8 ) );
    REQUIRE( bp.is_page_in_memory( 9 ) );
    REQUIRE( bp.is_page_in_memory( 10 ) );
    REQUIRE( bp.is_page_in_memory( 11 ) );
    REQUIRE( bp.is_page_in_memory( 12 ) );

    bp.get_page(7);
    bp.get_page(8);
    bp.get_page(9);
    bp.get_page(10); 
    bp.create_new_page(); //replace 1, create 13

    REQUIRE( not bp.is_page_in_memory( 1 ) );
    REQUIRE( bp.is_page_in_memory( 13 ) );

}

TEST_CASE( "Storage: Buffer Pool Pinning" ) {

    // 3 pages in memory
    size_t num_pages = 3;
    buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
    unlink( bp.get_backing_file_name().c_str() );
    bp.initialize();

    for( size_t i = 0; i < num_pages; i++ ) {
        bp.get_page(i); // Materialize the pages
    }

    page *p0 = bp.get_page(0);
    bp.pin_page( p0 );

    bp.create_new_page(); // create page 3

    REQUIRE( bp.is_page_in_memory( 0 ) );
    REQUIRE( not bp.is_page_in_memory( 1 ) );
    REQUIRE( bp.is_page_in_memory( 2 ) );
    REQUIRE( bp.is_page_in_memory( 3 ) );
}

TEST_CASE( "Storage: Buffer Pool All Pages Pinned" ) {
    // 3 pages in memory
    size_t num_pages = 3;
    buffer_pool bp( PAGE_SIZE * num_pages, "file_backing.db" );
    unlink( bp.get_backing_file_name().c_str() );
    bp.initialize();

    for( size_t i = 0; i < num_pages; i++ ) {
        page *p = bp.get_page(i); // Materialize the pages
        bp.pin_page( p ); // pin it
    }

    REQUIRE( not bp.create_new_page() );

    REQUIRE( bp.is_page_in_memory( 0 ) );
    REQUIRE( bp.is_page_in_memory( 1 ) );
    REQUIRE( bp.is_page_in_memory( 2 ) );
}
