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

TEST_CASE( "Compression: Single Dimension Corner Double Rectangle Polygon" ) {
    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );

    // 4 doubles, 1 uint16, 4 leading codes worst case;
    char buffer[64 *4 + 4 + 2];
    memset( buffer, '\0', sizeof(buffer) );
    double centroid_bits = 0.0;
    uint8_t bit_mask_offset = 0;

    int offset = encode_dimension_corner( buffer, &bit_mask_offset, 0, true, (uint64_t *) &centroid_bits,
            rectangles.begin(), rectangles.end() );

    // This function presumes we have already written the count and
    // centroid to the buffer. We straight up start reading the lower
    // bits.

    offset = 0;
    uint8_t offset_mask = 0;
    LengthTagBits tag = interpret_tag_bits( buffer + offset,
            &offset_mask, &offset );
    REQUIRE( tag == SHORT );
    REQUIRE( offset == 0 );
    REQUIRE( offset_mask == 3 );
    double converted = read_n_bits_as_double( 35, centroid_bits, buffer + offset,
            &offset_mask, &offset);
    // We should be pulling out
    // 000  00000000 00000000 11110000 00111111

    // 000 0000 0011 1100 0000
    REQUIRE( converted == 1.0 );
    REQUIRE( offset == 4 );
    REQUIRE( offset_mask == 6 );

    tag = interpret_tag_bits( buffer + offset, &offset_mask, &offset );
    REQUIRE( tag == REPEAT );

}

TEST_CASE( "Compression: Single Dimension Double Rectangle Polygon" ) {
    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );

    // 4 doubles, 1 uint16, 4 leading codes worst case;
    char buffer[64 *4 + 4 + 2];
    memset( buffer, '\0', sizeof(buffer) );
    Point centroid(0.0,0.0);
    uint8_t bit_mask_offset = 0;

    int offset = encode_dimension( buffer, &bit_mask_offset, 0, centroid,
            rectangles.begin(), rectangles.end() );

    offset = 0;
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    decode_dimension( 0, 2, buffer + offset, &offset_mask,
            poly_data );

    REQUIRE( poly_data.data_[0].lower_[0] == 1.0 );
    REQUIRE( poly_data.data_[0].lower_[1] == 1.0 );
    REQUIRE( poly_data.data_[0].upper_[0] == 2.0 );
    REQUIRE( poly_data.data_[0].upper_[1] == 2.0 );

}

TEST_CASE( "Compression: Double Dimension Double Rectangle Polygon" ) {
    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, -1.0, 2.0, -2.0 ) );
    rectangles.push_back( Rectangle( 1.0, -1.0, 2.0, -2.0 ) );

    // 8 doubles, 1 uint16, 8 leading codes worst case;
    char buffer[64 *8 + 8 + 2];
    memset( buffer, '\0', sizeof(buffer) );
    Point centroid(0.0,0.0);
    uint8_t bit_mask_offset = 0;

    int offset = encode_dimension( buffer, &bit_mask_offset, 0, centroid,
            rectangles.begin(), rectangles.end() );
    offset += encode_dimension( buffer+offset, &bit_mask_offset, 1, centroid,
            rectangles.begin(), rectangles.end() );

    int write_offset = offset;
    offset = 0;
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    offset = decode_dimension( 0, 2, buffer + offset, &offset_mask,
            poly_data );
    offset += decode_dimension( 1, 2, buffer + offset, &offset_mask,
            poly_data );

    REQUIRE( poly_data.data_[0].lower_[0] == 1.0 );
    REQUIRE( poly_data.data_[0].lower_[1] == 1.0 );
    REQUIRE( poly_data.data_[0].upper_[0] == 2.0 );
    REQUIRE( poly_data.data_[0].upper_[1] == 2.0 );

    REQUIRE( poly_data.data_[1].lower_[0] == -1.0 );
    REQUIRE( poly_data.data_[1].lower_[1] == -1.0 );
    REQUIRE( poly_data.data_[1].upper_[0] == -2.0 );
    REQUIRE( poly_data.data_[1].upper_[1] == -2.0 );

    REQUIRE( offset == write_offset );

}

