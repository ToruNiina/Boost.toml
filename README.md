Boost.toml
====

Boost.toml is a header-only toml parser depending on Boost.

__NOTE__: This library is not a part of Boost C++ Library.

## Table of Contents

- [example code](#example-code)
- [confirming value type](#confirming-value-type)
- [getting toml values](#getting-toml-values)
    - [basic usage of `toml::get`](#basic-usage-of-tomlget)
    - [getting `toml::array`](#getting-tomlarray)
    - [getting `toml::table`](#getting-tomltable)
    - [`toml::array` of `toml::array` having different types each other](#tomlarray-of-tomlarray-having-different-types-each-other)
    - [performance of getting `toml::array` or `toml::table`](#performance-of-getting-tomlarray-or-tomltable)
- [visiting value that has unknown type](#visiting-value-that-has-unknown-type)
- [formatting toml values](#formatting-toml-values)
- [datetime operation](#datetime-operation)
- [underlying types](#underlying-types)
    - [`toml::string` and `basic`, `literal` flags](#tomlstring-and-basic-literal-flags)
    - [map class that represents `toml::table`](#map-class-that-represents-tomltable)
    - [why not STL container?](#why-not-stl-container)
- [Licensing terms](#licensing-terms)

## example code

TODO

## confirming value type

If you don't know actually what type is contained in the data, you can get
type information as `enum`.

```cpp
toml::table data = toml::parse("sample.toml");
toml::value const& v = data["some_value"]; // what type does it have ???

std::cout << v.which() << std::endl; // outputs "integer" or something like that

switch(v.which())
{
    case toml::value::integer_tag  : /* do some stuff */; break;
    case toml::value::string_tag   : /* do some stuff */; break;
    // ...
}
```

`toml::value` also has `is()` member function that checks the type.

```cpp
if(v.is(toml::value::string_tag)) {/* do some stuff */}
```

But it is painful to write `switch-case` every time.
Boost.toml provides a way to visit contained value without knowing its value.

See also [visiting value that has unknown type](#visiting-value-that-has-unknown-type).

## getting toml values

Boost.toml provides really powerful function to get a value from TOML data.

### basic usage of `toml::get`

You can `get` values from toml file by using `toml::get<T>` function.

```cpp
toml::value v1(42);
// you can get a reference when you get as the exact toml type
toml::integer& i_ref = toml::get<toml::integer>(v1);

// you can cast the value if they are convertible
std::uint32_t i = toml::get<std::uint32_t>(v1);

toml::value v2(3.14);
const float f = toml::get<float>(v); // also, you can do this.
```

If you pass an exact toml type to `toml::get`'s template argument,
you can obtain an lvalue reference to the contained value.

```cpp
toml::value v(42);
toml::integer& i = toml::get<toml::integer>(v);
i = 6 * 9;
std::cout << toml::get<toml::integer>(v) << std::endl; // 54

toml::value t{{"int", 42}, {"float", 3.14}, {"str", "foo"}};
toml::table const& table = toml::get<toml::table>(t);
```

If you pass a different type to `toml::get`'s template argument, it casts the
value to the type, so it will return prvalue. In that case, you can't get the
reference that points to the contained value.

### getting `toml::array`

You can get `toml::array` as your favorite array type.

```cpp
toml::value v{1, 2, 3, 4, 5};
std::vector<int>        vec = toml::get<std::vector<int>>(v);
std::deque<unsigned>    deq = toml::get<std::deque<unsigned>>(v);
std::list<std::int64_t> lst = toml::get<std::list<std::int64_t>>(v);
std::array<char, 5>     ary = toml::get<std::array<char, 5>>(v);
```

Surprisingly, you can also get a `std::pair` or `std::tuple` type
from `toml::array`.

```cpp
toml::value v{1, 2};
std::pair<int, unsigned> p = toml::get<std::pair<int, unsigned>>(v);

toml::value v{1,2,3,4};
std::tuple<int, int, int, int> t = toml::get<std::tuple<int, int, int, int>>(v);
```

currently, `boost::tuple` is not supported.

### getting `toml::table`

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
So it has all the functionality that `flat_map` has.
In most cases, the conversion is not needed.

### `toml::array` of `toml::array` having different types each other

Consider that you have this toml file.

```toml
array = [[1,2,3], ["foo", "bar", "baz"]]
```

What is the corresponding C++ type? If you completely know about the type of
array before reading it, you can use `std::pair` or `std::tuple`.

```cpp
auto pr  = toml::get<std::pair <std::vector<int>, std::vector<std::string>>>(v);
auto tpl = toml::get<std::tuple<std::vector<int>, std::vector<std::string>>>(v);
```

But generally, you cannot know the length of array and the type of array element
in toml file. In that case, `toml::array` or `std::vector<toml::value>`
can be used (actually, `toml::array` is just a
`boost::container::vector<toml::value>`).

```cpp
toml::array a = toml::get<toml::array>(v);

std::vector<int>    a1 = toml::get<std::vector<int>        >(a.at(0));
std::vector<string> a2 = toml::get<std::vector<std::string>>(a.at(1));
```

You also can get this as `std::vector<toml::array>`.

```cpp
std::vector<toml::array> a = toml::get<std::vector<toml::array>>(v);

int         a1 = toml::get<int        >(a.at(0).at(0)); // 1
std::string a2 = toml::get<std::string>(a.at(1).at(0)); // "foo"
```

### performance of getting `toml::array` or `toml::table`

Because `toml::get` does type-casting when you pass your favorite container to
`toml::get`, it essentially do the same thing as following.

```cpp
std::vector<int> get_int_vec(const toml::value& v)
{
    const toml::array& ar = v.get<toml::array>();
    std::vector<int> retval(ar.size());
    std::transform(ar.begin(), ar.end(), retval.begin(),
        [](toml::value const& x){return toml::get<int>(x);});
    return retval;
}
```

getting `toml::table` is simpler than the case of `toml::array`.

```cpp
std::map<toml::key, toml::value> get_std_map(const toml::value& v)
{
    const toml::table& tb = v.get<toml::table>();
    // it does not need extra toml::get, so it's simpler than array.
    return std::map<toml::key, toml::value>(tb.begin(), tb.end());
}
```

If the array or table has many many elements, it will take time because it
essentially constructs completely new array or table.

Sometimes it is intolerable. In that case, you can get them as
just a (const) reference.

```cpp
toml::value  v{1,2,3,4,5};
toml::array& ar = toml::get<toml::array>(v);
```

Although it is a bit boring to call `toml::get` for all the elements in the
array, but there is a tradeoff between speed and usability.

## visiting value that has unknown type

Boost.toml provides `toml::apply_visitor` function to visit contained value
without knowing its type.

```cpp
toml::table data = toml::parse("sample.toml");
const auto twice = toml::apply_visitor(
    [](const auto& val) {return val + val}, data["number"]);
```

The usage is similar to `boost::apply_visitor` and `boost::variant`, bacause
it depends on it.

## formatting toml values

TODO

## datetime operation

Depending on Boost.Date\_Time, Boost.toml supports datetime operation.
You can just get `boost::posix_time::ptime` from `toml::datetime`,
so you can operate `datetime` in the following way.

```cpp
toml::value v;
boost::posix_time::ptime dt = toml::get<boost::posix_time::ptime>(v);
```

And just for ease, Boost.toml imports some classes and enums.
You can construct your `toml::datetime` value in the same way as
Boost.Date\_Time.

```cpp
toml::value d(toml::date(2018, toml::Apr, 1));
toml::value t(toml::hours(1) + toml::minutes(3) + toml::seconds(10));
toml::value dt(d, t);
```

## underlying types

`toml::value` is based on a `boost::variant` that contains following toml value
types.

| toml value type | boost.toml type                                         |
|:----------------|:--------------------------------------------------------|
| -               | `boost::blank` (uninitialized type)                     |
| `boolean`       | `bool`                                                  |
| `integer`       | `boost::int64_t`                                        |
| `float`         | `double`                                                |
| `string`        | `struct {std::string str; enum {basic, literal} kind;}` |
| `date`          | `boost::gregorian::date`                                |
| `time`          | `boost::posix_time::time_duration`                      |
| `datetime`      | `boost::posix_time::ptime`                              |
| `array`         | `boost::container::vector<value>`                       |
| `table`         | `boost::container::flat_map<key, value>`                |

### `toml::string` and `basic`, `literal` flags

`toml::string` has an `enum` to represent `basic_string` and `literal_string`.
But it can be converted to std::string automatically, so the users do not need
to consider about the difference between `toml::string` and `std::string`.

The enum value or tag class can be passed to make `toml::value` that contains
`toml::string`. It affects on the output format. By default, `basic_string` is
chosen.

```cpp
toml::value v1("foo", toml::string::basic);
toml::value v2("bar", toml::string::literal);
toml::value v3("baz", toml::basic_string);
toml::value v4("qux", toml::literal_string);
```

### map class that represents `toml::table`

Here, `flat_map` is used to store toml.table because normally the data from
file will not be added or erased so many times. You can change it to a
`boost::container::map` by defining `BOOST_TOML_USE_MAP_FOR_TABLE` as a compiler
flag.

### Why not STL container?

Because `Boost.Container` allows to contain incomplete types. So it allows to
contain recursive data type.
This feature removes the neccesity of using pointers to implement `toml::value`
that is defined recursively.

## Licensing Terms

This library is distributed under the MIT License.

- copyright 2018, Toru Niina

All rights reserved.
