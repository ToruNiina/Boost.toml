Boost.toml
====

[![Build Status](https://travis-ci.com/ToruNiina/Boost.toml.svg?token=3rmRuAkSVRafwq2Rbd9b&branch=master)](https://travis-ci.com/ToruNiina/Boost.toml)

Boost.tomlはBoostを使ったヘッダオンリーなTOMLライブラリです。
[TOML v0.5.0](https://github.com/toml-lang/toml/blob/master/versions/en/toml-v0.5.0.md)に対応しています。

自動テストは、LinuxまたはmacOSで`C++(98|11|14|17)`のそれぞれで行っています。

一部の機能は、その導入以前のC++バージョンが指定された場合、無効化されます。
例えば、`std::initializer_list`を用いた初期化はC++11以降で、`std::string_view`
としての文字列取得はC++17以降で有効になります。

Boost.tomlは比較的新しいバージョンのBoostライブラリを要求します。
自動テストではBoost 1.67.0が使用されています。

__注意__: このライブラリはBoostライブラリの一部ではありません。

## Table of Contents

- [サンプルコード](#サンプルコード)
- [ファイルのパース](#ファイルのパース)
- [値の取り出し](#値の取り出し)
    - [`toml::get`の使い方](#tomlgetの使い方)
    - [配列を取得する](#配列を取得する)
    - [互いに異なる型を持つ配列の配列](#互いに異なる型を持つ配列の配列)
    - [テーブルやテーブルの配列を取得する](#テーブルやテーブルの配列を取得する)
    - [パフォーマンスについての補足](#パフォーマンスについての補足)
- [dotted keyの扱い](#dotted-keyの扱い)
- [値の型を確認する](#confirming-value-type)
- [型を確認せずにアクセスする](#visiting-value-that-has-unknown-type)
- [データのフォーマット](#formatting-toml-values)
    - [TOMLデータを作る](#constructing-toml-values)
    - [値を出力する](#printing-toml-values)
    - [ファイルを出力する](#formatting-toml-data)
- [日時の取り扱い](#datetime-operation)
- [データの型について](#underlying-types)
    - [文字列の種類](#tomlstring-and-basic-literal-flags)
    - [テーブルに対応するクラス](#map-class-that-represents-tomltable)
    - [なぜBoost.Containerを使うのか](#why-not-stl-container)
    - [`toml::get`によって変換可能な型の一覧](#types-that-are-convertible-from-toml-value-by-using-tomlget)
- [補遺](#補遺)
- [synopsis](#synopsis)
- [ライセンス条項](#ライセンス条項)

## サンプルコード

以下にTOMLファイルの例を示します。

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

# ここで、説明のためtoml-langレポジトリから少し改変を加えています
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

以上のデータは、以下のようなC++コードで読み取ることが出来ます。

```cpp
#include <toml/toml.hpp>

int main()
{
    // ファイルのパース結果は、toml::key から toml::value への map として返却されます
    const toml::table file = toml::parse("example.toml");

    // データを取り出すには、toml::get関数を使います。
    const std::string title = toml::get<std::string>(file.at("title"));

    // toml::get は左辺値参照も取得できます。
    const toml::table&  owner = toml::get<toml::table>(file.at("owner"));
    const std::string&  name  = toml::get<std::string>(owner.at("name"));
    const auto&         dob   = toml::get<toml::offset_datetime>(owner.at("dob"));

    const auto& database = toml::get<toml::table>(file.at("database"));
    // C++17を使っているなら、const std::string& の代わりにstd::string_viewを使えます。
    const auto  server = toml::get<std::string_view>(database.at("server"));
    // toml::arrayは任意のコンテナ型として受け取ることができます。
    const auto  ports = toml::get<std::vector<int>>(database.at("ports"));
    // 必要なら、toml::getで型変換をすることもできます。
    const auto  connection_max = toml::get<std::size_t>(database.at("connection_max"));
    const auto  enabled = toml::get<bool>(database.at("enabled"));

    // Boost.tomlでは、array-of-tableは単にtoml::tableの配列です。
    const auto servers = toml::get<std::vector<toml::table>>(file.at("servers"));

    // C++17以前でも、boost::string_(view|ref) がサポートされています。
    const auto name_alpha = toml::get<boost::string_view>(servers.at(0).at("name"));
    const auto ip_alpha   = toml::get<boost::string_view>(servers.at(0).at("ip"));
    const auto dc_alpha   = toml::get<boost::string_view>(servers.at(0).at("dc"));
    const auto name_beta  = toml::get<boost::string_ref >(servers.at(1).at("name"));
    const auto ip_beta    = toml::get<boost::string_ref >(servers.at(1).at("ip"));
    const auto dc_beta    = toml::get<boost::string_ref >(servers.at(1).at("dc"));

    const auto& clients = toml::get<toml::table>(file.at("clients"));

    // こんなこともできます。
    // 元データの配列は、最初の要素が文字列の配列で、次の要素が数値の配列でした。
    // data = [ ["gamma", "delta"], [1, 2] ]
    // それを文字列の配列と数値の配列のペアとして一度に取得できます。
    const auto  data = toml::get<
        std::pair<std::vector<std::string>, std::vector<int>>
        >(clients.at("data"));
    // C++11以降では、`std::tuple`も同様に用いることができます。

    return 0;
}
```

## ファイルのパース

このライブラリはヘッダオンリーなので、`toml/toml.hpp`をincludeする以外に
使うために必要なことはありません。

ファイルをパースするには、`toml::parse`関数を使います。引数には、ファイル名を
示す`std::string`か、`std::istream&`を渡します。ファイルを開くのに失敗した場合、
`std::runtime_error`が発生します。

返却値は`toml::table`です。もしパース中に文法エラーなどが発生した場合、
`std::runtime_error`が発生します。

```cpp
const toml::table data = parse("example.toml");
// or
std::ifstream ifs("example.toml");
if(!ifs.good){return 1;}
const toml::table data = parse(ifs);
```

## 値の取り出し

Boost.tomlは、値を取り出すのに便利な`toml::get`関数を提供しています。

### `toml::get`の使い方

`toml::get<T>`を使えば、`toml::value`から`T`型の値を取り出すことが出来ます。

```cpp
// 変換が必要ない型を指定した場合、参照を取得することができます。
toml::value v1(42);
toml::integer& i_ref = toml::get<toml::integer>(v1);
i_ref = 6 * 9; // 代入により、v1の中身は54になります。

// 変換が必要な型を指定することもできます。この場合、内部で型変換がなされるので、
// 元の値を指す参照を取得することはできません。
std::uint32_t i = toml::get<std::uint32_t>(v1); // 54

// 大きな配列やテーブルを取得するときは、ディープコピーを避けるために参照を使うのが有効です。
toml::value v2{{"int", 42}, {"float", 3.14}, {"str", "foo"}};
const toml::table& tab = toml::get<toml::table>(v2);
```

`toml::get`に変換可能な型（`toml::integer`に対して`int`など）を渡した場合、
内部で値を取得した後指定した型に変換して返却します。この場合、返却されるのは
prvalueなので左辺値参照は取得できません。内部の型を知るには
[データの型について](#データの型について)を、型変換についてより詳しく知るには
[`toml::get`によって変換可能な型の一覧](#toml::getによって変換可能な型の一覧)
を参照して下さい。

`toml::get`に間違った型（`toml::integer`が格納されている値に対して`std::string`など）
を指定した場合、`toml::bad_get`が発生します。`toml::bad_get::what`は
`boost::typeindex::type_id().pretty_name()`によってデマングルされた型名を含むので、
エラーメッセージの出力などの助けになるでしょう。

```cpp
toml::value v(42);
std::string s = toml::get<std::string>(v);

// 以下のような例外が投げられます。
// terminate called after throwing an instance of 'toml::bad_get'
//   what():  toml::get: toml value has type `toml::integer`, but type `std::basic_string<char, std::char_traits<char>, std::allocator<char> >` is specified.
```

### `toml::array`を取得する

`toml::array`は好きなコンテナで取り出すことができます。この時、同時に要素の型も変換されます。

```cpp
toml::value v{1, 2, 3, 4, 5};
std::vector<int>        vec = toml::get<std::vector<int>       >(v);
std::deque<unsigned>    deq = toml::get<std::deque<unsigned>   >(v);
std::list<std::int64_t> lst = toml::get<std::list<std::int64_t>>(v);
std::array<char, 5>     ary = toml::get<std::array<char, 5>    >(v);
```

驚くべきことに、「好きなコンテナ」には`std::pair`と`std::tuple`も含まれます。

```cpp
toml::value v{1, 2};
std::pair<int, unsigned>  p = toml::get<std::pair<int, unsigned>>(v);

toml::value v{1, 2, 3};
std::tuple<int, int, int> t = toml::get<std::tuple<int, int, int>>(v);

// boost::tupleはまだサポートされていません。 
```

この機能は、以下のような状況で特に便利です。


### 互いに異なる型を持つ配列の配列

以下のようなTOMLファイルがあるとします。

```toml
array = [[1, 2, 3], ["foo", "bar", "baz"]]
```

対応する型は、もちろん`std::vector<int>`と`std::vector<std::string>`の`std::pair`でしょう。

Boost.tomlならまさにその型で取り出せます。

```cpp
auto v = toml::get<std::pair<std::vector<int>, std::vector<std::string>>>(v);
```

`std::tuple`もサポートされています。

```cpp
auto v = toml::get<std::tuple<std::vector<int>, std::vector<std::string>>>(v);
```

しかし、ファイルに書かれている値の型を前もって決めておけないこともあるでしょう。
その場合は、`toml::array`か`std::vector<toml::value>`として取得することが可能です
（実際には、`toml::array`は`boost::container::vector<toml::value>`なので、この
変換が必要になることはないでしょう）。

```cpp
const toml::array& a = toml::get<toml::array>(v);

std::vector<int>    a1 = toml::get<std::vector<int>        >(a.at(0));
std::vector<string> a2 = toml::get<std::vector<std::string>>(a.at(1));
```

または、配列の配列であることが明らかであれば、`std::vector<toml::array>`として
取得することもできます。

```cpp
std::vector<toml::array> a = toml::get<std::vector<toml::array>>(v);

int         a1 = toml::get<int        >(a.at(0).at(0)); // 1
std::string a2 = toml::get<std::string>(a.at(1).at(0)); // "foo"
```

### テーブルやテーブルの配列を取得する

テーブルの配列は何も特別な型ではないので、これまで説明した他の型と全く同じように取得できます。

```cpp
auto tables = toml::get<std::vector<toml::table>>(v);
```

また、テーブルのコンテナ型も変換可能です。

```cpp
toml::value v{{"int", 42}, {"float", 3.14}, {"str", "foo"}};

auto std_map  = toml::get<std::map<toml::key, toml::value>             >(v);
auto std_umap = toml::get<std::unordered_map<toml::key, toml::value>   >(v);
auto bst_map  = toml::get<boost::container::map<toml::key, toml::value>>(v);
auto bst_umap = toml::get<boost::unordered_map<toml::key, toml::value> >(v);
```

但し、`toml::table`は`boost::container::flat_map`のエイリアスなので、これらの
変換は通常必要ありません。

### パフォーマンスについての補足

配列を別の型で取得する場合、`toml::get`は全ての要素を変換するので、
本質的には以下のコードと同じことをしていることになります。

```cpp
// toml::array から std::vector<int>への変換では、
// toml::get<std::vector<int>>(val)は以下と本質的に同等のことをします。
std::vector<int> get_int_vec(const toml::value& v)
{
    const toml::array& ar = v.get<toml::array>();
    std::vector<int> retval(ar.size());
    std::transform(ar.begin(), ar.end(), retval.begin(),
        [](const toml::value& x){return toml::get<int>(x);});
    return retval;
}
```

テーブルでも同様です。

```cpp
std::map<toml::key, toml::value> get_std_map(const toml::value& v)
{
    const toml::table& tb = v.get<toml::table>();
    // it does not need extra toml::get, so it's simpler than array.
    return std::map<toml::key, toml::value>(tb.begin(), tb.end());
}
```

これらの操作は新規なコンテナの作成とディープコピーを行うので、元のオブジェクトが
大きかった時、時間がかかる処理になります。

この速度低下を見過ごせない場合もあるでしょう。

ディープコピーを避けるには、参照で受け取るべきです。但し、その場合要素が全て
`toml::value`なので、各要素で明示的に型変換をする必要があります。

```cpp
toml::value  v{1,2,3,4,5};
toml::array& ar = toml::get<toml::array>(v);
std::int64_t i  = toml::get<std::int64_t>(ar.at(0));
```

## dotted keyの扱い

Boost.tomlは、TOML仕様に従い、dotted keyをテーブルと見なします。

```toml
# dotted keys
physical.color = "red"
physical.shape = "sphere"
# 上記は以下と同等
physical = {color = "red", shape = "sphere"}
# 以下とも同等
[physical]
color = "red"
shape = "sphere"
```

以下のようなコードで、上記のデータを取得できます。

```cpp
const auto& physical = toml::get<toml::table>(data.at("physical"));
const auto& color    = toml::get<std::string>(physical.at("color"));
const auto& shape    = toml::get<std::string>(physical.at("shape"));
```

## 値の型を確認する

`toml::get`に間違った型を渡した場合、`toml::bad_get`（`std::exception`から派生しています）が
送出されます。なので、`toml::get`を実行する前に、持っている値の型を確認する必要が生じます。

```cpp
toml::value v(3.14);
std::string s = toml::get<std::string>(v);

// 例外が送出されます。
// terminate called after throwing an instance of 'toml::bad_get'
//   what():  toml::get: toml value has type `toml::float`, but type `std::basic_string<char, std::char_traits<char>, std::allocator<char> >` is specified.
```

`toml::value` は `toml::value::is()` メンバ関数を持っており、型情報をチェックできます。

```cpp
toml::table data = toml::parse("unknown.toml");
const toml::value& v = data["some_value"]; // 型が不明

// enumを受け取るバージョン
if(v.is(toml::value::string_tag)) {/* 対応する処理 */}
// 型を受け取るバージョン
if(v.is<toml::integer>()) {/* 対応する処理 */}
```

上記の通り、`toml::value` は型情報を意味する`enum`を持っており、これを使って`switch`することも可能です。

```cpp
std::cout << v.which() << std::endl; // enumを出力すると、文字列として書きだされます

switch(v.which())
{
    case toml::value::integer_tag  : /* 対応する処理 */; break;
    case toml::value::string_tag   : /* 対応する処理 */; break;
    // ...
}
```

この `enum` のリストは [synopsis](#synopsis)にあります。

しかし、`switch`や分岐を書き続けるのは疲れます。そのような場合のため、Boost.toml
はパターンマッチに似たやり方で値を取得する方法を用意しています。

## 型を確認せずにアクセスする

Boost.tomlは`toml::apply_visitor`と`toml::visit`関数を用意しています。
使い方は`boost::variant`に対する`boost::apply_visitor`と同じです。

```cpp
toml::table data = toml::parse("sample.toml");
toml::apply_visitor([](const auto& val) {std::cout << val << std::endl;},
                    data["number"]);
toml::visit([](const auto& val) {std::cerr << val << std::endl;},
            data["number"]);
```

## データのフォーマット

Boost.tomlはデータの出力もサポートしています。

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

## 補遺

### Boost.Date\_Timeとリンク

Boost.tomlはBoost.Date\_Timeに依存しています。
[Boost.Date\_Timeによると](https://www.boost.org/doc/libs/1_68_0/doc/html/date_time/details.html#date_time.buildinfo)、
Boost.Date\_Timeで提供されるいくつかの関数はリンクを必要とします。しかし、同時に
それらの関数が必要になるシーンは限られているので、大抵の場合リンク無しで
使用できるとも書かれています。Boost.tomlはBoost.Date\_Timeのヘッダオンリーで
使用できる部分のみに依存しているので、ヘッダオンリーで使うことが可能になっています。

### `error: wrong number of template arguments (11, should be at least 0)`

Boost.tomlは、多くのBoostライブラリがそうしているように、Boost.MPLに依存しています。
しかしBoost.MPLで一度に指定できる型引数の数がデフォルトでは少ないため、
`BOOST_MPL_LIMIT_METAFUNCTION_ARITY`を定義することでこれを拡張しています
([ソースコード](toml/predefine.hpp))。しかしながら、異なるBoostライブラリを先に
インクルードした場合、上書きできずこの項目のタイトルにあるようなエラーが出て
しまうことがあります。

これを回避するには、Boost.tomlを先にインクルードするようにしてください。

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
    value& operator=(value&& v); // C++11以降で有効化されます。

    // SFINAEとオーバーロードによって変換可能な型のそれぞれに定義されます。
    template<typename T> value(const T& v);
    template<typename T> value& operator=(const T& v);

    value(const char* v,        string::kind_t k);
    value(const std::string& v, string::kind_t k);
    value(const date&                d, const time& t);
    value(const local_datetime&     dt, const time_zone_ptr tzp);
    value(const date& d, const time& t, const time_zone_ptr tzp);

    // C++11以降で有効化されます。
    value(std::string&& v);
    value(toml::array&& v);
    value(toml::table&& v);
    value& operator=(std::string&& v);
    value& operator=(toml::array&& v);
    value& operator=(toml::table&& v);

    // C++11以降で有効化されます。
    template<typename T>
    value(std::initializer_list<T> v);
    value(std::initializer_list<std::pair<key, value>> v);

    // string, array, tableのそれぞれに対して実装されます。
    // Iteratorのvalue_typeがcharならstringに、valueならarrayに、
    // pair<key, value>ならtableになります。
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
    /* Visitorの返却値 */ apply_visitor(Visitor v);
    template<typename Visitor>
    /* Visitorの返却値 */ apply_visitor(Visitor v) const;

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

## ライセンス条項

このライブラリは[the Boost Software License](LICENSE)の元に頒布されています。

- copyright (c) 2018, Toru Niina

All rights reserved.