// NOW, Add tests to do repeats and continues explicitly.
// Do continues on both axises...
TEST_CASE( "Compression: Axis Continues" ) {

    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, -1.0, 2.0, -2.0 ) );
    rectangles.push_back( Rectangle( 2.0, 0.0, 3.0, -1.0 ) );
    rectangles.push_back( Rectangle( 3.0, 1.0, 4.0, 0.0 ) );

    // 12 doubles, 1 uint16, 12 leading codes worst case;
    char buffer[64*12 + 12 + 2];
    memset( buffer, '\0', sizeof(buffer) );
    Point centroid(0.0,0.0);
    uint8_t bit_mask_offset = 0;

    int offset = encode_dimension( buffer, &bit_mask_offset, 0, centroid,
            rectangles.begin(), rectangles.end() );
    offset += encode_dimension( buffer+offset, &bit_mask_offset, 1, centroid,
            rectangles.begin(), rectangles.end() );

    int write_offset = offset;
    offset = 0;
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    offset = decode_dimension( 0, 3, buffer + offset, &offset_mask,
            poly_data );
    offset += decode_dimension( 1, 3, buffer + offset, &offset_mask,
            poly_data );

    REQUIRE( poly_data.data_[0].lower_[0] == 1.0 );
    REQUIRE( poly_data.data_[0].lower_[1] == 2.0 );
    REQUIRE( poly_data.data_[0].lower_[2] == 3.0 );
    REQUIRE( poly_data.data_[0].upper_[0] == 2.0 );
    REQUIRE( poly_data.data_[0].upper_[1] == 3.0 );
    REQUIRE( poly_data.data_[0].upper_[2] == 4.0 );

    REQUIRE( poly_data.data_[1].lower_[0] == -1.0 );
    REQUIRE( poly_data.data_[1].lower_[1] == 0.0 );
    REQUIRE( poly_data.data_[1].lower_[2] == 1.0 );
    REQUIRE( poly_data.data_[1].upper_[0] == -2.0 );
    REQUIRE( poly_data.data_[1].upper_[1] == -1.0 );
    REQUIRE( poly_data.data_[1].upper_[2] == 0.0 );

    REQUIRE( offset == write_offset );
    REQUIRE( offset <= 60 );
}

TEST_CASE( "Compression: Axis Continues Then Repeats" ) {

    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );
    rectangles.push_back( Rectangle( 2.0, 2.0, 3.0, 3.0 ) );
    rectangles.push_back( Rectangle( 2.0, 3.0, 3.0, 3.0 ) );
    rectangles.push_back( Rectangle( 2.0, 3.0, 3.0, 3.0 ) );

    // 16 doubles, 1 uint16, 16 leading codes worst case;
    char buffer[64*16 + 16 + 2];
    memset( buffer, '\0', sizeof(buffer) );
    Point centroid(0.0,0.0);
    uint8_t bit_mask_offset = 0;

    int offset = encode_dimension( buffer, &bit_mask_offset, 0, centroid,
            rectangles.begin(), rectangles.end() );
    offset += encode_dimension( buffer+offset, &bit_mask_offset, 1, centroid,
            rectangles.begin(), rectangles.end() );

    int write_offset = offset;
    offset = 0;
    uint8_t offset_mask = 0;
    decoded_poly_data poly_data;
    offset = decode_dimension( 0, 4, buffer + offset, &offset_mask,
            poly_data );
    offset += decode_dimension( 1, 4, buffer + offset, &offset_mask,
            poly_data );


    REQUIRE( offset == write_offset );

    REQUIRE( poly_data.data_[0].lower_[0] == 1.0 );
    REQUIRE( poly_data.data_[0].lower_[1] == 2.0 );
    REQUIRE( poly_data.data_[0].lower_[2] == 2.0 );
    REQUIRE( poly_data.data_[0].lower_[3] == 2.0 );
    REQUIRE( poly_data.data_[0].upper_[0] == 2.0 );
    REQUIRE( poly_data.data_[0].upper_[1] == 3.0 );
    REQUIRE( poly_data.data_[0].upper_[2] == 3.0 );
    REQUIRE( poly_data.data_[0].upper_[3] == 3.0 );

    REQUIRE( poly_data.data_[1].lower_[0] == 1.0 );
    REQUIRE( poly_data.data_[1].lower_[1] == 2.0 );
    REQUIRE( poly_data.data_[1].lower_[2] == 3.0 );
    REQUIRE( poly_data.data_[1].lower_[3] == 3.0 );
    REQUIRE( poly_data.data_[1].upper_[0] == 2.0 );
    REQUIRE( poly_data.data_[1].upper_[1] == 3.0 );
    REQUIRE( poly_data.data_[1].upper_[2] == 3.0 );
    REQUIRE( poly_data.data_[1].upper_[3] == 3.0 );
}

