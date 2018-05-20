#define BOOST_TEST_MODULE "parse_integer_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_decimal)
{
    TOML_PARSE_CHECK_EQUAL(parse_integer,        "1234", integer,       1234);
    TOML_PARSE_CHECK_EQUAL(parse_integer,       "+1234", integer,       1234);
    TOML_PARSE_CHECK_EQUAL(parse_integer,       "-1234", integer,      -1234);
    TOML_PARSE_CHECK_EQUAL(parse_integer,           "0", integer,          0);
    TOML_PARSE_CHECK_EQUAL(parse_integer,     "1_2_3_4", integer,       1234);
    TOML_PARSE_CHECK_EQUAL(parse_integer,    "+1_2_3_4", integer,      +1234);
    TOML_PARSE_CHECK_EQUAL(parse_integer,    "-1_2_3_4", integer,      -1234);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "123_456_789", integer,  123456789);
}

BOOST_AUTO_TEST_CASE(test_hex)
{
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xDEADBEEF",  integer, 0xDEADBEEF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xdeadbeef",  integer, 0xDEADBEEF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xDEADbeef",  integer, 0xDEADBEEF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xDEAD_BEEF", integer, 0xDEADBEEF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xdead_beef", integer, 0xDEADBEEF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xdead_BEEF", integer, 0xDEADBEEF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0xFF",        integer, 0xFF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0x00FF",      integer, 0xFF);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0x0000FF",    integer, 0xFF);
}

BOOST_AUTO_TEST_CASE(test_oct)
{
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0o777",   integer, 64*7+8*7+7);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0o7_7_7", integer, 64*7+8*7+7);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0o007",   integer, 7);
}

BOOST_AUTO_TEST_CASE(test_bin)
{
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b10000",    integer, 16);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b010000",   integer, 16);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b01_00_00", integer, 16);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b111111",   integer, 63);
}
