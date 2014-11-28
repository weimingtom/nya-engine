//https://code.google.com/p/nya-engine/

#include "shader_code_parser.h"

namespace nya_render
{

bool shader_code_parser::convert_to_hlsl()
{
    //ToDo
    return false;
}

bool shader_code_parser::convert_to_modern_glsl()
{
    //ToDo
    return false;
}

int shader_code_parser::get_uniforms_count()
{
    if(m_uniforms.empty())
        parse_uniforms(false);

    return (int)m_uniforms.size();
}

shader_code_parser::variable shader_code_parser::get_uniform(int idx)
{
    if(idx<0 || idx>=(int)m_uniforms.size())
        return shader_code_parser::variable();

    return m_uniforms[idx];
}

int shader_code_parser::get_attributes_count()
{
    if(m_attributes.empty())
        parse_attributes(false);

    return (int)m_uniforms.size();
}

shader_code_parser::variable shader_code_parser::get_attribute(int idx)
{
    if(idx<0 || idx>=(int)m_attributes.size())
        return shader_code_parser::variable();

    return m_attributes[idx];
}

void shader_code_parser::remove_comments()
{
    while(m_code.find("/*")!=std::string::npos)
    {
        const size_t from=m_code.find("/*");
        m_code.erase(from,(m_code.find("*/",from+2)-from)+2);
    }
    while(m_code.find("//")!=std::string::npos)
    {
        const size_t from=m_code.find("//");
        m_code.erase(from,m_code.find_first_of("\n\r",from+1)-from);
    }
}

bool shader_code_parser::parse_uniforms(bool remove)
{
    //ToDo
    return false;
}

bool shader_code_parser::parse_attributes(const char *replace_str)
{
    //ToDo
    return false;
}

bool shader_code_parser::replace_main_function_header(const char *replace_str)
{
    //ToDo
    return false;
}

bool shader_code_parser::replace_string(const char *from,const char *to)
{
    if(!from || !from[0] || !to)
        return false;

    bool result=false;
    size_t start_pos=0;
    while((start_pos=m_code.find(from,start_pos))!=std::string::npos)
    {
        m_code.replace(start_pos,strlen(from),to);
        start_pos+=strlen(to);
        result=true;
    }

    return result;
}

}
