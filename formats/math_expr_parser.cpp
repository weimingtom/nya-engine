//https://code.google.com/p/nya-engine/

#include <math_expr_parser.h>

#include <sstream>
#include <stack>
#include <math.h>

namespace nya_formats
{

inline int precedence(char op)
{
    switch(op)
    {
        case '-': case '+': return 1;
        case '*': case '/': return 2;
        case '^': case '%': return 3;
    }

    return 0;
}

inline bool infix_to_rpn(const std::vector<std::string> &from,std::vector<std::string> &to)
{
    std::stack<char> stc;
    for(size_t i=0;i<from.size();i++)
    {
        if(from[i].empty())
            continue;

        const char c=from[i].front();

        if(c=='(')
        {
            stc.push(c);
            continue;
        }

        if(c==')')
        {
            if(stc.empty())
                return false;

            while(stc.top()!='(')
            {
                to.push_back(std::string(1,stc.top()));
                stc.pop();

                if(stc.empty())
                    return false;
            }
            stc.pop();
            continue;
        }

        if(!precedence(c))
        {
            to.push_back(from[i]);
            continue;
        }

        if(stc.empty() || precedence(c)>precedence(stc.top()))
        {
            stc.push(c);
            continue;
        }

        if(precedence(c)==precedence(stc.top()) && precedence(c)==3)
        {
            stc.push(c);
            continue;
        }

        to.push_back(std::string(1,stc.top()));
        stc.pop();
        stc.push(c);
    }

    while(!stc.empty())
    {
        to.push_back(std::string(1,stc.top()));
        stc.pop();
    }

    return true;
}

bool math_expr_parser::parse(const char *expr)
{
    if(!expr)
        return false;

    std::vector<std::string> tokens;

    std::string str;
    for(const char *c=expr;*c;++c)
    {
        if(*c<=' ')
            continue;

        if(!precedence(*c) && *c!='(' && *c!=')')
        {
            str.push_back(*c);
            continue;
        }

        if(!str.empty())
        {
            tokens.push_back(str);
            if(isalpha(str.front()))
                add_var(str.c_str());
            str.clear();
        }

        tokens.push_back(std::string(c,1));
    }

    if(!str.empty())
    {
        tokens.push_back(str);
        if(isalpha(str.front()))
            add_var(str.c_str());
    }

    return infix_to_rpn(tokens,m_tokens);
}

bool math_expr_parser::set_var(const char *name,float value,bool allow_unfound)
{
    if(!name)
        return false;

    for(int i=0;i<(int)m_vars.size();++i)
    {
        if(m_vars[i].first==name)
        {
            m_vars[i].second=value;
            return true;
        }
    }

    if(!allow_unfound)
        return false;

    const int idx=(int)m_vars.size();
    m_vars.resize(idx+1);
    m_vars.back().first=name;
    m_vars.back().second=value;

    return true;
}

void math_expr_parser::add_var(const std::string &str)
{
    for(int i=0;i<(int)m_vars.size();++i)
    {
        if(m_vars[i].first==str)
            return;
    }

    set_var(str.c_str(),0.0f,true);
}

float math_expr_parser::get_value(const std::string &str) const
{
    if(!isalpha(str.front()))
        return strtof(str.c_str(),NULL);

    for(int i=0;i<(int)m_vars.size();++i)
    {
        if(m_vars[i].first==str)
            return m_vars[i].second;
    }

    return 0.0f;
}

float math_expr_parser::calculate() const
{
    std::stack<std::string> st;
    for(size_t i=0;i<m_tokens.size();++i)
    {
        const std::string &token=m_tokens[i];
        if(!precedence(token.front()))
        {
            st.push(token);
            continue;
        }

        float result=0.0;

        const float d2=get_value(st.top());
        st.pop();
        if(!st.empty())
        {
            const float d1=get_value(st.top());
            st.pop();

            switch(token.front())
            {
                case '+': result=d1+d2; break;
                case '-': result=d1-d2; break;
                case '*': result=d1*d2; break;
                case '/': result=d1/d2; break;
                case '^': result=powf(d1,d2); break;
                case '%': result=int(d1)%int(d2); break;
            }
        }
        else
            result=token.front()=='-'?-d2:d2;

        std::ostringstream s;
        s<<result;
        st.push(s.str());
    }

    return get_value(st.top());
}

}
