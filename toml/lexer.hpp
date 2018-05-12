#ifndef TOML_LEXER_HPP
#define TOML_LEXER_HPP
#include <toml/combinator.hpp>
#include <boost/static_assert.hpp>
#include <boost/optional.hpp>
#include <stdexcept>
#include <istream>
#include <sstream>
#include <fstream>

namespace toml
{
namespace detail
{

typedef in_range<'a', 'z'> lex_lower;
typedef in_range<'A', 'Z'> lex_upper;
typedef either<lex_lower, lex_upper> lex_alpha;

typedef in_range<'0', '9'> lex_digit;
typedef in_range<'1', '9'> lex_nonzero;
typedef in_range<'0', '7'> lex_oct_dig;
typedef in_range<'0', '1'> lex_bin_dig;
typedef either<lex_digit, either<in_range<'A', 'F'>, in_range<'a', 'f'> > >
        lex_hex_dig;

typedef sequence<character<'0'>, character<'x'> > lex_hex_prefix;
typedef sequence<character<'0'>, character<'o'> > lex_oct_prefix;
typedef sequence<character<'0'>, character<'b'> > lex_bin_prefix;
typedef character<'_'> lex_underscore;
typedef character<'+'> lex_plus;
typedef character<'-'> lex_minus;
typedef either<lex_plus, lex_minus> lex_sign;

// digit | nonzero 1*(digit | _ digit)
typedef either<sequence<lex_nonzero, repeat<
    either<lex_digit, sequence<lex_underscore, lex_digit> >, at_least<1> > >,
    lex_digit
    > lex_unsigned_dec_int;
// (+|-)? unsigned_dec_int
typedef sequence<maybe<lex_sign>, lex_unsigned_dec_int
    > lex_dec_int;
// hex_prefix hex_dig *(hex_dig | _ hex_dig)
typedef sequence<lex_hex_prefix, sequence<lex_hex_dig, repeat<
    either<lex_hex_dig, sequence<lex_underscore, lex_hex_dig> >, unlimited> >
    > lex_hex_int;
// oct_prefix oct_dig *(oct_dig | _ oct_dig)
typedef sequence<lex_oct_prefix, sequence<lex_oct_dig, repeat<
    either<lex_oct_dig, sequence<lex_underscore, lex_oct_dig> >, unlimited> >
    > lex_oct_int;
// bin_prefix bin_dig *(bin_dig | _ bin_dig)
typedef sequence<lex_bin_prefix, sequence<lex_bin_dig, repeat<
    either<lex_bin_dig, sequence<lex_underscore, lex_bin_dig> >, unlimited> >
    > lex_bin_int;
// (dec_int | hex_int | oct_int | bin_int)
typedef either<lex_bin_int, either<lex_oct_int, either<lex_hex_int, lex_dec_int>
    > > lex_integer;

// ===========================================================================

typedef sequence<character<'i'>, sequence<character<'n'>, character<'f'> >
    > lex_inf;
typedef sequence<character<'n'>, sequence<character<'a'>, character<'n'> >
    > lex_nan;
typedef sequence<maybe<lex_sign>, either<lex_inf, lex_nan>
    > lex_special_float;

typedef sequence<maybe<either<character<'e'>, character<'E'> > >, lex_dec_int
    > lex_exponent_part;

typedef sequence<lex_digit,
    repeat<either<lex_digit, sequence<lex_underscore, lex_digit> >, unlimited>
    > lex_zero_prefixable_int;
typedef sequence<character<'.'>, lex_zero_prefixable_int
    > lex_fractional_part;

typedef either<lex_special_float,
    sequence<lex_dec_int, either<lex_exponent_part,
        sequence<lex_fractional_part, maybe<lex_exponent_part> > > >
    > lex_float;

// ===========================================================================

typedef sequence<character<'t'>, sequence<character<'r'>,
        sequence<character<'u'>, character<'e'> > >
    > lex_true;
typedef sequence<character<'f'>, sequence<character<'a'>,
        sequence<character<'l'>, sequence<character<'s'>, character<'e'> > > >
    > lex_false;
typedef either<lex_true, lex_false> lex_boolean;

// ===========================================================================

typedef repeat<lex_digit, exactly<4> > lex_date_fullyear;
typedef repeat<lex_digit, exactly<2> > lex_date_month;
typedef repeat<lex_digit, exactly<2> > lex_date_mday;

typedef either<character<'T'>, either<character<'t'>, character<' '> >
    > lex_time_delim;
typedef repeat<lex_digit, exactly<2> > lex_time_hour;
typedef repeat<lex_digit, exactly<2> > lex_time_minute;
typedef repeat<lex_digit, exactly<2> > lex_time_second;
typedef sequence<character<'.'>, repeat<lex_digit, at_least<1> >
    > lex_time_secfrac;
typedef sequence<either<character<'+'>, character<'-'> >,
        sequence<lex_time_hour, sequence<character<':'>, lex_time_minute> >
    > lex_time_numoffset;
typedef either<either<character<'Z'>, character<'z'> >, lex_time_numoffset
    > lex_time_offset;

typedef sequence<lex_time_hour, sequence<
    character<':'>, sequence<lex_time_minute, sequence<character<':'>,
    sequence<lex_time_second, maybe<lex_time_secfrac> >
    > > > > lex_partial_time;
typedef sequence<lex_date_fullyear, sequence<character<'-'>, sequence<
    lex_date_month, sequence<character<'-'>, lex_date_mday>
    > > > lex_full_date;
typedef sequence<lex_partial_time, lex_time_offset
    > lex_full_time;

typedef sequence<lex_full_date, sequence<lex_time_delim, lex_full_time>
    > lex_offset_date_time;
typedef sequence<lex_full_date, sequence<lex_time_delim, lex_partial_time>
    > lex_local_date_time;
typedef lex_full_date lex_local_date;
typedef lex_partial_time lex_local_time;

// ===========================================================================





















} // detail
} // toml
#endif // TOML_LEXER_HPP
