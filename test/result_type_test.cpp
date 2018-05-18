#define BOOST_TEST_MODULE "result_type_test"
#include <boost/test/included/unit_test.hpp>
#include <toml/result.hpp>
#include <iostream>
#include <iomanip>

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_construction)
{
    {
        result<int, double> r(ok(42));
        BOOST_CHECK(r);
        BOOST_CHECK(r.is_ok());
        BOOST_CHECK(!r.is_err());
    }
    {
        result<int, double> r(err(3.14));
        BOOST_CHECK(!r);
        BOOST_CHECK(!r.is_ok());
        BOOST_CHECK(r.is_err());
    }
    {
        result<int, double> r = ok(42);
        BOOST_CHECK(r);
        BOOST_CHECK(r.is_ok());
        BOOST_CHECK(!r.is_err());
    }
    {
        result<int, double> r = err(3.14);
        BOOST_CHECK(!r);
        BOOST_CHECK(!r.is_ok());
        BOOST_CHECK(r.is_err());
    }
}

BOOST_AUTO_TEST_CASE(test_unwrap)
{
    {
        result<int, double> r(ok(42));
        BOOST_CHECK_EQUAL(r.unwrap(), 42);
        BOOST_CHECK_EQUAL(r.ok_or(54),    42);
        BOOST_CHECK_EQUAL(r.err_or(2.71), 2.71);
    }
    {
        result<int, double> r(err(3.14));
        BOOST_CHECK_EQUAL(r.unwrap_err(), 3.14);
        BOOST_CHECK_EQUAL(r.ok_or(54),    54);
        BOOST_CHECK_EQUAL(r.err_or(2.71), 3.14);
    }
}
