#ifndef TOML_GET_HPP
#define TOML_GET_HPP
#include <toml/type_traits.hpp>
#include <toml/value.hpp>
#include <boost/format.hpp>
namespace toml
{

// toml::get<exact-toml-types> is defined in toml/value.hpp.
// in this file, get<convertible-types> are defined.

// toml::string -> string, returning lvalue.
template<typename T>
typename boost::enable_if<boost::is_same<T, std::string>, T>::type&
get(value& v)
{
    return v.get<string>().str;
}
template<typename T>
typename boost::enable_if<boost::is_same<T, std::string>, T>::type const&
get(value const& v)
{
    return v.get<string>().str;
}

// ---------------------------------------------------------------------------
// conversions. return a prvalue

// integral types but not exactly toml::integer...
template<typename T>
typename boost::enable_if<boost::mpl::and_<
        boost::mpl::not_<is_toml_type<T> >,
        boost::is_integral<T>
    >, T>::type
get(const toml::value& v)
{
    return static_cast<T>(v.get<integer>());
}

// floating types but not exactly toml::floating...
template<typename T>
typename boost::enable_if<boost::mpl::and_<
        boost::mpl::not_<is_toml_type<T> >,
        boost::is_floating_point<T>
    >, T>::type
get(const toml::value& v)
{
    return static_cast<T>(v.get<floating>());
}

// string_view
template<typename T>
typename boost::enable_if<is_string_view_like<T>, T>::type
get(const toml::value& v)
{
    return T(v.get<string>().str);
}

// array_like
template<typename Array>
typename boost::enable_if<is_array_like<Array>, Array>::type
get(const toml::value& v)
{
    typedef typename Array::value_type value_type;
    toml::array const& ar = v.get<toml::array>();
    Array retval(ar.size());
    typename Array::iterator out(retval.begin());
    for(toml::array::const_iterator i(ar.begin()), e(ar.end()); i!=e; ++i)
    {
        *(out++) = get<value_type>(*i);
    }
    return retval;
}

// (boost|std)::array
template<typename Array>
typename boost::enable_if<is_fixed_size_array<Array>, Array>::type
get(const toml::value& v)
{
    typedef typename Array::value_type value_type;
    toml::array const& ar = v.get<toml::array>();
    Array retval;

    if(ar.size() > retval.size())
    {
        throw std::out_of_range((boost::format("toml::get<fixed-sized-array>: "
            "no enough size (%1% > %2%).") % ar.size() % retval.size()).str());
    }

    typename Array::iterator out(retval.begin());
    for(toml::array::const_iterator i(ar.begin()), e(ar.end()); i!=e; ++i)
    {
        *(out++) = get<value_type>(*i);
    }
    return retval;
}

// table-like case
template<typename Map>
typename boost::enable_if<is_map_like<Map>, Map>::type
get(const toml::value& v)
{
    toml::table const& tb = v.get<toml::table>();
    Map retval;
    for(toml::table::const_iterator i(tb.begin()), e(tb.end()); i!=e; ++i)
    {
        retval.insert(*i);
    }
    return retval;
}

} // toml
#endif// TOML98_GET_HPP
