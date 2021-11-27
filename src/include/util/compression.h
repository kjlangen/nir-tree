#pragma once

#include <util/geometry.h>
#include <util/repacking.h>
#include <vector>
#include <bitset>
#include <cinttypes>
#include <cmath>
#include <arpa/inet.h>

#if __BIG_ENDIAN__
# define htonll(x) (x)
# define ntohll(x) (x)
#else
# define htonll(x) ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32)
# define ntohll(x) ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32)
#endif


template <typename T>
concept RectangleIterator = std::is_same_v<Rectangle, typename std::iterator_traits<T>::value_type>;

template <RectangleIterator iter>
Point compute_centroid( iter begin, iter end, unsigned rectangle_count ) {
    Point centroid = Point::atOrigin;
    for( auto rect_iter = begin; rect_iter != end; rect_iter++ ) {
        centroid += rect_iter->lowerLeft;
        centroid += rect_iter->upperRight;
    }
    return centroid / (2.0 * rectangle_count);
}

// Push a one bit_write_loc at bit offset bit_mask_offset
uint8_t *set_one( uint8_t *bit_write_loc, uint8_t *bit_mask_offset );

// Push a zero bit_write_loc at bit offset bit_mask_offset
uint8_t *set_zero( uint8_t *bit_write_loc, uint8_t *bit_mask_offset );

// Write n bits from xor_bits into the destination starting at bit_write_loc, starting 
// from bit_mask_offset bits in.
uint8_t *write_n_bits(
    uint8_t *bit_write_loc,
    uint8_t *bit_mask_offset,
    uint64_t bits_to_write,
    uint64_t xor_bits
);

// Write xor_bits into the destination starting at bit_write_loc, with
// an offset of bit_mask_offset bits. Writes 
// bits_required_to_represent bits there, even if the value is bigger.
uint8_t *set_xor_bits(
    uint8_t *bit_write_loc,
    uint8_t *bit_mask_offset,
    uint64_t bits_required_to_represent,
    uint64_t xor_bits
);

// Writes the lower or upper values for a dimension of the rectangles in
// iterator into the buffer, starting at bit_mask_offset bits in.
// Centroid bits is assumed to be a uint64_t pointer to double data,
// used solely for bit masking. I'm pretty sure this violates C standard type
// aliasing rules, but it seems to work well enough in practice. 
template <RectangleIterator iter>
int encode_dimension_corner(
    char *buffer,
    uint8_t *bit_mask_offset,
    unsigned dimension,
    bool is_lower_corner,
    uint64_t *centroid_bits,
    iter begin,
    iter end
) {
    double last_primary_val = * (double *) centroid_bits;
    double last_secondary_val = * (double *) centroid_bits;
    uint8_t *bit_write_loc = (uint8_t *) buffer;

    for( auto rect_iter = begin; rect_iter != end; rect_iter++ ) {
        double corner_val_primary = is_lower_corner ?
            rect_iter->lowerLeft[dimension] :
            rect_iter->upperRight[dimension];
        double corner_val_secondary = is_lower_corner ?
            rect_iter->upperRight[dimension] :
            rect_iter->lowerLeft[dimension];

        if( corner_val_primary == last_primary_val ) {
            bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        } else if( corner_val_primary == last_secondary_val ) {
            bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
            bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        } else {
            uint64_t *primary_bits = (uint64_t *) &corner_val_primary;
            uint64_t xor_bits = *primary_bits ^ *centroid_bits;
            uint64_t bits_required_to_represent;
            if( xor_bits == 0 ) {
                bits_required_to_represent = 1UL;
            } else {
                bits_required_to_represent = std::ceil( log2( xor_bits )
                        );
            }
            bit_write_loc = set_xor_bits( bit_write_loc,
                    bit_mask_offset, bits_required_to_represent,
                    xor_bits );
        }
        last_primary_val = corner_val_primary;
        last_secondary_val = corner_val_secondary;
    }
    return (bit_write_loc - (uint8_t *) buffer);
}

