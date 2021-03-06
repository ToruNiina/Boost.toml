#define BOOST_TEST_MODULE "get_test"
#include <toml/value.hpp>
#include <toml/get.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>

BOOST_AUTO_TEST_CASE(test_exact_toml_type)
{
    {
        toml::value v(true);
        BOOST_CHECK_EQUAL(true, toml::get<toml::boolean>(v));
    }
    {
        toml::value v(42);
        BOOST_CHECK_EQUAL(toml::integer(42), toml::get<toml::integer>(v));
    }
    {
        toml::value v(3.14);
        BOOST_CHECK_EQUAL(toml::floating(3.14), toml::get<toml::floating>(v));
    }
    {
        toml::value v("foo");
        BOOST_CHECK_EQUAL(toml::string("foo", toml::string::basic),
                          toml::get<toml::string>(v));
    }
    {
        toml::value v("foo", toml::string::literal);
        BOOST_CHECK_EQUAL(toml::string("foo", toml::string::literal),
                          toml::get<toml::string>(v));
    }
    {
        toml::date  d(2018, toml::Apr, 22);
        toml::value v(d);
        BOOST_CHECK(d == toml::get<toml::date>(v));
    }
    {
        toml::time  t = toml::hours(1) + toml::minutes(30) + toml::seconds(5);
        toml::value v(t);
        BOOST_CHECK(t == toml::get<toml::time>(v));
    }
    {
        toml::local_datetime dt(toml::date(2018, toml::Apr, 22),
                                toml::hours(1) + toml::minutes(30));
        toml::value v(toml::date(2018, toml::Apr, 22),
                      toml::hours(1) + toml::minutes(30));
        BOOST_CHECK(dt == toml::get<toml::local_datetime>(v));
    }
    {
        toml::array vec(1, toml::value(42));
        vec.push_back(toml::value(54));
        toml::value v(vec);
        BOOST_CHECK(vec == toml::get<toml::array>(v));
    }
    {
        toml::table tab;
        tab["key1"] = toml::value(42);
        tab["key2"] = toml::value(3.14);
        toml::value v(tab);
        BOOST_CHECK(tab == toml::get<toml::table>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_integer_type)
{
    {
        toml::value v(42);
        BOOST_CHECK_EQUAL(int(42),             toml::get<int            >(v));
        BOOST_CHECK_EQUAL(short(42),           toml::get<short          >(v));
        BOOST_CHECK_EQUAL(char(42),            toml::get<char           >(v));
        BOOST_CHECK_EQUAL(unsigned(42),        toml::get<unsigned       >(v));
        BOOST_CHECK_EQUAL(long(42),            toml::get<long           >(v));
        BOOST_CHECK_EQUAL(boost::int64_t(42),  toml::get<boost::int64_t >(v));
        BOOST_CHECK_EQUAL(boost::uint64_t(42), toml::get<boost::uint64_t>(v));
        BOOST_CHECK_EQUAL(boost::int16_t(42),  toml::get<boost::int16_t >(v));
        BOOST_CHECK_EQUAL(boost::uint16_t(42), toml::get<boost::uint16_t>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_floating_type)
{
    {
        toml::value v(3.14);
        BOOST_CHECK_EQUAL(static_cast<float      >(3.14), toml::get<float      >(v));
        BOOST_CHECK_EQUAL(static_cast<double     >(3.14), toml::get<double     >(v));
        BOOST_CHECK_EQUAL(static_cast<long double>(3.14), toml::get<long double>(v));
    }
}

BOOST_AUTO_TEST_CASE(test_get_string_type)
{
    {
        toml::value v("foo", toml::string::basic);
        BOOST_CHECK_EQUAL("foo", toml::get<std::string       >(v));
        BOOST_CHECK_EQUAL("foo", toml::get<boost::string_ref >(v));
        BOOST_CHECK_EQUAL("foo", toml::get<boost::string_view>(v));
#ifdef TOML_HAS_CXX17_STRING_VIEW
        std::cerr << "c++17 string_view tested!" << std::endl;
        BOOST_CHECK_EQUAL("foo", toml::get<std::string_view>(v));
#endif // TOML_HAS_CXX17_STRING_VIEW
    }
    {
        toml::value v("foo", toml::string::literal);
        BOOST_CHECK_EQUAL("foo", toml::get<std::string       >(v));
        BOOST_CHECK_EQUAL("foo", toml::get<boost::string_ref >(v));
        BOOST_CHECK_EQUAL("foo", toml::get<boost::string_view>(v));
#ifdef TOML_HAS_CXX17_STRING_VIEW
        std::cerr << "c++17 string_view tested!" << std::endl;
        BOOST_CHECK_EQUAL("foo", toml::get<std::string_view>(v));
#endif // TOML_HAS_CXX17_STRING_VIEW
    }
}

BOOST_AUTO_TEST_CASE(test_get_toml_array)
{
    toml::value v(toml::array(0));
    toml::get<toml::array>(v).push_back(toml::value(42));
    toml::get<toml::array>(v).push_back(toml::value(54));
    toml::get<toml::array>(v).push_back(toml::value(69));
    toml::get<toml::array>(v).push_back(toml::value(72));

    const std::vector<int>           vec = toml::get<std::vector<int> >(v);
    const std::list<short>           lst = toml::get<std::list<short> >(v);
    const std::deque<boost::int64_t> deq =
        toml::get<std::deque<boost::int64_t> >(v);

    BOOST_CHECK_EQUAL(42, vec.at(0));
    BOOST_CHECK_EQUAL(54, vec.at(1));
    BOOST_CHECK_EQUAL(69, vec.at(2));
    BOOST_CHECK_EQUAL(72, vec.at(3));

    std::list<short>::const_iterator iter = lst.begin();
    BOOST_CHECK_EQUAL(static_cast<short>(42), *(iter++));
    BOOST_CHECK_EQUAL(static_cast<short>(54), *(iter++));
    BOOST_CHECK_EQUAL(static_cast<short>(69), *(iter++));
    BOOST_CHECK_EQUAL(static_cast<short>(72), *(iter++));

    BOOST_CHECK_EQUAL(static_cast<boost::int64_t>(42), deq.at(0));
    BOOST_CHECK_EQUAL(static_cast<boost::int64_t>(54), deq.at(1));
    BOOST_CHECK_EQUAL(static_cast<boost::int64_t>(69), deq.at(2));
    BOOST_CHECK_EQUAL(static_cast<boost::int64_t>(72), deq.at(3));

#ifdef TOML_HAS_CXX11_ARRAY
    std::cerr << "c++11 array tested!" << std::endl;
    std::array<int, 4> ary = toml::get<std::array<int, 4> >(v);
    BOOST_CHECK_EQUAL(static_cast<int>(42), ary.at(0));
    BOOST_CHECK_EQUAL(static_cast<int>(54), ary.at(1));
    BOOST_CHECK_EQUAL(static_cast<int>(69), ary.at(2));
    BOOST_CHECK_EQUAL(static_cast<int>(72), ary.at(3));
#endif // TOML_HAS_CXX11_ARRAY

#ifdef TOML_HAS_CXX11_TUPLE
    std::cerr << "c++11 tuple tested!" << std::endl;
    std::tuple<int, int, int, int> tpl =
        toml::get<std::tuple<int, int, int, int> >(v);
    BOOST_CHECK_EQUAL(static_cast<int>(42), std::get<0>(tpl));
    BOOST_CHECK_EQUAL(static_cast<int>(54), std::get<1>(tpl));
    BOOST_CHECK_EQUAL(static_cast<int>(69), std::get<2>(tpl));
    BOOST_CHECK_EQUAL(static_cast<int>(72), std::get<3>(tpl));
#endif // TOML_HAS_CXX11_ARRAY

    toml::value p(toml::array(0));
    toml::get<toml::array>(p).push_back(toml::value(3.14));
    toml::get<toml::array>(p).push_back(toml::value(2.71));
    std::pair<double, double> pr = toml::get<std::pair<double, double> >(p);
    BOOST_CHECK_EQUAL(3.14, pr.first);
    BOOST_CHECK_EQUAL(2.71, pr.second);
}

BOOST_AUTO_TEST_CASE(test_get_toml_array_of_array)
{
    toml::value v1(toml::array(0));
    toml::get<toml::array>(v1).push_back(toml::value(42));
    toml::get<toml::array>(v1).push_back(toml::value(54));
    toml::get<toml::array>(v1).push_back(toml::value(69));
    toml::get<toml::array>(v1).push_back(toml::value(72));

    toml::value v2(toml::array(0));
    toml::get<toml::array>(v2).push_back(toml::value("foo"));
    toml::get<toml::array>(v2).push_back(toml::value("bar"));
    toml::get<toml::array>(v2).push_back(toml::value("baz"));

    toml::value v(toml::array(2));
    toml::get<toml::array>(v).at(0) = v1;
    toml::get<toml::array>(v).at(1) = v2;

    std::pair<std::vector<int>, std::vector<std::string> > p =
        toml::get<std::pair<std::vector<int>, std::vector<std::string> > >(v);

    BOOST_CHECK_EQUAL(p.first.at(0), 42);
    BOOST_CHECK_EQUAL(p.first.at(1), 54);
    BOOST_CHECK_EQUAL(p.first.at(2), 69);
    BOOST_CHECK_EQUAL(p.first.at(3), 72);

    BOOST_CHECK_EQUAL(p.second.at(0), "foo");
    BOOST_CHECK_EQUAL(p.second.at(1), "bar");
    BOOST_CHECK_EQUAL(p.second.at(2), "baz");

#ifdef TOML_HAS_CXX11_TUPLE
    std::cerr << "c++11 tuple tested!" << std::endl;
    std::tuple<std::vector<int>, std::vector<std::string> > t =
        toml::get<std::tuple<std::vector<int>, std::vector<std::string> > >(v);

    BOOST_CHECK_EQUAL(std::get<0>(t).at(0), 42);
    BOOST_CHECK_EQUAL(std::get<0>(t).at(1), 54);
    BOOST_CHECK_EQUAL(std::get<0>(t).at(2), 69);
    BOOST_CHECK_EQUAL(std::get<0>(t).at(3), 72);

    BOOST_CHECK_EQUAL(std::get<1>(t).at(0), "foo");
    BOOST_CHECK_EQUAL(std::get<1>(t).at(1), "bar");
    BOOST_CHECK_EQUAL(std::get<1>(t).at(2), "baz");
#endif // TOML_HAS_CXX11_ARRAY
}

BOOST_AUTO_TEST_CASE(test_get_std_tm)
{
    {
        toml::value v1(toml::date(2018, 5, 31));
        toml::value v2(toml::date(2018, 5, 31),
                       toml::hours(12) + toml::minutes(34) + toml::seconds(56));
        toml::time_zone_ptr utc(new boost::local_time::posix_time_zone("UTC"));
        toml::value v3(toml::date(2018, 5, 31),
                       toml::hours(12) + toml::minutes(34) + toml::seconds(56),
                       utc);

        const std::tm t1 = toml::get<std::tm>(v1);
        BOOST_CHECK_EQUAL(t1.tm_year, 118);
        BOOST_CHECK_EQUAL(t1.tm_mon,    4); // XXX 0-11
        BOOST_CHECK_EQUAL(t1.tm_mday,  31);
        BOOST_CHECK_EQUAL(t1.tm_hour,   0);
        BOOST_CHECK_EQUAL(t1.tm_min,    0);
        BOOST_CHECK_EQUAL(t1.tm_sec,    0);
        const std::tm t2 = toml::get<std::tm>(v2);
        BOOST_CHECK_EQUAL(t2.tm_year, 118);
        BOOST_CHECK_EQUAL(t2.tm_mon,    4);
        BOOST_CHECK_EQUAL(t2.tm_mday,  31);
        BOOST_CHECK_EQUAL(t2.tm_hour,  12);
        BOOST_CHECK_EQUAL(t2.tm_min,   34);
        BOOST_CHECK_EQUAL(t2.tm_sec,   56);
        const std::tm t3 = toml::get<std::tm>(v3);
        BOOST_CHECK_EQUAL(t3.tm_year, 118);
        BOOST_CHECK_EQUAL(t3.tm_mon,    4);
        BOOST_CHECK_EQUAL(t3.tm_mday,  31);
        BOOST_CHECK_EQUAL(t3.tm_hour,  12);
        BOOST_CHECK_EQUAL(t3.tm_min,   34);
        BOOST_CHECK_EQUAL(t3.tm_sec,   56);
    }
}

#ifdef TOML_HAS_CXX11_CHRONO
BOOST_AUTO_TEST_CASE(test_get_chrono_time_point)
{
    {
        toml::value v1(toml::date(2018, 5, 31));
        toml::value v2(toml::date(2018, 5, 31),
                       toml::hours(12) + toml::minutes(34) + toml::seconds(56));
        toml::time_zone_ptr utc(new boost::local_time::posix_time_zone("UTC"));
        toml::value v3(toml::date(2018, 5, 31),
                       toml::hours(12) + toml::minutes(34) + toml::seconds(56),
                       utc);

        const std::time_t t1 = std::chrono::system_clock::to_time_t(
                toml::get<std::chrono::system_clock::time_point>(v1));
        std::tm* tm1  = std::gmtime(&t1);
        std::tm  tm1_ = toml::get<std::tm>(v1);
        BOOST_CHECK_EQUAL(tm1->tm_year, tm1_.tm_year);
        BOOST_CHECK_EQUAL(tm1->tm_mon,  tm1_.tm_mon);
        BOOST_CHECK_EQUAL(tm1->tm_mday, tm1_.tm_mday);
        BOOST_CHECK_EQUAL(tm1->tm_hour, tm1_.tm_hour);
        BOOST_CHECK_EQUAL(tm1->tm_min,  tm1_.tm_min);
        BOOST_CHECK_EQUAL(tm1->tm_sec,  tm1_.tm_sec);


        const std::time_t t2 = std::chrono::system_clock::to_time_t(
                toml::get<std::chrono::system_clock::time_point>(v2));
        std::tm* tm2 = std::gmtime(&t2);
        std::tm  tm2_ = toml::get<std::tm>(v2);
        BOOST_CHECK_EQUAL(tm2->tm_year, tm2_.tm_year);
        BOOST_CHECK_EQUAL(tm2->tm_mon,  tm2_.tm_mon);
        BOOST_CHECK_EQUAL(tm2->tm_mday, tm2_.tm_mday);
        BOOST_CHECK_EQUAL(tm2->tm_hour, tm2_.tm_hour);
        BOOST_CHECK_EQUAL(tm2->tm_min,  tm2_.tm_min);
        BOOST_CHECK_EQUAL(tm2->tm_sec,  tm2_.tm_sec);

        const std::time_t t3 = std::chrono::system_clock::to_time_t(
                toml::get<std::chrono::system_clock::time_point>(v3));
        std::tm* tm3 = std::gmtime(&t3);
        std::tm  tm3_ = toml::get<std::tm>(v3);
        BOOST_CHECK_EQUAL(tm3->tm_year, tm3_.tm_year);
        BOOST_CHECK_EQUAL(tm3->tm_mon,  tm3_.tm_mon);
        BOOST_CHECK_EQUAL(tm3->tm_mday, tm3_.tm_mday);
        BOOST_CHECK_EQUAL(tm3->tm_hour, tm3_.tm_hour);
        BOOST_CHECK_EQUAL(tm3->tm_min,  tm3_.tm_min);
        BOOST_CHECK_EQUAL(tm3->tm_sec,  tm3_.tm_sec);
    }
}
#endif // TOML_HAS_CXX11_CHRONO

