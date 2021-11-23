#pragma once
#include <cstring>

template <typename T>
size_t write_data_to_buffer( char *buffer, T *data ) {
    memcpy( buffer, data, sizeof(T));
    return sizeof(T);
}

template <typename T>
inline T *read_pointer_from_buffer( char *buffer ) {
    // Read whatever is at this address as a 64 bit int,
    // which represents a pointer of type T.
    return (T *) (* (uint64_t *) buffer );
}

struct packed_node {
    char buffer_[1]; //dynamically sized
};


enum NodeHandleTypeCodes {
    UNASSIGNED = 0,
    LEAF_NODE = 1,
    BRANCH_NODE = 2,
    BIG_POLYGON = 3,
    REPACKED_LEAF_NODE = 4,
    REPACKED_BRANCH_NODE = 5
};

