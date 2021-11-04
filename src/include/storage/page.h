#pragma once
#include <cstddef>

#define PAGE_SIZE (4096)
#define PAGE_ID_TO_OFFSET( page_id ) page_id * PAGE_SIZE
#define OFFSET_TO_PAGE_ID( offset ) offset / PAGE_SIZE

typedef struct page_header {
    // This offset should probably act as the page identifier as well
    // Should probably put the clock counter here
    size_t page_id_;
    size_t pin_count_;
    bool clock_active_;
} page_header;

constexpr size_t PAGE_DATA_SIZE = PAGE_SIZE - sizeof(page_header);

typedef struct page {
    page_header header_;
    char data_[PAGE_DATA_SIZE];
} page;


static_assert( sizeof(page) == PAGE_SIZE, "Page is not correctly sized" );
