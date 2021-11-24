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
        std::cout << "Hopped a byte forward." << std::endl;
        return bit_write_loc;
    }
    return bit_write_loc;
}

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
        std::cout << "Hopped a byte forward." << std::endl;
        return bit_write_loc;
    }

    return bit_write_loc;
}

uint8_t *write_n_bits(
    uint8_t *bit_write_loc,
    uint8_t *bit_mask_offset,
    uint64_t bits_to_write,
    uint64_t xor_bits
) {
    uint64_t set_bit = 1UL << (bits_to_write-1);
    std::cout << "bits_to_write: " << bits_to_write << std::endl;
    std::cout << "Set bit: " << set_bit << std::endl;
    // This could be more efficient by &'ing in byte-chunks at a time, but
    // we can fix that later if this is effective.
    std::cout << "Going to write " << bits_to_write << " bits." <<
        std::endl;
    std::cout << std::bitset<64>( xor_bits ) << std::endl;
    for( uint64_t i = 0; i < bits_to_write; i++ ) {
        std::cout << "Looking at offset: " << set_bit << std::endl;
        if( set_bit & xor_bits ) {
            std::cout << "Wrote 1" << std::endl;
            bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        } else {
            std::cout << "Wrote 0" << std::endl;
            bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        }
        set_bit >>= 1;
    }

    std::cout << "Done setting bits." << std::endl;
    return bit_write_loc;
}


uint8_t *set_xor_bits(
    uint8_t *bit_write_loc,
    uint8_t *bit_mask_offset,
    uint64_t bits_required_to_represent,
    uint64_t xor_bits
) {
    if( bits_required_to_represent <= 35 ) {
        std::cout << "Setting 35 bits." << std::endl;
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
        bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        std::cout << "Set one, one, zero." << std::endl;
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

template <RectangleIterator iter>
int encode_dimension_corner(
    char *buffer,
    unsigned dimension,
    bool is_lower_corner,
    uint64_t *centroid_bits,
    iter begin,
    iter end
) {
    double last_primary_val = *centroid_bits;
    double last_secondary_val = *centroid_bits;
    uint8_t *bit_write_loc = (uint8_t *) buffer;
    uint8_t bit_mask_offset = 0;

    for( auto rect_iter = begin; rect_iter != end; rect_iter++ ) {
        double corner_val_primary = is_lower_corner ?
            rect_iter->lowerLeft[dimension] :
            rect_iter->upperRight[dimension];
        std::cout << "Going lowerLeft: " << corner_val_primary <<
            std::endl;
        double corner_val_secondary = is_lower_corner ?
            rect_iter->upperRight[dimension] :
            rect_iter->lowerLeft[dimension];
        if( corner_val_primary == last_primary_val ) {
            std::cout << "match use zero." << std::endl;
            bit_write_loc = set_zero( bit_write_loc, &bit_mask_offset );
        } else if( corner_val_primary == last_secondary_val ) {
            bit_write_loc = set_one( bit_write_loc, &bit_mask_offset );
            std::cout << "match use one." << std::endl;
        } else {
            uint64_t *primary_bits = (uint64_t *) &corner_val_primary;
            std::cout << "Primary: " << *primary_bits << std::endl;
            // Flip for mantissa to be on the top
            uint64_t xor_bits = htonll(*primary_bits ^ *centroid_bits);
            std::cout << "Got xor bits: " << xor_bits <<  std::endl;
            uint64_t bits_required_to_represent = log2( xor_bits );
            std::cout << "Needs # bits to rep: " <<
                bits_required_to_represent << std::endl;
            bit_write_loc = set_xor_bits( bit_write_loc,
                    &bit_mask_offset, bits_required_to_represent,
                    xor_bits );
        }
        last_primary_val = corner_val_primary;
        last_secondary_val = corner_val_secondary;
    }
    return 0;
}

template <RectangleIterator iter>
int encode_dimension(
    unsigned dimension,
    Point &centroid,
    iter begin,
    iter end,
    char *buffer
) {
    static_assert( sizeof(centroid[0]) == sizeof(uint64_t), 
            "Point values mismatch int size to hold bits." );
    uint64_t *centroid_bits = (uint64_t *) &(centroid[dimension]);
    int offset = 0;
    offset += write_data_to_buffer( buffer, centroid_bits );

    encode_dimension_corner( buffer, dimension, true, centroid_bits, begin, end );
    return offset;
}

template <RectangleIterator iter>
void compress_polygon( iter begin, iter end, unsigned rectangle_count ) {
    Point centroid = compute_centroid( begin, end, rectangle_count );
    char *buffer = (char *) malloc( sizeof(rectangle_count*8) + sizeof(centroid)
            + sizeof(uint16_t) );
    assert( rectangle_count <= std::numeric_limits<uint16_t>::max() );
    uint16_t shrunk_count = (uint16_t) rectangle_count;
    int offset = 0;
    offset += write_data_to_buffer( buffer, &shrunk_count );
    for( unsigned d = 0; d < dimensions; d++ ) {
        offset += encode_dimension( d, centroid, begin, end, buffer + offset );
    }
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
    LONG = 3
};

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
            offset_mask = 0;
            one_bit = 0b10000000;
        } else {
            one_bit >>= 1;
        }
    }

    // Advance past the zero
    (*offset_mask)++;
    if( *offset_mask == 8 ) {
        (*offset_to_update)++; // Update the offset
        offset_mask = 0;
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
    }
    return LONG;
}

