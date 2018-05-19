//              Copyright Toru Niina 2018.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
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

typedef either<character<' '>, character<'\t'> > lex_wschar;
typedef repeat<lex_wschar, at_least<1> > lex_ws;

typedef either<character<'\n'>, sequence<character<'\r'>, character<'\n'> >
    > lex_newline;

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

typedef character<'"'> lex_quotation_mark;
typedef exclude<either<in_range<0x00, 0x1F>, either<character<0x22>,
        either<character<0x5C>, character<0x7F> > > >
    > lex_basic_unescaped;
typedef character<'\\'> lex_escape;
typedef either<character<'"'>,
        either<character<'\\'>,
        either<character<'/'>,
        either<character<'b'>,
        either<character<'f'>,
        either<character<'n'>,
        either<character<'r'>,
        either<character<'t'>,
        either<sequence<character<'u'>, repeat<lex_hex_dig, exactly<4> > >,
               sequence<character<'U'>, repeat<lex_hex_dig, exactly<8> > >
        > > > > > > > >
    > lex_escape_seq_char;
typedef sequence<lex_escape, lex_escape_seq_char> lex_escaped;
typedef either<lex_basic_unescaped, lex_escaped> lex_basic_char;
typedef sequence<lex_quotation_mark, sequence<repeat<lex_basic_char, unlimited>,
            lex_quotation_mark>
    > lex_basic_string;

typedef repeat<lex_quotation_mark, exactly<3>
    > lex_ml_basic_string_delim;
typedef exclude<either<in_range<0x00, 0x1F>,
        either<character<0x5C>, character<0x7F> > >
    > lex_ml_basic_unescaped;
typedef either<lex_ml_basic_unescaped, lex_escaped
    > lex_ml_basic_char;
typedef repeat<either<lex_ml_basic_char, either<lex_newline,
        sequence<lex_escape, sequence<lex_ws, lex_newline> > > >, unlimited
    > lex_ml_basic_body;
typedef sequence<lex_ml_basic_string_delim, sequence<lex_ml_basic_body,
        lex_ml_basic_string_delim>
    > lex_ml_basic_string;

typedef exclude<either<in_range<0x00, 0x08>,
        either<in_range<0x10, 0x19>, character<0x27> > >
    > lex_literal_char;
typedef character<'\''> lex_apostrophe;
typedef sequence<lex_apostrophe, sequence<repeat<lex_literal_char, unlimited>,
            lex_apostrophe>
    > lex_literal_string;

typedef repeat<lex_apostrophe, exactly<3>
    > lex_ml_literal_string_delim;
typedef exclude<either<in_range<0x00, 0x08>, in_range<0x10, 0x1F> >
    > lex_ml_literal_char;
typedef repeat<either<lex_ml_literal_char, lex_newline>, unlimited
    > lex_ml_literal_body;
typedef sequence<lex_ml_literal_string_delim, sequence<lex_ml_literal_char,
        lex_ml_literal_string_delim>
    > lex_ml_literal_string;

typedef either<either<lex_ml_basic_string,   lex_basic_string>,
               either<lex_ml_literal_string, lex_literal_string>
    > lex_string;

// ===========================================================================

typedef character<'#'> lex_comment_start_symbol;
typedef either<character<'\t'>, exclude<in_range<0x00, 0x19> >
    > lex_non_eol;
typedef sequence<lex_comment_start_symbol, repeat<lex_non_eol, unlimited>
    > lex_comment;

typedef sequence<lex_ws, sequence<character<'.'>, lex_ws> > lex_dot_sep;

typedef repeat<either<lex_alpha, either<lex_digit,
        either<character<'-'>, character<'_'> > > >, at_least<1>
    > lex_unquoted_key;
typedef either<lex_basic_string, lex_literal_string> lex_quoted_key;
typedef either<lex_unquoted_key, lex_unquoted_key> lex_simple_key;
typedef sequence<lex_simple_key,
        repeat<sequence<lex_dot_sep, lex_simple_key>, at_least<1> >
    > lex_dotted_key;
typedef either<lex_simple_key, lex_dotted_key> lex_key;

typedef sequence<maybe<lex_ws>, sequence<character<'='>, maybe<lex_ws> >
    > lex_keyval_sep;

typedef character<'['> lex_std_table_open;
typedef character<']'> lex_std_table_close;
typedef sequence<lex_std_table_open, sequence<lex_key, lex_std_table_close>
    > lex_std_table;

typedef sequence<lex_std_table_open,  lex_std_table_open>  lex_array_table_open;
typedef sequence<lex_std_table_close, lex_std_table_close
    > lex_array_table_close;
typedef sequence<lex_array_table_open, sequence<lex_key, lex_array_table_close>
    > lex_array_table;

} // detail
} // toml
#endif // TOML_LEXER_HPP
