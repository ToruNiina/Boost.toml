#define BOOST_TEST_MODULE "parse_datetime_test"
#include <toml/types.hpp>
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include "parse_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_time)
{
    TOML_PARSE_CHECK_EQUAL(parse_local_time, "07:32:00",
        toml::time,  hours(7) + minutes(32));
    TOML_PARSE_CHECK_EQUAL(parse_local_time, "07:32:00.99",
        toml::time,  hours(7) + minutes(32) + milliseconds(990));
    TOML_PARSE_CHECK_EQUAL(parse_local_time, "07:32:00.999",
        toml::time,  hours(7) + minutes(32) + milliseconds(999));
    TOML_PARSE_CHECK_EQUAL(parse_local_time, "07:32:00.999999",
        toml::time,  hours(7) + minutes(32) + milliseconds(999) + microseconds(999));
}

BOOST_AUTO_TEST_CASE(test_date)
{
    TOML_PARSE_CHECK_EQUAL(parse_local_date, "1979-05-27",
        toml::date, toml::date(1979, 5, 27));
}

BOOST_AUTO_TEST_CASE(test_datetime)
{
    TOML_PARSE_CHECK_EQUAL(parse_local_datetime, "1979-05-27T07:32:00",
        toml::local_datetime, toml::local_datetime(toml::date(1979, 5, 27),
            toml::hours(7) + toml::minutes(32)));
    TOML_PARSE_CHECK_EQUAL(parse_local_datetime, "1979-05-27T07:32:00.99",
        toml::local_datetime, toml::local_datetime(toml::date(1979, 5, 27),
            toml::hours(7) + toml::minutes(32) + toml::milliseconds(990)));
    TOML_PARSE_CHECK_EQUAL(parse_local_datetime, "1979-05-27T07:32:00.999999",
        toml::local_datetime, toml::local_datetime(toml::date(1979, 5, 27),
            toml::hours(7) + toml::minutes(32) + toml::milliseconds(999) + toml::microseconds(999)));

    TOML_PARSE_CHECK_EQUAL(parse_local_datetime, "1979-05-27 07:32:00",
        toml::local_datetime, toml::local_datetime(toml::date(1979, 5, 27),
            toml::hours(7) + toml::minutes(32)));
    TOML_PARSE_CHECK_EQUAL(parse_local_datetime, "1979-05-27 07:32:00.99",
        toml::local_datetime, toml::local_datetime(toml::date(1979, 5, 27),
            toml::hours(7) + toml::minutes(32) + toml::milliseconds(990)));
    TOML_PARSE_CHECK_EQUAL(parse_local_datetime, "1979-05-27 07:32:00.999999",
        toml::local_datetime, toml::local_datetime(toml::date(1979, 5, 27),
            toml::hours(7) + toml::minutes(32) + toml::milliseconds(999) + toml::microseconds(999)));
}

BOOST_AUTO_TEST_CASE(test_offset_datetime)
{
    TOML_PARSE_CHECK_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00Z",
        toml::offset_datetime, toml::offset_datetime(toml::local_datetime(
            toml::date(1979, 5, 27), toml::hours(7) + toml::minutes(32)),
            time_zone_ptr(new boost::local_time::posix_time_zone("UTC"))));
    TOML_PARSE_CHECK_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00.99Z",
        toml::offset_datetime, toml::offset_datetime(toml::local_datetime(
            toml::date(1979, 5, 27),toml::hours(7) + toml::minutes(32) + toml::milliseconds(990)),
            time_zone_ptr(new boost::local_time::posix_time_zone("UTC"))));
    TOML_PARSE_CHECK_EQUAL(parse_offset_datetime, "1979-05-27T07:32:00.999999Z",
        toml::offset_datetime, toml::offset_datetime(toml::local_datetime(
            toml::date(1979, 5, 27), toml::hours(7) + toml::minutes(32) +
            toml::milliseconds(999) + toml::microseconds(999)),
            time_zone_ptr(new boost::local_time::posix_time_zone("UTC"))));

    {
        const std::string token("1979-05-27T18:50:00-04:00");
        std::string::const_iterator iter(token.begin());
        const result<toml::offset_datetime, std::string> r =
            parse_offset_datetime(iter, token.end());
        const toml::local_datetime answer(
            toml::date(1979, 5, 27), toml::hours(22) + toml::minutes(50));

        BOOST_CHECK(r.is_ok());
        BOOST_CHECK(r.unwrap().utc_time() == answer);
    }
}
