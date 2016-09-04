#pragma once

#include <string>
#include <vector>
#define BOOST_SPIRIT_UNICODE
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/optional.hpp>

#include "ast.h"

namespace overmath
{
  using namespace boost::spirit::unicode;
  namespace qi = boost::spirit::qi;

  template<typename Iterator>
  struct function_parser : qi::grammar<Iterator, function(), space_type>
  {
    function_parser() : function_parser::base_type(start)
    {
      using qi::lit;

      param %=
        +alnum % ','
        >> ':'
        >> numeric_types
        >> -char_(',');

      assignment %=
        +alnum
        >> '='
        >> +(char_ - ';')
        >> ';';

      start %= lit("void ")
        >> +(char_ - '(')
        >> '('
        >> *param
        >> ')'
        >> '{'
        >> *assignment
        >> '}';
    }

    qi::rule<Iterator, parameter(), space_type> param;
    qi::rule<Iterator, assignment_statement(), space_type> assignment;
    qi::rule<Iterator, function(), space_type> start;
  };
  
  // relies on boost fusion also
  template<typename Iterator>
  wstring parse(Iterator first, Iterator last)
  {
    using boost::spirit::qi::phrase_parse;

    function f;
    function_parser<wstring::const_iterator> fp{};
    auto b = phrase_parse(first, last, fp, space, f);
    if (b)
    {
      wostringstream buffer;
      f.render(buffer, rendering_options{});
      return buffer.str();
    }
    return wstring(L"FAIL");
  }
}