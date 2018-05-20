#define BOOST_TEST_MODULE "parse_floating_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_fractional)
{
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1.0",               floating,  1.0);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "0.1",               floating,  0.1);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "0.001",             floating,  0.001);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "0.100",             floating,  0.1);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+3.14",             floating,  3.14);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-3.14",             floating, -3.14);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "3.1415_9265_3589",  floating,  3.141592653589);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+3.1415_9265_3589", floating,  3.141592653589);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-3.1415_9265_3589", floating, -3.141592653589);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "123_456.789",       floating,  123456.789);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+123_456.789",      floating,  123456.789);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-123_456.789",      floating, -123456.789);
}

BOOST_AUTO_TEST_CASE(test_exponential)
{
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1e10",       floating, 1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1e+10",      floating, 1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1e-10",      floating, 1e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+1e10",      floating, 1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+1e+10",     floating, 1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+1e-10",     floating, 1e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-1e10",      floating, -1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-1e+10",     floating, -1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-1e-10",     floating, -1e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "123e-10",    floating, 123e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1E10",       floating, 1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1E+10",      floating, 1e10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1E-10",      floating, 1e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "123E-10",    floating, 123e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1_2_3E-10",  floating, 123e-10);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1_2_3E-1_0", floating, 123e-10);
}

BOOST_AUTO_TEST_CASE(test_fe)
{
    TOML_PARSE_CHECK_EQUAL(parse_floating, "6.02e23",          floating, 6.02e23);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "6.02e+23",         floating, 6.02e23);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1.112_650_06e-17", floating, 1.11265006e-17);
}

BOOST_AUTO_TEST_CASE(test_inf)
{
    TOML_PARSE_CHECK_EQUAL(parse_floating, "inf",  floating,
            std::numeric_limits<toml::floating>::infinity());
    TOML_PARSE_CHECK_EQUAL(parse_floating, "+inf", floating,
            std::numeric_limits<toml::floating>::infinity());
    TOML_PARSE_CHECK_EQUAL(parse_floating, "-inf", floating,
            -std::numeric_limits<toml::floating>::infinity());
}

BOOST_AUTO_TEST_CASE(test_nan)
{
    {
        const std::string token("nan");
        std::string::const_iterator iter(token.begin());
        const result<floating, std::string> r =
            parse_floating(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap() != r.unwrap());
        }
        else
        {
            std::cerr << r << std::endl;
        }
    }

    {
        const std::string token("+nan");
        std::string::const_iterator iter(token.begin());
        const result<floating, std::string> r =
            parse_floating(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap() != r.unwrap());
        }
        else
        {
            std::cerr << r << std::endl;
        }
    }

    {
        const std::string token("-nan");
        std::string::const_iterator iter(token.begin());
        const result<floating, std::string> r =
            parse_floating(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap() != r.unwrap());
        }
        else
        {
            std::cerr << r << std::endl;
        }
    }
}
