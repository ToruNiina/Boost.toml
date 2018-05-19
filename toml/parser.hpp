//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP
#include <toml/result.hpp>
#include <toml/lexer.hpp>
#include <boost/config.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/static_assert.hpp>
#include <boost/optional/optional_io.hpp>
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
    return err("toml::detail::parse_boolean: "
            "the next token is not a boolean -> " + current_line(first, last));
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
    return err("toml::detail::parse_integer: "
            "the next token is not an integer -> " + current_line(first, last));
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
    return err("toml::detail::parse_integer: "
            "the next token is not a integer -> " + current_line(first, last));
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
parse_ml_basic_string(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    {
        boost::optional<std::string> open =
                lex_ml_basic_string_delim::invoke(iter, last);
        if(!open)
        {
            throw std::invalid_argument("toml::detail::parse_ml_basic_string: "
                "internal error appeared -> \"\"\" not found");
        }
    }
    {
        const InputIterator bfr(iter);
        const boost::optional<std::string> nl = lex_newline::invoke(iter, last);
        if(!nl){iter = bfr;}
    }

    std::string token;
    while(iter != last)
    {
        {
            const InputIterator bfr(iter);
            if(const boost::optional<std::string> close =
                lex_ml_basic_string_delim::invoke(iter, last))
            {
                return ok(string(token, string::basic));
            }
            iter = bfr;
        }
        {
            typedef sequence<character<'\\'>,
                repeat<either<lex_ws, lex_newline>, at_least<1> > > trim_ws_nl;
            const InputIterator bfr(iter);
            const boost::optional<std::string> trimmed =
                trim_ws_nl::invoke(iter, last);
            if(!trimmed) {iter = bfr;}
        }

        typedef either<lex_ml_basic_unescaped, lex_newline> lex_ml_basic_letter;
        if(*iter == '\\')
        {
            const result<string, std::string> unesc =
                parse_escape_sequence(iter, last);
            if(unesc.is_err()) {return unesc;}
            token += unesc.unwrap();
        }
        else if(const boost::optional<std::string> ch =
                lex_ml_basic_letter::invoke(iter, last))
        {
            token += *ch;
        }
        else
        {
            const int code = *iter;
            std::ostringstream oss; oss << std::hex << code;
            return err("toml::detail::parse_basic_string: "
                "bare control character appeared -> 0x" + oss.str());
        }
    }
    return err("toml::detail::parse_ml_basic_string: "
        "multi line basic string is not closed by `\"\"\"` -> " +
        current_line(first, last));
}

