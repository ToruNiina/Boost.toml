#define BOOST_TEST_MODULE "parse_boolean_test"
#include <boost/test/included/unit_test.hpp>
#include <toml/lexer.hpp>
#include <iostream>
#include <iomanip>

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_lex_boolean_correct)
{
    {
        const std::string token("true");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(result);
        if(result)
        {
            BOOST_CHECK_EQUAL(*result, "true");
        }
        BOOST_CHECK(iter == token.end());
    }
    {
        const std::string token("false");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(result);
        if(result)
        {
            BOOST_CHECK_EQUAL(*result, "false");
        }
        BOOST_CHECK(iter == token.end());
    }
}

BOOST_AUTO_TEST_CASE(test_lex_boolean_continues)
{
    {
        const std::string token("true foo bar");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(result);
        if(result)
        {
            BOOST_CHECK_EQUAL(*result, "true");
        }
        BOOST_CHECK(iter == token.begin() + 4);
    }
    {
        const std::string token("false foo bar");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(result);
        if(result)
        {
            BOOST_CHECK_EQUAL(*result, "false");
        }
        BOOST_CHECK(iter == token.begin() + 5);
    }
}

BOOST_AUTO_TEST_CASE(test_boolean_invalid)
{
    {
        const std::string token("True");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(!result);
        BOOST_CHECK(iter != token.end());
    }
    {
        const std::string token("False");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(!result);
        BOOST_CHECK(iter != token.end());
    }
    {
        const std::string token("trua");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(!result);
        BOOST_CHECK(iter != token.end());
    }
    {
        const std::string token("falze");
        std::string::const_iterator iter = token.begin();
        const boost::optional<std::string> result =
            lex_boolean::invoke(iter, token.end());

        BOOST_CHECK(!result);
        BOOST_CHECK(iter != token.end());
    }
}
