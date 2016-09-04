#pragma once

#include "rendering.h"

namespace overmath
{
  using namespace std;
  namespace qi = boost::spirit::qi;
  using namespace boost::spirit::unicode;

  struct element
  {
    virtual ~element() = default;
    virtual void render(wostringstream& buffer, const rendering_options& options) = 0;
  };

  struct parameter : element
  {
    vector<wstring> names;
    wstring type;
    void render(wostringstream& buffer, const rendering_options& options) override 
    {
      for (int i = 0; i < names.size(); ++i)
      {
        buffer << type << " " << names[i];
        if (i + 1 != names.size())
          buffer << ", ";
      }
    }
  };

  struct assignment_statement : element
  {
    wstring variable_being_assigned;
    wstring value;

    wstring infer_type() const
    {
      if (value.find(L'.') == wstring::npos)
        return L"int"s;
      return L"float"s;
    }

    void render(wostringstream& buffer, const rendering_options& options) override
    {
      buffer << variable_being_assigned << " = " << value;
    }
  };

  struct function : element
  {
    wstring name;
    vector<parameter> params;
    vector<assignment_statement> assignments;

    void render(wostringstream& buffer, const rendering_options& options) override
    {
      // signature goes first
      buffer << "void " << name << "(";
      for (size_t i = 0; i < params.size(); i++)
      {
        params[i].render(buffer, options);
        if (i + 1 != params.size())
          buffer << ", ";
      }
      buffer << ")\r\n{\r\n";

      // here come the assignment statements
      for (size_t i = 0; i < assignments.size(); i++)
      {
        auto& a = assignments[i];
        auto p = find_parameter(a.variable_being_assigned);
        buffer << "  ";
        if (!p) buffer << a.infer_type() << " ";
        a.render(buffer, options);
        buffer << "\r\n";
      }

      buffer << "}\r\n";
    }

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

  typedef boost::variant<assignment_statement, function> top_level_construct;

  struct program : element
  {
    vector<top_level_construct> content;

    struct render_generic : boost::static_visitor<>
    {
      wostringstream& buffer;
      const rendering_options& options;

      render_generic(wostringstream& buffer, const rendering_options& options)
        : buffer(buffer),
          options(options)
      {
      }

      template <typename T> void operator()(T& t) const {
        t.render(buffer, options);
      }

      template <> void operator()(assignment_statement& a) const
      {
        buffer << a.infer_type() << " ";
        a.render(buffer, options);
        buffer << ";\r\n";
      }
    };

    void render(wostringstream& buffer, const rendering_options& options) override {
      auto visitor = render_generic{ buffer, options };
      for (auto& tlc : content)
        apply_visitor(visitor, tlc);
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
  overmath::program,
  (std::vector<overmath::top_level_construct>, content)
)

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