template<typename InputIterator>
result<string, std::string>
parse_ml_literal_string(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    {
        boost::optional<std::string> open =
                lex_ml_literal_string_delim::invoke(iter, last);
        if(!open)
        {
            throw std::invalid_argument("toml::detail::parse_ml_literal_string:"
                " internal error appeared -> ''' not found");
        }
    }
    {
        const InputIterator bfr(iter);
        const boost::optional<std::string> nl = lex_newline::invoke(iter, last);
        if(!nl){iter = bfr;}
    }

    std::string token;
    while(iter != last)
    {
        {
            const InputIterator bfr(iter);
            if(const boost::optional<std::string> close =
                lex_ml_literal_string_delim::invoke(iter, last))
            {
                return ok(string(token, string::literal));
            }
            iter = bfr;
        }

        typedef either<lex_ml_literal_char, lex_newline> lex_ml_literal_letter;
        if(const boost::optional<std::string> ch =
                lex_ml_literal_letter::invoke(iter, last))
        {
            token += *ch;
        }
        else
        {
            const int code = *iter;
            std::ostringstream oss; oss << std::hex << code;
            return err("toml::detail::parse_basic_string: "
                "bare control character appeared -> 0x" + oss.str());
        }
    }
    return err("toml::detail::parse_ml_literal_string: "
        "multi line basic string is not closed by `'''` -> " +
        current_line(first, last));
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
    while(iter != last)
    {
        if(*iter == '"')
        {
            ++iter;
            return ok(string(token, string::basic));
        }
        else if(*iter == '\\')
        {
            const result<string, std::string> unesc =
                parse_escape_sequence(iter, last);
            if(unesc.is_err()) {return unesc;}
            token += unesc.unwrap();
        }
        else if(const boost::optional<std::string> ch =
                lex_basic_unescaped::invoke(iter, last))
        {
            token += *ch;
        }
        else
        {
            const int code = *iter;
            std::ostringstream oss; oss << std::hex << code;
            return err("toml::detail::parse_basic_string: "
                "bare control character appeared -> 0x" + oss.str());
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
    while(iter != last)
    {
        if(*iter == '\'')
        {
            ++iter;
            return ok(string(token, string::literal));
        }
        else if(const boost::optional<std::string> ch =
                lex_literal_char::invoke(iter, last))
        {
            token += *ch;
        }
        else
        {
            const int code = *iter;
            std::ostringstream oss; oss << std::hex << code;
            return err("toml::detail::parse_literal_string: "
                "bare control character appeared -> 0x" + oss.str());
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
            return parse_ml_basic_string(iter, last);
        }
        iter = first;
        return parse_basic_string(iter, last);
    }
    else if(*iter == '\'')
    {
        if(++iter != last && *iter == '\'' && ++iter != last && *iter == '\'')
        {
            iter = first;
            return parse_ml_literal_string(iter, last);
        }
        iter = first;
        return parse_literal_string(iter, last);
    }
    return err("toml::detail::parse_string: next token is not a string -> " +
            current_line(first, last));
}

template<typename InputIterator>
result<date, std::string>
parse_local_date(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_local_date: input is empty"));
    }

    const boost::optional<std::string> y =
        lex_date_fullyear::invoke(iter, last);
    if(!y || *iter != '-')
    {
        iter = first;
        return err("toml::detail::parse_local_date: did not match year -> "
                + current_line(iter, last));
    }
    ++iter;

    const boost::optional<std::string> m =
        lex_date_month::invoke(iter, last);
    if(!m || *iter != '-')
    {
        iter = first;
        return err("toml::detail::parse_local_date: did not match month -> "
                + current_line(iter, last));
    }
    ++iter;

    const boost::optional<std::string> d =
        lex_date_mday::invoke(iter, last);
    if(!d)
    {
        iter = first;
        return err("toml::detail::parse_local_date: did not match day -> "
                + current_line(iter, last));
    }
    return ok(date(boost::lexical_cast<int>(*y), boost::lexical_cast<int>(*m),
                   boost::lexical_cast<int>(*d)));
}

template<typename InputIterator>
result<time, std::string>
parse_local_time(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_local_time: input is empty"));
    }

    const boost::optional<std::string> h = lex_time_hour::invoke(iter, last);
    if(!h || *iter != ':')
    {
        iter = first;
        return err("toml::detail::parse_local_time: did not match hour -> " +
                  current_line(iter, last));
    }
    ++iter;

    const boost::optional<std::string> m = lex_time_minute::invoke(iter, last);
    if(!m || *iter != ':')
    {
        iter = first;
        return err("toml::detail::parse_local_time: did not match min -> " +
                  current_line(iter, last));
    }
    ++iter;

    const boost::optional<std::string> s = lex_time_second::invoke(iter, last);
    if(!s)
    {
        iter = first;
        return err("toml::detail::parse_local_time: did not match sec -> " +
                  current_line(iter, last));
    }

    time tm(hours(boost::lexical_cast<int>(*h)) +
            minutes(boost::lexical_cast<int>(*m)) +
            seconds(boost::lexical_cast<int>(*s)));

    if(iter != last && *iter == '.')
    {
        ++iter;
        boost::optional<std::string> subseconds =
            repeat<lex_digit, at_least<1> >::invoke(iter, last);
        if(!subseconds)
        {
            return err("toml::detail::parse_local_time: a point appeared but "
                "subsec part did not appeared -> " + current_line(first, last));
        }
        switch(subseconds->size() % 3)
        {
            case 2: *subseconds += '0'; BOOST_FALLTHROUGH;
            case 1: *subseconds += '0'; BOOST_FALLTHROUGH;
            case 0: break;
        }
        tm += milliseconds(boost::lexical_cast<int>(subseconds->substr(0, 3)));
        if(subseconds->size() >= 6)
        {
            tm += microseconds(
                    boost::lexical_cast<int>(subseconds->substr(3, 3)));
        }
#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
        if(subseconds->size() >= 9)
        {
            tm += nanoseconds(
                    boost::lexical_cast<int>(subseconds->substr(6, 3)));
        }
#endif
    }
    return ok(tm);
}

template<typename InputIterator>
result<local_datetime, std::string>
parse_local_datetime(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_local_datetime: input is empty"));
    }

    const result<date, std::string> dr = parse_local_date(iter, last);
    if(!dr)
    {
        iter = first;
        return err("toml::detail::parse_local_datetime: " + dr.unwrap_err());
    }

    if(iter == last || (*iter != 'T' && *iter != 't' && *iter != ' '))
    {
        iter = first;
        return err("toml::detail::parse_local_datetime: "
            "invalid datetime delimiter -> " + current_line(first, last));
    }
    ++iter;

    const result<time, std::string> tr = parse_local_time(iter, last);
    if(!tr)
    {
        iter = first;
        return err("toml::detail::parse_local_datetime: " + tr.unwrap_err());
    }
    return ok(local_datetime(dr.unwrap(), tr.unwrap()));
}

