Boost.toml
====

[![Build Status](https://travis-ci.com/ToruNiina/Boost.toml.svg?token=3rmRuAkSVRafwq2Rbd9b&branch=master)](https://travis-ci.com/ToruNiina/Boost.toml)

Boost.toml is a header-only toml library depending on Boost.

compatible with [TOML v0.5.0](https://github.com/toml-lang/toml/blob/master/versions/en/toml-v0.5.0.md).

tested with `C++(98|11|14|17)` on Linux/macOS. Some functionalities
(e.g. construction from `std::initilalizer_list` (after c++11), getting toml
String as a `std::string_view` (after c++17)) would be disabled if older standard
version is given.

Boost.toml depends on relatively later versions of the Boost C++ Library
(tested with Boost 1.67.0 on Travis CI).

__NOTE__: This library is not a part of Boost C++ Library.

## Table of Contents

- [example code](#example-code)
- [parsing toml file](#parsing-toml-file)
- [getting toml values](#getting-toml-values)
    - [basic usage of `toml::get`](#basic-usage-of-tomlget)
    - [getting `toml::array`](#getting-tomlarray)
    - [`toml::array` of `toml::array`s having different types each other](#tomlarray-of-tomlarrays-having-different-types-each-other)
    - [getting `toml::table`](#getting-tomltable-and-array-of-tables)
    - [performance of getting `toml::array` or `toml::table`](#performance-of-getting-tomlarray-or-tomltable)
- [handling dotted keys](#handling-dotted-keys)
- [confirming value type](#confirming-value-type)
- [visiting value that has unknown type](#visiting-value-that-has-unknown-type)
- [formatting toml values](#formatting-toml-values)
    - [constructing toml values](#constructing-toml-values)
    - [printing toml values](#printing-toml-values)
    - [formatting toml data](#formatting-toml-data)
- [datetime operation](#datetime-operation)
- [underlying types](#underlying-types)
    - [`toml::string` and `basic`, `literal` flags](#tomlstring-and-basic-literal-flags)
    - [map class that represents `toml::table`](#map-class-that-represents-tomltable)
    - [why not STL container?](#why-not-stl-container)
    - [types that are convertible from toml value by using `toml::get`](#types-that-are-convertible-from-toml-value-by-using-tomlget)
- [supplemental notes](#supplemental-notes)
- [synopsis](#synopsis)
- [Licensing terms](#licensing-terms)

## example code

Here is an example toml file (a bit modified from original file found in toml repository).

```toml
title = "TOML Example"

[owner]
name = "Tom Preston-Werner"
dob = 1979-05-27T07:32:00-08:00

[database]
server = "192.168.1.1"
ports = [ 8001, 8001, 8002 ]
connection_max = 5000
enabled = true

# modified to explain how to get an array of table
[[servers]]
name = "alpha"
ip = "10.0.0.1"
dc = "eqdc10"

[[servers]]
name = "beta"
ip = "10.0.0.2"
dc = "eqdc10"

[clients]
data = [ ["gamma", "delta"], [1, 2] ]
```

You can read this file with a code like the following.

```cpp
#include <toml/toml.hpp>

int main()
{
    // reads toml file and return it as a map (toml::key -> toml::value).
    const toml::table file = toml::parse("example.toml");

    // you can get toml values by toml::get function.
    const std::string title = toml::get<std::string>(file.at("title"));

    // toml::get returns lvalue reference.
    const toml::table&  owner = toml::get<toml::table>(file.at("owner"));
    const std::string&  name  = toml::get<std::string>(owner.at("name"));
    const auto&         dob   = toml::get<toml::offset_datetime>(owner.at("dob"));

    const auto& database = toml::get<toml::table>(file.at("database"));
    // you can use a std::string_view if you have a c++17 compatible compiler.
    const auto  server = toml::get<std::string_view>(database.at("server"));
    // you can get a toml::array as your favorite container.
    const auto  ports = toml::get<std::vector<int>>(database.at("ports"));
    // you can cast types if they are convertible.
    const auto  connection_max = toml::get<std::size_t>(database.at("connection_max"));
    const auto  enabled = toml::get<bool>(database.at("enabled"));

    // an array of table is simply an `array<table>`.
    const auto servers = toml::get<std::vector<toml::table>>(file.at("servers"));

    // boost::string_(view|ref) are also supported.
    const auto name_alpha = toml::get<boost::string_view>(servers.at(0).at("name"));
    const auto ip_alpha   = toml::get<boost::string_view>(servers.at(0).at("ip"));
    const auto dc_alpha   = toml::get<boost::string_view>(servers.at(0).at("dc"));
    const auto name_beta  = toml::get<boost::string_ref >(servers.at(1).at("name"));
    const auto ip_beta    = toml::get<boost::string_ref >(servers.at(1).at("ip"));
    const auto dc_beta    = toml::get<boost::string_ref >(servers.at(1).at("dc"));

    const auto& clients = toml::get<toml::table>(file.at("clients"));

    // you can do this!
    // the first array is array of string, the second one is array of int.
    // data = [ ["gamma", "delta"], [1, 2] ]
    const auto  data = toml::get<
        std::pair<std::vector<std::string>, std::vector<int>>
        >(clients.at("data"));
    // it supports std::tuple also (after c++11).

    return 0;
}
```

## parsing toml file

Since this library is header-only, including `toml/toml.hpp` is the only thing
required.

`toml::parse` function parses toml file. You can pass `std::string` that
represents file name or `std::istream&` to the function. `std::runtime_error`
would be thrown if a file open error happens.

Both returns `toml::table` that includes all the data in a file.
If an error appeared while parsing a file, it throws `std::runtime_error`.

```cpp
const toml::table data = parse("example.toml");
// or
std::ifstream ifs("example.toml");
if(!ifs.good){return 1;}
const toml::table data = parse(ifs);
```

## getting toml values

Boost.toml provides a powerful function, `toml::get`, to get a value from TOML data.

### basic usage of `toml::get`

You can extract value from `toml::table` by specifying a type of a value through
`toml::get<T>`.

```cpp
// you can get a reference when you specify a toml type
toml::value v1(42);
toml::integer& i_ref = toml::get<toml::integer>(v1);
i_ref = 6 * 9; // v1 will be 54.

// you can get a value as non-toml type(e.g. uint32_t) if they are convertible.
// in that case, a reference that points to the internal value cannot be gotten.
std::uint32_t i = toml::get<std::uint32_t>(v1); // 54

// to avoid deep-copy, it is useful to get const reference.
toml::value v2{{"int", 42}, {"float", 3.14}, {"str", "foo"}};
const toml::table& tab = toml::get<toml::table>(v2);
```

If you pass a convertible type (like `int` for `toml::integer`) to `toml::get`'s
template argument, it converts a value to the specified type.
In that case, you can't get a lvalue reference that points to the contained
value because it returns `prvalue`.
You can find underlying types in [this section](#underlying-types).
See [this section](#types-that-are-convertible-from-toml-value-by-using-tomlget)
for more information about type conversions.

If you pass a wrong type (like `std::string` for `toml::integer`) to
`toml::get`, it will throw `toml::bad_get`. `toml::bad_get::what` might
be helpful as an error message because it contains demangled typename
powered by `boost::typeindex::type_id().pretty_name()`.

```cpp
toml::value v(42);
std::string s = toml::get<std::string>(v);

// exception thrown. the message would be something like this.
// terminate called after throwing an instance of 'toml::bad_get'
//   what():  toml::get: toml value has type `toml::integer`, but type `std::basic_string<char, std::char_traits<char>, std::allocator<char> >` is specified.
```

### getting `toml::array`

You can get a `toml::array` as your favorite array type.
The elements will also be converted to the specified type.

```cpp
toml::value v{1, 2, 3, 4, 5};
std::vector<int>        vec = toml::get<std::vector<int>       >(v);
std::deque<unsigned>    deq = toml::get<std::deque<unsigned>   >(v);
std::list<std::int64_t> lst = toml::get<std::list<std::int64_t>>(v);
std::array<char, 5>     ary = toml::get<std::array<char, 5>    >(v);
```

Surprisingly, you can also get a `std::pair` or `std::tuple` from
`toml::array`.

```cpp
toml::value v{1, 2};
std::pair<int, unsigned>  p = toml::get<std::pair<int, unsigned>>(v);

toml::value v{1, 2, 3};
std::tuple<int, int, int> t = toml::get<std::tuple<int, int, int>>(v);

// unfortunately, boost::tuple is currently not supported.
```

This feature is convenient in the following case.

### `toml::array` of `toml::array`s having different types each other

Consider that you have this toml file.

```toml
array = [[1, 2, 3], ["foo", "bar", "baz"]]
```

What is the corresponding C++ type? Of course, its a `std::pair` of
`std::vector<int>` and `std::vector<std::string>`.

Boost.toml supports this.

```cpp
auto v = toml::get<std::pair<std::vector<int>, std::vector<std::string>>>(v);
```

Or it can be a tuple of two elements.

```cpp
auto v = toml::get<std::tuple<std::vector<int>, std::vector<std::string>>>(v);
```

But generally, you might not be able to know the length or type of an array
in a file before reading it. In that case, `toml::array` or `std::vector<toml::value>`
can be used (actually, `toml::array` is just an alias of
`boost::container::vector<toml::value>`, so in this case conversion may not
be needed).

```cpp
const toml::array& a = toml::get<toml::array>(v);

std::vector<int>    a1 = toml::get<std::vector<int>        >(a.at(0));
std::vector<string> a2 = toml::get<std::vector<std::string>>(a.at(1));
```

Or you can get this as `std::vector<toml::array>` because it is an array of
arrays.

```cpp
std::vector<toml::array> a = toml::get<std::vector<toml::array>>(v);

int         a1 = toml::get<int        >(a.at(0).at(0)); // 1
std::string a2 = toml::get<std::string>(a.at(1).at(0)); // "foo"
```

### getting `toml::table` and Array of Tables

Array of Tables can be obtained in the same way described before.

```cpp
// it is just an array of tables.
auto tables = toml::get<std::vector<toml::table>>(v);
```

And you can get also `toml::table` as your favorite map type.

```cpp
toml::value v{{"int", 42}, {"float", 3.14}, {"str", "foo"}};

auto std_map  = toml::get<std::map<toml::key, toml::value>             >(v);
auto std_umap = toml::get<std::unordered_map<toml::key, toml::value>   >(v);
auto bst_map  = toml::get<boost::container::map<toml::key, toml::value>>(v);
auto bst_umap = toml::get<boost::unordered_map<toml::key, toml::value> >(v);
```

__NOTE__: `toml::table` is an alias of `boost::container::flat_map`.
So it has all the functionalities that `boost::container::flat_map` has.
In most cases, conversion is not needed.


### performance of getting `toml::array` or `toml::table`

Because `toml::get` converts all the elements in an array,
it essentially does the same thing as following.

```cpp
// conversion from toml::array to std::vector<int>.
// essentially same as toml::get<std::vector<int>>(val).
std::vector<int> get_int_vec(const toml::value& v)
{
    const toml::array& ar = v.get<toml::array>();
    std::vector<int> retval(ar.size());
    std::transform(ar.begin(), ar.end(), retval.begin(),
        [](const toml::value& x){return toml::get<int>(x);});
    return retval;
}
```

getting `toml::table` as a different `map` type also copies elements deeply.

```cpp
std::map<toml::key, toml::value> get_std_map(const toml::value& v)
{
    const toml::table& tb = v.get<toml::table>();
    // it does not need extra toml::get, so it's simpler than array.
    return std::map<toml::key, toml::value>(tb.begin(), tb.end());
}
```

If the array or table has many many elements, it will take
a long time because it constructs completely new array or table and
deeply copy all the elements.

In some cases, it is intolerable.

To avoid deep copy, you can get them as a (const) reference.
All the elements are kept intact, so you need explicit conversion for
each element, but it copies nothing.

```cpp
toml::value  v{1,2,3,4,5};
toml::array& ar = toml::get<toml::array>(v);
std::int64_t i  = toml::get<std::int64_t>(ar.at(0));
// one more toml::get is needed because toml::array is an array of toml::values.
```

## handling dotted keys

Boost.toml regards dotted keys as a table.

```toml
# dotted keys
physical.color = "red"
physical.shape = "sphere"
# the above key-value pairs are equivalent to
physical = {color = "red", shape = "sphere"}
# also equivalent to
[physical]
color = "red"
shape = "sphere"
```

You can get these values as following.

```cpp
const auto& physical = toml::get<toml::table>(data.at("physical"));
const auto& color    = toml::get<std::string>(physical.at("color"));
const auto& shape    = toml::get<std::string>(physical.at("shape"));
```

## confirming value type

If you pass wrong template argument to `toml::get`, `toml::bad_get` (that is
derived from `std::exception`) will be thrown. Before using `toml::get`, it
should be known what type does the value actually contain.

```cpp
toml::value v(3.14);
std::string s = toml::get<std::string>(v);

// exception thrown. the message would be something like this.
// terminate called after throwing an instance of 'toml::bad_get'
//   what():  toml::get: toml value has type `toml::float`, but type `std::basic_string<char, std::char_traits<char>, std::allocator<char> >` is specified.
```

`toml::value` has `toml::value::is()` member function that can be used
to check a type information.

```cpp
toml::table data = toml::parse("unknown.toml");
const toml::value& v = data["some_value"]; // what type does it have ???

// receives enum
if(v.is(toml::value::string_tag)) {/* do some stuff */}
// receives type
if(v.is<toml::integer>()) {/* do some stuff */}
```

`toml::value` has an `enum` value that represents type information.

```cpp
std::cout << v.which() << std::endl; // outputs "integer" or something like that

switch(v.which())
{
    case toml::value::integer_tag  : /* do some stuff */; break;
    case toml::value::string_tag   : /* do some stuff */; break;
    // ...
}
```

the complete set of this `enum` values is found in [synopsis](#synopsis).

But it is painful to write `switch-case` every time.
Boost.toml provides a way to visit contained value without knowing its value.

See also [visiting value that has unknown type](#visiting-value-that-has-unknown-type).

## visiting value that has unknown type

Boost.toml provides `toml::apply_visitor` function to visit contained value
without knowing its type.

```cpp
toml::table data = toml::parse("sample.toml");
toml::apply_visitor([](const auto& val) {std::cout << val << std::endl;},
                    data["number"]);
toml::visit([](const auto& val) {std::cerr << val << std::endl;},
            data["number"]);
```

The usage is similar to `boost::apply_visitor` and `boost::variant`, bacause
it uses them internally.

## formatting toml values

Boost.toml also supports formatting toml values.

### constructing toml values

`toml::value` is constructible from toml-convertible types like `std::int64_t`,
`double`, `float`, `std::string`, `const char*`, and many others.

```cpp
toml::value v1(42u);
toml::value v2(3.1416);
toml::value v3("string!");

toml::date  d(2018, 4, 1);
toml::time  t(toml::hours(1), toml::minutes(30));
toml::value v3(d, t); // datetime!
```

Iterators are also supported.

```cpp
std::vector<toml::integer> is{1,2,3,4,5};
toml::value v1(is.begin(), is.end()); // toml::array of integers

std::map<std::string, double> ds{{"pi", 3.1416}, {"e", 2.7183}};
toml::value v2(ds.begin(), ds.end()); // become a toml::table!
```

And you can construct `toml::value` from `std::initializer_list` when you use
C++11 compatible compiler.

```cpp
toml::value v4{1,2,3,4,5}; // become an array
toml::value v5{{"i", 42}, {"pi", 3.14}, {"key", "value"}}; // become a table
```

### printing toml values

Boost.toml has `toml::format` function that converts a `toml::value` to
`std::string`. Since it returns a string, you can output it.

```cpp
std::string i = toml::format(toml::value(42));
// 42
std::string s = toml::format(toml::value("string with\nnewline"));
// """
// string with
// newline
// """

std::cout << toml::format(toml::value(toml::date(1979, 5, 27), toml::hours(7) + toml::minutes(32)));
// 1979-05-27T07:32:00
std::cout << toml::format(toml::value{1, 2, 3});
// [1,2,3]
```

`toml::format` sometimes adds newlines to avoid too long line.

```cpp
toml::value str("too long string would be splitted to multiple lines, "
                "and the threshould can be passed to toml::format function. "
                "By default, the threshold is 80.");
std::cout << toml::format(str);
// """
// too long string would be splitted to multiple lines, and the threshould can be \
// passed to toml::format function. By default, the threshold is 80.\
// """
toml::value ary{
    "If an array has many elements so the result of toml::format become longer",
    "than a threshold toml::format will print them in multi-line array."
    };
std::cout << toml::format(ary);
// [
//  "If an array has many elements so the result of toml::format become longer",
//  "than a threshold toml::format will print them in multi-line array.",
// ]
```

`toml::format` automatically makes an element of array of tables inline when
the length of lines will be less than a threshold.

```cpp
toml::value aot{
    toml::table{{"key1", 1}, {"key2", 3.14}},
    toml::table{{"key1", 2}, {"key2", 6.28}},
    toml::table{{"key1", 3}, {"key2", 9.42}}
};
std::cout << foml::format(aot);
// [
//  {key1 = "value1-1", key2 = "value2-1"},
//  {key1 = "value1-2", key2 = "value2-2"},
//  {key1 = "value1-3", key2 = "value2-3"},
// ]
```

You can change this threshold by passing a value explicitly to `toml::format`.

```cpp
std::cout << toml::format(val, 100);
```

By default, the threshold is set as 80.

It also supports `iostream`. When you output it into `std::ostream`,
`toml::format` will be called automatically.

```cpp
toml::value val;
std::cout << val;
```

You can set the threshold by putting `std::setw` before outputting your toml value.

```cpp
toml::value val;
std::cout << val; // calls toml::format(val, 80)
std::cout << std::setw(100) << val; // calls toml::format(val, 100)
```

### formatting toml data

If you pass a `toml::table` to `toml::format`, it prints a TOML file considering
the passed `toml::table` as a root object.

```cpp
toml::table tab;
tab["a"]   = toml::integer(42);
tab["pi"]  = toml::floating(3.14);
tab["key"] = toml::string("value");
tab["tab"] = toml::table{{"b", 54}, {"e", 2.718}};

std::cout << tab << std::endl;
// a = 42
// pi = 3.140000
// key = "value"
// [tab]
// b = 54
// e = 2.718000
```

You can find the simplest example in `sample/` directory that reads toml file
and outputs its content into `stdout`. You can see what happens if Boost.toml
formats your toml file by passing a file to the sample program.

## datetime operation

Depending on Boost.Date\_Time, Boost.toml supports datetime operation.
Since `toml::local_datetime` is actually just an alias of
`boost::posix_time::ptime`,
you can operate `datetime` in the same way as Boost.Date\_Time.

```cpp
boost::posix_time::ptime dt = toml::get<boost::posix_time::ptime>(v);
dt += boost::date_time::days(7);
dt += boost::date_time::hours(12);
```

And just for ease, Boost.toml imports some classes and enums.
You can operate your datetime value in the same way as
Boost.Date\_Time without writing namespace `boost::date_time::` explicitly.

```cpp
toml::value d(toml::date(2018, toml::Apr, 1));
toml::value t(toml::hours(1) + toml::minutes(3) + toml::seconds(10));
toml::value dt(d, t);
toml::get<toml::local_datetime>(dt) += toml::years(1);
toml::get<toml::local_datetime>(dt) += toml::months(2);
toml::get<toml::local_datetime>(dt) += toml::days(10);
```

See also [underlying types](#underlying-types).

__NOTE__: It uses only a header-only portion of the Boost.Date\_Time library.
So Boost.toml can be used without linking. But in order to use whole the
functionalities of Boost.Date\_Time library, you might need linking.

## underlying types

`toml::value` is based on a `boost::variant` that contains following toml value
types.

| toml value type   | boost.toml type                                                |
|:------------------|:---------------------------------------------------------------|
| -                 | `boost::blank` (uninitialized type)                            |
| `boolean`         | `bool`                                                         |
| `integer`         | `boost::int64_t`                                               |
| `floating`        | `double`                                                       |
| `string`          | `struct {std::string str; enum kind_t {basic, literal} kind;}` |
| `date`            | `boost::gregorian::date`                                       |
| `time`            | `boost::posix_time::time_duration`                             |
| `local_datetime`  | `boost::posix_time::ptime`                                     |
| `offset_datetime` | `boost::local_time::local_date_time`                           |
| `array`           | `boost::container::vector<value>`                              |
| `table`           | `boost::container::flat_map<key, value>`                       |

### `toml::string` and `basic`, `literal` flags

`toml::string` has an `enum kind_t` to represent `basic_string` and `literal_string`.
But it can be converted to std::string automatically, so users do not need
to consider about a difference between `toml::string` and `std::string`.

an enum value can be passed to make `toml::value` that contains `toml::string`.
It affects on the output format. By default, it is set as `basic_string`.

```cpp
toml::value v0("foo"); // It will be a basic string by default.
toml::value v1("foo", toml::string::basic);
toml::value v2("bar", toml::string::literal); // become a literal-string.
```

### map class that represents `toml::table`

Here, `flat_map` is used to store toml.table because normally the number of
elements in data from file will never be changed. You can change it to a
`boost::container::map` by defining `BOOST_TOML_USE_MAP_FOR_TABLE` before
including it or as a compiler flag.

### Why not STL container?

Because `Boost.Container` allows to contain incomplete types. So it allows to
contain recursive data type.
This feature removes the neccesity of using pointers to implement `toml::value`
that is defined recursively. It will make the whole process faster.

NOTE:

`toml::value` is a recursively defined data type. `toml::array` is an
array of `toml::value`, and it is also one of the types that `toml::value`
contains.

So the code looks like this.

```cpp
struct value
{
    boost::variant<string, integer, // ...
            boost::container::vector<value> // value contains itself inside!
        > storage;
};
```

In C++17, "Minimal incomplete type support for standard containers" is
incorporated into the standard specification.
But to realize this implementation before C++17, it may be the best way
to use Boost.Container.

### types that are convertible from toml value by using `toml::get`

You can get a reference that points an internal value if you specify one of
the underlying types.
See also [underlying types](#underlying-types).

Here, types can be converted from toml types are listed.

| toml value type   | convertible types                                                             |
|:------------------|:------------------------------------------------------------------------------|
| `boolean`         | `bool` only                                                                   |
| `integer`         | types that makes `boost::is_integral<T>::value` true (excepting `bool`).      |
| `floating`        | types that makes `boost::is_floating_point<T>::value` true.                   |
| `string`          | `std::string&`, `std::string_view`, `boost::string_view`, `boost::string_ref` |
| `date`            | `std::tm`, `std::chrono::time_point`                                          |
| `time`            | `std::tm`, `std::chrono::duration`                                            |
| `local_datetime`  | `std::tm`, `std::chrono::time_point`                                          |
| `offset_datetime` | `std::tm`, `std::chrono::time_point`                                          |
| `array`           | container classes (see below).                                                |
| `table`           | map-like classes (see below).                                                 |

`toml::string` can safely be converted to an lvalue of `std::string`.

Because `std::time_t` is an integral type, it collides with `toml::integer`.
Thus getting `std::time_t` from datetime objects is not supported.

Boost.chrono is currently not supported.

`toml::array` can be converted to a class that ...
* has member types named `iterator` and `value_type`
* does not have a member type named `key_type` or `mapped_type`
* is not one of the `std::string`, `std::string_view`, `boost::string_view` or `boost::string_ref`

`toml::table` can be converted to a class that ...
* has member types named `iterator`, `value_type`, `key_type` and `mapped_type`.

## supplemental notes

Although Boost.toml depends Boost.Date\_Time that requires linking, it normally
does not require linking because most of the functionalities of Boost.Date\_Time
can be used without linking.

## Synopsis

### `toml::value`

```cpp
struct value
{
    typedef boost::variant<boost::blank, boolean, integer, floating, string,
        date, time, local_datetime, offset_datetime, array, table> storage_type;

    enum kind
    {
        empty_tag           = 0,
        boolean_tag         = 1,
        integer_tag         = 2,
        float_tag           = 3,
        string_tag          = 4,
        date_tag            = 5,
        time_tag            = 6,
        local_datetime_tag  = 7,
        offset_datetime_tag = 8,
        array_tag           = 9,
        table_tag           = 10,
        undefined_tag       = 11
    };

    value();
    ~value();
    value(const value& v);
    value(value&& v);
    value& operator=(const value& v);
    value& operator=(value&& v); // after c++11.

    // these are enabled for each TOML type by using SFINAE or just overloading.
    template<typename T> value(const T& v);
    template<typename T> value& operator=(const T& v);

    value(const char* v,        string::kind_t k);
    value(const std::string& v, string::kind_t k);
    value(const date&                d, const time& t);
    value(const local_datetime&     dt, const time_zone_ptr tzp);
    value(const date& d, const time& t, const time_zone_ptr tzp);

    // enabled after c++11.
    value(std::string&& v);
    value(toml::array&& v);
    value(toml::table&& v);
    value& operator=(std::string&& v);
    value& operator=(toml::array&& v);
    value& operator=(toml::table&& v);

    // enabled after c++11.
    template<typename T>
    value(std::initializer_list<T> v);
    value(std::initializer_list<std::pair<key, value>> v);

    // enabled for each toml type (string, array, or table).
    template<typename Iterator>
    value(Iterator first, Iterator last);

    int  index() const noexcept;
    kind which() const noexcept;

    template<typename T>
    bool is()       const noexcept;
    bool is(kind k) const noexcept;

    template<typename T> T&       get();
    template<typename T> T const& get();

    void swap(value& rhs);

    template<typename Visitor>
    /* result type of Visitor */ apply_visitor(Visitor v);
    template<typename Visitor>
    /* result type of Visitor */ apply_visitor(Visitor v) const;

    bool operator==(const value& r) const;
    bool operator!=(const value& r) const;
    bool operator< (const value& r) const;
    bool operator> (const value& r) const;
    bool operator<=(const value& r) const;
    bool operator>=(const value& r) const;
};

void swap(value& lhs, value& rhs);

template<typename Visitor>
/* result type of Visitor */ apply_visitor(Visitor, value&);
template<typename Visitor>
/* result type of Visitor */ apply_visitor(Visitor, value const&);

template<typename Visitor>
/* result type of Visitor */ visit(Visitor, value&);
template<typename Visitor>
/* result type of Visitor */ visit(Visitor, value const&);

// output value::kind.
template<typename charT, typename traits>
std::basic_ostream<charT, traits>&
operator<<(std::basic_ostream<charT, traits>& os, value::kind k);
```

## Licensing Terms

This library is distributed under [the Boost Software License](LICENSE).

- copyright (c) 2018, Toru Niina

All rights reserved.
