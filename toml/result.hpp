//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_RESULT_HPP
#define TOML_RESULT_HPP
#include <boost/config.hpp>
#include <boost/variant.hpp>
#include <utility>
#include <ostream>

namespace toml
{
namespace detail
{

// to contain convertible types (like toml::string and std::string)
// at the same time, add extra type information to each type
template<typename T>
struct success
{
    typedef T value_type;

    explicit success(const value_type& val): value(val) {}
    ~success(){}

    template<typename U>
    explicit success(const U& val, typename boost::enable_if<
            boost::is_convertible<U, T> >::type* = 0)
        : value(value_type(val))
    {}

    success(const success& rhs): value(rhs.value){}
    success& operator=(const success& rhs) {value = rhs.value; return *this;}

#ifdef BOOST_HAS_RVALUE_REFS
    explicit success(success&& rhs): value(std::move(rhs.value)){}
    success& operator=(success&& rhs)
    {value = std::move(rhs.value); return *this;}
#endif

    value_type value;
};

template<typename T>
struct failure
{
    typedef T value_type;

    explicit failure(const value_type& val): value(val) {}
    ~failure(){}

    template<typename U>
    explicit failure(const U& val, typename boost::enable_if<
            boost::is_convertible<U, T> >::type* = 0)
        : value(value_type(val))
    {}

    failure(const failure& rhs): value(rhs.value){}
    failure& operator=(const failure& rhs) {value = rhs.value; return *this;}

#ifdef BOOST_HAS_RVALUE_REFS
    explicit failure(failure&& rhs): value(std::move(rhs.value)){}
    failure& operator=(failure&& rhs)
    {value = std::move(rhs.value); return *this;}
#endif

    value_type value;
};

template<typename T>
success<typename boost::remove_const<
    typename boost::remove_reference<T>::type>::type>
ok(const T& v)
{
    return success<typename boost::remove_const<
        typename boost::remove_reference<T>::type>::type>(v);
}
template<typename T>
failure<typename boost::remove_const<
    typename boost::remove_reference<T>::type>::type>
err(const T& v)
{
    return failure<typename boost::remove_const<
        typename boost::remove_reference<T>::type>::type>(v);
}

#ifdef BOOST_HAS_RVALUE_REFS
template<typename T>
success<T> ok(T&& v)  {return success<T>(std::forward<T>(v));}
template<typename T>
failure<T> err(T&& v) {return failure<T>(std::forward<T>(v));}
#endif

template<typename Ok, typename Err>
struct result
{
    typedef success<Ok>  success_type;
    typedef failure<Err> failure_type;
    typedef boost::variant<success_type, failure_type> storage_type;

    result(const success_type& s): storage_(s){}
    result(const failure_type& f): storage_(f){}
    ~result(){}

    result(const result& rhs): storage_(rhs.storage_){}
    result& operator=(const result& rhs){storage_ = rhs.storage_; return *this;}

    template<typename Ok_, typename Err_>
    result(const result<Ok_, Err_>& rhs)
    {
        if(rhs.is_ok()){this->storage_ = success_type(Ok(rhs.unwrap()));}
        else           {this->storage_ = failure_type(Err(rhs.unwrap_err()));}
    }

    template<typename Ok_>
    result(const success<Ok_>& s, typename boost::enable_if<
            boost::is_convertible<Ok_, Ok> >::type* = 0)
        : storage_(success_type(s.value))
    {}
    template<typename Err_>
    result(const failure<Err_>& f, typename boost::enable_if<
            boost::is_convertible<Err_, Err> >::type* = 0)
        : storage_(failure_type(f.value))
    {}

#ifdef BOOST_HAS_RVALUE_REFS
    result(success_type&& s): storage_(std::move(s)){}
    result(failure_type&& f): storage_(std::move(f)){}

    result(result&& rhs): storage_(std::move(rhs.storage_)){}
    result& operator=(result&& rhs)
    {storage_ = std::move(rhs.storage_); return *this;}
#endif // rvalue_refs

    bool is_ok()  const BOOST_NOEXCEPT_OR_NOTHROW {return storage_.which()==0;}
    bool is_err() const BOOST_NOEXCEPT_OR_NOTHROW {return storage_.which()==1;}

    operator bool() const BOOST_NOEXCEPT_OR_NOTHROW {return this->is_ok();}

    Ok& ok_or(Ok& v) BOOST_NOEXCEPT_OR_NOTHROW
    {
        if(this->is_ok()){return this->unwrap();} else {return v;}
    }
    Ok const& ok_or(const Ok& v) const BOOST_NOEXCEPT_OR_NOTHROW
    {
        if(this->is_ok()){return this->unwrap();} else {return v;}
    }

    Err& err_or(Err& s) BOOST_NOEXCEPT_OR_NOTHROW
    {
        if(this->is_err()){return this->unwrap_err();} else {return s;}
    }
    Err const& err_or(const Err& s) const BOOST_NOEXCEPT_OR_NOTHROW
    {
        if(this->is_err()){return this->unwrap_err();} else {return s;}
    }

    Ok& unwrap()
    {return boost::get<success_type>(this->storage_).value;}
    Ok const& unwrap() const
    {return boost::get<success_type>(this->storage_).value;}

    Err& unwrap_err()
    {return boost::get<failure_type>(this->storage_).value;}
    Err const& unwrap_err() const
    {return boost::get<failure_type>(this->storage_).value;}

  private:
    storage_type storage_;
};

template<typename charT, typename traits, typename Ok, typename Err>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, const result<Ok, Err>& res)
{
    if(res.is_ok()){os << "Ok("      << res.unwrap()     << ')';}
    else           {os << "Error( "  << res.unwrap_err() << ')';}
    return os;
}

} // detail
} // toml
#endif// TOML_RESULT_HPP
