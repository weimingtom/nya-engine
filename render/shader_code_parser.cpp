//https://code.google.com/p/nya-engine/

#include "shader_code_parser.h"
#include <stdlib.h>

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
        parse_attributes(0);

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
    m_uniforms.clear();

    for(size_t i=m_code.find("uniform");i!=std::string::npos;i=m_code.find("uniform",i+7))
    {
        size_t type_from=i+8;
        while(m_code[type_from]<=' ') if(++type_from>=m_code.length()) return false;
        size_t type_to=type_from;
        while(m_code[type_to]>' ') if(++type_to>=m_code.length()) return false;

        size_t name_from=type_to+1;
        while(m_code[name_from]<=' ') if(++name_from>=m_code.length()) return false;
        size_t name_to=name_from;
        while(m_code[name_to]>' ' && m_code[name_to]!=';' && m_code[name_to]!='[') if(++name_to>=m_code.length()) return false;

        const std::string name=m_code.substr(name_from,name_to-name_from);
        const std::string type_name=m_code.substr(type_from,type_to-type_from);

        size_t last=name_to;
        while(m_code[last]!=';') if(++last>=m_code.length()) return false;

        int count=1;
        size_t array_from=m_code.find('[',name_to);
        if(array_from<last)
            count=atoi(&m_code[array_from+1]);

        if(count<=0)
            return false;

        if(type_name.compare(0,3,"vec")==0)
        {
            char dim=(type_name.length()==4)?type_name[3]:'\0';
            switch(dim)
            {
                case '2': m_uniforms.push_back(variable(type_vec2,name.c_str(),count)); break;
                case '3': m_uniforms.push_back(variable(type_vec3,name.c_str(),count)); break;
                case '4': m_uniforms.push_back(variable(type_vec4,name.c_str(),count)); break;
                default: return false;
            };
        }
        else if(type_name=="sampler2D")
            m_uniforms.push_back(variable(type_sampler2d,name.c_str(),count));
        else if(type_name=="samplerCube")
            m_uniforms.push_back(variable(type_sampler_cube,name.c_str(),count));
        else if(type_name=="float")
            m_uniforms.push_back(variable(type_float,name.c_str(),count));

        if(remove)
            m_code.erase(i,last-i+1);
    }

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
