#include <catch2/catch.hpp>

#include <bitset>
#include <util/geometry.h>
#include <util/compression.h>
#include <cinttypes>
#include <arpa/inet.h>
#include <cmath>

TEST_CASE("Compression: set ones") {

    uint8_t test_bits = 0;
    uint8_t bit_mask_offset = 0;

    uint8_t *ret_ptr;
    ret_ptr = set_one( &test_bits, &bit_mask_offset );

    REQUIRE( test_bits == 0b10000000 );
    REQUIRE( ret_ptr == &test_bits );
    REQUIRE( bit_mask_offset == 1 );

    ret_ptr = set_one( &test_bits, &bit_mask_offset );
    REQUIRE( test_bits == 0b11000000 );
    REQUIRE( ret_ptr == &test_bits );
    REQUIRE( bit_mask_offset == 2 );

    for( unsigned int i = 0; i < 6; i++ ) {
        ret_ptr = set_one( &test_bits, &bit_mask_offset );
    }
    REQUIRE( test_bits == 0b11111111 );
    REQUIRE( bit_mask_offset == 0 );
    REQUIRE( ret_ptr == ((&test_bits)+1) );

}

TEST_CASE("Compression: set zeros") {

    uint8_t test_bits = 0b11111111;
    uint8_t bit_mask_offset = 0;

    uint8_t *ret_ptr;
    ret_ptr = set_zero( &test_bits, &bit_mask_offset );

    REQUIRE( test_bits == 0b01111111 );
    REQUIRE( ret_ptr == &test_bits );
    REQUIRE( bit_mask_offset == 1 );

    ret_ptr = set_zero( &test_bits, &bit_mask_offset );
    REQUIRE( test_bits == 0b00111111 );
    REQUIRE( ret_ptr == &test_bits );
    REQUIRE( bit_mask_offset == 2 );

    for( unsigned int i = 0; i < 6; i++ ) {
        ret_ptr = set_zero( &test_bits, &bit_mask_offset );
    }
    REQUIRE( test_bits == 0b00000000 );
    REQUIRE( bit_mask_offset == 0 );
    REQUIRE( ret_ptr == ((&test_bits)+1) );
}

TEST_CASE( "Compression: write n bits" ) {

    uint8_t test_bits = 0;
    uint8_t bit_mask_offset = 0;
    uint64_t bits_required_to_represent = 8;
    uint64_t number_to_write = 0b11111111;
    uint8_t *ret_ptr = write_n_bits( &test_bits, &bit_mask_offset,
             bits_required_to_represent, number_to_write);
    REQUIRE( test_bits == 0b11111111 );
    REQUIRE( ret_ptr == (&test_bits)+1 );
    REQUIRE( bit_mask_offset == 0 );

    test_bits = 0;
    number_to_write = 0b0101;
    bits_required_to_represent = 4;
    ret_ptr = write_n_bits( &test_bits, &bit_mask_offset,
             bits_required_to_represent, number_to_write);
    // Because we only wrote the first bits, the rest are zeroed.
    REQUIRE( test_bits == 0b01010000 );
    REQUIRE( ret_ptr == &test_bits );
    REQUIRE( bit_mask_offset == 4 );
    ret_ptr = write_n_bits( &test_bits, &bit_mask_offset,
             bits_required_to_represent, number_to_write);
    REQUIRE( test_bits == 0b01010101 );
    REQUIRE( ret_ptr == (&test_bits)+1 );
    REQUIRE( bit_mask_offset == 0 );
}

