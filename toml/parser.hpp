#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP
#include <toml/result.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include <istream>
#include <sstream>
#include <fstream>

namespace toml
{
namespace detail
{

// for error messages. not for parser.
template<typename InputIterator>
InputIterator
find_linebreak(const InputIterator first, const InputIterator last)
    BOOST_NOEXCEPT_OR_NOTHROW
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
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
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
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
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    typedef result<integer, InputIterator> result_t;
    char type = 'd'; // d, x, o, b for dec, hex, oct, bin, respectively.

    integer sign = 1;
    InputIterator iter = first;
    if(*iter == '+') {++iter;} else if(*iter == '-') {++iter; sign = -1;}
    if(iter == last)
    {
        return result_t("toml::detail::parse_integer: input is empty -> " +
                std::string(first, find_linebreak(first, last)), iter);
    }

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
                "toml::detail::parse_integer: leading 0 is not allowed -> " +
                std::string(first, find_linebreak(first, last)), iter);
        }
        else // just 0.
        {
            return result_t(0, iter);
        }
    }

    const char t = type;
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
                    "`_` must be surrounded by at least one number -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            else
            {
                underscore = true;
            }
        }
        else if((t == 'd' && isdec(n)) || (t == 'x' && ishex(n)) ||
                (t == 'o' && isoct(n)) || (t == 'b' && isbin(n)))
        {
            token += n;
            underscore = false;
        }
        else
        {
            break;
        }
    }
    if(token.empty())
    {
        return result_t("toml::detail::parse_integer: input is empty -> " +
                std::string(first, find_linebreak(first, last)), iter);
    }
    switch(t)
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

template<typename InputIterator>
result<floating, InputIterator>
parse_floating(const InputIterator first, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    typedef result<floating, InputIterator> result_t;

    floating sign = 1;
    InputIterator iter = first;
    if(*iter == '+') {++iter;} else if(*iter == '-') {++iter; sign = -1;}
    if(iter == last)
    {
        return result_t("toml::detail::parse_integer: input is empty -> " +
                std::string(first, find_linebreak(first, last)), iter);
    }

    // special values (nan, inf)
    if(*iter == 'n')
    {
        if(++iter != last && *iter == 'a' && ++iter != last && *iter == 'n')
        {
            return result_t(std::numeric_limits<floating>::quiet_NaN(), iter);
        }
        return result_t(
            "toml::detail::parse_floating: unknown keyword, maybe `nan`? ->" +
            std::string(first, find_linebreak(first, last)), iter);
    }
    if(*iter == 'i')
    {
        if(++iter != last && *iter == 'n' && ++iter != last && *iter == 'f')
        {
            return result_t(
                    sign * std::numeric_limits<floating>::infinity(), iter);
        }
        return result_t(
            "toml::detail::parse_floating: unknown keyword, maybe `inf`? ->" +
            std::string(first, find_linebreak(first, last)), iter);
    }

    std::string token;
    bool underscore = true; // to avoid leading _
    bool integer_part_started = false;
    bool is_fractional_part   = false;
    bool is_exponential_part  = false;
    // integer part...
    if(*iter == '0')
    {
        token += *iter;
        ++iter;
        if(iter == last)
        {
            return result_t("toml::detail::parse_floating: got `0`. "
                    "it's not a floating point number, it's integer -> " +
                    std::string(first, find_linebreak(first, last)), iter);
        }
        integer_part_started = true;
    }
    for(; iter != last; ++iter)
    {
        const char n = *iter;
        if(n == '_')
        {
            if(underscore)
            {
                return result_t("toml::detail::parse_floating: "
                    "`_` must be surrounded by at least one number -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            else
            {
                underscore = true;
            }
        }
        else if(isdec(n))
        {
            token += n;
            underscore = false;
            integer_part_started = true;
        }
        else if(n == '.')
        {
            if(!integer_part_started)
            {
                return result_t(
                    "toml::detial::parse_floating: invalid `.` appeared -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            is_fractional_part = true;
            token += n;
            ++iter;
            break;
        }
        else if(n == 'e' || n == 'E')
        {
            if(!integer_part_started)
            {
                return result_t(
                    "toml::detial::parse_floating: invalid `e|E` appeared -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            is_exponential_part = true;
            token += n;
            ++iter;
            break;
        }
        else
        {
            break;
        }
    }
    if(is_fractional_part)
    {
        underscore = true; //to avoid leading 0 for fractional part (ex. 0._1)
        bool fractional_part_has_size = false;
        for(; iter != last; ++iter)
        {
            const char n = *iter;
            if(n == '_')
            {
                if(underscore)
                {
                    return result_t("toml::detail::parse_floating: "
                        "`_` must be surrounded by at least one number -> " +
                        std::string(first, find_linebreak(first, last)), iter);
                }
                else
                {
                    underscore = true;
                }
            }
            else if(isdec(n))
            {
                token += n;
                underscore = false;
                fractional_part_has_size = true;
            }
            else if(n == '.')
            {
                return result_t(
                    "toml::detial::parse_floating: invalid `.` appeared -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            else if(n == 'e' || n == 'E')
            {
                is_exponential_part = true;
                token += n;
                ++iter;
                break;
            }
            else
            {
                if(!fractional_part_has_size)
                {
                    return result_t("toml::detial::parse_floating: "
                        "fractional part must have size -> " +
                        std::string(first, find_linebreak(first, last)), iter);

                }
                break;
            }
        }
    }
    if(is_exponential_part)
    {
        underscore = true; //to avoid leading 0 for fractional part (ex. 1e_10)
        if(*iter == '+' || *iter == '-')
        {
            token += *iter;
            ++iter;
        }
        for(; iter != last; ++iter)
        {
            const char n = *iter;
            if(n == '_')
            {
                if(underscore)
                {
                    return result_t("toml::detail::parse_floating: "
                        "`_` must be surrounded by at least one number -> " +
                        std::string(first, find_linebreak(first, last)), iter);
                }
                else
                {
                    underscore = true;
                }
            }
            else if(isdec(n))
            {
                token += n;
                underscore = false;
            }
            else if(n == '.')
            {
                return result_t(
                    "toml::detial::parse_floating: invalid `.` appeared -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            else if(n == 'e' || n == 'E')
            {
                return result_t(
                    "toml::detial::parse_floating: invalid `e|E` appeared -> " +
                    std::string(first, find_linebreak(first, last)), iter);
            }
            else
            {
                break;
            }
        }
    }
    if(token.empty())
    {
        return result_t("toml::detail::parse_floating: input is empty -> " +
                std::string(first, find_linebreak(first, last)), iter);
    }
    return result_t(sign * boost::lexical_cast<floating>(token), iter);
}

} // detail
} // toml
#endif// TOML_PARSER_HPP
