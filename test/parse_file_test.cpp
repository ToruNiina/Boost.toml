#define BOOST_TEST_MODULE "lex_datetime_test"
#include <toml/toml.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>

BOOST_AUTO_TEST_CASE(test_sample_toml)
{
    const toml::table data = toml::parse("toml/tests/example.toml");

    BOOST_CHECK(data.count("title") == 1);
    BOOST_CHECK(data.at("title").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(data.at("title")) == "TOML Example");


    BOOST_CHECK(data.count("owner") == 1);
    BOOST_CHECK(data.at("owner").is(toml::value::table_tag));
    const toml::table& owner = toml::get<toml::table>(data.at("owner"));

    BOOST_CHECK(owner.count("name") == 1);
    BOOST_CHECK(owner.at("name").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(owner.at("name")) == "Tom Preston-Werner");

    BOOST_CHECK(owner.count("organization") == 1);
    BOOST_CHECK(owner.at("organization").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(owner.at("organization")) == "GitHub");

    BOOST_CHECK(owner.count("bio") == 1);
    BOOST_CHECK(owner.at("bio").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(owner.at("bio")) == "GitHub Cofounder & CEO\nLikes tater tots and beer.");

    BOOST_CHECK(owner.count("dob") == 1);
    BOOST_CHECK(owner.at("dob").is(toml::value::offset_datetime_tag));
    toml::offset_datetime dob(toml::local_datetime(
            toml::date(1979, 5, 27), toml::hours(7) + toml::minutes(32)),
            toml::time_zone_ptr(new boost::local_time::posix_time_zone("UTC")));
    BOOST_CHECK(toml::get<toml::offset_datetime>(owner.at("dob")) == dob);



    BOOST_CHECK(data.count("database") == 1);
    BOOST_CHECK(data.at("database").is(toml::value::table_tag));
    const toml::table& database = toml::get<toml::table>(data.at("database"));

    BOOST_CHECK(database.count("server") == 1);
    BOOST_CHECK(database.at("server").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(database.at("server")) == "192.168.1.1");

    BOOST_CHECK(database.count("ports") == 1);
    BOOST_CHECK(database.at("ports").is(toml::value::array_tag));
    std::vector<int> ports(3);
    ports[0] = 8001;
    ports[1] = 8001;
    ports[2] = 8002;
    BOOST_CHECK(toml::get<std::vector<int> >(database.at("ports")) == ports);

    BOOST_CHECK(database.count("connection_max") == 1);
    BOOST_CHECK(database.at("connection_max").is(toml::value::integer_tag));
    BOOST_CHECK(toml::get<int>(database.at("connection_max")) == 5000);

    BOOST_CHECK(database.count("enabled") == 1);
    BOOST_CHECK(database.at("enabled").is(toml::value::boolean_tag));
    BOOST_CHECK(toml::get<bool >(database.at("enabled")) == true);



    BOOST_CHECK(data.count("servers") == 1);
    BOOST_CHECK(data.at("servers").is(toml::value::table_tag));
    const toml::table& servers = toml::get<toml::table>(data.at("servers"));

    BOOST_CHECK(servers.count("alpha") == 1);
    BOOST_CHECK(servers.at("alpha").is(toml::value::table_tag));
    const toml::table& alpha = toml::get<toml::table>(servers.at("alpha"));

    BOOST_CHECK(alpha.count("ip") == 1);
    BOOST_CHECK(alpha.at("ip").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(alpha.at("ip")) == "10.0.0.1");

    BOOST_CHECK(alpha.count("dc") == 1);
    BOOST_CHECK(alpha.at("dc").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(alpha.at("dc")) == "eqdc10");

    BOOST_CHECK(servers.count("beta") == 1);
    BOOST_CHECK(servers.at("beta").is(toml::value::table_tag));
    const toml::table& beta = toml::get<toml::table>(servers.at("beta"));

    BOOST_CHECK(beta.count("ip") == 1);
    BOOST_CHECK(beta.at("ip").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(beta.at("ip")) == "10.0.0.2");

    BOOST_CHECK(beta.count("dc") == 1);
    BOOST_CHECK(beta.at("dc").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(beta.at("dc")) == "eqdc10");

    BOOST_CHECK(beta.count("country") == 1);
    BOOST_CHECK(beta.at("country").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(beta.at("country")) == "中国");



    BOOST_CHECK(data.count("clients") == 1);
    BOOST_CHECK(data.at("clients").is(toml::value::table_tag));
    const toml::table& clients = toml::get<toml::table>(data.at("clients"));

    BOOST_CHECK(clients.count("data") == 1);
    BOOST_CHECK(clients.at("data").is(toml::value::array_tag));
    std::vector<std::string> data_1(2);
    data_1[0] = "gamma";
    data_1[1] = "delta";
    std::vector<int> data_2(2);
    data_2[0] = 1;
    data_2[1] = 2;
    std::pair<std::vector<std::string>, std::vector<int> > data_ =
        toml::get<std::pair<std::vector<std::string>, std::vector<int> > >(
            clients.at("data"));
    BOOST_CHECK(data_ == std::make_pair(data_1, data_2));

    BOOST_CHECK(clients.count("hosts") == 1);
    BOOST_CHECK(clients.at("hosts").is(toml::value::array_tag));
    std::vector<std::string> hosts(2);
    hosts[0] = "alpha";
    hosts[1] = "omega";
    BOOST_CHECK(toml::get<std::vector<std::string> >(clients.at("hosts")) ==
                hosts);


    BOOST_CHECK(data.count("products") == 1);
    BOOST_CHECK(data.at("products").is(toml::value::array_tag));

    toml::table products1;
    products1["name"] = "Hammer";
    products1["sku"]  = 738594937;
    toml::table products2;
    products2["name"]  = "Nail";
    products2["sku"]   = 284758393;
    products2["color"] = "gray";

    BOOST_CHECK(toml::get<std::vector<toml::table> >(data.at("products")).at(0)
            == products1);
    BOOST_CHECK(toml::get<std::vector<toml::table> >(data.at("products")).at(1)
            == products2);
}