TEST_CASE( "Compression: set xor bits" ) {
    uint64_t test_bits = 0;
    uint8_t *bit_ptr = (uint8_t *) &test_bits;
    uint8_t bit_mask_offset = 0;

    uint64_t xor_bits = 0b1111000011110000;
    uint64_t bit_count = 16;

    uint8_t *ret_ptr = set_xor_bits( bit_ptr, &bit_mask_offset, bit_count, xor_bits );
    // Packing will be:
    // 11000000 <- header + 5 leading zeroes from 35 bytes. 30 bytes left
    // 00000000 < - 8 more zeroes, 22 bytes left
    uint16_t *read_bits = (uint16_t *) &test_bits;
    // Since low bits are packed before hand in big endian, its just the
    // low byte
    REQUIRE( *read_bits == 0b11000000 );
    read_bits++;
    // 00000011 <- low bytes
    // 11000011 <- high bytes
    REQUIRE( *read_bits  == 0b1100001100000011 );
    // 11000000 <- remaining bits, then unset zeroes
    // 00000000 <- unset zeroes
    read_bits++;
    REQUIRE( *read_bits == 0b11000000 );
    REQUIRE( bit_mask_offset == 6 );
    REQUIRE( ret_ptr == bit_ptr+4 );
}

TEST_CASE( "Compression: Double Rectangle Polygon" ) {
    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );

    // 4 doubles, 1 uint16, 4 leading codes worst case;
    char buffer[64 *4 + 4 + 2];
    memset( buffer, '\0', sizeof(buffer) );
    double centroid_bits = 0.0;

    int offset = encode_dimension_corner( buffer, 0, true, (uint64_t *) &centroid_bits,
            rectangles.begin(), rectangles.end() );

    // This function presumes we have already written the count and
    // centroid to the buffer. We straight up start reading the lower
    // bits.

    uint8_t offset_mask = 0;
    int offset_to_update = 0;
    LengthTagBits tag = interpret_tag_bits( buffer + offset,
            &offset_mask, &offset_to_update );
    REQUIRE( tag == SHORT );
    REQUIRE( offset_to_update == 0 );
    REQUIRE( offset_mask == 3 );
    double converted = read_n_bits_as_double( 35, centroid_bits, buffer + offset_to_update,
            &offset_mask, &offset_to_update );
    // We should be pulling out
    // 000  00000000 00000000 11110000 00111111
    REQUIRE( converted == 1.0 );

}

TEST_CASE( "Compression: Interpret Tag Bits" ) {
    uint8_t bits = 0b00000000;
    uint8_t offset_mask = 0;
    int offset = 0;
    LengthTagBits tag = interpret_tag_bits( (char *) &bits, &offset_mask,
            &offset );
    REQUIRE( tag == REPEAT );
    REQUIRE( offset_mask == 1 );

    offset_mask = 0;
    bits = 0b10000000;
    tag = interpret_tag_bits( (char *) &bits, &offset_mask,
            &offset );
    REQUIRE( tag == CONTINUE );
    REQUIRE( offset_mask == 2 );
    
    offset_mask = 0;
    bits = 0b11000000;
    tag = interpret_tag_bits( (char *) &bits, &offset_mask,
            &offset );
    REQUIRE( tag == SHORT );
    REQUIRE( offset_mask == 3 );

    offset_mask = 0;
    bits = 0b11100000;
    tag = interpret_tag_bits( (char *) &bits, &offset_mask,
            &offset );
    REQUIRE( tag == LONG );
    REQUIRE( offset_mask == 4 );

    bits = 0b110000;
    offset_mask = 2;
    tag = interpret_tag_bits( (char *) &bits, &offset_mask,
            &offset );
    REQUIRE( tag == SHORT );
    REQUIRE( offset_mask == 5 );
}

/*
TEST_CASE( "Compression: endian test" ) {
    double d1 = -122.06309149999998453;
    double d2 = -122.04237399999999525;
    uint64_t *i1 = (uint64_t *) &d1;
    uint64_t *i2 = (uint64_t *) &d2;
    uint64_t bytes = htonl(*i1 ^ *i2);
    std::cout << bytes << std::endl;
    std::cout << log2( bytes ) << std::endl;
    std::cout << log2( *i1 ^ *i2 ) << std::endl;
    std::cout << std::bitset<64>(*i1) << std::endl;
    std::cout << std::bitset<64>(*i2) << std::endl;
    std::cout << std::bitset<64>(bytes) << std::endl;
    std::cout << std::bitset<64>(*i1 ^ *i2) << std::endl;
}
*/
