#define BOOST_TEST_MODULE "write_file_test"
#include <toml/toml.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

BOOST_AUTO_TEST_CASE(test_sample_toml)
{
    const toml::table parsed = toml::parse("toml/tests/example.toml");
    {
        std::ofstream ofs("tmp1.toml");
        ofs << parsed;
    }
    const toml::table formatted = toml::parse("tmp1.toml");

    BOOST_CHECK(parsed == formatted);
}

BOOST_AUTO_TEST_CASE(test_fruits_toml)
{
    const toml::table parsed = toml::parse("toml/tests/fruit.toml");
    {
        std::ofstream ofs("tmp2.toml");
        ofs << parsed;
    }
    const toml::table formatted = toml::parse("tmp2.toml");

    BOOST_CHECK(parsed == formatted);
}


BOOST_AUTO_TEST_CASE(test_hard_example_toml)
{
    const toml::table parsed = toml::parse("toml/tests/hard_example.toml");
    {
        std::ofstream ofs("tmp3.toml");
        ofs << parsed;
    }
    const toml::table formatted = toml::parse("tmp3.toml");

    BOOST_CHECK(parsed == formatted);
}

BOOST_AUTO_TEST_CASE(test_hard_example_unicode_toml)
{
    const toml::table parsed = toml::parse("toml/tests/hard_example_unicode.toml");
    {
        std::ofstream ofs("tmp4.toml");
        ofs << parsed;
    }
    const toml::table formatted = toml::parse("tmp4.toml");

    BOOST_CHECK(parsed == formatted);
}
