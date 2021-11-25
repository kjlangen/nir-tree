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
uint8_t *set_one( uint8_t *bit_write_loc, uint8_t *bit_mask_offset ) {
    assert( *bit_mask_offset <= 7 );
    uint8_t one_bit = 0b10000000;
    // shift it to the right by mask offset to get it in the right spot
    one_bit >>= *bit_mask_offset;
    *bit_write_loc |= one_bit;
    (*bit_mask_offset)++;
    if( *bit_mask_offset == 8 ) {
        *bit_mask_offset = 0;
        // Move one byte forward
        bit_write_loc++;
        return bit_write_loc;
    }
    return bit_write_loc;
}

// Push a zero bit_write_loc at bit offset bit_mask_offset
uint8_t *set_zero( uint8_t *bit_write_loc, uint8_t *bit_mask_offset ) {
    assert( *bit_mask_offset <= 7 );
    uint8_t one_bit = 0b10000000;
    // shift it to the right by mask offset to get it in the right spot
    one_bit >>= *bit_mask_offset;
    one_bit = ~one_bit; // bit flip, now its all ones except that one spot
    *bit_write_loc &= one_bit; // & to turn that bit off
    (*bit_mask_offset)++;
    if( *bit_mask_offset == 8 ) {
        *bit_mask_offset = 0;
        // Move one byte forward
        bit_write_loc++;
        return bit_write_loc;
    }

    return bit_write_loc;
}

// Write n bits from xor_bits into the destination starting at bit_write_loc, starting 
// from bit_mask_offset bits in.
uint8_t *write_n_bits(
    uint8_t *bit_write_loc,
    uint8_t *bit_mask_offset,
    uint64_t bits_to_write,
    uint64_t xor_bits
) {
    uint64_t set_bit = 1UL << (bits_to_write-1);
    // This could be more efficient by &'ing in byte-chunks at a time, but
    // we can fix that later if this is effective.
    for( uint64_t i = 0; i < bits_to_write; i++ ) {
        if( set_bit & xor_bits ) {
            bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        } else {
            bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        }
        set_bit >>= 1;
    }
    return bit_write_loc;
}

// Write xor_bits into the destination starting at bit_write_loc, with
// an offset of bit_mask_offset bits. Writes 
// bits_required_to_represent bits there, even if the value is bigger.
uint8_t *set_xor_bits(
    uint8_t *bit_write_loc,
    uint8_t *bit_mask_offset,
    uint64_t bits_required_to_represent,
    uint64_t xor_bits
) {
    if( bits_required_to_represent <= 35 ) {
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        bit_write_loc = write_n_bits( bit_write_loc, bit_mask_offset,
                35, xor_bits );
    } else if( bits_required_to_represent <= 45 ) {
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        bit_write_loc = write_n_bits( bit_write_loc, bit_mask_offset,
                45, xor_bits );
    } else {
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        bit_write_loc = write_n_bits( bit_write_loc, bit_mask_offset,
                64, xor_bits );
    }

    return bit_write_loc;
}

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
    double last_primary_val = *centroid_bits;
    double last_secondary_val = *centroid_bits;
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
) {
    uint8_t *byte_ptr = (uint8_t *) buffer;
    uint8_t one_bit = 0b10000000;
    one_bit >>= *offset_mask;

    int one_bit_count = 0;
    while( one_bit & *byte_ptr ) {
        one_bit_count++;
        (*offset_mask)++;
        if( *offset_mask == 8 ) {
            byte_ptr++; // Walk one byte forwrd
            (*offset_to_update)++; // Update the offset
            *offset_mask = 0;
            one_bit = 0b10000000;
        } else {
            one_bit >>= 1;
        }
    }

    // Advance past the zero
    (*offset_mask)++;
    if( *offset_mask == 8 ) {
        (*offset_to_update)++; // Update the offset
        *offset_mask = 0;
        one_bit = 0b10000000;
    } else {
        one_bit >>= 1;
    }


    if( one_bit_count == 0 ) {
        return REPEAT;
    } else if( one_bit_count == 1 ) {
        return CONTINUE;
    } else if( one_bit_count == 2 ) {
        return SHORT;
    } else if( one_bit_count == 3 ) {
        return LONG;
    }
    assert( one_bit_count == 4 );
    return ALL;
}

// Interpret the next n bits starting at buffer with an offset of
// offset_mask as a network ordered double xor'd with centroid. Update
// the mask and offset accordingly.
uint64_t read_n_bits_internal(
    int n,
    char *buffer,
    uint8_t *offset_mask,
    int *offset_to_update
) {
    uint64_t sink = 0;
    uint8_t *buffer_ptr = (uint8_t *) buffer;
    int start_bit;

    // Interpret as uint64, only keep bits after the offset
    // mask which indicates starting point
    // Read from the offset_mask until a byte boundary, copying over
    // bits into the right position in sink. Then copy byte at a time
    // until we are out of bits to copy
    for( start_bit = n; start_bit >= 8;  ) {
        uint64_t interpreted_byte = (uint64_t) *buffer_ptr;
        uint64_t bit_mask = 0b11111111 >> (uint64_t) (*offset_mask);
        interpreted_byte &= bit_mask;
        interpreted_byte <<= (start_bit-8+*offset_mask);
        sink |= interpreted_byte;
        start_bit -= (8-*offset_mask);
        *offset_mask = 0;
        buffer_ptr++;
        (*offset_to_update)++;
    }

    // We have a few bits left.
    assert( start_bit < 8 ); 

    // There's an edge case here where someone asks us to read fewer
    // than 8 bytes, but starting from an unaligned position so it makes
    // us walk a byte boundary. That will force a loop.
    while( start_bit > 0 ) {
        uint64_t interpreted_byte = *buffer_ptr;
        uint64_t bit_mask = 0b11111111 >> *offset_mask;
        interpreted_byte &= bit_mask;

        // We may only want a few of these bits if start_bit is small,
        // so shift excess off if needed
        if( start_bit + *offset_mask >= 8 ) {
            interpreted_byte <<= (start_bit-8+*offset_mask);
        } else {
            // What's left
            interpreted_byte >>= (8-start_bit+*offset_mask);
        }
        sink |= interpreted_byte;

        uint64_t bits_read = std::min( 8 - *offset_mask, start_bit );
        start_bit -= bits_read;
        *offset_mask += bits_read;
        // If we crossed a byte boundary, then update so we know about it
        if( *offset_mask >= 8 ) {
            assert( *offset_mask == 8 );
            *offset_mask = 0;
            buffer_ptr++;
            (*offset_to_update)++;
        }
    }
    return sink;
}

