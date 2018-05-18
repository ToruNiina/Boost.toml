//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_COMBINATOR_HPP
#define TOML_COMBINATOR_HPP
#include <boost/optional.hpp>
#include <string>

namespace toml
{
namespace detail
{

// these elemental parsers consume tokens even if it failed (e.g. if one pass
// "123A" to `sequence<repeat<char_range<'0', '9'>, 0>, character<'B'>>`, it
// advances `first` as it points 'A'). parse_* functions manage the tokens.

template<char C>
struct character
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        if(first == last || *first != C){return boost::none;}
        std::string token; token += *(first++);
        return token;
    }
};

template<char First, char Last>
struct in_range
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        if(first==last || *first < First || Last < *first){return boost::none;}
        std::string token; token += *(first++);
        return token;
    }
};

template<typename T>
struct exclude
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        if(first == last){return boost::none;}
        InputIterator iter = first;
        const boost::optional<std::string> rslt = T::invoke(iter, last);
        if(rslt){return boost::none;}
        std::string token; token += *(first++);
        return token;
    }
};

template<typename T1, typename T2>
struct sequence
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        InputIterator iter = first;
        const boost::optional<std::string> s1 = T1::invoke(iter, last);
        if(!s1){return boost::none;}
        const boost::optional<std::string> s2 = T2::invoke(iter, last);
        if(!s2){return boost::none;}
        first = iter;
        return *s1 + *s2;
    }
};

template<typename T1, typename T2>
struct either
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        InputIterator iter = first;
        const boost::optional<std::string> s1 = T1::invoke(iter, last);
        if(s1) {first = iter; return s1;}
        const boost::optional<std::string> s2 = T2::invoke(iter, last);
        if(s2) {first = iter; return s2;}
        return boost::none;
    }
};

template<typename T>
struct maybe
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        const boost::optional<std::string> s1 = T::invoke(first, last);
        if(s1) {return s1;}
        return std::string("");
    }
};

template<typename T, typename N>
struct repeat;

template<std::size_t N> struct exactly{};
template<std::size_t N> struct at_least{};
struct unlimited{};

template<typename T, std::size_t N>
struct repeat<T, exactly<N> >
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        InputIterator iter = first;
        std::string retval;
        for(std::size_t i=0; i<N; ++i)
        {
            const boost::optional<std::string> str = T::invoke(iter, last);
            if(!str) {return boost::none;} else {retval += (*str);}
        }
        first = iter;
        return retval;
    }
};

template<typename T, std::size_t N>
struct repeat<T, at_least<N> >
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        InputIterator iter = first;
        std::string retval;
        for(std::size_t i=0; i<N; ++i)
        {
            const boost::optional<std::string> str = T::invoke(iter, last);
            if(!str) {return boost::none;} else {retval += (*str);}
        }
        while(true)
        {
            const boost::optional<std::string> str = T::invoke(iter, last);
            if(!str) {first = iter; return retval;} else {retval += (*str);}
        }
    }
};

template<typename T>
struct repeat<T, unlimited>
{
    template<typename InputIterator>
    static boost::optional<std::string>
    invoke(InputIterator& first, const InputIterator last)
    {
        InputIterator iter = first;
        std::string retval;
        while(true)
        {
            const boost::optional<std::string> str = T::invoke(iter, last);
            if(!str) {first = iter; return retval;} else {retval += (*str);}
        }
    }
};

} // detail
} // toml
#endif // TOML_COMBINATOR_HPP