TEST_CASE( "Compression: Full compress/decompress workflow" ) {

    std::vector<Rectangle> rectangles;
    rectangles.push_back( Rectangle( 1.0, 1.0, 2.0, 2.0 ) );
    rectangles.push_back( Rectangle( 2.0, 2.0, 3.0, 3.0 ) );
    rectangles.push_back( Rectangle( 2.0, 3.0, 3.0, 3.0 ) );
    rectangles.push_back( Rectangle( 2.0, 3.0, 3.0, 3.0 ) );

    auto compress_result = compress_polygon( rectangles.begin(),
            rectangles.end(), 4 );

    IsotheticPolygon decompress_result = decompress_polygon(
            compress_result.first );

    REQUIRE( decompress_result.basicRectangles.size() == 4 );
    REQUIRE( decompress_result.basicRectangles.at(0) == rectangles.at(0) );
    REQUIRE( decompress_result.basicRectangles.at(1) == rectangles.at(1) );
    REQUIRE( decompress_result.basicRectangles.at(2) == rectangles.at(2) );
    REQUIRE( decompress_result.basicRectangles.at(3) == rectangles.at(3) );
}

TEST_CASE( "Compression: Full compress/decompress real data" ) {

    std::vector<Rectangle> rectangles;
    rectangles.push_back(Rectangle(-122.07329500000000166,
                37.700079999999999814, -122.04237399999999525,
                37.73029499999999814));
    rectangles.push_back(Rectangle(-122.04237399999999525,
                37.700079999999999814, -122.03842199999998286,
                37.726629000000009739));
    rectangles.push_back(Rectangle(-122.03842199999998286,
                37.700079999999999814, -122.03807199999998545,
                37.702030000000000598));
    rectangles.push_back(Rectangle(-122.06309149999998453,
                37.817076000000007241, -122.04237399999999525,
                37.965603500000007386));
    rectangles.push_back(Rectangle(-122.03773949999998649,
                37.817076000000007241, -122.03757199999998306,
                37.965603500000007386));
    rectangles.push_back(Rectangle(-122.03566549999997903,
                37.817076000000007241, -122.0353240000000028,
                37.965603500000007386));
    rectangles.push_back(Rectangle(-122.03492399999997531,
                37.817076000000007241, -122.03482850000000326,
                37.965603500000007386));
    rectangles.push_back(Rectangle(-122.03442399999998713,
                37.817076000000007241, -122.03391699999998821,
                37.965603500000007386));
    rectangles.push_back(Rectangle(-122.03391699999998821,
                37.817076000000007241, -122.03387299999998561,
                37.917422999999999433));
    rectangles.push_back(Rectangle(-122.03387299999998561,
                37.817076000000007241, -122.03386999999999318,
                37.914123000000003572));
    rectangles.push_back(Rectangle(-122.04237399999999525,
                37.817376000000002989, -122.0423564999999968,
                37.930323000000001343));
    rectangles.push_back(Rectangle(-122.0423564999999968,
                37.817376000000002989, -122.03773949999998649,
                37.913181500000000312));
    rectangles.push_back(Rectangle(-122.03757199999998306,
                37.817376000000002989, -122.03566549999997903,
                37.913522999999997865));
    rectangles.push_back(Rectangle(-122.0353240000000028,
                37.817376000000002989, -122.03492399999997531,
                37.93836849999999572));
    rectangles.push_back(Rectangle(-122.03482850000000326,
                37.817376000000002989, -122.03442399999998713,
                37.93836849999999572));

    auto compress_result = compress_polygon( rectangles.begin(),
            rectangles.end(), rectangles.size() );

    IsotheticPolygon decompress_result = decompress_polygon(
            compress_result.first );

    REQUIRE( decompress_result.basicRectangles.size() ==
            rectangles.size() );

    std::cout << "Size for " << rectangles.size() << " = " <<
        compress_result.second << std::endl;

    for( int i = 0; i < rectangles.size(); i++ ) {
        REQUIRE( decompress_result.basicRectangles.at(i) ==
                rectangles.at(i) );
    }
}


TEST_CASE( "Compress: Read N bits short reads" ) {
    char buffer[8];

    // Write 2 bytes to the buffer
    uint8_t val = 0b11001100;
    uint8_t *buffer_ptr = (uint8_t *) buffer;
    *buffer_ptr = val;
    buffer_ptr++;
    *buffer_ptr = val;

    uint8_t offset_mask = 6;
    int offset = 0;
    double centroid_bits = 0.0;
    double converted = read_n_bits_as_double( 6, centroid_bits, buffer,
            &offset_mask, &offset );
    // We should pull out 00|001100
    // That will get ntohll'd, which will put those bytes at the top.
    uint64_t read_value = * (uint64_t *) &converted;
    read_value = htonll( read_value ); // flip bytes back
    REQUIRE( read_value == 12 );
    REQUIRE( offset == 1 );
    REQUIRE( offset_mask == 4 );


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