double read_n_bits_as_double(
    int n,
    double centroid,
    char *buffer,
    uint8_t *offset_mask,
    int *offset_to_update
) {
    uint64_t sink = read_n_bits_internal( n, buffer, offset_mask,
            offset_to_update );

    uint64_t *centroid_bits = (uint64_t *) &centroid;
    sink ^= *centroid_bits;
    double converted = * (double *) &sink;
    return converted;
}

int decode_dimension(
    unsigned d,
    uint16_t rect_count,
    char *buffer,
    uint8_t *offset_mask,
    decoded_poly_data &poly_data
) {
    // Read centroid, adjust offset
    int offset = 0;

    uint64_t centroid_bits = read_n_bits_internal( 64, buffer, offset_mask, &offset );
    double centroid = * (double *) &centroid_bits;

    assert( offset == 8 );

    // OK, now the packing starts
    double last_primary_val = centroid;

    // Read all primary vals
    for( uint16_t i = 0; i < rect_count; i++ ) {
        LengthTagBits tag = interpret_tag_bits( buffer + offset,
                 offset_mask, &offset );
        if( tag == CONTINUE ) {
            // We won't have secondary values at this point, so we will need
            // to fill them in post hoc
            // Put sentinel value in to fix later
            last_primary_val = std::numeric_limits<double>::infinity();
        } else if( tag == SHORT ) {
            last_primary_val = read_n_bits_as_double( 35, centroid, buffer + offset, offset_mask, &offset );
        } else if( tag == LONG ) {
            last_primary_val = read_n_bits_as_double( 45, centroid, buffer + offset,
                    offset_mask, &offset );
        } else if( tag == ALL ) {
            last_primary_val = read_n_bits_as_double( 64, centroid, buffer + offset,
                    offset_mask, &offset );
        } else {
            assert( tag == REPEAT );
        }

        if( tag == REPEAT and std::isinf(last_primary_val) ) {
            // Flip signed-ness to say we repeating whatever value was
            // there before, not doing another CONTINUE.
            last_primary_val = -std::numeric_limits<double>::infinity();
        }
        poly_data.data_[d].lower_.push_back( last_primary_val );
    }
    assert( poly_data.data_[d].lower_.size() == rect_count );

    last_primary_val = centroid;
    for( uint16_t i = 0; i < rect_count; i++ ) {

        // First, fix up the stubbed entries in lower if I have them
        if( poly_data.data_[d].lower_.at(i) ==
                std::numeric_limits<double>::infinity() ) {
            poly_data.data_[d].lower_.at(i) = last_primary_val;
        } else if( poly_data.data_[d].lower_.at(i) ==
                -std::numeric_limits<double>::infinity() ) {
            poly_data.data_[d].lower_.at(i) =
                poly_data.data_[d].lower_.at(i-1);
        }

        // OK carry on.
        LengthTagBits tag = interpret_tag_bits( buffer + offset,
                 offset_mask, &offset );
        if( tag == CONTINUE ) {
            last_primary_val = poly_data.data_[d].lower_.at(i-1);
        } else if( tag == SHORT ) {
            last_primary_val = read_n_bits_as_double( 35, centroid, buffer + offset, offset_mask, &offset );
        } else if( tag == LONG ) {
            last_primary_val = read_n_bits_as_double( 45, centroid, buffer + offset,
                    offset_mask, &offset );
        } else if( tag == ALL ) {
            last_primary_val = read_n_bits_as_double( 64, centroid, buffer + offset,
                    offset_mask, &offset );
        } else {
        }
        poly_data.data_[d].upper_.push_back( last_primary_val );
    }
    return offset;

}

IsotheticPolygon decompress_polygon( char *buffer ) {
    int offset = 0;
    uint16_t shrunk_count = * (uint16_t *) (buffer + offset);
    offset += sizeof(uint16_t);
    // X-dimension encoding
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    for( unsigned d = 0; d < dimensions; d++ ) {
        offset += decode_dimension( d,shrunk_count, buffer+offset,
                &offset_mask, poly_data );
    }
    IsotheticPolygon polygon;
    for( uint16_t i = 0; i < shrunk_count; i++ ) {
        Point lowerLeft(
            poly_data.data_[0].lower_[i],
            poly_data.data_[1].lower_[i]
        );
        Point upperRight(
            poly_data.data_[0].upper_[i],
            poly_data.data_[1].upper_[i]
        );
        polygon.basicRectangles.push_back( Rectangle( lowerLeft,
                    upperRight ) );
    }
    polygon.recomputeBoundingBox();
    return polygon;
}
