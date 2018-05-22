#define BOOST_TEST_MODULE "parse_array_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_oneline_array)
{
    TOML_PARSE_CHECK_EQUAL(parse_array, "[]", array, array());
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_array, "[3,1,4,1,5]", array, a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_array, "[\"foo\", \"bar\",  \"baz\"]", array, a);
    }
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_array, "[3,1,4,1,5,]", array, a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_array, "[\"foo\", \"bar\",  \"baz\",]", array, a);
    }
}

BOOST_AUTO_TEST_CASE(test_oneline_array_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "[]", value, toml::value(array()));
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_value, "[3,1,4,1,5]", value, toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_value, "[\"foo\", \"bar\",  \"baz\"]", value, toml::value(a));
    }
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_value, "[3,1,4,1,5,]", value, toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_value, "[\"foo\", \"bar\",  \"baz\",]", value, toml::value(a));
    }
}

BOOST_AUTO_TEST_CASE(test_multiline_array)
{
    TOML_PARSE_CHECK_EQUAL(parse_array, "[\n#comment\n]", array, array());
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_array, "[3,\n1,\n4,\n1,\n5]", array, a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_array, "[\"foo\",\n\"bar\",\n\"baz\"]", array, a);
    }

    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_array, "[3,#comment\n1,#comment\n4,#comment\n1,#comment\n5]", array, a);
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("b#r");
        a[2] = toml::value("b#z");
        TOML_PARSE_CHECK_EQUAL(parse_array, "[\"foo\",#comment\n\"b#r\",#comment\n\"b#z\"#comment\n]", array, a);
    }
}

BOOST_AUTO_TEST_CASE(test_multiline_array_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "[\n#comment\n]", value, toml::value(array()));
    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_value, "[3,\n1,\n4,\n1,\n5]", value, toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("bar");
        a[2] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_value, "[\"foo\",\n\"bar\",\n\"baz\"]", value, toml::value(a));
    }

    {
        array a(5);
        a[0] = toml::value(3); a[1] = toml::value(1); a[2] = toml::value(4);
        a[3] = toml::value(1); a[4] = toml::value(5);
        TOML_PARSE_CHECK_EQUAL(parse_value, "[3,#comment\n1,#comment\n4,#comment\n1,#comment\n5]", value, toml::value(a));
    }
    {
        array a(3);
        a[0] = toml::value("foo"); a[1] = toml::value("b#r");
        a[2] = toml::value("b#z");
        TOML_PARSE_CHECK_EQUAL(parse_value, "[\"foo\",#comment\n\"b#r\",#comment\n\"b#z\"#comment\n]", value, toml::value(a));
    }
}
