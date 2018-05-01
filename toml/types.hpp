#ifndef TOML_TYPES_HPP
#define TOML_TYPES_HPP
#include <toml/strings.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/container/vector.hpp>

#ifdef TOML_USE_MAP_FOR_TABLE
#include <boost/container/map.hpp>
#else
#include <boost/container/flat_map.hpp>
#endif

namespace toml
{

typedef std::string key;
struct value;

typedef bool                               boolean;
typedef boost::int64_t                     integer;
typedef double                             floating;
// typedef string                          string;
typedef boost::gregorian::date             date;
typedef boost::posix_time::time_duration   time;
typedef boost::posix_time::ptime           datetime;
typedef boost::local_time::time_zone_ptr   time_zone_ptr;
typedef boost::local_time::local_date_time offset_datetime;
typedef boost::container::vector<value>    array;

#ifdef TOML_USE_MAP_FOR_TABLE
typedef boost::container::map<key, value> table;
#else
typedef boost::container::flat_map<key, value> table;
#endif

using boost::date_time::months_of_year;
using boost::date_time::Jan;
using boost::date_time::Feb;
using boost::date_time::Mar;
using boost::date_time::Apr;
using boost::date_time::May;
using boost::date_time::Jun;
using boost::date_time::Jul;
using boost::date_time::Aug;
using boost::date_time::Sep;
using boost::date_time::Oct;
using boost::date_time::Nov;
using boost::date_time::Dec;

using boost::date_time::weekdays;
using boost::date_time::Sunday;
using boost::date_time::Monday;
using boost::date_time::Tuesday;
using boost::date_time::Wednesday;
using boost::date_time::Thursday;
using boost::date_time::Friday;
using boost::date_time::Saturday;

using boost::gregorian::years;
using boost::gregorian::months;
using boost::gregorian::days;

using boost::posix_time::hours;
using boost::posix_time::minutes;
using boost::posix_time::seconds;
using boost::posix_time::milliseconds;
using boost::posix_time::microseconds;

#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
using boost::posix_time::nanoseconds;
#endif // BOOST_DATE_TIME_HAS_NANOSECONDS

} // toml
#endif// TOML_TYPES_HPP
