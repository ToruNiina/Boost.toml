//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP
#include <toml/result.hpp>
#include <toml/lexer.hpp>
#include <toml/value.hpp>
#include <boost/config.hpp>
#include <boost/static_assert.hpp>
#include <boost/core/addressof.hpp>
#include <iterator>
#include <vector>
#include <istream>
#include <sstream>
#include <fstream>

namespace toml
{
namespace detail
{

// for error messages.
template<typename InputIterator>
std::string current_line(const InputIterator first, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    return std::string(first, std::find(first, last, '\n'));
}

// for error messages.
template<typename InputIterator>
std::string format_dotted_keys(InputIterator first, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, key>::value);

    std::string retval(*first++);
    for(; first != last; ++first)
    {
        retval += '.';
        retval += *first;
    }
    return retval;
}

template<typename T>
T from_string(const std::string& str, const T& opt)
{
    T v(opt);
    std::istringstream iss(str);
    iss >> v;
    return v;
}

template<typename InputIterator>
result<boost::blank, std::string>
insert_nested_key(table& root, const toml::value& v,
                  InputIterator iter, const InputIterator last,
                  const bool is_array_of_table = false)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, key>::value);
    const InputIterator first(iter);
    assert(iter != last);

    table* tab = boost::addressof(root);
    for(; iter != last; ++iter)
    {
        const key& k = *iter;
        InputIterator next(iter); ++next;
        if(next == last) // k is the last key
        {
            if(is_array_of_table)
            {
                if(tab->count(k) == 1)
                {
                    if(!(tab->at(k).is(value::array_tag)))
                    {
                        return err(std::string("toml::detail::insert_nested_key"
                            ": not an array of table"));
                    }
                    array& a = tab->at(k).template get<array>();
                    if(!(a.front().is(value::table_tag)))
                    {
                        std::ostringstream oss;
                        oss << "toml::detail::insert_nested_key: invalid key "
                            << format_dotted_keys(first, last) << ": value is "
                            << "not a table but an array of tables";
                        return err(oss.str());
                    }
                    a.push_back(v);
                    return ok(boost::blank());
                }
                else
                {
                    array aot(1, v);
                    tab->insert(std::make_pair(k, value(aot)));
                    return ok(boost::blank());
                }
            }
            if(tab->count(k) == 1)
            {
                return err("toml::detail::insert_nested_key: value already "
                    "exists -> " + format_dotted_keys(first, last));
            }
            tab->insert(std::make_pair(k, v));
            return ok(boost::blank());
        }
        else
        {
            // if there is no corresponding value, insert it first.
            if(tab->count(k) == 0) {(*tab)[k] = value(table());}

            // type checking...
            if(tab->at(k).is(value::table_tag))
            {
                tab = boost::addressof((*tab)[k].template get<table>());
            }
            else if(tab->at(k).is(value::array_tag)) // array-of-table case
            {
                array& a = (*tab)[k].template get<array>();
                if(!a.back().is(value::table_tag))
                {
                    std::ostringstream oss;
                    oss << "toml::detail::insert_nested_key: invalid key "
                        << format_dotted_keys(first, last) << ": value is not a"
                        << " table but an array of " << a.back().which() << 's';
                    return err(oss.str());
                }
                tab = boost::addressof(a.back().template get<table>());
            }
            else
            {
                std::ostringstream oss;
                oss << "toml::detail::insert_nested_key: invalid key "
                    << format_dotted_keys(first, last) << ": value is not a"
                    << " table but a " << tab->at(k).which();
                return err(oss.str());
            }
        }
    }
    return err(std::string("toml::detail::insert_nested_key: never reach here"));
}

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
                    if     (*i == '1'){retval += base; base *= 2;}
                    else if(*i == '0'){base *= 2;}
                    else if(*i == '_'){/* do nothing. */}
                    else
                    {
                        throw std::logic_error("toml::detail::parse_integer: "
                            "lexer returns invalid token -> " + *token);
                    }
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
        if(*token == "inf" || *token == "+inf")
        {
            return ok(std::numeric_limits<floating>::infinity());
        }
        else if(*token == "-inf")
        {
            return ok(-std::numeric_limits<floating>::infinity());
        }
        else if(*token == "nan" || *token == "+nan")
        {
            return ok(std::numeric_limits<floating>::quiet_NaN());
        }
        else if(*token == "-nan")
        {
            return ok(-std::numeric_limits<floating>::quiet_NaN());
        }
        const std::string::iterator token_last =
            std::remove(token->begin(), token->end(), '_');
        token->erase(token_last, token->end());

        return ok(from_string<toml::floating>(*token, 0.0));
    }
    iter = first;
    return err("toml::detail::parse_floating: "
            "the next token is not a floating -> " + current_line(first, last));
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
        if(*iter == '\\')
        {
            typedef sequence<maybe<lex_ws>, sequence<lex_newline,
                repeat<either<lex_ws, lex_newline>, unlimited>
                > > trim_ws_nl;
            const InputIterator bfr(iter);
            ++iter;
            const boost::optional<std::string> trimmed =
                trim_ws_nl::invoke(iter, last);
            if(!trimmed) {iter = bfr;}
            else{continue;}
        }

        typedef either<lex_ml_basic_unescaped,
                either<character<'"'>, lex_newline> > lex_ml_basic_letter;
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

    return ok(date(from_string<int>(*y, 0), from_string<int>(*m, 0),
                   from_string<int>(*d, 0)));
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

    time tm(hours  (from_string<int>(*h, 0)) +
            minutes(from_string<int>(*m, 0)) +
            seconds(from_string<int>(*s, 0)));

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
        tm += milliseconds(from_string<int>(subseconds->substr(0, 3), 0));
        if(subseconds->size() >= 6)
        {
            tm += microseconds(from_string<int>(subseconds->substr(3, 3), 0));
        }
