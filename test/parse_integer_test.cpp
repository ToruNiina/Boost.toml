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

BOOST_AUTO_TEST_CASE(test_decimal_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value,        "1234", value, toml::value(     1234));
    TOML_PARSE_CHECK_EQUAL(parse_value,       "+1234", value, toml::value(     1234));
    TOML_PARSE_CHECK_EQUAL(parse_value,       "-1234", value, toml::value(    -1234));
    TOML_PARSE_CHECK_EQUAL(parse_value,           "0", value, toml::value(        0));
    TOML_PARSE_CHECK_EQUAL(parse_value,     "1_2_3_4", value, toml::value(     1234));
    TOML_PARSE_CHECK_EQUAL(parse_value,    "+1_2_3_4", value, toml::value(    +1234));
    TOML_PARSE_CHECK_EQUAL(parse_value,    "-1_2_3_4", value, toml::value(    -1234));
    TOML_PARSE_CHECK_EQUAL(parse_value, "123_456_789", value, toml::value(123456789));
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

BOOST_AUTO_TEST_CASE(test_hex_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xDEADBEEF",  value, value(0xDEADBEEF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xdeadbeef",  value, value(0xDEADBEEF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xDEADbeef",  value, value(0xDEADBEEF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xDEAD_BEEF", value, value(0xDEADBEEF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xdead_beef", value, value(0xDEADBEEF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xdead_BEEF", value, value(0xDEADBEEF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0xFF",        value, value(0xFF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0x00FF",      value, value(0xFF));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0x0000FF",    value, value(0xFF));
}

BOOST_AUTO_TEST_CASE(test_oct)
{
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0o777",   integer, 64*7+8*7+7);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0o7_7_7", integer, 64*7+8*7+7);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0o007",   integer, 7);
}

BOOST_AUTO_TEST_CASE(test_oct_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "0o777",   value, value(64*7+8*7+7));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0o7_7_7", value, value(64*7+8*7+7));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0o007",   value, value(7));
}

BOOST_AUTO_TEST_CASE(test_bin)
{
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b10000",    integer, 16);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b010000",   integer, 16);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b01_00_00", integer, 16);
    TOML_PARSE_CHECK_EQUAL(parse_integer, "0b111111",   integer, 63);
}

BOOST_AUTO_TEST_CASE(test_bin_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "0b10000",    value, value(16));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0b010000",   value, value(16));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0b01_00_00", value, value(16));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0b111111",   value, value(63));
}
