//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_SERIALIZER_HPP
#define TOML_SERIALIZER_HPP
#include <toml/value.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>
#include <iomanip>

namespace toml
{

BOOST_CONSTEXPR inline std::size_t forceinline()
{
    return std::numeric_limits<std::size_t>::max();
}

struct serializer : boost::static_visitor<std::string>
{
    serializer(const std::size_t w): width_(w){}
    ~serializer(){}

    std::string operator()(const boost::blank) const
    {
        return "<blank>";
    }
    std::string operator()(const boolean b) const
    {
        return b ? "true" : "false";
    }
    std::string operator()(const integer i) const
    {
        return boost::lexical_cast<std::string>(i);
    }
    std::string operator()(const floating f) const
    {
        std::ostringstream oss; oss << std::fixed << std::showpoint << f;
        return oss.str();
    }
    std::string operator()(const string& s) const
    {
        if(s.kind == string::basic)
        {
            if(std::find(s.str.begin(), s.str.end(), '\n') != s.str.end())
            {
                const std::string open("\"\"\"\n");
                const std::string close("\"\"\"");
                const std::string b = escape_ml_basic_string(s.str);
                return open + b + close;
            }
            else
            {
                const std::string quote("\"");
                const std::string b = escape_basic_string(s.str);
                return quote + b + quote;
            }
        }
        else
        {
            if(std::find(s.str.begin(), s.str.end(), '\n') != s.str.end())
            {
                const std::string open("'''\n");
                const std::string close("'''");
                return open + s.str + close;
            }
            else
            {
                const std::string quote("'");
                return quote + s.str + quote;
            }
        }
    }
    // TODO format datetime
    std::string operator()(const date& v) const
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
    std::string operator()(const time& v) const
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
    std::string operator()(const local_datetime& v) const
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
    std::string operator()(const offset_datetime& v) const
    {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }
    std::string operator()(const array& v) const
    {
        std::string serial;
        serial += '[';
        for(typename array::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            serial += apply_visitor(*this, *i);
            serial += ", ";
        }
        serial += ']';
        return serial;
    }
    std::string operator()(const table& v) const
    {
        std::string serial;
        serial += '{';
        for(typename table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            serial += i->first;
            serial += " = ";
            serial += apply_visitor(*this, i->second);
            serial += ", ";
        }
        serial += '}';
        return serial;
    }

  private:

    std::string escape_basic_string(const std::string& s) const
    {
        std::string retval;
        for(std::string::const_iterator i(s.begin()), e(s.end()); i!=e; ++i)
        {
            switch(*i)
            {
                case '\\': retval += "\\\\"; break;
                case '\"': retval += "\\\""; break;
                case '\b': retval += "\\b";  break;
                case '\t': retval += "\\t";  break;
                case '\f': retval += "\\f";  break;
                case '\n': retval += "\\n";  break;
                case '\r': retval += "\\r";  break;
                default  : retval += *i;     break;
            }
        }
        return retval;
    }

    std::string escape_ml_basic_string(const std::string& s) const
    {
        std::string retval;
        for(std::string::const_iterator i(s.begin()), e(s.end()); i!=e; ++i)
        {
            switch(*i)
            {
                case '\\': retval += "\\\\"; break;
                case '\"': retval += "\\\""; break;
                case '\b': retval += "\\b";  break;
                case '\t': retval += "\\t";  break;
                case '\f': retval += "\\f";  break;
                case '\n': retval += "\n";   break;
                case '\r':
                {
                    if((i+1) != e && *(i+1) == '\n') {retval += "\r\n"; ++i;}
                    else                             {retval += "\\r";}
                    break;
                }
                default  : retval += *i;     break;
            }
        }
        return retval;
    }



  private:
    std::size_t width_;   // TODO use it to serialize
    std::string valname_; // to output table...
};

inline std::string serialize(const value& v, std::size_t w = 80)
{
    return apply_visitor(serializer(w), v);
}

template<typename charT, typename traits>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const value& v)
{
    os << serialize(v);
    return os;
}

} // toml
#endif// TOML_SERIALIZER_HPP