#ifdef BOOST_DATE_TIME_HAS_NANOSECONDS
        if(subseconds->size() >= 9)
        {
            tm += nanoseconds(from_string<int>(subseconds->substr(6, 3), 0));
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
        offset += hours  (from_string<int>(*offset_h, 0));
        offset += minutes(from_string<int>(*offset_m, 0));
    }
    else
    {
        offset -= hours  (from_string<int>(*offset_h, 0));
        offset -= minutes(from_string<int>(*offset_m, 0));
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
result<std::vector<key>, std::string> // dotted key become vector of keys
parse_key(InputIterator& iter, const InputIterator last);

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
            ++iter; // skip ']'
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
                ++iter; // skip ']'
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
result<std::pair<std::vector<key>, value>, std::string>
parse_key_value_pair(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_key_value_pair: input is empty"));
    }

    const result<std::vector<key>, std::string> key_r = parse_key(iter, last);
    if(!key_r)
    {
        return err("toml::detail::parse_key_value_pair: " +
                key_r.unwrap_err());
    }

    const boost::optional<std::string> kvsp =
        lex_keyval_sep::invoke(iter, last);
    if(!kvsp)
    {
        return err("toml::detail::parse_key_value_pair: "
            "key-value separator `=` missing -> " + current_line(first, last));
    }

    const result<value, std::string> val_r = parse_value(iter, last);
    if(!val_r)
    {
        return err("toml::detail::parse_key_value_pair: " +
                val_r.unwrap_err());
    }
    return ok(std::make_pair(key_r.unwrap(), val_r.unwrap()));
}

template<typename InputIterator>
result<table, std::string>
parse_inline_table(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string("toml::detail::parse_inline_table: "
                    "input is empty"));
    }

    if(*iter != '{')
    {
        return err("toml::detail::parse_inline_table: "
            "the next token is not an inline table -> " +
            current_line(first, last));
    }
    ++iter;

    table retval;
    while(iter != last)
    {
        maybe<lex_ws>::invoke(iter, last);
        if(iter != last && *iter == '}')
        {
            ++iter; // skip `}`
            return ok(retval);
        }
        const InputIterator bfr(iter);

        const result<std::pair<std::vector<key>, value>, std::string> kv_r =
            parse_key_value_pair(iter, last);
        if(!kv_r)
        {
            return err("toml::detail::parse_inline_table: " +
                    kv_r.unwrap_err());
        }
        const std::vector<key>& keys = kv_r.unwrap().first;
        const value&            val  = kv_r.unwrap().second;

        const result<boost::blank, std::string> inserted =
            insert_nested_key(retval, val, keys.begin(), keys.end());
        if(!inserted)
        {
            return err("toml::detail::parse_inline_table: " +
                    inserted.unwrap_err());
        }

        typedef sequence<maybe<lex_ws>, character<','> > lex_table_separator;
        const boost::optional<std::string> sp =
            lex_table_separator::invoke(iter, last);
        if(!sp)
        {
            maybe<lex_ws>::invoke(iter, last);
            if(iter != last && *iter == '}')
            {
                ++iter; // skip `}`
                return ok(retval);
            }
            else
            {
                return err("toml::detail::parse_inline_table: "
                    "missing table separator `,` -> " + current_line(bfr,last));
            }
        }
    }
    return err("toml::detail::parse_inline_table: "
        "inline table did not closed by `}` -> " + current_line(first, last));
}

