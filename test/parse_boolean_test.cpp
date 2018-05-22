#define BOOST_TEST_MODULE "parse_boolean_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_boolean)
{
    TOML_PARSE_CHECK_EQUAL(parse_boolean,  "true", boolean,  true);
    TOML_PARSE_CHECK_EQUAL(parse_boolean, "false", boolean, false);
}

BOOST_AUTO_TEST_CASE(test_boolean_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value,  "true", value, toml::value( true));
    TOML_PARSE_CHECK_EQUAL(parse_value, "false", value, toml::value(false));
}
