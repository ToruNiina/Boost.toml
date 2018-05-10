Boost.toml
====

Boost.toml is a header-only toml parser depending on Boost.

tested with `-std=c++(98|03|11|14|17)`. Some functionalities
(e.g. construction from `std::initilalizer_list`(after c++11), getting toml
String as a `std::string_view` (after c++17)) are disabled when older standard
version is given.

__NOTE__: This library is not a part of Boost C++ Library.

## Table of Contents

- [example code](#example-code)
- [getting toml values](#getting-toml-values)
    - [basic usage of `toml::get`](#basic-usage-of-tomlget)
    - [getting `toml::array`](#getting-tomlarray)
    - [getting `toml::table`](#getting-tomltable)
    - [`toml::array` of `toml::array` having different types each other](#tomlarray-of-tomlarray-having-different-types-each-other)
    - [performance of getting `toml::array` or `toml::table`](#performance-of-getting-tomlarray-or-tomltable)
- [confirming value type](#confirming-value-type)
- [visiting value that has unknown type](#visiting-value-that-has-unknown-type)
- [formatting toml values](#formatting-toml-values)
- [datetime operation](#datetime-operation)
- [underlying types](#underlying-types)
    - [`toml::string` and `basic`, `literal` flags](#tomlstring-and-basic-literal-flags)
    - [map class that represents `toml::table`](#map-class-that-represents-tomltable)
    - [why not STL container?](#why-not-stl-container)
- [synopsis](#synopsis)
- [Licensing terms](#licensing-terms)

## example code

Here is an example toml file (a bit modified from original toml repository).

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

You can read the file with the code below.

```cpp
// This library is header-only.
// To use it, including this file is the only thing required.
#include <toml/toml.hpp>

int main()
{
    // reads toml file and return it as a map (toml::key -> toml::value).
    const toml::table file = toml::parse("example.toml");

    // you can get toml values by toml::get function.
    const std::string title = toml::get<std::string>(file.at("title"));

    // it returns lvalue reference if you specify exact toml::* types
    const toml::table& owner = toml::get<toml::table   >(file.at("owner"));
    const std::string  name  = toml::get<std::string   >(owner.at("name"));
    const toml::datetime dob = toml::get<toml::datetime>(owner.at("dob"));

    const auto& database = toml::get<toml::table>(file.at("database"));
    // you can use std::string_view if you have c++17 compiler.
    const auto  server = toml::get<std::string_view>(database.at("server"));
    // you can get toml::array as your favorite container type.
    const auto  ports = toml::get<std::vector<int>>(database.at("ports"));
    // you can cast types if they are convertible (excepting Boolean -> Integer)
    const auto  connection_max = toml::get<std::size_t>(database.at("connection_max"));
    const auto  enabled = toml::get<bool>(database.at("enabled"));

    // array of table is simply an `array<table>`.
    const auto servers  = toml::get<std::vector<toml::table>>(file.at("servers"));

    // you can use boost::string_view if you don't have c++17 compatible compiler
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

## getting toml values

Boost.toml provides really powerful function to get a value from TOML data.

### basic usage of `toml::get`

You can `get` values from toml file by using `toml::get<T>` function.

```cpp
toml::value v1(42);
// you can get a reference when you get as the exact toml type
toml::integer& i_ref = toml::get<toml::integer>(v1);
i_ref = 6 * 9; // v1 will be 54.

// you can cast the value if they are convertible
std::uint32_t i = toml::get<std::uint32_t>(v1); // 54

// to avoid deep-copy, it is useful to get const reference.
toml::value v2{{"int", 42}, {"float", 3.14}, {"str", "foo"}};
const toml::table& tab = toml::get<toml::table>(v2);
```

If you pass a convertible type (like `int` for `toml::integer`) to `toml::get`'s
template argument, it casts the value to the type.
In that case, you can't get a lvalue reference that points to the contained
value because it returns `prvalue`.

If you pass a wrong type (like `std::string` for `toml::integer`) to
`toml::get`, it will throw `boost::bad_get`.

### getting `toml::array`

You can get `toml::array` as your favorite array type.

```cpp
toml::value v{1, 2, 3, 4, 5};
std::vector<int>        vec = toml::get<std::vector<int>       >(v);
std::deque<unsigned>    deq = toml::get<std::deque<unsigned>   >(v);
std::list<std::int64_t> lst = toml::get<std::list<std::int64_t>>(v);
std::array<char, 5>     ary = toml::get<std::array<char, 5>    >(v);
```

Surprisingly, you can also get a `std::pair` or `std::tuple` type
from `toml::array`.

```cpp
toml::value v{1, 2};
std::pair<int, unsigned> p = toml::get<std::pair<int, unsigned>>(v);

toml::value v{1,2,3,4};
std::tuple<int, int, int, int> t = toml::get<std::tuple<int, int, int, int>>(v);
```

currently, `boost::tuple` is not supported for this purpose.

### getting `toml::table` and Array of Table

Array of Tables can be obtained as the same way described before.

```cpp
std::vector<toml::table> tables = toml::get<std::vector<toml::table> >(v);
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
So it has all the functionality that `boost::container::flat_map` has.
In most cases, the conversion is not needed.

### `toml::array` of `toml::array` having different types each other

Consider that you have this toml file.

```toml
array = [[1, 2, 3], ["foo", "bar", "baz"]]
```

What is the corresponding C++ type? If you know about the length and the type of
array before reading it, you can use `std::pair` or `std::tuple`.

```cpp
auto pr  = toml::get<std::pair <std::vector<int>, std::vector<std::string>>>(v);
auto tpl = toml::get<std::tuple<std::vector<int>, std::vector<std::string>>>(v);
```

But generally, you cannot know the length of array and the type of array element
in toml file. In that case, `toml::array` or `std::vector<toml::value>`
can be used (actually, `toml::array` is just an alias of
`boost::container::vector<toml::value>`, so in this case the conversion might
not be needed).

```cpp
const toml::array& a = toml::get<toml::array>(v); // get without copy

std::vector<int>    a1 = toml::get<std::vector<int>        >(a.at(0));
std::vector<string> a2 = toml::get<std::vector<std::string>>(a.at(1));
```

Or you can get this as `std::vector<toml::array>`.

```cpp
std::vector<toml::array> a = toml::get<std::vector<toml::array>>(v);

int         a1 = toml::get<int        >(a.at(0).at(0)); // 1
std::string a2 = toml::get<std::string>(a.at(1).at(0)); // "foo"
```

### performance of getting `toml::array` or `toml::table`

Because `toml::get` does type-casting when you pass your favorite container to
`toml::get`, it essentially does the same thing as following.

```cpp
std::vector<int> get_int_vec(const toml::value& v)
{
    const toml::array& ar = v.get<toml::array>();
    std::vector<int> retval(ar.size());
    std::transform(ar.begin(), ar.end(), retval.begin(),
        [](const toml::value& x){return toml::get<int>(x);});
    return retval;
}
```

getting `toml::table` as a different `map` type is basically the same.

```cpp
std::map<toml::key, toml::value> get_std_map(const toml::value& v)
{
    const toml::table& tb = v.get<toml::table>();
    // it does not need extra toml::get, so it's simpler than array.
    return std::map<toml::key, toml::value>(tb.begin(), tb.end());
}
```

If the array or table has many many elements, it will take time because it
constructs completely new array or table.

Sometimes it is intolerable.
In that case, you can get them as a (const) reference to avoid deep-copy.

```cpp
toml::value  v{1,2,3,4,5};
toml::array& ar = toml::get<toml::array>(v);
```

Although it is a bit boring to call `toml::get` for all the elements in the
array, but there is a tradeoff between speed and usability.

## confirming value type

If you pass wrong template argument to `toml::get`, `toml::bad_get` (that is
derived from `std::exception`) will be thrown. Before using `toml::get`, it
should be known what type does the value have.

```cpp
toml::value v(3.14);
std::string s = toml::get<std::string>(v);

// exception thrown. the message would be something like this.
// terminate called after throwing an instance of 'toml::bad_get'
//   what():  toml::get: toml value has type `float`, but type `std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >` is specified.
```

When you cannot know actually what type is contained in the data file,
`enum` value representing type information is useful.

```cpp
toml::table data = toml::parse("sample.toml");
const toml::value& v = data["some_value"]; // what type does it have ???

std::cout << v.which() << std::endl; // outputs "integer" or something like that

switch(v.which())
{
    case toml::value::integer_tag  : /* do some stuff */; break;
    case toml::value::string_tag   : /* do some stuff */; break;
    // ...
}
```

the complete set of this `enum` values is found in [synopsis](#synopsis).

`toml::value` also has `is()` member function that checks the type.

```cpp
if(v.is(toml::value::string_tag)) {/* do some stuff */}
```

But it is painful to write `switch-case` every time.
Boost.toml provides a way to visit contained value without knowing its value.

See also [visiting value that has unknown type](#visiting-value-that-has-unknown-type).

## visiting value that has unknown type

Boost.toml provides `toml::apply_visitor` function to visit contained value
without knowing its type.

```cpp
toml::table data = toml::parse("sample.toml");
const auto twice = toml::apply_visitor(
    [](const auto& val) {return val + val}, data["number"]);
const auto half  = toml::visit( // completely same function as above
    [](const auto& val) {return val / 2},   data["number"]);
```

The usage is similar to `boost::apply_visitor` and `boost::variant`, bacause
it uses them internally.

## formatting toml values

TODO

## datetime operation

Depending on Boost.Date\_Time, Boost.toml supports datetime operation.
Since `toml::datetime` is actually just an alias of `boost::posix_time::ptime`,
you can operate `datetime` in the same way as Boost.Date\_Time.

```cpp
boost::posix_time::ptime dt = toml::get<boost::posix_time::ptime>(v);
dt += boost::date_time::days(7);
dt += boost::date_time::hours(12);
```

And just for ease, Boost.toml imports some classes and enums.
You can operate your `toml::datetime` value in the same way as
Boost.Date\_Time without writing namespace `boost::date_time::` explicitly.

```cpp
toml::value d(toml::date(2018, toml::Apr, 1));
toml::value t(toml::hours(1) + toml::minutes(3) + toml::seconds(10));
toml::value dt(d, t);
toml::get<toml::datetime>(dt) += toml::years(1);
toml::get<toml::datetime>(dt) += toml::months(2);
toml::get<toml::datetime>(dt) += toml::days(10);
```

## underlying types

`toml::value` is based on a `boost::variant` that contains following toml value
types.

| toml value type   | boost.toml type                                         |
|:------------------|:--------------------------------------------------------|
| -                 | `boost::blank` (uninitialized type)                     |
| `boolean`         | `bool`                                                  |
| `integer`         | `boost::int64_t`                                        |
| `floating`        | `double`                                                |
| `string`          | `struct {std::string str; enum {basic, literal} kind;}` |
| `date`            | `boost::gregorian::date`                                |
| `time`            | `boost::posix_time::time_duration`                      |
| `datetime`        | `boost::posix_time::ptime`                              |
| `offset_datetime` | `boost::local_time::local_date_time`                    |
| `array`           | `boost::container::vector<value>`                       |
| `table`           | `boost::container::flat_map<key, value>`                |

### `toml::string` and `basic`, `literal` flags

`toml::string` has an `enum` to represent `basic_string` and `literal_string`.
But it can be converted to std::string automatically, so the users do not need
to consider about the difference between `toml::string` and `std::string`.

an enum value can be passed to make `toml::value` that contains `toml::string`.
It affects on the output format. By default, it is set as `basic_string`.

```cpp
toml::value v1("foo", toml::string::basic);
toml::value v2("bar", toml::string::literal);
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
that is defined recursively.

## Synopsis

TODO

## Licensing Terms

This library is distributed under the MIT License.

- copyright (c) 2018, Toru Niina

All rights reserved.
