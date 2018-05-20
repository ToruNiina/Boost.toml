#define BOOST_TEST_MODULE "lex_string_test"
#include <toml/lexer.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/optional/optional_io.hpp>
#include <iostream>
#include <iomanip>
#include "lex_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_string)
{
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\"The quick brown fox jumps over the lazy dog\"",
            "\"The quick brown fox jumps over the lazy dog\"");
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\'The quick brown fox jumps over the lazy dog\'",
            "\'The quick brown fox jumps over the lazy dog\'");
    TOML_LEX_CHECK_ACCEPT(lex_ml_basic_string,
            "\"\"\"The quick brown fox \\\njumps over the lazy dog\"\"\"",
            "\"\"\"The quick brown fox \\\njumps over the lazy dog\"\"\"");
    TOML_LEX_CHECK_ACCEPT(lex_ml_literal_string,
            "'''The quick brown fox \njumps over the lazy dog'''",
            "'''The quick brown fox \njumps over the lazy dog'''");
}

BOOST_AUTO_TEST_CASE(test_basic_string)
{
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"",
            "\"GitHub Cofounder & CEO\\nLikes tater tots and beer.\"");
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\"192.168.1.1\"",
            "\"192.168.1.1\"");
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\"中国\"",
            "\"中国\"");
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\"You'll hate me after this - #\"",
            "\"You'll hate me after this - #\"");
    TOML_LEX_CHECK_ACCEPT(lex_string,
            "\" And when \\\"'s are in the string, along with # \\\"\"",
            "\" And when \\\"'s are in the string, along with # \\\"\"");
}

BOOST_AUTO_TEST_CASE(test_ml_basic_string)
{
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"",
        "\"\"\"\nThe quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\"\"\"");
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"",
        "\"\"\"\\\n  The quick brown \\\n\n  fox jumps over \\\n  the lazy dog.\\\n  \"\"\"");
}

BOOST_AUTO_TEST_CASE(test_literal_string)
{
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "'C:\\Users\\nodejs\\templates'",
        "'C:\\Users\\nodejs\\templates'");
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "'\\\\ServerX\\admin$\\system32\\'",
        "'\\\\ServerX\\admin$\\system32\\'");
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "'Tom \"Dubs\" Preston-Werner'",
        "'Tom \"Dubs\" Preston-Werner'");
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "'<\\i\\c*\\s*>'",
        "'<\\i\\c*\\s*>'");
}

BOOST_AUTO_TEST_CASE(test_ml_literal_string)
{
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "'''I [dw]on't need \\d{2} apples'''",
        "'''I [dw]on't need \\d{2} apples'''");
    TOML_LEX_CHECK_ACCEPT(lex_string,
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''",
        "'''\nThe first newline is\ntrimmed in raw strings.\n   All other whitespace\n   is preserved.\n'''");
}
