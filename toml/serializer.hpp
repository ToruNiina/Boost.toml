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
    serializer(const std::size_t w, const std::vector<toml::key>& ks,
               const bool is_aot)
        : width_(w), is_array_of_table_(is_aot), keys_(ks)
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
        {
            const std::string inl = this->make_inline_array(v);
            if(inl.size() < this->width_ &&
               std::find(inl.begin(), inl.end(), '\n') == inl.end())
            {return inl;}
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
        if(!is_array_of_table_){
            const std::string inl = this->make_inline_table(v);
            if(inl.size() < this->width_ &&
               std::find(inl.begin(), inl.end(), '\n') == inl.end())
            {
                std::string token;
                if(!keys_.empty() && !is_array_of_table_)
                {
                    token += serialize_dotted_key(keys_);
                    token += " = ";
                }
                token += inl;
                if(!is_array_of_table_)
                {
                    token += '\n';
                }
                return token;
            }
        }

        std::string token;
        if(!keys_.empty())
        {
            if(is_array_of_table_) {token += '[';}
            token += '[';
            token += serialize_dotted_key(keys_);
            if(is_array_of_table_) {token += ']';}
            token += "]\n";
        }

        // print non-table stuff first
        for(table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            if(i->second.is(value::table_tag)){continue;}
            if(i->second.is(value::array_tag) &&
               i->second.get<array>().front().is(value::table_tag)) {continue;}

            token += serialize_key(i->first);
            token += " = ";
            token += apply_visitor(*this, i->second);
            token += "\n";
        }

        for(table::const_iterator i(v.begin()), e(v.end()); i!=e; ++i)
        {
            if(i->second.is(value::table_tag))
            {
                std::vector<toml::key> ks(keys_);
                ks.push_back(i->first);
                token += apply_visitor(serializer(width_, ks, false), i->second);
            }
            if(i->second.is(value::array_tag) &&
               i->second.get<array>().front().is(value::table_tag))
            {
                std::vector<toml::key> ks(keys_);
                ks.push_back(i->first);

                const array& a = i->second.get<array>();
                for(array::const_iterator
                        ai(a.begin()), ae(a.end()); ai!=ae; ++ai)
                {
                    token += apply_visitor(serializer(width_, ks, true), *ai);
                }
            }
        }
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
        detail::lex_unquoted_key::invoke(i, key.end());
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
            token += apply_visitor(*this, *i);
            token += ", ";
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
            token += " = ";
            token += apply_visitor(*this, i->second);
            token += ", ";
        }
        token += '}';
        return token;
    }

  private:

    std::size_t width_;   // TODO use it to serialize
    // to serialize table...
    bool      is_array_of_table_;
    std::vector<toml::key> keys_; // to format nested table
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