template<typename InputIterator>
result<key, std::string>
parse_simple_key(InputIterator& iter, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    const InputIterator first = iter;
    if(first == last)
    {
        return err(std::string(
                    "toml::detail::parse_simple_key: input is empty"));
    }

    if(*first == '"')
    {
        const result<string, std::string> bquoted =
            parse_basic_string(iter, last);
        if(bquoted)
        {
            return ok(bquoted.unwrap().str);
        }
        else
        {
            return err(bquoted.unwrap_err());
        }
    }
    else if(*first == '\'')
    {
        const result<string, std::string> lquoted =
            parse_literal_string(iter, last);
        if(lquoted)
        {
            return ok(lquoted.unwrap().str);
        }
        else
        {
            return err(lquoted.unwrap_err());
        }
    }

    const boost::optional<std::string> unq =
        lex_unquoted_key::invoke(iter, last);
    if(unq)
    {
        return ok(*unq);
    }

    return err("toml::detail::parse_simple_key: "
        "the next token is not a simple key -> " + current_line(first, last));
}

template<typename InputIterator>
result<std::vector<key>, std::string>
parse_key(InputIterator& iter, const InputIterator last)
{
    BOOST_STATIC_ASSERT(boost::is_same<
            typename boost::iterator_value<InputIterator>::type, char>::value);
    const InputIterator first = iter;
    if(first == last)
    {
        return err(std::string("toml::detail::parse_key: input is empty"));
    }

    // dotted key -> foo.bar.baz
    const boost::optional<std::string> dots =
        lex_dotted_key::invoke(iter, last);
    if(dots)
    {
        std::vector<key> keys;
        const std::string::const_iterator e(dots->end());
        std::string::const_iterator i(dots->begin());
        while(i != e)
        {
            const result<key, std::string> k = parse_simple_key(i, e);
            if(k)
            {
                keys.push_back(k.unwrap());
            }
            else
            {
                return err("toml::detail::parse_key: "
                    "dotted key contains invalid key -> " + k.unwrap_err());
            }
            lex_ws::invoke(i, e); // skip whitespace before `.` (if any)

            if(!(i == e || *i == '.'))
            {
                return err("toml::detail::parse_key: "
                    "dotted key contains invalid key -> " +
                    current_line(i, e));
            }
            if(i != e)
            {
                ++i; // to skip `.`
                lex_ws::invoke(i, e); // skip whitespace after `.` (if any)
            }
        }
        return ok(keys);
    }
    iter = first;

    // simple key
    const result<key, std::string> smpl = parse_simple_key(iter, last);
    if(smpl)
    {
        return ok(std::vector<key>(1, smpl.unwrap()));
    }
    return err("toml::detail::parse_key: the next token is not a key -> " +
            current_line(first, last));
}

template<typename InputIterator>
result<value, std::string>
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
        const result<table, std::string> r = parse_inline_table(iter, last);
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

template<typename InputIterator>
result<std::vector<key>, std::string>
parse_table_key(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_table_key: input is empty"));
    }

    const boost::optional<std::string> open =
        lex_std_table_open::invoke(iter, last);
    if(!open || iter == last)
    {
        iter = first;
        return err("toml::detail::parse_table_key: not a table title -> " +
                current_line(first, last));
    }

    const result<std::vector<key>, std::string> keys = parse_key(iter, last);
    if(!keys)
    {
        return err("toml::detail::parse_table_key: invalid key in table title "
                "-> " + current_line(first, last));
    }
    // skip whitespace after key; like [ a.b ]
    //                                      ^- this
    lex_ws::invoke(iter, last);

    const boost::optional<std::string> close =
        lex_std_table_close::invoke(iter, last);
    if(!close)
    {
        return err("toml::detail::parse_table_key: table title is not closed by"
                " `]` -> " + current_line(first, last));
    }
    return keys;
}

template<typename InputIterator>
result<std::vector<key>, std::string>
parse_array_table_key(InputIterator& iter, const InputIterator last)
{
    const InputIterator first = iter;
    if(iter == last)
    {
        return err(std::string(
                    "toml::detail::parse_array_table_key: input is empty"));
    }

    const boost::optional<std::string> open =
        lex_array_table_open::invoke(iter, last);
    if(!open || iter == last)
    {
        iter = first;
        return err("toml::detail::parse_array_table_key: not a table title -> "
                + current_line(first, last));
    }

    const result<std::vector<key>, std::string> keys = parse_key(iter, last);
    if(!keys)
    {
        return err("toml::detail::parse_array_table_key: invalid key in table "
                "title -> " + current_line(first, last));
    }
    // skip whitespace after key; like [[ a.b ]]
    //                                       ^- this
    lex_ws::invoke(iter, last);

    const boost::optional<std::string> close =
        lex_array_table_close::invoke(iter, last);
    if(!close)
    {
        return err("toml::detail::parse_array_table_key: table title is not "
                "closed by `]` -> " + current_line(first, last));
    }
    return keys;
}

