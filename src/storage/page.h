#pragma once
#include <cstddef>

#define PAGE_SIZE 4096
#define PAGE_ID_TO_OFFSET( page_id ) page_id


typedef struct page_header {
    // This offset should probably act as the page identifier as well
    // Should probably put the clock counter here
    size_t page_id_;
} page_header;

typedef struct page {
    page_header header_;
    char data_[PAGE_SIZE - sizeof(header_)]; //page size
} page;

static_assert( sizeof(page) == PAGE_SIZE, "Page is not correctly sized" );
