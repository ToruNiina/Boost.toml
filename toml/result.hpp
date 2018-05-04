//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
#ifndef TOML_RESULT_HPP
#define TOML_RESULT_HPP
#include <toml/value.hpp>
#include <ostream>

namespace toml
{
namespace detail
{

struct success_t{};
struct failure_t{};

// contains value and the current iterator
// case...
// 1. success & token was consumed     : normal result
// 2. success & token was not consumed : not used here
// 3. failure & token was not consumed : normal failure, try next candidate
// 4. failure & token was consumed     : partially correct, maybe syntax error
template<typename Value, typename Iterator>
struct result
{
    // to contain toml::string and std::string simultaneously,
    // add extra type information to each type
    typedef std::pair<Value,       success_t> success_type;
    typedef std::pair<std::string, failure_t> failure_type;
    typedef boost::variant<success_type, failure_type> storage_type;

    result(): storage_(failure_type("uninitialized", failure_t())){}
    ~result(){}

    result(const result& rhs): iter(rhs.iter), storage_(rhs.storage_){}
    result& operator=(const result& rhs)
    {iter = rhs.iter; storage_ = rhs.storage_; return *this;}

    result(const Value& v, Iterator i, success_t s)
        : iter(i), storage_(success_type(v, s))
    {}
    result(const std::string& s, Iterator i, failure_t f)
        : iter(i), storage_(failure_type(s, f))
    {}

    template<typename U, typename J>
    result(const result<U, J>& rhs): iter(rhs.iterator())
    {
        if(rhs.is_ok())
        {
            this->storage_ = success_type(rhs.unwrap(), success_t());
        }
        else
        {
            this->storage_ = failure_type(rhs.unwrap_err(), failure_t());
        }
    }

#ifdef BOOST_HAS_RVALUE_REFS
    result(result&& rhs)
        : iter(std::move(rhs.iter)), storage_(std::move(rhs.storage_)){}
    result& operator=(result&& rhs)
    {iter = rhs.iter; storage_ = rhs.storage_; return *this;}

    result(Value&& v, Iterator i, success_t s)
        : iter(i), storage_(success_type(std::move(v), s))
    {}
    result(std::string&& s, Iterator i, failure_t f)
        : iter(i), storage_(failure_type(std::move(s), f))
    {}
#endif // rvalue_refs

    bool is_ok()  const BOOST_NOEXCEPT_OR_NOTHROW {return storage_.which()==0;}
    bool is_err() const BOOST_NOEXCEPT_OR_NOTHROW {return storage_.which()==1;}

    operator bool() const BOOST_NOEXCEPT_OR_NOTHROW {return this->is_ok();}

    Value& ok_or(Value& v) BOOST_NOEXCEPT_OR_NOTHROW
    {if(this->is_ok()){return this->unwrap();} else {return v;}}
    Value const& ok_or(const Value& v) const BOOST_NOEXCEPT_OR_NOTHROW
    {if(this->is_ok()){return this->unwrap();} else {return v;}}

    std::string& err_or(std::string& s) BOOST_NOEXCEPT_OR_NOTHROW
    {if(this->is_err()){return this->unwrap_err();} else {return s;}}
    std::string const&
    err_or(const std::string& s) const BOOST_NOEXCEPT_OR_NOTHROW
    {if(this->is_err()){return this->unwrap_err();} else {return s;}}

    Value& unwrap()
    {
        if(this->is_err()){throw std::runtime_error(this->unwrap_err());}
        return boost::get<success_type>(this->storage_).first;
    }
    Value const& unwrap() const
    {
        if(this->is_err()){throw std::runtime_error(this->unwrap_err());}
        return boost::get<success_type>(this->storage_).first;
    }

    std::string& unwrap_err()
    {return boost::get<failure_type>(this->storage_).first;}
    std::string const& unwrap_err() const
    {return boost::get<failure_type>(this->storage_).first;}

    Iterator&       iterator()       BOOST_NOEXCEPT_OR_NOTHROW {return iter;}
    Iterator const& iterator() const BOOST_NOEXCEPT_OR_NOTHROW {return iter;}

  private:
    Iterator     iter;
    storage_type storage_;
};

template<typename charT, typename traits, typename Value, typename Iterator>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os,
           const result<Value, Iterator>& res)
{
    if(res.is_ok()){os << "success: " << res.unwrap();}
    else           {os << "failed: "  << res.unwrap_err();}
    return os;
}

} // detail
} // toml
#endif// TOML_RESULT_HPP
