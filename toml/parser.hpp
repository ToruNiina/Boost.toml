#ifndef TOML98_PARSER_HPP
#define TOML98_PARSER_HPP
#include <toml/result.hpp>
#include <boost/lexical_cast.hpp>
#include <istream>
#include <fstream>

namespace toml
{
namespace detail
{

// for error messages. not for parser.
template<typename InputIterator>
InputIterator
find_linebreak(const InputIterator first, const InputIterator last)
{
    const InputIterator CR = std::find(first, last, '\r');
    if(CR != last)
    {
        InputIterator nxt = CR;
        if(*++nxt == '\n')
        {
            return CR;
        }
        else
        {
            return find_linebreak(nxt, last);
        }
    }
    return std::find(first, last, '\n');
}

inline bool ishex(const char c) BOOST_NOEXCEPT_OR_NOTHROW
{
    return ('0' <= c && c <= '9') ||
           ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
}
inline bool isdec(const char c) BOOST_NOEXCEPT_OR_NOTHROW
{
    return ('0' <= c && c <= '9');
}
inline bool isoct(const char c) BOOST_NOEXCEPT_OR_NOTHROW
{
    return ('0' <= c && c <= '7');
}
inline bool isbin(const char c) BOOST_NOEXCEPT_OR_NOTHROW
{
    return ('0' == c || c == '1');
}

// returns iterator that points next of the value
// t r u e
//         ^ return here
template<typename InputIterator>
result<boolean, InputIterator>
parse_boolean(const InputIterator first, const InputIterator last)
{
    typedef result<boolean, InputIterator> result_t;

    if(first == last)
    {
        return result_t(std::string(
            "toml::detail::parse_boolean: input is empty."), first);
    }

    InputIterator iter = first;
    if(*iter == 't')
    {
        if(++iter != last && *iter == 'r' &&
           ++iter != last && *iter == 'u' &&
           ++iter != last && *iter == 'e')
        {
            return result_t(true, ++iter);
        }
        return result_t("toml::detail::parse_boolean: boolean keywords are "
            "`true` or `false` -> " +
            std::string(first, find_linebreak(first, last)), iter);
    }
    else if(*iter == 'f')
    {
        if(++iter != last && *iter == 'a' &&
           ++iter != last && *iter == 'l' &&
           ++iter != last && *iter == 's' &&
           ++iter != last && *iter == 'e')
        {
            return result_t(false, ++iter);
        }
        return result_t("toml::detail::parse_boolean: boolean keywords are "
            "`true` or `false` -> " +
            std::string(first, find_linebreak(first, last)), iter);
    }
    else if(*iter == 'T' || *iter == 'F')
    {
        return result_t("toml::detail::parse_boolean: boolean keywords are "
            "`true` or `false` -> " +
            std::string(first, find_linebreak(first, last)), iter);
    }
    return result_t(std::string(
        "toml::detail::parse_boolean: failed. try next"), first);
}

template<typename InputIterator>
result<integer, InputIterator>
parse_integer(const InputIterator first, const InputIterator last)
{
    typedef result<integer, InputIterator> result_t;
    char type = 'd'; // d, x, o, b for dec, hex, oct, bin, respectively.

    integer sign = 1;
    InputIterator iter = first;
    if(*iter == '+') {++iter;} else if(*iter == '-') {++iter; sign = -1;}
    if(*iter == '0')
    {
        ++iter;
        if(iter == last) {return result_t(0, iter);}

        const char n = *iter;
        if(n == 'x' || n == 'o' || n == 'b')
        {
            ++iter;
            type = n;
        }
        else if('0' <= n && n <= '9')
        {
            return result_t(
                "toml::detail::parse_integer: leading 0 is not allowed: " +
                std::string(first, find_linebreak(first, last)), iter);
        }
        else // just 0.
        {
            return result_t(0, iter);
        }
    }

    std::string token;
    bool underscore = true; // to avoid leading _
    for(; iter != last; ++iter)
    {
        const char n = *iter;
        if(n == '_')
        {
            if(underscore)
            {
                return result_t("toml::detail::parse_integer: "
                    "`_` must be surrounded by at least one number.", iter);
            }
            else
            {
                underscore = true;
            }
        }
        else if((type == 'd' && isdec(n)) || (type == 'x' && ishex(n)) ||
                (type == 'o' && isoct(n)) || (type == 'b' && isbin(n)))
        {
            token += *iter;
            underscore = false;
        }
        else
        {
            break;
        }
    }
    switch(type)
    {
        case 'd':
        {
            return result_t(sign * boost::lexical_cast<integer>(token), iter);
        }
        case 'x':
        {
            integer i;
            std::stringstream iss(token);
            iss >> std::hex >> i;
            return result_t(sign * i, iter);
        }
        case 'o':
        {
            integer i;
            std::stringstream iss(token);
            iss >> std::oct >> i;
            return result_t(sign * i, iter);
        }
        case 'b':
        {
            integer ret = 0;
            integer base = 1;
            for(std::string::const_reverse_iterator
                    i(token.rbegin()), e(token.rend()); i!=e; ++i)
            {
                if(*i == '1') {ret += base;}
                base *= 2;
            }
            return result_t(sign * ret, iter);
        }
        default:
        {
            throw std::logic_error("toml::detail::integer_parser: "
                    "internal error: NEVER REACH HERE");
        }
    }
}

} // detail
} // toml
#endif// TOML98_PARSER_HPP
