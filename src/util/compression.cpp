#include <util/compression.h>

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

IsotheticPolygon decompress_polygon( char *buffer, int *new_offset ) {
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

    if( offset_mask != 0 ) {
        offset++;
    }
    if( new_offset != nullptr ) {
        *new_offset = offset;
    }
    return polygon;
}