BOOST_AUTO_TEST_CASE(test_fruits_toml)
{
    const toml::table data = toml::parse("toml/tests/fruit.toml");

    BOOST_CHECK(data.count("fruit") == 1);
    BOOST_CHECK(data.at("fruit").is(toml::value::table_tag));
    const toml::table& fruit = toml::get<toml::table>(data.at("fruit"));

    BOOST_CHECK(fruit.count("blah") == 1);
    BOOST_CHECK(fruit.at("blah").is(toml::value::array_tag));
    const std::vector<toml::table> blah =
        toml::get<std::vector<toml::table> >(fruit.at("blah"));
    BOOST_CHECK(blah.size() == 2);


    BOOST_CHECK(blah.at(0).count("name") == 1);
    BOOST_CHECK(blah.at(0).at("name").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(blah.at(0).at("name")) == "apple");

    BOOST_CHECK(blah.at(0).count("physical") == 1);
    BOOST_CHECK(blah.at(0).at("physical").is(toml::value::table_tag));
    const toml::table& physical0 = toml::get<toml::table>(blah.at(0).at("physical"));

    BOOST_CHECK(physical0.count("color") == 1);
    BOOST_CHECK(physical0.at("color").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(physical0.at("color")) == "red");

    BOOST_CHECK(physical0.count("shape") == 1);
    BOOST_CHECK(physical0.at("shape").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(physical0.at("shape")) == "round");


    BOOST_CHECK(blah.at(1).count("name") == 1);
    BOOST_CHECK(blah.at(1).at("name").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(blah.at(1).at("name")) == "banana");

    BOOST_CHECK(blah.at(1).count("physical") == 1);
    BOOST_CHECK(blah.at(1).at("physical").is(toml::value::table_tag));
    const toml::table& physical1 = toml::get<toml::table>(blah.at(1).at("physical"));

    BOOST_CHECK(physical1.count("color") == 1);
    BOOST_CHECK(physical1.at("color").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(physical1.at("color")) == "yellow");

    BOOST_CHECK(physical1.count("shape") == 1);
    BOOST_CHECK(physical1.at("shape").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(physical1.at("shape")) == "bent");
}


