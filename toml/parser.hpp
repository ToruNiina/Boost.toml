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
                        std::cerr << "token -> " << *i << std::endl;
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

// inline std::string read_utf8_codepoint(const std::string& str)
// {
//     boost::uint_least32_t codepoint;
//     std::istringstream iss(str);
//     iss >> std::hex >> codepoint;
//
//     std::string character;
//     if(codepoint < 0x80)
//     {
//         character += static_cast<unsigned char>(codepoint);
//     }
//     else if(codepoint < 0x800)
//     {
//         character += static_cast<unsigned char>(0xC0| codepoint >> 6);
//         character += static_cast<unsigned char>(0x80|(codepoint & 0x3F));
//     }
//     else if(codepoint < 0x10000)
//     {
//         character += static_cast<unsigned char>(0xE0| codepoint >> 12);
//         character += static_cast<unsigned char>(0x80|(codepoint >> 6 & 0x3F));
//         character += static_cast<unsigned char>(0x80|(codepoint      & 0x3F));
//     }
//     else
//     {
//         character += static_cast<unsigned char>(0xF0| codepoint >> 18);
//         character += static_cast<unsigned char>(0x80|(codepoint >> 12 & 0x3F));
//         character += static_cast<unsigned char>(0x80|(codepoint >> 6  & 0x3F));
//         character += static_cast<unsigned char>(0x80|(codepoint       & 0x3F));
//     }
//     return character;
// }
//
// template<typename InputIterator>
// result<string, InputIterator>
// parse_escape_sequence(const InputIterator first, const InputIterator last)
// {
//     // literal tag does not have any escape sequence. the result is basic_string
//     typedef result<string, InputIterator> result_t;
//
//     InputIterator iter = first;
//     if(iter == last || *iter != '\\')
//     {
//         throw std::invalid_argument(
//                 "toml::detail::unescape: got empty or invalid string");
//     }
//     ++iter; // this is for backslash.
//
//     switch(*iter)
//     {
//         case '\\': return result_t(string("\\"), ++iter, success_t());
//         case '"' : return result_t(string("\""), ++iter, success_t());
//         case 'b' : return result_t(string("\b"), ++iter, success_t());
//         case 't' : return result_t(string("\t"), ++iter, success_t());
//         case 'n' : return result_t(string("\n"), ++iter, success_t());
//         case 'f' : return result_t(string("\f"), ++iter, success_t());
//         case 'r' : return result_t(string("\r"), ++iter, success_t());
//         case 'u' :
//         {
//             if(std::distance(iter, last) < 5)
//             {
//                 return result_t("toml::detail::parse_escape_sequence: "
//                     "\\uXXXX must have 4 numbers -> " +
//                     std::string(first, find_linebreak(first, last)),
//                     iter, failure_t());
//             }
//
//             InputIterator cp_begin = iter; std::advance(cp_begin, 1);
//             InputIterator cp_end   = iter; std::advance(cp_end,   5);
//             if(!::toml::detail::all_of(cp_begin, cp_end, ishex))
//             {
//                 return result_t("toml::detail::parse_escape_sequence: "
//                     "\\uXXXX must be represented by hex -> " +
//                     std::string(first, find_linebreak(first, last)),
//                     iter, failure_t());
//             }
//
//             const std::string unesc =
//                 read_utf8_codepoint(std::string(cp_begin, cp_end));
//             return result_t(string(unesc), cp_end, success_t());
//         }
//         case 'U':
//         {
//             if(std::distance(iter, last) < 9)
//             {
//                 return result_t("toml::detail::parse_escape_sequence: "
//                     "\\UXXXXXXXX must have 8 numbers -> " +
//                     std::string(first, find_linebreak(first, last)),
//                     iter, failure_t());
//             }
//
//             InputIterator cp_begin = iter; std::advance(cp_begin, 1);
//             InputIterator cp_end   = iter; std::advance(cp_end,   9);
//             if(!::toml::detail::all_of(cp_begin, cp_end, ishex))
//             {
//                 return result_t("toml::detail::parse_escape_sequence: "
//                     "\\UXXXXXXXX must be represented by hex -> " +
//                     std::string(first, find_linebreak(first, last)),
//                     iter, failure_t());
//             }
//
//             const std::string unesc =
//                 read_utf8_codepoint(std::string(cp_begin, cp_end));
//             return result_t(string(unesc), cp_end, success_t());
//         }
//         default:
//         {
//             return result_t("toml::detail::parse_escape_sequence: "
//                 "unknown escape sequence appeared. -> " +
//                 std::string(first, find_linebreak(first, last)),
//                 iter, failure_t());
//         }
//     }
// }


