#define BOOST_TEST_MODULE "lex_key_comment_test"
#include <toml/lexer.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/optional/optional_io.hpp>
#include <iostream>
#include <iomanip>
#include "lex_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_bare_key)
{
    TOML_LEX_CHECK_ACCEPT(lex_key, "barekey",  "barekey");
    TOML_LEX_CHECK_ACCEPT(lex_key, "bare-key", "bare-key");
    TOML_LEX_CHECK_ACCEPT(lex_key, "bare_key", "bare_key");
    TOML_LEX_CHECK_ACCEPT(lex_key, "1234",     "1234");
}

BOOST_AUTO_TEST_CASE(test_quoted_key)
{
    TOML_LEX_CHECK_ACCEPT(lex_key, "\"127.0.0.1\"", "\"127.0.0.1\"");
    TOML_LEX_CHECK_ACCEPT(lex_key, "\"character encoding\"", "\"character encoding\"");
    TOML_LEX_CHECK_ACCEPT(lex_key, "\"ʎǝʞ\"", "\"ʎǝʞ\"");
    TOML_LEX_CHECK_ACCEPT(lex_key, "'key2'", "'key2'");
    TOML_LEX_CHECK_ACCEPT(lex_key, "'quoted \"value\"'", "'quoted \"value\"'");
}

BOOST_AUTO_TEST_CASE(test_dotted_key)
{
    TOML_LEX_CHECK_ACCEPT(lex_key, "physical.color", "physical.color");
    TOML_LEX_CHECK_ACCEPT(lex_key, "physical.shape", "physical.shape");
    TOML_LEX_CHECK_ACCEPT(lex_key, "x.y.z.w", "x.y.z.w");
    TOML_LEX_CHECK_ACCEPT(lex_key, "site.\"google.com\"", "site.\"google.com\"");
}

BOOST_AUTO_TEST_CASE(test_comment)
{
    TOML_LEX_CHECK_ACCEPT(lex_comment, "#   ",     "#   ");
    TOML_LEX_CHECK_ACCEPT(lex_comment, "#   \n",   "#   ");
    TOML_LEX_CHECK_ACCEPT(lex_comment, "#   \r\n", "#   ");
    TOML_LEX_CHECK_ACCEPT(lex_comment, "# # \n",   "# # ");
}
