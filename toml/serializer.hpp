//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_SERIALIZER_HPP
#define TOML_SERIALIZER_HPP
#include <toml/value.hpp>
#include <sstream>
#include <locale>
#include <iomanip>

namespace toml
{
namespace detail
{
inline bool is_array_of_table(const value& v)
{
    return v.is(value::array_tag) && (!v.get<array>().empty()) &&
           v.get<array>().front().is(value::table_tag);
}

struct serializer : boost::static_visitor<std::string>
{
    serializer(const std::size_t w): width_(w)
    {}
    serializer(const std::size_t w, const std::vector<toml::key>& ks)
        : width_(w), keys_(ks)
    {}
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
        std::ostringstream oss; oss << i;
        return oss.str();
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

            std::string oneline = escape_basic_string(s.str);
            if(oneline.size() + 2 < width_ || width_ < 2)
            {
                const std::string quote("\"");
                return quote + oneline + quote;
            }
            // split into multiple lines...
            std::string token("\"\"\"\n");
            while(!oneline.empty())
            {
                if(oneline.size() < width_)
                {
                    token += oneline;
                    oneline.clear();
                }
                else if(oneline.at(width_ - 2) == '\\')
                {
                    token += oneline.substr(0, width_-2);
                    token += "\\\n";
                    oneline.erase(0, width_-2);
                }
                else
                {
                    token += oneline.substr(0, width_-1);
                    token += "\\\n";
                    oneline.erase(0, width_-1);
                }
            }
            return token + std::string("\"\"\"");
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
        boost::gregorian::date_facet*
            facet(new boost::gregorian::date_facet("%Y-%m-%d"));
        oss.imbue(std::locale(oss.getloc(), facet));
        oss << v;
        return oss.str();
    }
    std::string operator()(const time& v) const
    {
        std::ostringstream oss;
        boost::gregorian::date_facet*
            facet(new boost::gregorian::date_facet("%F *"));
        oss.imbue(std::locale(oss.getloc(), facet));
        oss << v;
        return oss.str();
    }
    std::string operator()(const local_datetime& v) const
    {
        std::ostringstream oss;
        boost::posix_time::time_facet*
            facet(new boost::posix_time::time_facet("%Y-%m-%dT%H:%M:%S%F"));
        oss.imbue(std::locale(oss.getloc(), facet));
        oss << v;
        return oss.str();
    }
    std::string operator()(const offset_datetime& v) const
    {
        std::ostringstream oss;
        boost::local_time::local_time_facet*
            facet(new boost::local_time::local_time_facet(
                        "%Y-%m-%dT%H:%M:%S%F%Q"));
        oss.imbue(std::locale(oss.getloc(), facet));
        oss << v;
        return oss.str();
    }
    std::string operator()(const array& v) const
    {
        if(!v.empty() && v.front().is(value::table_tag) && !keys_.empty())
        {
            bool width_exceeds = false;
            std::string token(serialize_key(keys_.back()));
            token += " = [\n";
            for(array::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
            {
                const std::string t = make_inline_table(i->get<table>());
                if(t.size() > width_){width_exceeds = true; break;}
                token += t;
                token += ",\n";
            }
            if(!width_exceeds)
            {
                token += "]\n";
                return token;
            }

            token.clear();
            for(array::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
            {
                token += "[[";
                token += serialize_dotted_key(keys_);
                token += "]]\n";
                token += make_multiline_table(i->get<table>());
            }
            return token;
        }

        { // try to print inline
            const std::string inl = this->make_inline_array(v);
            if(inl.size() < this->width_ &&
               std::find(inl.begin(), inl.end(), '\n') == inl.end())
            {
                return inl;
            }
        }
        std::string token;
        token += "[\n";
        for(typename array::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            token += apply_visitor(*this, *i);
            token += ",\n";
        }
        token += "]\n";
        return token;
    }

    std::string operator()(const table& v) const
    {
        if(!keys_.empty())
        {
            const std::string inl = this->make_inline_table(v);
            if(inl.size() < this->width_ &&
               std::find(inl.begin(), inl.end(), '\n') == inl.end())
            {
                std::string token(serialize_key(keys_.back()));
                token += " = ";
                token += inl;
                token += '\n';
                return token;
            }
        }

        std::string token;
        if(!keys_.empty())
        {
            token += '[';
            token += serialize_dotted_key(keys_);
            token += "]\n";
        }
        token += make_multiline_table(v);
        return token;
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

    std::string serialize_key(const toml::key& key) const
    {
        toml::key::const_iterator i(key.begin());
        lex_unquoted_key::invoke(i, key.end());
        if(i == key.end())
        {
            return key;
        }
        std::string token("\"");
        token += escape_basic_string(key);
        token += "\"";
        return token;
    }

    std::string serialize_dotted_key(const std::vector<toml::key>& keys) const
    {
        std::string token;
        for(std::vector<toml::key>::const_iterator
                i(keys.begin()), e(keys.end()); i!=e; ++i)
        {
            token += this->serialize_key(*i);
            token += '.';
        }
        token.erase(token.size() - 1, 1); // remove trailing `.`
        return token;
    }

    std::string make_inline_array(const array& v) const
    {
        std::string token;
        token += '[';
        for(typename array::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            token += apply_visitor(serializer(
                        std::numeric_limits<std::size_t>::max()), *i);
            token += ',';
        }
        token += ']';
        return token;
    }

    std::string make_inline_table(const table& v) const
    {
        std::string token;
        token += '{';
        for(typename table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            token += i->first;
            token += '=';
            token += apply_visitor(serializer(
                        std::numeric_limits<std::size_t>::max()), i->second);
            token += ',';
        }
        token += '}';
        return token;
    }

    std::string make_multiline_table(const table& v) const
    {
        std::string token;
        // 1. print non-table stuff first
        for(table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            if(i->second.is(value::table_tag) || is_array_of_table(i->second))
            {continue;}

            token += serialize_key(i->first);
            token += " = ";
            token += apply_visitor(serializer(width_), i->second);
            token += "\n";
        }

        // 2. array of tables
        for(table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            if(!is_array_of_table(i->second)){continue;}

            std::vector<toml::key> ks(keys_);
            ks.push_back(i->first);

            const std::string tmp = apply_visitor(serializer(width_, ks), i->second);
            token += tmp;
        }

        // 3. normal tables
        for(typename table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            if(!i->second.is(value::table_tag)){continue;}
            std::vector<toml::key> ks(this->keys_);
            ks.push_back(i->first);
            token += apply_visitor(serializer(width_, ks), i->second);
        }
        return token;
    }

  private:

    std::size_t width_;
    std::vector<toml::key> keys_;
};
} // detail

inline std::string serialize(const value& v, std::size_t w = 50)
{
    return v.apply_visitor(detail::serializer(w));
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
