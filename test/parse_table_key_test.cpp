#define BOOST_TEST_MODULE "parse_table_key_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/optional/optional_io.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_table_bare_key)
{
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[barekey]",  std::vector<key>, std::vector<key>(1, "barekey"));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[bare-key]", std::vector<key>, std::vector<key>(1, "bare-key"));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[bare_key]", std::vector<key>, std::vector<key>(1, "bare_key"));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[1234]",     std::vector<key>, std::vector<key>(1, "1234"));
}

BOOST_AUTO_TEST_CASE(test_table_quoted_key)
{
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[\"127.0.0.1\"]",          std::vector<key>, std::vector<key>(1, "127.0.0.1"         ));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[\"character encoding\"]", std::vector<key>, std::vector<key>(1, "character encoding"));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "[\"ʎǝʞ\"]",                std::vector<key>, std::vector<key>(1, "ʎǝʞ"               ));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "['key2']",                 std::vector<key>, std::vector<key>(1, "key2"              ));
    TOML_PARSE_CHECK_EQUAL(parse_table_key, "['quoted \"value\"']",     std::vector<key>, std::vector<key>(1, "quoted \"value\""  ));
}

BOOST_AUTO_TEST_CASE(test_table_dotted_key)
{
    {
        std::vector<key> keys(2);
        keys[0] = "physical";
        keys[1] = "color";
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[physical.color]", std::vector<key>, keys);
    }
    {
        std::vector<key> keys(2);
        keys[0] = "physical";
        keys[1] = "shape";
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[physical.shape]", std::vector<key>, keys);
    }
    {
        std::vector<key> keys(4);
        keys[0] = "x";
        keys[1] = "y";
        keys[2] = "z";
        keys[3] = "w";
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[x.y.z.w]",         std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[ x.y.z.w ]",       std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[x. y. z. w]",      std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[x .y .z .w]",      std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[x .y. z .w]",      std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[x . y . z . w]",   std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[ x . y . z . w ]", std::vector<key>, keys);
    }
    {
        std::vector<key> keys(2);
        keys[0] = "site";
        keys[1] = "google.com";
        TOML_PARSE_CHECK_EQUAL(parse_table_key, "[site.\"google.com\"]", std::vector<key>, keys);
    }
}

BOOST_AUTO_TEST_CASE(test_array_of_table_bare_key)
{
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[barekey]]",  std::vector<key>, std::vector<key>(1, "barekey"));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[bare-key]]", std::vector<key>, std::vector<key>(1, "bare-key"));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[bare_key]]", std::vector<key>, std::vector<key>(1, "bare_key"));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[1234]]",     std::vector<key>, std::vector<key>(1, "1234"));
}

BOOST_AUTO_TEST_CASE(test_array_of_table_quoted_key)
{
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[\"127.0.0.1\"]]",          std::vector<key>, std::vector<key>(1, "127.0.0.1"         ));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[\"character encoding\"]]", std::vector<key>, std::vector<key>(1, "character encoding"));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[\"ʎǝʞ\"]]",                std::vector<key>, std::vector<key>(1, "ʎǝʞ"               ));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[['key2']]",                 std::vector<key>, std::vector<key>(1, "key2"              ));
    TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[['quoted \"value\"']]",     std::vector<key>, std::vector<key>(1, "quoted \"value\""  ));
}

BOOST_AUTO_TEST_CASE(test_array_of_table_dotted_key)
{
    {
        std::vector<key> keys(2);
        keys[0] = "physical";
        keys[1] = "color";
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[physical.color]]", std::vector<key>, keys);
    }
    {
        std::vector<key> keys(2);
        keys[0] = "physical";
        keys[1] = "shape";
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[physical.shape]]", std::vector<key>, keys);
    }
    {
        std::vector<key> keys(4);
        keys[0] = "x";
        keys[1] = "y";
        keys[2] = "z";
        keys[3] = "w";
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[x.y.z.w]]",         std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[ x.y.z.w ]]",       std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[x. y. z. w]]",      std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[x .y .z .w]]",      std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[x .y. z .w]]",      std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[x . y . z . w]]",   std::vector<key>, keys);
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[ x . y . z . w ]]", std::vector<key>, keys);

    }
    {
        std::vector<key> keys(2);
        keys[0] = "site";
        keys[1] = "google.com";
        TOML_PARSE_CHECK_EQUAL(parse_array_table_key, "[[site.\"google.com\"]]", std::vector<key>, keys);
    }
}