// parse table body (key-value pairs until the iter hits the next [tablekey])
template<typename InputIterator>
result<table, std::string>
parse_ml_table(InputIterator& iter, const InputIterator last)
{
    const InputIterator first(iter);
    if(first == last)
    {
        return err(std::string("toml::detail::parse_ml_table: input is empty"));
    }

    typedef repeat<sequence<maybe<lex_ws>,
            sequence<maybe<lex_comment>, lex_newline> >, unlimited
            > skip_line;
    skip_line::invoke(iter, last);

    table tab;
    while(iter != last)
    {
        lex_ws::invoke(iter, last);
        const InputIterator bfr(iter);
        {
            const result<std::vector<key>, std::string>
                tabkey(parse_array_table_key(iter, last));
            if(tabkey || iter != bfr)
            {
                iter = bfr;
                return ok(tab);
            }
        }
        {
            const result<std::vector<key>, std::string>
                tabkey(parse_table_key(iter, last));
            if(tabkey || iter != bfr)
            {
                iter = bfr;
                return ok(tab);
            }
        }

        const result<std::pair<std::vector<key>, value>, std::string>
            kv(parse_key_value_pair(iter, last));
        if(kv)
        {
            const std::vector<key>& keys = kv.unwrap().first;
            const value&            val  = kv.unwrap().second;
            const result<boost::blank, std::string> inserted =
                insert_nested_key(tab, val, keys.begin(), keys.end());
            if(!inserted)
            {
                return err(inserted.unwrap_err());
            }
        }
        else
        {
            return err("toml::detail::parse_ml_table: invalid line appeared -> "
                + kv.unwrap_err());
        }
        skip_line::invoke(iter, last);
        // comment lines are skipped by the above function call.
        // However, if the file ends with comment without newline,
        // it might cause parsing error because skip_line matches
        // `comment + newline`, not `comment` itself. to skip the
        // last comment, call this one more time.
        lex_comment::invoke(iter, last);
    }
    return ok(tab);
}

template<typename InputIterator>
result<table, std::string>
parse_toml_file(InputIterator& iter, const InputIterator last)
{
    const InputIterator first(iter);
    if(first == last)
    {
        return err(std::string("toml::detail::parse_toml_file: input is empty"));
    }

    table data;
    {
        const result<table, std::string> tab = parse_ml_table(iter, last);
        if(tab) {data = tab.unwrap();}
        else    {return err(tab.unwrap_err());}
    }
    while(iter != last)
    {
        const InputIterator bfr(iter);
        {
            const result<std::vector<key>, std::string>
                tabkey(parse_array_table_key(iter, last));
            if(tabkey)
            {
                const result<table, std::string> tab = parse_ml_table(iter, last);
                if(!tab){return err(tab.unwrap_err());}
                const result<boost::blank, std::string> inserted(
                    insert_nested_key(data, tab.unwrap(),
                        tabkey.unwrap().begin(), tabkey.unwrap().end(), true));
                if(!inserted) {return err(inserted.unwrap_err());}
                continue;
            }
        }
        {
            const result<std::vector<key>, std::string>
                tabkey(parse_table_key(iter, last));
            if(tabkey)
            {
                const result<table, std::string> tab = parse_ml_table(iter, last);
                if(!tab){return err(tab.unwrap_err());}
                const result<boost::blank, std::string> inserted(
                    insert_nested_key(data, tab.unwrap(),
                        tabkey.unwrap().begin(), tabkey.unwrap().end()));
                if(!inserted) {return err(inserted.unwrap_err());}
                continue;
            }
        }
        return err("toml::detail::parse_toml_file: " + current_line(bfr, last));
    }
    return ok(data);
}

} // detail

inline table parse(std::istream& is)
{
    const std::ios::pos_type beg = is.tellg();
    is.seekg(0, std::ios::end);
    const std::ios::pos_type end = is.tellg();
    const std::size_t fsize = end - beg;
    is.seekg(beg);

    std::vector<char> letters(fsize);
    is.read(letters.data(), fsize);

    std::vector<char>::const_iterator first = letters.begin(),
                                       last = letters.end();
    const detail::result<table, std::string> res(
            detail::parse_toml_file(first, last));
    if(!res)
    {
        throw std::runtime_error(res.unwrap_err());
    }
    else
    {
        return res.unwrap();
    }
}

inline table parse(const std::string& fname)
{
    std::ifstream ifs(fname.c_str());
    if(!ifs.good())
    {
        throw std::runtime_error("toml::parse: file open error -> " + fname);
    }

    return parse(ifs);
}

} // toml
#endif// TOML_PARSER_HPP