BOOST_AUTO_TEST_CASE(test_hard_example_toml)
{
    const toml::table data = toml::parse("toml/tests/hard_example.toml");

    BOOST_CHECK(data.count("the") == 1);
    BOOST_CHECK(data.at("the").is(toml::value::table_tag));
    const toml::table& the = toml::get<toml::table>(data.at("the"));

    BOOST_CHECK(the.count("test_string") == 1);
    BOOST_CHECK(the.at("test_string").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(the.at("test_string")) == "You'll hate me after this - #");

    BOOST_CHECK(the.count("hard") == 1);
    BOOST_CHECK(the.at("hard").is(toml::value::table_tag));
    const toml::table& hard = toml::get<toml::table>(data.at("hard"));

    BOOST_CHECK(hard.count("test_array") == 1);
    BOOST_CHECK(hard.at("test_array").is(toml::value::array_tag));
    std::vector<std::string> test_array(2);
    test_array[0] = "] ";
    test_array[1] = " # ";
    BOOST_CHECK(toml::get<std::vector<std::string> >(hard.at("test_array"))
                == test_array);

    BOOST_CHECK(hard.count("test_array2") == 1);
    BOOST_CHECK(hard.at("test_array2").is(toml::value::array_tag));
    std::vector<std::string> test_array2(2);
    test_array2[0] = "Test #11 ]proved that";
    test_array2[1] = "Experiment #9 was a success";
    BOOST_CHECK(toml::get<std::vector<std::string> >(hard.at("test_array2"))
                == test_array2);

    BOOST_CHECK(hard.count("another_test_string") == 1);
    BOOST_CHECK(hard.at("another_test_string").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(hard.at("another_test_string")) ==
                " Same thing, but with a string #");

    BOOST_CHECK(hard.count("harder_test_string") == 1);
    BOOST_CHECK(hard.at("harder_test_string").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(hard.at("harder_test_string")) ==
                " And when \"'s are in the string, along with # \"");



    BOOST_CHECK(the.count("bit#") == 1);
    BOOST_CHECK(the.at("bit#").is(toml::value::table_tag));
    const toml::table& bit = toml::get<toml::table>(data.at("bit#"));

    BOOST_CHECK(bit.count("what?") == 1);
    BOOST_CHECK(bit.at("what?").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(bit.at("what?")) ==
                "You don't think some user won't do that?");

    BOOST_CHECK(bit.count("multi_line_array") == 1);
    BOOST_CHECK(bit.at("multi_line_array").is(toml::value::array_tag));
    std::vector<std::string> multi_line_array(1);
    multi_line_array[0] = "]";
    BOOST_CHECK(toml::get<std::vector<std::string> >(bit.at("multi_line_array"))
                == multi_line_array);
}

BOOST_AUTO_TEST_CASE(test_hard_example_unicode_toml)
{
    const toml::table data = toml::parse("toml/tests/hard_example_unicode.toml");

    BOOST_CHECK(data.count("the") == 1);
    BOOST_CHECK(data.at("the").is(toml::value::table_tag));
    const toml::table& the = toml::get<toml::table>(data.at("the"));

    BOOST_CHECK(the.count("test_string") == 1);
    BOOST_CHECK(the.at("test_string").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(the.at("test_string")) == "Ýôú'ℓℓ λáƭè ₥è áƒƭèř ƭλïƨ - #");

    BOOST_CHECK(the.count("hard") == 1);
    BOOST_CHECK(the.at("hard").is(toml::value::table_tag));
    const toml::table& hard = toml::get<toml::table>(data.at("hard"));

    BOOST_CHECK(hard.count("test_array") == 1);
    BOOST_CHECK(hard.at("test_array").is(toml::value::array_tag));
    std::vector<std::string> test_array(2);
    test_array[0] = "] ";
    test_array[1] = " # ";
    BOOST_CHECK(toml::get<std::vector<std::string> >(hard.at("test_array"))
                == test_array);

    BOOST_CHECK(hard.count("test_array2") == 1);
    BOOST_CHECK(hard.at("test_array2").is(toml::value::array_tag));
    std::vector<std::string> test_array2(2);
    test_array2[0] = "Tèƨƭ #11 ]ƥřôƲèδ ƭλáƭ";
    test_array2[1] = "Éжƥèřï₥èñƭ #9 ωáƨ á ƨúççèƨƨ";
    BOOST_CHECK(toml::get<std::vector<std::string> >(hard.at("test_array2"))
                == test_array2);

    BOOST_CHECK(hard.count("another_test_string") == 1);
    BOOST_CHECK(hard.at("another_test_string").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(hard.at("another_test_string")) ==
                "§á₥è ƭλïñϱ, βúƭ ωïƭλ á ƨƭřïñϱ #");

    BOOST_CHECK(hard.count("harder_test_string") == 1);
    BOOST_CHECK(hard.at("harder_test_string").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(hard.at("harder_test_string")) ==
                " Âñδ ωλèñ \"'ƨ ářè ïñ ƭλè ƨƭřïñϱ, áℓôñϱ ωïƭλ # \"");



    BOOST_CHECK(the.count("βïƭ#") == 1);
    BOOST_CHECK(the.at("βïƭ#").is(toml::value::table_tag));
    const toml::table& bit = toml::get<toml::table>(data.at("βïƭ#"));

    BOOST_CHECK(bit.count("ωλáƭ?") == 1);
    BOOST_CHECK(bit.at("ωλáƭ?").is(toml::value::string_tag));
    BOOST_CHECK(toml::get<std::string>(bit.at("ωλáƭ?")) ==
                "Ýôú δôñ'ƭ ƭλïñƙ ƨô₥è úƨèř ωôñ'ƭ δô ƭλáƭ?");

    BOOST_CHECK(bit.count("multi_line_array") == 1);
    BOOST_CHECK(bit.at("multi_line_array").is(toml::value::array_tag));
    std::vector<std::string> multi_line_array(1);
    multi_line_array[0] = "]";
    BOOST_CHECK(toml::get<std::vector<std::string> >(bit.at("multi_line_array"))
                == multi_line_array);
}
