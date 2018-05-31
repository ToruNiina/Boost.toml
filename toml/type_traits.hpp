//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_TYPE_TRAITS_HPP
#define TOML_TYPE_TRAITS_HPP
#include <toml/types.hpp>
#include <boost/config.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/not.hpp>
#include <boost/tti/has_type.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/cstdint.hpp>
#include <boost/array.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/utility/string_view.hpp>

#if __cplusplus >= 201703L
#  ifdef __has_include
#    if __has_include(<string_view>)
#      include <string_view>
#      define TOML_HAS_CXX17_STRING_VIEW
#    endif // string view
#  endif // has_include
#endif // c++17

// if defined, include will be skipped.
#if __cplusplus >= 201103L
#  ifndef BOOST_NO_CXX11_HDR_ARRAY
#    include <array>
#    define TOML_HAS_CXX11_ARRAY
#  endif // BOOST_NO_CXX11_HDR_ARRAY
#  ifndef BOOST_NO_CXX11_HDR_TUPLE
#    include <tuple>
#    define TOML_HAS_CXX11_TUPLE
#  endif // BOOST_NO_CXX11_HDR_TUPLE
#  ifndef BOOST_NO_CXX11_HDR_CHRONO
#    include <chrono>
#    define TOML_HAS_CXX11_CHRONO
#  endif // BOOST_NO_CXX11_HDR_CHRONO
#endif //c++11

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

template<> struct is_toml_type<boolean>         : boost::true_type {};
template<> struct is_toml_type<integer>         : boost::true_type {};
template<> struct is_toml_type<floating>        : boost::true_type {};
template<> struct is_toml_type<string>          : boost::true_type {};
template<> struct is_toml_type<date>            : boost::true_type {};
template<> struct is_toml_type<time>            : boost::true_type {};
template<> struct is_toml_type<local_datetime>  : boost::true_type {};
template<> struct is_toml_type<offset_datetime> : boost::true_type {};
template<> struct is_toml_type<array>           : boost::true_type {};
template<> struct is_toml_type<table>           : boost::true_type {};

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
template<typename T> struct is_convertible<T, local_datetime>
    : boost::is_same<typename boost::remove_cv_ref<T>::type, local_datetime>{};
template<typename T> struct is_convertible<T, offset_datetime>
    : boost::is_same<typename boost::remove_cv_ref<T>::type, offset_datetime>{};
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
        is_convertible<typename boost::remove_cv_ref<T>::type, local_datetime>,
        is_convertible<typename boost::remove_cv_ref<T>::type, offset_datetime>,
        is_convertible<typename boost::remove_cv_ref<T>::type, array>,
        is_convertible<typename boost::remove_cv_ref<T>::type, table>
    >{};

// is_fixed_size_array, is_stirng_view_like ------------------------------------

// (boost|std)::array have no ctor. toml::get should treat this in different way
template<typename T> struct is_fixed_size_array : boost::false_type{};
template<typename T, std::size_t N>
struct is_fixed_size_array<boost::array<T, N> > : boost::true_type{};
#ifdef TOML_HAS_CXX11_ARRAY
template<typename T, std::size_t N>
struct is_fixed_size_array<std::array<T, N> > : boost::true_type{};
#endif

template<typename T> struct is_string_view_like : boost::false_type{};
template<> struct is_string_view_like<boost::string_view> : boost::true_type{};
template<> struct is_string_view_like<boost::string_ref>  : boost::true_type{};
#ifdef TOML_HAS_CXX17_STRING_VIEW
template<> struct is_string_view_like<std::string_view>   : boost::true_type{};
#endif

// is_pair/tuple_type ----------------------------------------------------------

template<typename T> struct is_pair_type : boost::false_type {};
template<typename T1, typename T2> struct is_pair_type<std::pair<T1, T2> >
: boost::true_type {};

#if defined(TOML_HAS_CXX11_TUPLE)
template<typename T> struct is_tuple_type : boost::false_type {};
template<typename ... Ts>
struct is_tuple_type<std::tuple<Ts...> > : boost::true_type {};

// to use toml::get<std::tuple<T1, T2, ...>> in C++11
template<std::size_t ... Ns> struct index_sequence{};

template<typename IS, std::size_t N> struct push_back_index_sequence{};
template<std::size_t N, std::size_t ... Ns>
struct push_back_index_sequence<index_sequence<Ns...>, N>
{
    typedef index_sequence<Ns..., N> type;
};

template<std::size_t N>
struct index_sequence_maker
{
    typedef typename push_back_index_sequence<
        typename index_sequence_maker<N-1>::type, N>::type type;
};
template<>
struct index_sequence_maker<0>
{
    typedef index_sequence<0> type;
};
template<std::size_t N>
using make_index_sequence = typename index_sequence_maker<N-1>::type;
#endif

// is_array_like, is_map_like -----------------------------------------------

BOOST_TTI_HAS_TYPE(iterator)    // has_type_iterator
BOOST_TTI_HAS_TYPE(value_type)  // has_type_value_type
BOOST_TTI_HAS_TYPE(key_type)    // has_type_key_type
BOOST_TTI_HAS_TYPE(mapped_type) // has_type_mapped_type

template<typename T> struct is_array_like : boost::mpl::and_<
        boost::mpl::not_<boost::is_same<T, std::string> >,
        boost::mpl::not_<is_toml_type<T> >,        // is not an exact toml type
        boost::mpl::not_<is_string_view_like<T> >, // is not a string_view type
        boost::mpl::not_<is_fixed_size_array<T> >, // is not an array type
        has_type_iterator<T>,                      // has iterator
        has_type_value_type<T>,                    // has value_type
        boost::mpl::not_<has_type_key_type<T> >,   // does not have key_type
        boost::mpl::not_<has_type_mapped_type<T> > // does not have mapped_type
    >{};
template<typename T> struct is_map_like : boost::mpl::and_<
        boost::mpl::not_<is_toml_type<T> >,        // is not an exact toml type
        has_type_iterator<T>,                      // has iterator
        has_type_value_type<T>,                    // has value_type
        has_type_key_type<T>,                      // has key_type
        has_type_mapped_type<T>                    // has mapped_type
    >{};

// std::chrono -----------------------------------------------------------------

#ifdef TOML_HAS_CXX11_CHRONO
template<typename T>
struct is_std_chrono_time_point_type : boost::false_type {};
template<typename Clock, typename Duaration>
struct is_std_chrono_time_point_type<
    std::chrono::time_point<Clock, Duaration> > : boost::true_type {};

template<typename T>
struct is_std_chrono_duration_type : boost::false_type {};
template<typename Rep, typename Period>
struct is_std_chrono_duration_type<
    std::chrono::duration<Rep, Period> > : boost::true_type {};
#endif

} // toml
#endif// TOML_TYPE_TRAITS_HPP
