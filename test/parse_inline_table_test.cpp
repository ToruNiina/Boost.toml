#define BOOST_TEST_MODULE "parse_inline_table_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_inline_table)
{
    TOML_PARSE_CHECK_EQUAL(parse_inline_table, "{}", table, table());
    {
        table t;
        t["foo"] = toml::value(42);
        t["bar"] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_inline_table, "{foo = 42, bar = \"baz\"}", table, t);
    }
    {
        table t;
        table t_sub;
        t_sub["name"] = toml::value("pug");
        t["type"] = toml::value(t_sub);
        TOML_PARSE_CHECK_EQUAL(parse_inline_table, "{type.name = \"pug\"}", table, t);
    }
}

BOOST_AUTO_TEST_CASE(test_inline_table_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "{}", value, value(table()));
    {
        table t;
        t["foo"] = toml::value(42);
        t["bar"] = toml::value("baz");
        TOML_PARSE_CHECK_EQUAL(parse_value, "{foo = 42, bar = \"baz\"}", value, value(t));
    }
    {
        table t;
        table t_sub;
        t_sub["name"] = toml::value("pug");
        t["type"] = toml::value(t_sub);
        TOML_PARSE_CHECK_EQUAL(parse_value, "{type.name = \"pug\"}", value, value(t));
    }
}
