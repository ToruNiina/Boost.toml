//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP
#include <toml/result.hpp>
#include <toml/lexer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include <stdexcept>
#include <iterator>
#include <vector>
#include <istream>
#include <sstream>
#include <fstream>

namespace toml
{
namespace detail
{

// for error messages. not for parser.
template<typename InputIterator>
std::string current_line(const InputIterator first, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    return std::string(first, std::find(first, last, '\n'));
}


// returns iterator that points next of the value
// t r u e
//         ^ return here
template<typename InputIterator>
result<boolean, std::string>
parse_boolean(InputIterator& iter, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);

    const InputIterator first = iter;
    const boost::optional<std::string> token = lex_boolean::invoke(iter, last);
    if(token)
    {
        if(*token == "true")
        {
            return ok(true);
        }
        else if(*token == "false")
        {
            return ok(false);
        }
        else
        {
            // internal error.
            throw std::invalid_argument("toml::detail::parse_boolean: "
                    "lexer returns invalid token -> " + *token);
        }
    }
    if(iter != first)
    {
        return err("toml::detail::parse_boolean: partially match to boolean -> "
                + current_line(first, last));
    }
    return err("toml::detail::parse_boolean: next token is not a boolean -> " +
            current_line(first, last));
}

template<typename InputIterator>
result<integer, std::string>
parse_integer(InputIterator& iter, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);

    const InputIterator first = iter;
    if(first != last && *first == '0')
    {
        {
            const boost::optional<std::string> token =
                lex_bin_int::invoke(iter, last);
            if(token)
            {
                integer retval(0), base(1);
                for(std::reverse_iterator<std::string::const_iterator>
                        i(token->end()), e(token->begin() + 2); i!=e; ++i)
                {
                    if     (*i == '1'){retval += base;}
                    else if(*i == '0' || *i == '_'){/* do nothing. */}
                    else
                    {
                        throw std::logic_error("toml::detail::parse_integer: "
                            "lexer returns invalid token -> " + *token);
                    }
                    base *= 2;
                }
                return ok(retval);
            }
            iter = first;
        }
        {
            boost::optional<std::string> token =
                lex_oct_int::invoke(iter, last);
            if(token)
            {
                const std::string::iterator token_last =
                    std::remove(token->begin(), token->end(), '_');
                token->erase(token_last, token->end());
                token->erase(token->begin()); // 0
                token->erase(token->begin()); // o

                std::istringstream iss(*token);
                integer retval;
                iss >> std::oct >> retval;
                return ok(retval);
            }
            iter = first;
        }
        {
            boost::optional<std::string> token =
                lex_hex_int::invoke(iter, last);
            if(token)
            {
                const std::string::iterator token_last =
                    std::remove(token->begin(), token->end(), '_');
                token->erase(token_last, token->end());
                token->erase(token->begin()); // 0
                token->erase(token->begin()); // x

                std::istringstream iss(*token);
                integer retval;
                iss >> std::hex >> retval;
                return ok(retval);
            }
            iter = first;
        }
    }

    boost::optional<std::string> token = lex_dec_int::invoke(iter, last);
    if(token)
    {
        const std::string::iterator token_last =
            std::remove(token->begin(), token->end(), '_');
        token->erase(token_last, token->end());

        std::istringstream iss(*token);
        integer retval;
        iss >> retval;
        return ok(retval);
    }

    iter = first;
    return err("toml::detail::parse_integer: next token is not a integer -> " +
            current_line(first, last));
}

template<typename InputIterator>
result<floating, std::string>
parse_floating(InputIterator& iter, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);

    const InputIterator first = iter;
    boost::optional<std::string> token = lex_float::invoke(iter, last);
    if(token)
    {
        const std::string::iterator token_last =
            std::remove(token->begin(), token->end(), '_');
        token->erase(token_last, token->end());
        return ok(boost::lexical_cast<floating>(*token));

    }
    iter = first;
    return err("toml::detail::parse_integer: next token is not a integer -> " +
            current_line(first, last));
}

