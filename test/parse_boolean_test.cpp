#define BOOST_TEST_MODULE "parse_boolean_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>

inline void check_equal(const std::string token, const toml::boolean answer)
{
    const toml::detail::result<toml::boolean, std::string::const_iterator>
        result = toml::detail::parse_boolean(token.begin(), token.end());
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
    const toml::detail::result<toml::boolean, std::string::const_iterator>
        result = toml::detail::parse_boolean(token.begin(), token.end());
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
    const toml::detail::result<toml::boolean, std::string::const_iterator>
        result = toml::detail::parse_boolean(token.begin(), token.end());
    BOOST_CHECK(result.iterator() != token.begin());
    BOOST_CHECK(result.iterator() != token.end());
}

BOOST_AUTO_TEST_CASE(test_boolean_correct)
{
    check_equal("true",  true);
    check_equal("false", false);
}

BOOST_AUTO_TEST_CASE(test_boolean_invalid)
{
    check_fail("True");
    check_fail("False");
    check_fail("tlue");
    check_fail("farse");
}
