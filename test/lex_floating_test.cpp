#define BOOST_TEST_MODULE "lex_floating_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include <limits>

inline void check_equal(const std::string token, const toml::floating answer)
{
    const toml::detail::result<toml::floating, std::string::const_iterator>
        result = toml::detail::parse_floating(token.begin(), token.end());
    BOOST_CHECK(result);
    BOOST_CHECK(result.iterator() == token.end());
    if(!result)
    {
        std::cerr << result << std::endl;
    }
    BOOST_CHECK_EQUAL(result.unwrap(), answer);
}
inline void check_fail(const std::string token)
{
    const toml::detail::result<toml::floating, std::string::const_iterator>
        result = toml::detail::parse_floating(token.begin(), token.end());
    BOOST_CHECK(!result);
    if(result)
    {
        std::cerr << result << std::endl;
    }
    std::cout << "successfully failed. error message is -> "
              << result.unwrap_err() << std::endl;
}
inline void check_partially_read(const std::string token)
{
    const toml::detail::result<toml::floating, std::string::const_iterator>
        result = toml::detail::parse_floating(token.begin(), token.end());
    BOOST_CHECK(result.iterator() != token.begin());
    BOOST_CHECK(result.iterator() != token.end());
}


BOOST_AUTO_TEST_CASE(test_fractional_valid)
{
    check_equal("1.0",               1.0);
    check_equal("0.1",               0.1);
    check_equal("0.001",             0.001);
    check_equal("0.100",             0.1);
    check_equal("+3.14",             3.14);
    check_equal("-3.14",            -3.14);
    check_equal("3.1415_9265_3589",  3.141592653589);
    check_equal("+3.1415_9265_3589", 3.141592653589);
    check_equal("-3.1415_9265_3589", -3.141592653589);
    check_equal("123_456.789",       123456.789);
    check_equal("+123_456.789",      123456.789);
    check_equal("-123_456.789",      -123456.789);
}

BOOST_AUTO_TEST_CASE(test_fractional_invalid)
{
    check_fail("1.");
    check_fail(".0");
    check_fail("01.0");
    check_fail("3,14");
    check_fail("+-1.0");
//     check_fail("1_.0");
    check_fail("1._0");
}

BOOST_AUTO_TEST_CASE(test_exponential_valid)
{
    check_equal("1e10",       1e10);
    check_equal("1e+10",      1e10);
    check_equal("1e-10",      1e-10);
    check_equal("+1e10",      1e10);
    check_equal("+1e+10",     1e10);
    check_equal("+1e-10",     1e-10);
    check_equal("-1e10",     -1e10);
    check_equal("-1e+10",    -1e10);
    check_equal("-1e-10",    -1e-10);
    check_equal("123e-10",    123e-10);
    check_equal("1E10",       1e10);
    check_equal("1E+10",      1e10);
    check_equal("1E-10",      1e-10);
    check_equal("123E-10",    123e-10);
    check_equal("1_2_3E-10",  123e-10);
    check_equal("1_2_3E-1_0", 123e-10);
}
BOOST_AUTO_TEST_CASE(test_exponential_invalid)
{
    check_fail("1e1E0");
    check_fail("1E1e0");
}

BOOST_AUTO_TEST_CASE(test_both_valid)
{
    check_equal("6.02e23",  6.02e23);
    check_equal("6.02e+23", 6.02e23);
    check_equal("1.112_650_06e-17", 1.11265006e-17);
}
BOOST_AUTO_TEST_CASE(test_both_invalid)
{
    check_fail("1e1.0");
    check_fail("01e1.0");
}

BOOST_AUTO_TEST_CASE(test_special_floating_point)
{
    check_equal("inf",   std::numeric_limits<toml::floating>::infinity());
    check_equal("+inf",  std::numeric_limits<toml::floating>::infinity());
    check_equal("-inf", -std::numeric_limits<toml::floating>::infinity());

    {
        std::string token("nan");
        const toml::detail::result<toml::floating, std::string::const_iterator>
            result = toml::detail::parse_floating(token.begin(), token.end());
        BOOST_CHECK(result);
        BOOST_CHECK(result.iterator() == token.end());
        if(!result)
        {
            std::cerr << result << std::endl;
        }
        BOOST_CHECK(result.unwrap() != result.unwrap()); // isnan
    }
}
