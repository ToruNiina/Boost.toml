#define BOOST_TEST_MODULE "parse_string_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"
using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_string)
{
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"The quick brown fox jumps over the lazy dog\"", string,
        string("The quick brown fox jumps over the lazy dog", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\'The quick brown fox jumps over the lazy dog\'", string,
        string("The quick brown fox jumps over the lazy dog", string::literal));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"\"\"The quick brown fox \\\njumps over the lazy dog\"\"\"", string,
        string("The quick brown fox jumps over the lazy dog", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'''The quick brown fox \njumps over the lazy dog'''", string,
        string("The quick brown fox \njumps over the lazy dog", string::literal));
}

BOOST_AUTO_TEST_CASE(test_basic_string)
{
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"", string,
        string("GitHub Cofounder & CEO\nLikes tater tots and beer.", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"192.168.1.1\"", string,
        string("192.168.1.1", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"中国\"", string,
        string("中国", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"You'll hate me after this - #\"", string,
        string("You'll hate me after this - #", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\" And when \\\"'s are in the string, along with # \\\"\"", string,
        string(" And when \"'s are in the string, along with # \"", string::basic));
}

BOOST_AUTO_TEST_CASE(test_ml_basic_string)
{
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"", string,
        string("The quick brown fox jumps over the lazy dog.", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"", string,
        string("The quick brown fox jumps over the lazy dog.", string::basic));
}

BOOST_AUTO_TEST_CASE(test_literal_string)
{
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'C:\\Users\\nodejs\\templates'", string,
        string("C:\\Users\\nodejs\\templates", string::literal));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'\\\\ServerX\\admin$\\system32\\'", string,
        string("\\\\ServerX\\admin$\\system32\\", string::literal));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'Tom \"Dubs\" Preston-Werner'", string,
        string("Tom \"Dubs\" Preston-Werner", string::literal));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'<\\i\\c*\\s*>'", string,
        string("<\\i\\c*\\s*>", string::literal));
}

BOOST_AUTO_TEST_CASE(test_ml_literal_string)
{
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'''I [dw]on't need \\d{2} apples'''", string,
        string("I [dw]on't need \\d{2} apples", string::literal));
    TOML_PARSE_CHECK_EQUAL(parse_string,
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''", string,
        string("The first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n", string::literal));
}

BOOST_AUTO_TEST_CASE(test_unicode)
{
    TOML_PARSE_CHECK_EQUAL(parse_string,
            "\"\\u03B1\\u03B2\\u03B3\"", string,
            string("αβγ", string::basic));
    TOML_PARSE_CHECK_EQUAL(parse_string,
            "\"\\U0001D7AA\"",           string,
            string("𝞪", string::basic));
}