template<typename InputIterator>
result<offset_datetime, std::string>
parse_offset_datetime(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_local_datetime: input is empty"));
    }
    const result<local_datetime, std::string> ldt =
        parse_local_datetime(iter, last);
    if(!ldt)
    {
        iter = first;
        return err("toml::detail::parse_offset_datetime: " + ldt.unwrap_err());
    }

    if(*iter == 'Z')
    {
        ++iter;
        return ok(offset_datetime(ldt.unwrap(),
            time_zone_ptr(new boost::local_time::posix_time_zone("UTC"))));
    }
    else if(*iter != '+' && *iter != '-')
    {
        iter = first;
        return err("toml::detail::parse_offset_datetime: invalid time numoffset"
                + current_line(first, last));
    }

    const char sign = *(iter++);
    boost::posix_time::time_duration offset(0, 0, 0);

    const boost::optional<std::string> offset_h =
        lex_time_hour::invoke(iter, last);
    if(!offset_h || *iter != ':')
    {
        return err("toml::detail::parse_offset_datetime: "
            "did not match offset-hour -> " + current_line(iter, last));
    }
    ++iter;

    const boost::optional<std::string> offset_m =
        lex_time_minute::invoke(iter, last);
    if(!offset_m)
    {
        return err("toml::detail::parse_offset_datetime: "
            "did not match offset minute" + current_line(iter, last));
    }

    if(sign == '+')
    {
        offset += hours  (boost::lexical_cast<int>(*offset_h));
        offset += minutes(boost::lexical_cast<int>(*offset_m));
    }
    else
    {
        offset -= hours  (boost::lexical_cast<int>(*offset_h));
        offset -= minutes(boost::lexical_cast<int>(*offset_m));
    }

    boost::local_time::time_zone_names tzn(
        "TOML User Input", "TML", "", "");
    boost::local_time::dst_adjustment_offsets dst_offset(
        boost::posix_time::time_duration(0,0,0),
        boost::posix_time::time_duration(0,0,0),
        boost::posix_time::time_duration(0,0,0));
    boost::shared_ptr<boost::local_time::dst_calc_rule> rules; // null

    return ok(offset_datetime(ldt.unwrap() - offset,
        time_zone_ptr(new boost::local_time::custom_time_zone(
                tzn, offset, dst_offset, rules))));
}

// forward-decl
template<typename InputIterator>
result<value, std::string>
parse_value(InputIterator& iter, const InputIterator last);

template<typename InputIterator>
result<array, std::string>
parse_array(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string("toml::detail::parse_array: input is empty"));
    }

    if(*iter != '[')
    {
        return err("toml::detail::parse_array: "
            "the next token is not an array -> " + current_line(first, last));
    }
    ++iter;

    array retval;
    while(iter != last)
    {
        typedef repeat<either<lex_wschar, either<lex_newline, lex_comment> >,
            unlimited> lex_ws_comment_newline;
        lex_ws_comment_newline::invoke(iter, last);

        if(iter != last && *iter == ']')
        {
            return ok(retval);
        }

        const InputIterator bfr(iter);
        result<value, std::string> val_r = parse_value(iter, last);
        if(val_r)
        {
            retval.push_back(val_r.unwrap());
        }
        else
        {
            return err("toml::detail::parse_array: " + val_r.unwrap_err());
        }
        typedef sequence<maybe<lex_ws>, character<','> > lex_array_separator;
        const boost::optional<std::string> sp =
            lex_array_separator::invoke(iter, last);
        if(!sp)
        {
            lex_ws_comment_newline::invoke(iter, last);
            if(iter != last && *iter == ']')
            {
                return ok(retval);
            }
            else
            {
                return err("toml::detail::parse_array: "
                    "missing array separator `,` -> " + current_line(bfr,last));
            }
        }
    }

    return err("toml::detail::parse_array: array did not closed by `]` -> "
            + current_line(first, last));
}


template<typename InputIterator>
result<toml::value, std::string>
parse_value(InputIterator& iter, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    const InputIterator first = iter;
    if(first == last)
    {
        return err(std::string("toml::detail::parse_value: input is empty"));
    }

    {
        const result<string, std::string> r = parse_string(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<array, std::string> r = parse_array(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<boolean, std::string> r = parse_boolean(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<offset_datetime, std::string> r =
            parse_offset_datetime(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<local_datetime, std::string> r =
            parse_local_datetime(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<time, std::string> r = parse_local_time(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<date, std::string> r = parse_local_date(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<floating, std::string> r = parse_floating(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }
    {
        const result<integer, std::string> r = parse_integer(iter, last);
        if(r.is_ok())
        {
            return ok(r.unwrap());
        }
        else if(iter != first)
        {
            return err("toml::detail::parse_value: partial match: " +
                    r.unwrap_err());
        }
    }

    return err("toml::detail::parse_value: unknown token appeared -> " +
            current_line(first, last));
}

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