// template<typename InputIterator>
// result<string, InputIterator>
// parse_multi_basic_string(const InputIterator first, const InputIterator last)
// {
//     return result<string, InputIterator>("TODO", first, failure_t());
// }
//
// template<typename InputIterator>
// result<string, InputIterator>
// parse_multi_literal_string(const InputIterator first, const InputIterator last)
// {
//     return result<string, InputIterator>("TODO", first, failure_t());
// }
//
// template<typename InputIterator>
// result<string, InputIterator>
// parse_basic_string(const InputIterator first, const InputIterator last)
// {
//     typedef result<string, InputIterator> result_t;
//
//     InputIterator iter(first);
//     if(*iter != '"')
//     {
//         throw std::invalid_argument("toml::detail::parse_basic_string: "
//                 "internal error appeared");
//     }
//     ++iter;
//
//     std::string token;
//     for(; iter != last; ++iter)
//     {
//         if(*iter == '"')
//         {
//             return result_t(string(token, string::basic), ++iter, success_t());
//         }
//         else if(*iter == '\\')
//         {
//             const result_t unesc = parse_escape_sequence(iter, last);
//             if(unesc.is_err()) {return unesc;}
//             token += unesc.unwrap();
//             InputIterator unesc_end = unesc.iterator();
//             // XXX after this, the iterator will be incremented. so retrace by 1
//             const typename std::iterator_traits<InputIterator>::difference_type
//                 len_escape_sequence = std::distance(iter, unesc_end);
//             std::advance(iter, len_escape_sequence - 1);
//         }
//         else if((0x00 <= *iter && *iter <= 0x1F) || *iter == 0x7F)
//         {
//             const int ch = *iter;
//             std::ostringstream oss; oss << std::hex << ch;
//             return result_t("toml::detail::parse_basic_string: "
//                 "bare control character appeared -> 0x" + oss.str(),
//                 iter, failure_t());
//         }
//         else
//         {
//             token += *iter;
//         }
//     }
//     return result_t("toml::detail::parse_basic_string: "
//         "basic string is not closed by `\"` -> " +
//         std::string(first, find_linebreak(first, last)), iter, failure_t());
// }
//
// template<typename InputIterator>
// result<string, InputIterator>
// parse_literal_string(const InputIterator first, const InputIterator last)
// {
//     typedef result<string, InputIterator> result_t;
//
//     InputIterator iter(first);
//     if(*iter != '\'')
//     {
//         throw std::invalid_argument("toml::detail::parse_literal_string: "
//                 "internal error appeared");
//     }
//     ++iter;
//
//     std::string token;
//     for(; iter != last; ++iter)
//     {
//         if(*iter == '\'')
//         {
//             return result_t(string(token, string::literal), iter, success_t());
//         }
//         else if((*iter != 0x09 && 0x00 <= *iter && *iter <= 0x1F) ||
//                 *iter == 0x7F)
//         {
//             const int ch = *iter;
//             std::ostringstream oss; oss << std::hex << ch;
//             return result_t("toml::detail::parse_literal_string: "
//                 "bare control character appeared -> 0x" + oss.str(),
//                 iter, failure_t());
//         }
//         else
//         {
//             token += *iter;
//         }
//     }
//     return result_t("toml::detail::parse_literal_string: "
//         "literal string is not closed by `'` -> " +
//         std::string(first, find_linebreak(first, last)), iter, failure_t());
// }
//
// template<typename InputIterator>
// result<string, InputIterator>
// parse_string(const InputIterator first, const InputIterator last)
// {
//     typedef result<string, InputIterator> result_t;
//
//     if(first == last)
//     {
//         return result_t(std::string(
//             "toml::detail::parse_string: input is empty."), first, failure_t());
//     }
//
//     InputIterator iter = first;
//     if(*iter == '"')
//     {
//         if(++iter != last && *iter == '"' && ++iter != last && *iter == '"')
//         {
//             return parse_multi_basic_string(first, last);
//         }
//         return parse_basic_string(first, last);
//     }
//     else if(*iter == '\'')
//     {
//         if(++iter != last && *iter == '\'' && ++iter != last && *iter == '\'')
//         {
//             return parse_multi_literal_string(first, last);
//         }
//         return parse_literal_string(first, last);
//     }
//     return result_t(std::string(
//         "toml::detail::parse_string: failed. try next"), first, failure_t());
// }

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
