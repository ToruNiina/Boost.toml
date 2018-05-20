#define BOOST_TEST_MODULE "lex_floating_test"
#include <toml/parser.hpp>
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <iomanip>
#include <limits>
#include "lex_aux.hpp"

using namespace toml;
using namespace detail;

BOOST_AUTO_TEST_CASE(test_fractional_valid)
{
    TOML_LEX_CHECK_ACCEPT(lex_float, "1.0",               "1.0"              );
    TOML_LEX_CHECK_ACCEPT(lex_float, "0.1",               "0.1"              );
    TOML_LEX_CHECK_ACCEPT(lex_float, "0.001",             "0.001"            );
    TOML_LEX_CHECK_ACCEPT(lex_float, "0.100",             "0.100"            );
    TOML_LEX_CHECK_ACCEPT(lex_float, "+3.14",             "+3.14"            );
    TOML_LEX_CHECK_ACCEPT(lex_float, "-3.14",             "-3.14"            );
    TOML_LEX_CHECK_ACCEPT(lex_float, "3.1415_9265_3589",  "3.1415_9265_3589" );
    TOML_LEX_CHECK_ACCEPT(lex_float, "+3.1415_9265_3589", "+3.1415_9265_3589");
    TOML_LEX_CHECK_ACCEPT(lex_float, "-3.1415_9265_3589", "-3.1415_9265_3589");
    TOML_LEX_CHECK_ACCEPT(lex_float, "123_456.789",       "123_456.789"      );
    TOML_LEX_CHECK_ACCEPT(lex_float, "+123_456.789",      "+123_456.789"     );
    TOML_LEX_CHECK_ACCEPT(lex_float, "-123_456.789",      "-123_456.789"     );
}

BOOST_AUTO_TEST_CASE(test_fractional_invalid)
{
    TOML_LEX_CHECK_REJECT(lex_float, "0.");
    TOML_LEX_CHECK_REJECT(lex_float, ".0");
    TOML_LEX_CHECK_REJECT(lex_float, "01.0");
    TOML_LEX_CHECK_REJECT(lex_float, "3,14");
    TOML_LEX_CHECK_REJECT(lex_float, "+-1.0");
    TOML_LEX_CHECK_REJECT(lex_float, "1._0");
}

BOOST_AUTO_TEST_CASE(test_exponential_valid)
{
    TOML_LEX_CHECK_ACCEPT(lex_float, "1e10",       "1e10");      
    TOML_LEX_CHECK_ACCEPT(lex_float, "1e+10",      "1e+10");     
    TOML_LEX_CHECK_ACCEPT(lex_float, "1e-10",      "1e-10");     
    TOML_LEX_CHECK_ACCEPT(lex_float, "+1e10",      "+1e10");     
    TOML_LEX_CHECK_ACCEPT(lex_float, "+1e+10",     "+1e+10");    
    TOML_LEX_CHECK_ACCEPT(lex_float, "+1e-10",     "+1e-10");    
    TOML_LEX_CHECK_ACCEPT(lex_float, "-1e10",      "-1e10");     
    TOML_LEX_CHECK_ACCEPT(lex_float, "-1e+10",     "-1e+10");    
    TOML_LEX_CHECK_ACCEPT(lex_float, "-1e-10",     "-1e-10");    
    TOML_LEX_CHECK_ACCEPT(lex_float, "123e-10",    "123e-10");   
    TOML_LEX_CHECK_ACCEPT(lex_float, "1E10",       "1E10");      
    TOML_LEX_CHECK_ACCEPT(lex_float, "1E+10",      "1E+10");     
    TOML_LEX_CHECK_ACCEPT(lex_float, "1E-10",      "1E-10");     
    TOML_LEX_CHECK_ACCEPT(lex_float, "123E-10",    "123E-10");   
    TOML_LEX_CHECK_ACCEPT(lex_float, "1_2_3E-10",  "1_2_3E-10"); 
    TOML_LEX_CHECK_ACCEPT(lex_float, "1_2_3E-1_0", "1_2_3E-1_0");
}

BOOST_AUTO_TEST_CASE(test_exponential_invalid)
{
    TOML_LEX_CHECK_ACCEPT(lex_float, "1e1E0", "1e1"); 
    TOML_LEX_CHECK_ACCEPT(lex_float, "1E1e0", "1E1");
}

BOOST_AUTO_TEST_CASE(test_both_valid)
{
    TOML_LEX_CHECK_ACCEPT(lex_float, "6.02e23",          "6.02e23");
    TOML_LEX_CHECK_ACCEPT(lex_float, "6.02e+23",         "6.02e+23");
    TOML_LEX_CHECK_ACCEPT(lex_float, "1.112_650_06e-17", "1.112_650_06e-17");
}

BOOST_AUTO_TEST_CASE(test_both_invalid)
{
    TOML_LEX_CHECK_ACCEPT(lex_float, "1e1.0",  "1e1");
    TOML_LEX_CHECK_REJECT(lex_float, "01e1.0");
}

BOOST_AUTO_TEST_CASE(test_special_floating_point)
{
    TOML_LEX_CHECK_ACCEPT(lex_float,  "inf",  "inf");
    TOML_LEX_CHECK_ACCEPT(lex_float, "+inf", "+inf");
    TOML_LEX_CHECK_ACCEPT(lex_float, "-inf", "-inf");

    TOML_LEX_CHECK_ACCEPT(lex_float,  "nan",  "nan");
    TOML_LEX_CHECK_ACCEPT(lex_float, "+nan", "+nan");
    TOML_LEX_CHECK_ACCEPT(lex_float, "-nan", "-nan");
}