inline std::string read_utf8_codepoint(const std::string& str)
{
    boost::uint_least32_t codepoint;
    std::istringstream iss(str);
    iss >> std::hex >> codepoint;

    std::string character;
    if(codepoint < 0x80)
    {
        character += static_cast<unsigned char>(codepoint);
    }
    else if(codepoint < 0x800)
    {
        character += static_cast<unsigned char>(0xC0| codepoint >> 6);
        character += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
    }
    else if(codepoint < 0x10000)
    {
        character += static_cast<unsigned char>(0xE0| codepoint >> 12);
        character += static_cast<unsigned char>(0x80|(codepoint >> 6 & 0x3F));
        character += static_cast<unsigned char>(0x80|(codepoint      & 0x3F));
    }
    else
    {
        character += static_cast<unsigned char>(0xF0| codepoint >> 18);
        character += static_cast<unsigned char>(0x80|(codepoint >> 12 & 0x3F));
        character += static_cast<unsigned char>(0x80|(codepoint >> 6  & 0x3F));
        character += static_cast<unsigned char>(0x80|(codepoint       & 0x3F));
    }
    return character;
}

template<typename InputIterator>
result<string, std::string>
parse_escape_sequence(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(*iter != '\\')
    {
        throw std::invalid_argument("toml::detail::parse_escape_sequence: "
                "internal error appeared");
    }
    ++iter;

    switch(*iter)
    {
        case '\\':{++iter; return ok(string("\\"));}
        case '"' :{++iter; return ok(string("\""));}
        case 'b' :{++iter; return ok(string("\b"));}
        case 't' :{++iter; return ok(string("\t"));}
        case 'n' :{++iter; return ok(string("\n"));}
        case 'f' :{++iter; return ok(string("\f"));}
        case 'r' :{++iter; return ok(string("\r"));}
        case 'u' :
        {
            ++iter;
            const boost::optional<std::string> token =
                    repeat< lex_hex_dig, exactly<4> >::invoke(iter, last);
            if(token)
            {
                return ok(string(read_utf8_codepoint(*token)));
            }
            return err("toml::detail::parse_escape_sequence: "
                "\\uXXXX must have 4 hex numbers -> " +
                current_line(first, last));
        }
        case 'U':
        {
            ++iter;
            const boost::optional<std::string> token =
                    repeat< lex_hex_dig, exactly<8> >::invoke(iter, last);
            if(token)
            {
                return ok(string(read_utf8_codepoint(*token)));
            }
            return err("toml::detail::parse_escape_sequence: "
                "\\UXXXXXXXX must have 8 hex numbers -> " +
                current_line(first, last));
        }
    }
    return err("toml::detail::parse_escape_sequence: "
        "unknown escape sequence appeared. -> " + current_line(first, last));
}


template<typename InputIterator>
result<string, std::string>
parse_multi_basic_string(const InputIterator first, const InputIterator last)
{
    return err(std::string("TODO"));
}

template<typename InputIterator>
result<string, std::string>
parse_multi_literal_string(const InputIterator first, const InputIterator last)
{
    return err(std::string("TODO"));
}

template<typename InputIterator>
result<string, std::string>
parse_basic_string(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(*iter != '"')
    {
        throw std::invalid_argument("toml::detail::parse_basic_string: "
            "internal error appeared -> basic_string does not starts with \"");
    }
    ++iter;

    std::string token;
    for(; iter != last; ++iter)
    {
        if(*iter == '"')
        {
            ++iter;
            return ok(string(token, string::basic));
        }
        else if(*iter == '\\')
        {
            // since here is inside a loop, after this, iter will be incremented
            // to deal with this, retrace iter by 1. but it is not guaranteed
            // that InputIterator is a bidirectional iterator, so store the
            // iterator value before reading escape seq.
            const InputIterator bfr(iter);

            const result<string, std::string> unesc =
                parse_escape_sequence(iter, last);
            if(unesc.is_err()) {return unesc;}
            token += unesc.unwrap();

            // retrace by 1 without using operator--.
            const typename std::iterator_traits<InputIterator>::difference_type
                dist = std::distance(bfr, iter) - 1;
            iter = bfr;
            std::advance(iter, dist);
        }
        else if((0x00 <= *iter && *iter <= 0x1F) || *iter == 0x7F)
        {
            const int ch = *iter;
            std::ostringstream oss; oss << std::hex << ch;
            return err("toml::detail::parse_basic_string: "
                "bare control character appeared -> 0x" + oss.str());
        }
        else
        {
            token += *iter;
        }
    }
    return err("toml::detail::parse_basic_string: "
        "basic string is not closed by `\"` -> " + current_line(first, last));
}

