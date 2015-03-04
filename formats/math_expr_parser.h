//https://code.google.com/p/nya-engine/

#pragma once

#include <vector>
#include <string>

namespace nya_formats
{

class math_expr_parser
{
public:
    bool parse(const char *expr);
    bool set_var(const char *name,float value,bool allow_unfound=true);
    float calculate() const;

    math_expr_parser() {}
    math_expr_parser(const char *expr) { parse(expr); }

private:
    float get_value(const std::string &str) const;
    void add_var(const std::string &str);

private:
    std::vector<std::pair<std::string,float> > m_vars;
    std::vector<std::string> m_tokens;
};

}
