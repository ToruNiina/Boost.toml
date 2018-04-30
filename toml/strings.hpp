#ifndef TOML_STRINGS_HPP
#define TOML_STRINGS_HPP
#include <toml/predefine.hpp>
#include <boost/config.hpp>
#include <string>
#include <ostream>

namespace toml
{

struct string
{
    enum kind_t
    {
        basic,
        literal
    };

    string(){}
    ~string(){}
    string(const string& s): kind(s.kind), str(s.str){}
    string& operator=(const string& s)
    {kind = s.kind; str = s.str; return *this;}

    string(const std::string& s)          : kind(basic), str(s){}
    string(const std::string& s, kind_t k): kind(k),     str(s){}
    string(const char* s)                 : kind(basic), str(s){}
    string(const char* s,        kind_t k): kind(k),     str(s){}

    string& operator=(const std::string& s)
    {kind = basic; str = s; return *this;}

#ifdef BOOST_HAS_RVALUE_REFS
    string(string&& s)               : kind(s.kind), str(std::move(s.str)){}
    string(std::string&& s)          : kind(basic),  str(std::move(s)){}
    string(std::string&& s, kind_t k): kind(k),      str(std::move(s)){}

    string& operator=(string&& s)
    {kind = s.kind; str = std::move(s.str); return *this;}
    string& operator=(std::string&& s)
    {kind = basic;  str = std::move(s); return *this;}
#endif

    operator std::string&       ()       BOOST_NOEXCEPT_OR_NOTHROW {return str;}
    operator std::string const& () const BOOST_NOEXCEPT_OR_NOTHROW {return str;}

    kind_t      kind;
    std::string str;
};

inline bool operator==(const string& lhs, const string& rhs)
{
    return lhs.kind == rhs.kind && lhs.str == rhs.str;
}
inline bool operator!=(const string& lhs, const string& rhs)
{
    return !(lhs == rhs);
}
inline bool operator<(const string& lhs, const string& rhs)
{
    return (lhs.kind == rhs.kind) ? (lhs.str < rhs.str) : (lhs.kind < rhs.kind);
}
inline bool operator>(const string& lhs, const string& rhs)
{
    return rhs < lhs;
}
inline bool operator<=(const string& lhs, const string& rhs)
{
    return !(rhs < lhs);
}
inline bool operator>=(const string& lhs, const string& rhs)
{
    return !(lhs < rhs);
}

inline bool
operator==(const string& lhs, const std::string& rhs) {return lhs.str == rhs;}
inline bool
operator!=(const string& lhs, const std::string& rhs) {return lhs.str != rhs;}
inline bool
operator< (const string& lhs, const std::string& rhs) {return lhs.str <  rhs;}
inline bool
operator> (const string& lhs, const std::string& rhs) {return lhs.str >  rhs;}
inline bool
operator<=(const string& lhs, const std::string& rhs) {return lhs.str <= rhs;}
inline bool
operator>=(const string& lhs, const std::string& rhs) {return lhs.str >= rhs;}

inline bool
operator==(const std::string& lhs, const string& rhs) {return lhs == rhs.str;}
inline bool
operator!=(const std::string& lhs, const string& rhs) {return lhs != rhs.str;}
inline bool
operator< (const std::string& lhs, const string& rhs) {return lhs <  rhs.str;}
inline bool
operator> (const std::string& lhs, const string& rhs) {return lhs >  rhs.str;}
inline bool
operator<=(const std::string& lhs, const string& rhs) {return lhs <= rhs.str;}
inline bool
operator>=(const std::string& lhs, const string& rhs) {return lhs >= rhs.str;}

template<typename charT, typename traits>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const string& str)
{
    os << str.str;
    return os;
}

} // toml
#endif// TOML_STRINGS_HPP
