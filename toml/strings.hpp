#ifndef TOML_STRINGS_HPP
#define TOML_STRINGS_HPP
#include <toml/predefine.hpp>
#include <boost/config.hpp>
#include <string>

namespace toml
{

struct basic_tag{};
struct literal_tag{};
extern const basic_tag   basic_string;
extern const literal_tag literal_string;

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

    string(const std::string& s): kind(basic), str(s){}
    string(const char* s)       : kind(basic), str(s){}

    string(const std::string& s, basic_tag)  : kind(basic),   str(s){}
    string(const char* s,        basic_tag)  : kind(basic),   str(s){}
    string(const std::string& s, literal_tag): kind(literal), str(s){}
    string(const char* s,        literal_tag): kind(literal), str(s){}
    string(const std::string& s, kind_t k)   : kind(k),       str(s){}
    string(const char* s,        kind_t k)   : kind(k),       str(s){}

    string& operator=(const std::string& s)
    {kind = basic; str = s; return *this;}

#ifdef BOOST_HAS_RVALUE_REFS
    string(string&& s) : kind(s.kind), str(std::move(s.str)){}

    string(std::string&& s) : kind(basic), str(std::move(s)){}
    string(std::string&& s, basic_tag)  : kind(basic),   str(std::move(s)){}
    string(std::string&& s, literal_tag): kind(literal), str(std::move(s)){}
    string(std::string&& s, kind_t k)   : kind(k),       str(std::move(s)){}

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

} // toml
#endif// TOML_STRINGS_HPP