template<typename InputIterator>
result<string, std::string>
parse_literal_string(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(*iter != '\'')
    {
        throw std::invalid_argument("toml::detail::parse_literal_string: "
            "internal error appeared -> literal string does not starts with '");
    }
    ++iter;

    std::string token;
    for(; iter != last; ++iter)
    {
        const char c = *iter;
        if(c == '\'')
        {
            ++iter;
            return ok(string(token, string::literal));
        }
        else if((c != 0x09 && 0x00 <= c && c <= 0x1F) || c == 0x7F)
        {
            const int ch = c;
            std::ostringstream oss; oss << std::hex << ch;
            return err("toml::detail::parse_literal_string: "
                "bare control character appeared -> 0x" + oss.str());
        }
        else
        {
            token += *iter;
        }
    }
    return err("toml::detail::parse_literal_string: "
        "literal string is not closed by `'` -> " + current_line(first, last));
}

template<typename InputIterator>
result<string, std::string>
parse_string(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string("toml::detail::parse_string: input is empty"));
    }

    if(*iter == '"')
    {
        if(++iter != last && *iter == '"' && ++iter != last && *iter == '"')
        {
            iter = first;
            return parse_multi_basic_string(iter, last);
        }
        iter = first;
        return parse_basic_string(iter, last);
    }
    else if(*iter == '\'')
    {
        if(++iter != last && *iter == '\'' && ++iter != last && *iter == '\'')
        {
            iter = first;
            return parse_multi_literal_string(iter, last);
        }
        iter = first;
        return parse_literal_string(iter, last);
    }
    return err("toml::detail::parse_string: next token is not a string -> " +
            current_line(first, last));
}

// template<typename InputIterator>
// result<toml::value, std::string>
// parse_value(InputIterator& iter, const InputIterator last)
// {
//     BOOST_STATIC_ASSERT(boost::is_same<
//             typename boost::iterator_value<InputIterator>::type, char>::value);
//
//     const InputIterator first = iter;
//     if(first == last)
//     {
//         return err("toml::detail::parse_value: input is empty");
//     }
//
//     {
//         const result<boolean, std::string> r = parse_boolean(first, last);
//         if(r.is_ok()) {return r;} else if(r.iterator() != first) {return r;}
//     }
//     {
//         // floating parser should be applied earlier than integer parser.
//         const result<floating, InputIterator> r = parse_floating(first, last);
//         if(r.is_ok()) {return result_t(r.unwrap(), r.iterator(), success_t());}
//         else if(r.iterator() != first) {return r;}
//     }
//     {
//         const result<integer, InputIterator> r = parse_integer(first, last);
//         if(r.is_ok()) {return result_t(r.unwrap(), r.iterator(), success_t());}
//         else if(r.iterator() != first) {return r;}
//     }
//     {
//         const result<string, InputIterator> r = parse_string(first, last);
//         if(r.is_ok()) {return result_t(r.unwrap(), r.iterator(), success_t());}
//         else if(r.iterator() != first) {return r;}
//     }
//     return result_t("toml::detail::parse_value: unknown token appeared -> " +
//         std::string(first, find_linebreak(first, last)), first, failure_t());
// }
//
} // detail

// inline toml::table parse(const std::string& fname)
// {
//     std::ifstream ifs(fname.c_str());
//     if(!ifs.good()){throw std::runtime_error("file open error -> " + fname);}
//
//     const std::ios::pos_type beg = ifs.tellg();
//     ifs.seekg(0, ios::end);
//     const std::ios::pos_type end = ifs.tellg();
//     const std::size_t fsize = end - beg;
//     ifs.seekg(beg);
//
//     std::vector<char> letters(fsize);
//     ifs.read(letters.data(), fsize);
//
//     return detail::parse_toml_file(letters.begin(), letters.end());
// }
//
// inline toml::table parse(const std::istream& is)
// {
//     const std::ios::pos_type beg = ifs.tellg();
//     ifs.seekg(0, ios::end);
//     const std::ios::pos_type end = ifs.tellg();
//     const std::size_t fsize = end - beg;
//     ifs.seekg(beg);
//
//     std::vector<char> letters(fsize);
//     ifs.read(letters.data(), fsize);
//
//     return detail::parse_toml_file(letters.begin(), letters.end());
// }

} // toml
#endif// TOML_PARSER_HPP
