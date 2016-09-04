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
  struct function_parser : qi::grammar<Iterator, program(), space_type>
  {
    function_parser() : function_parser::base_type(program_rule)
    {
      using qi::lit;

      parameter_rule %=
        +alnum % ','
        >> ':'
        >> numeric_types
        >> -char_(',');

      assignment_statement_rule %=
        +alnum
        >> '='
        >> +(char_ - ';')
        >> ';';

      function_rule %= lit("void ")
        >> +(char_ - '(')
        >> '('
        >> *parameter_rule
        >> ')'
        >> '{'
        >> *assignment_statement_rule
        >> '}';

      interface_declaration_rule %= 
        lit("interface ") >> +(char_ - '{')
        >> '{'
        >> *assignment_statement_rule
        >> '}';

      program_rule %= boost::spirit::eps >> *(function_rule | assignment_statement_rule | interface_declaration_rule);
    }

    qi::rule<Iterator, parameter(), space_type> parameter_rule;
    qi::rule<Iterator, assignment_statement(), space_type> assignment_statement_rule;
    qi::rule<Iterator, function(), space_type> function_rule;
    qi::rule<Iterator, interface_declaration(), space_type> interface_declaration_rule;
    qi::rule<Iterator, program(), space_type> program_rule;
  };
  
  // relies on boost fusion also
  template<typename Iterator>
  wstring parse(Iterator first, Iterator last)
  {
    using boost::spirit::qi::phrase_parse;

    program p;
    function_parser<wstring::const_iterator> fp{};
    auto b = phrase_parse(first, last, fp, space, p);
    if (b)
    {
      wostringstream buffer;
      p.render(buffer, rendering_options{});
      return buffer.str();
    }
    return wstring(L"FAIL");
  }
}