// Write the polygons in the iterator along the given dimension to the
// buffer starting at the bit_mask_offset. Returns how many bytes we
// wrote and updates the bit_mask_offset for the offset into the
// byte.
template <RectangleIterator iter>
int encode_dimension(
    char *buffer,
    uint8_t *bit_mask_offset,
    unsigned dimension,
    Point &centroid,
    iter begin,
    iter end
) {
    static_assert( sizeof(centroid[0]) == sizeof(uint64_t), 
            "Point values mismatch int size to hold bits." );

    // Write the centroid using all bits to buffer, update our mask and
    // offset accordingly. No tag for centroid.
    uint64_t *centroid_bits = (uint64_t *) &(centroid[dimension]);
    uint8_t *buffer_ptr = (uint8_t *) buffer;
    uint8_t *new_ptr = write_n_bits( buffer_ptr, bit_mask_offset, 64, *centroid_bits );
    int offset = (new_ptr - buffer_ptr);
    assert( offset == 8 );

    offset += encode_dimension_corner( buffer+offset, bit_mask_offset, dimension, true, centroid_bits, begin, end );
    offset += encode_dimension_corner( buffer+offset, bit_mask_offset, dimension, false, centroid_bits, begin, end );
    return offset;
}


// We can't know ahead of time how much space we need for this polygon.
// We can precompute an upper bound and prealloc that space, but that
// would ruin packing for tree nodes
// So, this function mallocs a buffer that is overprovisioned and writes
// the polygon there. We return a pointer to the allocd and filled
// buffer, and the size that we actually used.
template <RectangleIterator iter>
std::pair<char *, int> compress_polygon( iter begin, iter end, unsigned rectangle_count ) {
    Point centroid = compute_centroid( begin, end, rectangle_count );

    char *buffer = (char *) malloc(
            sizeof(Rectangle)*rectangle_count +
            sizeof(double)*dimensions + sizeof(uint16_t) +
            rectangle_count*1 );
    assert( rectangle_count <= std::numeric_limits<uint16_t>::max() );
    uint16_t shrunk_count = (uint16_t) rectangle_count;
    int offset = 0;
    offset += write_data_to_buffer( buffer, &shrunk_count );
    uint8_t bit_mask_offset = 0;
    for( unsigned d = 0; d < dimensions; d++ ) {
        offset += encode_dimension( buffer + offset, &bit_mask_offset, d, centroid, begin,
                end );
    }
    // Pad to end of current byte
    if( bit_mask_offset != 0 ) {
        offset++;
    }
    return std::make_pair( buffer, offset );
}

struct decoded_poly_data {
    struct decoded_dimension {
        std::vector<double> lower_;
        std::vector<double> upper_;
    };
    decoded_dimension data_[dimensions];
};

enum LengthTagBits {
    REPEAT = 0,
    CONTINUE = 1,
    SHORT = 2,
    LONG = 3,
    ALL = 4
};

// Given that a length tag starts at buffer at bit offset offset_mask,
// read that length tag and return it, updating the offset_mask and
// offset_to_update accordingly
LengthTagBits interpret_tag_bits(
    char *buffer,
    uint8_t *offset_mask,
    int *offset_to_update
); 

// Interpret the next n bits starting at buffer with an offset of
// offset_mask as a network ordered double xor'd with centroid. Update
// the mask and offset accordingly.
uint64_t read_n_bits_internal(
    int n,
    char *buffer,
    uint8_t *offset_mask,
    int *offset_to_update
); 

double read_n_bits_as_double(
    int n,
    double centroid,
    char *buffer,
    uint8_t *offset_mask,
    int *offset_to_update
);

int decode_dimension(
    unsigned d,
    uint16_t rect_count,
    char *buffer,
    uint8_t *offset_mask,
    decoded_poly_data &poly_data
);

IsotheticPolygon decompress_polygon( char *buffer, int *new_offset = nullptr );