double read_n_bits_as_double(
    int n,
    double centroid,
    char *buffer,
    uint8_t *offset_mask,
    int *offset_to_update
) {
    uint64_t sink = 0;
    int start_bit = n;

    // Create a single set bit, move it to the position we should start
    // filling in bits in the sink
    uint64_t sink_one_bit = 1UL << (start_bit-1);

    uint8_t buffer_one_bit = 0b10000000;
    buffer_one_bit >>= *offset_mask;
    uint8_t *buffer_ptr = (uint8_t *) buffer;

    std::cout << std::bitset<8>( *buffer_ptr ) << std::endl;

    for( int i = 0; i < n; i++ ) {
        // If their bit is set, set our bit
        if( *buffer_ptr & buffer_one_bit ) {
            std::cout << "Set one bit at pos: " << i << std::endl;
            sink |= sink_one_bit;
        }
        // Move to next bit
        sink_one_bit >>= 1;
        (*offset_mask)++;
        buffer_one_bit >>= 1;

        // Move to next byte, reset bit offsets
        if( *offset_mask == 8 ) {
            buffer_ptr++; // walk forward one byte
            (*offset_to_update)++;
            *offset_mask = 0;
            buffer_one_bit = 0b10000000;
        }
    }
    std::cout << "sink bits: " << std::bitset<64>( sink ) << std::endl;
    uint64_t *centroid_bits = (uint64_t *) &centroid;
    sink ^= *centroid_bits;
    std::cout << "sink post xor: " << sink << std::endl;
    sink = ntohll( sink );
    std::cout << "sink post nothll: " << sink << std::endl;
    double converted = * (double *) &sink;
    return converted;
}

int decode_next_dimension(
    unsigned d,
    bool is_lower,
    uint16_t rect_count,
    char *buffer,
    int offset,
    uint8_t *offset_mask,
    decoded_poly_data &poly_data
) {
    // Read centroid, adjust offset
    double centroid = * (double *) (buffer + offset);
    offset += sizeof(double);
    // OK, now the packing starts
    double last_primary_val = centroid;
    double last_secondary_val = centroid;

    LengthTagBits tag = interpret_tag_bits( buffer + offset,
             offset_mask, &offset );
    if( tag == REPEAT ) {
        poly_data.data_[0].lower_.push_back( last_primary_val );
    } else if( tag == CONTINUE ) {
        // We won't have secondary values at this point, so we will need
        // to fill them in post hoc
        // FIXME 
        poly_data.data_[0].lower_.push_back( last_primary_val );
    }  else if( tag == SHORT ) {
        read_n_bits_as_double( 35, centroid, buffer + offset, offset_mask, &offset );
    }
    

    return 0;

}

IsotheticPolygon decompress_polygon( char *buffer ) {
    int offset = 0;
    uint16_t shrunk_count = * (uint16_t *) (buffer + offset);
    buffer += sizeof(uint16_t);
    // X-dimension encoding
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    for( unsigned d = 0; d < dimensions; d++ ) {
        offset = decode_next_dimension( d, true, shrunk_count, buffer,
                offset, &offset_mask, poly_data );
        offset = decode_next_dimension( d, false, shrunk_count, buffer,
                offset, &offset_mask, poly_data );
    }
}
