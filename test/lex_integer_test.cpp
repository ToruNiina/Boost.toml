#define BOOST_TEST_MODULE "parse_integer_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>

inline void check_equal(const std::string token, const toml::integer answer)
{
    const toml::detail::result<toml::integer, std::string::const_iterator>
        result = toml::detail::parse_integer(token.begin(), token.end());
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
    const toml::detail::result<toml::integer, std::string::const_iterator>
        result = toml::detail::parse_integer(token.begin(), token.end());
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
    const toml::detail::result<toml::integer, std::string::const_iterator>
        result = toml::detail::parse_integer(token.begin(), token.end());
    BOOST_CHECK(result.iterator() != token.begin());
    BOOST_CHECK(result.iterator() != token.end());
}

BOOST_AUTO_TEST_CASE(test_decimal_correct)
{
    check_equal("1234",             1234);
    check_equal("+1234",           +1234);
    check_equal("-1234",           -1234);
    check_equal("0",                   0);
    check_equal("1_2_3_4",          1234);
    check_equal("+1_2_3_4",         1234);
    check_equal("-1_2_3_4",        -1234);
    check_equal("123_456_789", 123456789);
}

BOOST_AUTO_TEST_CASE(test_decimal_invalid)
{
    check_fail("01234");
    check_fail("_1234");
    check_fail("123__45");
    check_partially_read("123+45");
    check_partially_read("123-45");
}

BOOST_AUTO_TEST_CASE(test_hex_correct)
{
    check_equal("0xDEADBEEF",  3735928559);
    check_equal("0xdeadbeef",  3735928559);
    check_equal("0xDEADbeef",  3735928559);
    check_equal("0xDEAD_BEEF", 3735928559);
    check_equal("0xdead_beef", 3735928559);
    check_equal("0xdead_BEEF", 3735928559);

    check_equal("0xFF",     255);
    check_equal("0x00FF",   255);
    check_equal("0x0000FF", 255);
}

BOOST_AUTO_TEST_CASE(test_hex_invalid)
{
    check_fail("0x_DEADBEEF");
    check_fail("0xDEAD__BEEF");
    check_fail("0x+DEADBEEF");
    check_fail("-0xFF");
    check_fail("-0x00FF");
    check_partially_read("0xAPPLE");
    check_partially_read("0xDEAD+BEEF");
}

BOOST_AUTO_TEST_CASE(test_oct_correct)
{
    check_equal("0o777",   511);
    check_equal("0o7_7_7", 511);
    check_equal("0o007",     7);
}

BOOST_AUTO_TEST_CASE(test_oct_invalid)
{
    check_fail("0o800");
    check_fail("-0o777");
    check_fail("0o+777");
    check_fail("0o1__0");
    check_fail("0o_10");
    check_partially_read("0o77+7");
}

BOOST_AUTO_TEST_CASE(test_bin_correct)
{
    check_equal("0b10000",    16);
    check_equal("0b010000",   16);
    check_equal("0b01_00_00", 16);
    check_equal("0b111111",   63);
}

BOOST_AUTO_TEST_CASE(test_bin_invalid)
{
    check_fail("-0b10000");
    check_fail("0b11__11");
    check_fail("0b_1111");
    check_partially_read("0b11+11");
}
