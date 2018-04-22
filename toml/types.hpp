#ifndef TOML_TYPES_HPP
#define TOML_TYPES_HPP
#include <toml/strings.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
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

typedef bool                              boolean;
typedef boost::int64_t                    integer;
typedef double                            floating;
typedef string                            string;
typedef boost::gregorian::date            date;
typedef boost::posix_time::time_duration  time;
struct  datetime {date d; time t;};
typedef boost::container::vector<value>   array;

#ifdef TOML_USE_MAP_FOR_TABLE
typedef boost::container::map<key, value> table;
#else
typedef boost::container::flat_map<key, value> table;
#endif

inline bool operator==(const datetime& lhs, const datetime& rhs)
{
    return lhs.d == rhs.d && lhs.t == rhs.t;
}
inline bool operator!=(const datetime& lhs, const datetime& rhs)
{
    return !(lhs == rhs);
}
inline bool operator< (const datetime& lhs, const datetime& rhs)
{
    return (lhs.d == rhs.d) ? (lhs.t < rhs.t) : (lhs.d < rhs.d);
}
inline bool operator> (const datetime& lhs, const datetime& rhs)
{
    return rhs < lhs;
}
inline bool operator<=(const datetime& lhs, const datetime& rhs)
{
    return !(lhs > rhs);
}
inline bool operator>=(const datetime& lhs, const datetime& rhs)
{
    return !(lhs < rhs);
}

} // toml
#endif// TOML_TYPES_HPP
