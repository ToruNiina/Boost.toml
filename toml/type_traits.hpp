#ifndef TOML98_TYPE_TRAITS_H
#define TOML98_TYPE_TRAITS_H
#include <toml/types.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/not.hpp>
#include <boost/cstdint.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_traits.hpp>

namespace toml
{

template<typename T> struct is_toml_type : boost::false_type {};
template<typename T> struct is_toml_type<T&>          : is_toml_type<T>{};
template<typename T> struct is_toml_type<T const>     : is_toml_type<T>{};
template<typename T> struct is_toml_type<T volatile>  : is_toml_type<T>{};
template<typename T> struct is_toml_type<T const&>    : is_toml_type<T>{};
template<typename T> struct is_toml_type<T volatile&> : is_toml_type<T>{};

#ifdef BOOST_HAS_RVALUE_REFS
template<typename T> struct is_toml_type<T &&>         : is_toml_type<T>{};
template<typename T> struct is_toml_type<T const&&>    : is_toml_type<T>{};
template<typename T> struct is_toml_type<T volatile&&> : is_toml_type<T>{};
#endif // rvalue_refs

template<> struct is_toml_type<boolean>  : boost::true_type {};
template<> struct is_toml_type<integer>  : boost::true_type {};
template<> struct is_toml_type<floating> : boost::true_type {};
template<> struct is_toml_type<string>   : boost::true_type {};
template<> struct is_toml_type<date>     : boost::true_type {};
template<> struct is_toml_type<time>     : boost::true_type {};
template<> struct is_toml_type<datetime> : boost::true_type {};
template<> struct is_toml_type<array>    : boost::true_type {};
template<> struct is_toml_type<table>    : boost::true_type {};

// convertible by static_cast<value_t>(v).
template<typename T, typename U> struct is_convertible : boost::false_type{};

template<typename T> struct is_convertible<T, boolean>
    : boost::is_same<typename boost::remove_cv_ref<T>::type, bool>{};

template<typename T> struct is_convertible<T, integer>
    : boost::mpl::and_<
        boost::is_integral<typename boost::remove_cv_ref<T>::type>,
        boost::mpl::not_<
            boost::is_same<typename boost::remove_cv_ref<T>::type, bool>
        >
    > {};
template<typename T> struct is_convertible<T, floating>
    : boost::is_floating_point<typename boost::remove_cv_ref<T>::type> {};
template<typename T> struct is_convertible<T, string>
    : boost::is_convertible<typename boost::remove_cv_ref<T>::type, string>{};
template<typename T> struct is_convertible<T, date>
    : boost::is_convertible<typename boost::remove_cv_ref<T>::type, date> {};
template<typename T> struct is_convertible<T, time>
    : boost::is_convertible<typename boost::remove_cv_ref<T>::type, time> {};
template<typename T> struct is_convertible<T, datetime>
    : boost::is_same<typename boost::remove_cv_ref<T>::type, datetime>{};
template<typename T> struct is_convertible<T, array>
    : boost::is_same<typename boost::remove_cv_ref<T>::type, array>{};
template<typename T> struct is_convertible<T, table>
    : boost::is_same<typename boost::remove_cv_ref<T>::type, table> {};

template<typename T> struct is_convertible<T, value>
    : boost::mpl::or_<
        boost::is_same<typename boost::remove_cv_ref<T>::type, value>,
        is_convertible<typename boost::remove_cv_ref<T>::type, boolean>,
        is_convertible<typename boost::remove_cv_ref<T>::type, integer>,
        is_convertible<typename boost::remove_cv_ref<T>::type, floating>,
        is_convertible<typename boost::remove_cv_ref<T>::type, string>,
        is_convertible<typename boost::remove_cv_ref<T>::type, date>,
        is_convertible<typename boost::remove_cv_ref<T>::type, time>,
        is_convertible<typename boost::remove_cv_ref<T>::type, datetime>,
        is_convertible<typename boost::remove_cv_ref<T>::type, array>,
        is_convertible<typename boost::remove_cv_ref<T>::type, table>
    >{};


} // toml
#endif// TOML98_TYPE_TRAITS_H
