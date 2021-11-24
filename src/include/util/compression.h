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
        std::cout << "Writing rectangle." << std::endl;
        double corner_val_primary = is_lower_corner ?
            rect_iter->lowerLeft[dimension] :
            rect_iter->upperRight[dimension];
        double corner_val_secondary = is_lower_corner ?
            rect_iter->upperRight[dimension] :
            rect_iter->lowerLeft[dimension];
        if( corner_val_primary == last_primary_val ) {
            std::cout << "match use zero." << std::endl;
            bit_write_loc = set_zero( bit_write_loc, bit_mask_offset );
        } else if( corner_val_primary == last_secondary_val ) {
            bit_write_loc = set_one( bit_write_loc, bit_mask_offset );
            std::cout << "match use one." << std::endl;
        } else {
            std::cout << "Priomary val: " << corner_val_primary <<
                std::endl;
            uint64_t *primary_bits = (uint64_t *) &corner_val_primary;
            std::cout << "Primary: " << *primary_bits << std::endl;
            // Flip for mantissa to be on the top
            uint64_t xor_bits = htonll(*primary_bits ^ *centroid_bits);
            std::cout << "Got xor bits: " << xor_bits <<  std::endl;
            uint64_t bits_required_to_represent = log2( xor_bits );
            std::cout << "Needs # bits to rep: " <<
                bits_required_to_represent << std::endl;
            bit_write_loc = set_xor_bits( bit_write_loc,
                    bit_mask_offset, bits_required_to_represent,
                    xor_bits );
        }
        last_primary_val = corner_val_primary;
        last_secondary_val = corner_val_secondary;
        std::cout << "After rectangle, at: " << (bit_write_loc -
                (uint8_t *) buffer) << std::endl;
        std::cout << "Bitmask: offset: " << *bit_mask_offset <<
            std::endl;
    }
    return (bit_write_loc - (uint8_t *) buffer);
}

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
    uint64_t *centroid_bits = (uint64_t *) &(centroid[dimension]);
    int offset = 0;
    std::cout << "Going to write centroid to " << (void *) buffer <<
        std::endl;
    offset += write_data_to_buffer( buffer, centroid_bits );
    std::cout << "Offset is: " << offset << std::endl;
    offset += encode_dimension_corner( buffer+offset, bit_mask_offset, dimension, true, centroid_bits, begin, end );
    std::cout << "End offset of first corner: " << offset << std::endl;
    offset += encode_dimension_corner( buffer+offset, bit_mask_offset, dimension, false, centroid_bits, begin, end );
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
    uint8_t bit_mask_offset = 0;
    for( unsigned d = 0; d < dimensions; d++ ) {
        offset = encode_dimension( buffer + offset, &bit_mask_offset, d, centroid, begin,
                end );
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
    LONG = 3,
    ALL = 4
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

double read_n_bits_as_double(
    int n,
    double centroid,
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
        std::cout << "Internal walk, read byte." << std::endl;
    }

    // We have a few bits left.
    assert( start_bit < 8 ); 

    // There's an edge case here where someone asks us to read fewer
    // than 8 bytes, but starting from an unaligned position so it makes
    // us walk a byte boundary. That will force a loop.
    while( start_bit > 0 ) {
        std::cout << "last iteration. Need bits: " << start_bit <<
            std::endl;
        uint64_t interpreted_byte = *buffer_ptr;
        uint64_t bit_mask = 0b11111111 >> *offset_mask;
        std::cout << "Interpreted byte: " << std::bitset<64>(
                interpreted_byte ) <<  std::endl;
        std::cout << "Bit_mask: " << std::bitset<64>(
                bit_mask) <<  std::endl;


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
            std::cout << "Outer bound, increasing again." << std::endl;
            (*offset_to_update)++;
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

int decode_dimension(
    unsigned d,
    bool is_lower,
    uint16_t rect_count,
    char *buffer,
    uint8_t *offset_mask,
    decoded_poly_data &poly_data
) {
    // Read centroid, adjust offset
    int offset = 0;
    std::cout << "Going to read centroid from: " << (void *) buffer <<
        std::endl;
    double centroid = * (double *) (buffer + offset);
    offset += sizeof(double);
    // OK, now the packing starts
    double last_primary_val = centroid;
    std::cout << "In decode dimension. Obtained Centroid: " << centroid
        << std::endl;

    // Read all primary vals
    for( uint16_t i = 0; i < rect_count; i++ ) {
        LengthTagBits tag = interpret_tag_bits( buffer + offset,
                 offset_mask, &offset );
        if( tag == CONTINUE ) {
            // We won't have secondary values at this point, so we will need
            // to fill them in post hoc
            // Put sentinel value in to fix later
            std::cout << "Got continue tag." << std::endl;
            last_primary_val = std::numeric_limits<double>::infinity();
        } else if( tag == SHORT ) {
            std::cout << "Got short tag." << std::endl;
            last_primary_val = read_n_bits_as_double( 35, centroid, buffer + offset, offset_mask, &offset );
        } else if( tag == LONG ) {
            std::cout << "Got long tag." << std::endl;
            last_primary_val = read_n_bits_as_double( 45, centroid, buffer + offset,
                    offset_mask, &offset );
        } else if( tag == ALL ) {
            std::cout << "Got All tag." << std::endl;
            last_primary_val = read_n_bits_as_double( 64, centroid, buffer + offset,
                    offset_mask, &offset );
        }

        if( tag == REPEAT and last_primary_val ==
                std::numeric_limits<double>::infinity() ) {
            std::cout << "Flipped sign." << std::endl;
            // Flip signed-ness to say we repeating whatever value was
            // there before, not doing another CONTINUE.
            last_primary_val = -std::numeric_limits<double>::infinity();
        }
        std::cout << "Pushing back lower." << last_primary_val <<
            std::endl;
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
        }
        poly_data.data_[d].upper_.push_back( last_primary_val );
    }

    return offset;

}

IsotheticPolygon decompress_polygon( char *buffer ) {
    int offset = 0;
    uint16_t shrunk_count = * (uint16_t *) (buffer + offset);
    buffer += sizeof(uint16_t);
    // X-dimension encoding
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    for( unsigned d = 0; d < dimensions; d++ ) {
        offset = decode_dimension( d, true, shrunk_count, buffer+offset,
                &offset_mask, poly_data );
        offset = decode_dimension( d, false, shrunk_count, buffer+offset,
                 &offset_mask, poly_data );
    }
    assert( false );
}
