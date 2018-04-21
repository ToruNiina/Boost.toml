Boost.toml
====

Boost.toml is a header-only toml parser depends on Boost.

## example code

TODO

## getting toml values

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

Surprisingly, you can also get `toml::array` as a `std::pair` or `std::tuple` type.

```cpp
toml::value v{1, 2};
std::pair<int, unsigned> p = toml::get<std::pair<int, unsigned>>(v);

toml::value v{1,2,3,4};
std::tuple<int, int, int, int> t = toml::get<std::tuple<int, int, int, int>>(v);
```

### performance of getting `toml::array`

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

If the array has many many elements, it will take time.
Sometimes it become intolerable. In that case, you can get `toml::array` as
just a `(const) toml::array&`.

```cpp
toml::value  v{1,2,3,4,5};
toml::array& ar = toml::get<toml::array>(v);
```

This is just the same as `boost::get` for `boost::variant`.

### `toml::array` of `toml::array` having different types each other

Let us consider you have this toml file.

```toml
array = [[1,2,3], ["foo", "bar", "baz"]]
```

What is the corresponding C++ type? In that case, `toml::array` or
`std::vector<toml::value>` can be used. Actually, `toml::array` is a
`boost::container::vector<toml::value>`.

```cpp
{
    toml::array a = toml::get<toml::array>(v);

    std::vector<int>    a1 = toml::get<std::vector<int>        >(a.at(0));
    std::vector<string> a2 = toml::get<std::vector<std::string>>(a.at(1));
}
```

You also can get this as `std::vector<toml::array>`.

```cpp
{
    std::vector<toml::array> a = toml::get<std::vector<toml::array>>(v);

    int         a1 = toml::get<int        >(a.at(0).at(0)); // 1
    std::string a2 = toml::get<std::string>(a.at(1).at(0)); // "foo"
}
```

## formatting toml values

TODO

## underlying types

`toml::value` is based on a `boost::variant` that contains following toml value types.

| toml value type | boost.toml type                                         |
|:----------------|:--------------------------------------------------------|
| -               | `boost::blank` (uninitialized type)                     |
| `boolean`       | `bool`                                                  |
| `integer`       | `boost::int64_t`                                        |
| `float`         | `double`                                                |
| `string`        | `struct {std::string str; enum {basic, literal} kind;}` |
| `date`          | `boost::gregorian::date`                                |
| `time`          | `boost::posix_time::time_duration`                      |
| `datetime`      | `struct {date d; time t};`                              |
| `array`         | `boost::container::vector<value>`                       |
| `table`         | `boost::container::flat_map<key, value>`                |

### `toml::string` and `basic`, `literal` flags

`toml::string` has an `enum` to represent `basic_string` and `literal_string`.
But `toml::get` supports getting lvalue reference that points `std::string`
contained in `toml::string`, so user may feel it is just a 

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
