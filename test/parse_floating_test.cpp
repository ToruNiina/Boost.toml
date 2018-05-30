#define BOOST_TEST_MODULE "parse_floating_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
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

BOOST_AUTO_TEST_CASE(test_fractional_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "1.0",               value, value( 1.0));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0.1",               value, value( 0.1));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0.001",             value, value( 0.001));
    TOML_PARSE_CHECK_EQUAL(parse_value, "0.100",             value, value( 0.1));
    TOML_PARSE_CHECK_EQUAL(parse_value, "+3.14",             value, value( 3.14));
    TOML_PARSE_CHECK_EQUAL(parse_value, "-3.14",             value, value(-3.14));
    TOML_PARSE_CHECK_EQUAL(parse_value, "3.1415_9265_3589",  value, value( 3.141592653589));
    TOML_PARSE_CHECK_EQUAL(parse_value, "+3.1415_9265_3589", value, value( 3.141592653589));
    TOML_PARSE_CHECK_EQUAL(parse_value, "-3.1415_9265_3589", value, value(-3.141592653589));
    TOML_PARSE_CHECK_EQUAL(parse_value, "123_456.789",       value, value( 123456.789));
    TOML_PARSE_CHECK_EQUAL(parse_value, "+123_456.789",      value, value( 123456.789));
    TOML_PARSE_CHECK_EQUAL(parse_value, "-123_456.789",      value, value(-123456.789));
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

BOOST_AUTO_TEST_CASE(test_exponential_value)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "1e10",       value, value(1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1e+10",      value, value(1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1e-10",      value, value(1e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "+1e10",      value, value(1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "+1e+10",     value, value(1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "+1e-10",     value, value(1e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "-1e10",      value, value(-1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "-1e+10",     value, value(-1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "-1e-10",     value, value(-1e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "123e-10",    value, value(123e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1E10",       value, value(1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1E+10",      value, value(1e10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1E-10",      value, value(1e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "123E-10",    value, value(123e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1_2_3E-10",  value, value(123e-10));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1_2_3E-1_0", value, value(123e-10));
}
BOOST_AUTO_TEST_CASE(test_fe)
{
    TOML_PARSE_CHECK_EQUAL(parse_floating, "6.02e23",          floating, 6.02e23);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "6.02e+23",         floating, 6.02e23);
    TOML_PARSE_CHECK_EQUAL(parse_floating, "1.112_650_06e-17", floating, 1.11265006e-17);
}
BOOST_AUTO_TEST_CASE(test_fe_vaule)
{
    TOML_PARSE_CHECK_EQUAL(parse_value, "6.02e23",          value, value(6.02e23));
    TOML_PARSE_CHECK_EQUAL(parse_value, "6.02e+23",         value, value(6.02e23));
    TOML_PARSE_CHECK_EQUAL(parse_value, "1.112_650_06e-17", value, value(1.11265006e-17));
}

BOOST_AUTO_TEST_CASE(test_inf)
{
    {
        const std::string token("inf");
        std::string::const_iterator iter(token.begin());
        const result<floating, std::string> r =
            parse_floating(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(boost::math::isinf(r.unwrap()));
            BOOST_CHECK(r.unwrap() > 0.0);
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
    {
        const std::string token("+inf");
        std::string::const_iterator iter(token.begin());
        const result<floating, std::string> r =
            parse_floating(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(boost::math::isinf(r.unwrap()));
            BOOST_CHECK(r.unwrap() > 0.0);
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
    {
        const std::string token("-inf");
        std::string::const_iterator iter(token.begin());
        const result<floating, std::string> r =
            parse_floating(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(boost::math::isinf(r.unwrap()));
            BOOST_CHECK(r.unwrap() < 0.0);
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
}

BOOST_AUTO_TEST_CASE(test_inf_value)
{
    {
        const std::string token("inf");
        std::string::const_iterator iter(token.begin());
        const result<value, std::string> r = parse_value(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap().is(value::float_tag));
            BOOST_CHECK(boost::math::isinf(r.unwrap().get<floating>()));
            BOOST_CHECK(r.unwrap().get<floating>() > 0.0);
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
    {
        const std::string token("+inf");
        std::string::const_iterator iter(token.begin());
        const result<value, std::string> r = parse_value(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap().is(value::float_tag));
            BOOST_CHECK(boost::math::isinf(r.unwrap().get<floating>()));
            BOOST_CHECK(r.unwrap().get<floating>() > 0.0);
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
    {
        const std::string token("-inf");
        std::string::const_iterator iter(token.begin());
        const result<value, std::string> r =
            parse_value(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap().is(value::float_tag));
            BOOST_CHECK(boost::math::isinf(r.unwrap().get<floating>()));
            BOOST_CHECK(r.unwrap().get<floating>() < 0.0);
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
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
            BOOST_CHECK(boost::math::isnan(r.unwrap()));
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
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
            BOOST_CHECK(boost::math::isnan(r.unwrap()));
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
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
            BOOST_CHECK(boost::math::isnan(r.unwrap()));
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
}

BOOST_AUTO_TEST_CASE(test_nan_value)
{
    {
        const std::string token("nan");
        std::string::const_iterator iter(token.begin());
        const result<value, std::string> r =
            parse_value(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap().is(value::float_tag));
            BOOST_CHECK(boost::math::isnan(r.unwrap().get<floating>()));
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }

    {
        const std::string token("+nan");
        std::string::const_iterator iter(token.begin());
        const result<value, std::string> r =
            parse_value(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap().is(value::float_tag));
            BOOST_CHECK(boost::math::isnan(r.unwrap().get<floating>()));
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }

    {
        const std::string token("-nan");
        std::string::const_iterator iter(token.begin());
        const result<value, std::string> r =
            parse_value(iter, token.end());
        BOOST_CHECK(r);
        BOOST_CHECK(iter == token.end());
        if(r)
        {
            BOOST_CHECK(r.unwrap().is(value::float_tag));
            BOOST_CHECK(boost::math::isnan(r.unwrap().get<floating>()));
        }
        else
        {
            std::cerr << r.unwrap_err() << std::endl;
        }
    }
}
