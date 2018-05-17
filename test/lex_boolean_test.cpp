#define BOOST_TEST_MODULE "parse_boolean_test"
#include <boost/test/included/unit_test.hpp>
#include <toml/lexer.hpp>
#include <iostream>
#include <iomanip>
#include "lex_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_lex_boolean_correct)
{
    TOML_LEX_CHECK_ACCEPT(lex_boolean, "true",  "true");
    TOML_LEX_CHECK_ACCEPT(lex_boolean, "false", "false");

    TOML_LEX_CHECK_ACCEPT(lex_boolean, "true foo bar",  "true");
    TOML_LEX_CHECK_ACCEPT(lex_boolean, "false foo bar", "false");
}

BOOST_AUTO_TEST_CASE(test_boolean_invalid)
{
    TOML_LEX_CHECK_REJECT(lex_boolean, "True");
    TOML_LEX_CHECK_REJECT(lex_boolean, "False");
    TOML_LEX_CHECK_REJECT(lex_boolean, "trua");
    TOML_LEX_CHECK_REJECT(lex_boolean, "falze");
}
