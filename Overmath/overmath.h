#pragma once

#include <string>
#include <vector>
#define BOOST_SPIRIT_UNICODE
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/optional.hpp>

namespace overmath
{
  using namespace std;
  namespace qi = boost::spirit::qi;
  using namespace boost::spirit::unicode;

  struct parameter
  {
    vector<wstring> names;
    wstring type;
  };

  struct assignment_statement
  {
    wstring variable_being_assigned;
    wstring value;

    wstring infer_type() const
    {
      if (value.find(L'.') == wstring::npos)
        return L"int"s;
      return L"float"s;
    }
  };

  struct function
  {
    wstring name;
    vector<parameter> params;
    vector<assignment_statement> assignments;

    boost::optional<const parameter&> find_parameter(const wstring& name) const
    {
      auto found = find_if(begin(params), end(params),
        [&](const parameter& p)
      {
        return find(begin(p.names), end(p.names), name) != end(p.names);
      });
      if (found == end(params)) return boost::none;
      return *found;
    }
  };

  struct numeric_types_ : qi::symbols<wchar_t, wstring>
  {
    numeric_types_()
    {
      add(L"i32", L"int32_t");
      add(L"f32", L"float");
      add(L"f64", L"double");
    }
  } numeric_types;
}

BOOST_FUSION_ADAPT_STRUCT(
  overmath::function,
  (std::wstring, name)
  (std::vector<overmath::parameter>, params)
  (std::vector<overmath::assignment_statement>, assignments)
)

BOOST_FUSION_ADAPT_STRUCT(
  overmath::parameter,
  (std::vector<std::wstring>, names)
  (std::wstring, type)
)

BOOST_FUSION_ADAPT_STRUCT(
  overmath::assignment_statement,
  (std::wstring, variable_being_assigned)
  (std::wstring, value)
)

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
        >> +alnum
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

  inline wstring render(const function& f)
  {
    wostringstream s;

    // name of the function (assume void-returning)
    s << "void " << f.name << "(";

    // each of the parameters
    const auto param_count = f.params.size();
    for (auto i = 0u; i < param_count; ++i)
    {
      auto& p = f.params[i];
      for (int j = 0; j < p.names.size(); ++j)
      {
        s << p.type << " " << p.names[j];
        if (j + 1 < p.names.size()) s << ", ";
      }
      if (i + 1 < param_count) s << ", ";
    }
    s << ")\r\n{\r\n";

    // each of the assignments (later)
    const auto assign_count = f.assignments.size();
    for (auto i = 0u; i < assign_count; ++i)
    {
      s << "  ";

      auto& a = f.assignments[i];
      auto type = a.infer_type();

      bool is_param = f.find_parameter(a.variable_being_assigned) != boost::none;
      if (!is_param)
        s << type << " ";
      s << a.variable_being_assigned << " = " << a.value << ";\r\n";
    }

    s << "}";
    return s.str();
  }


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
      return render(f);
    }
    return wstring(L"FAIL");
  }
}