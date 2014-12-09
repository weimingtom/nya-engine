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
    if(!parse_predefined_uniforms(m_replace_str.c_str()))
        return false;

    if(!parse_attributes(m_replace_str.c_str()))
        return false;

    std::string prefix("precision mediump float;\n");

    for(int i=0;i<(int)m_attributes.size();++i)
    {
        prefix.append("attribute ");
        prefix.append(m_attributes[i].type==type_vec3?"vec3 ":"vec4 ");
        prefix.append(m_attributes[i].name);
        prefix.append(";\n");
    }

    m_code.insert(0,prefix);
    return true;
}

int shader_code_parser::get_uniforms_count()
{
    if(m_uniforms.empty())
        parse_uniforms(false);

    return (int)m_uniforms.size();
}

shader_code_parser::variable shader_code_parser::get_uniform(int idx) const
{
    if(idx<0 || idx>=(int)m_uniforms.size())
        return shader_code_parser::variable();

    return m_uniforms[idx];
}

int shader_code_parser::get_attributes_count() const { return (int)m_attributes.size(); }

shader_code_parser::variable shader_code_parser::get_attribute(int idx) const
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
        size_t type_from=i+8; //strlen("uniform ")
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

    return true;
}

namespace
{

template<typename t> void push_unique_to_vec(std::vector<t> &v,const t &e)
{
    for(size_t i=0;i<v.size();++i)
    {
        if(v[i].name==e.name)
        {
            v[i]=e;
            return;
        }
    }

    v.push_back(e);
}

}

bool shader_code_parser::parse_predefined_uniforms(const char *replace_prefix_str)
{
    if(!replace_prefix_str)
        return false;

    const char *gl_matrix_names[]={"gl_ModelViewMatrix","gl_ModelViewProjectionMatrix","gl_ProjectionMatrix"};

    for(size_t i=0;i<sizeof(gl_matrix_names)/sizeof(gl_matrix_names[0]);++i)
    {
        std::string to=std::string(replace_prefix_str)+std::string(gl_matrix_names[i]+3); //strlen("gl_")
        if(replace_string(gl_matrix_names[i],to.c_str()))
            push_unique_to_vec(m_uniforms,variable(type_mat4,to.c_str(),1));
    }

    return true;
}

bool shader_code_parser::parse_attributes(const char *replace_prefix_str)
{
    if(!replace_prefix_str)
        return false;

    const char *gl_attr_names[]={"gl_Vertex","gl_Normal","gl_Color"};
    variable_type gl_attr_types[]={type_vec4,type_vec3,type_vec4};

    for(size_t i=0;i<sizeof(gl_attr_names)/sizeof(gl_attr_names[0]);++i)
    {
        std::string to=std::string(replace_prefix_str)+std::string(gl_attr_names[i]+3); //strlen("gl_")
        if(replace_string(gl_attr_names[i],to.c_str()))
            push_unique_to_vec(m_attributes,variable(gl_attr_types[i],to.c_str(),0));
    }

    const char *tc_atr_name="gl_MultiTexCoord";

    size_t start_pos=0;
    while((start_pos=m_code.find(tc_atr_name,start_pos))!=std::string::npos)
    {
        m_code.replace(start_pos,3,replace_prefix_str); //strlen("gl_")
        start_pos+=strlen(tc_atr_name);
        const int idx=atoi(&m_code[start_pos]);
        char buf[255];
        sprintf(buf,"%s%d",tc_atr_name,idx);
        push_unique_to_vec(m_attributes,variable(type_vec4,buf,idx));
    }

    return true;
}

bool shader_code_parser::replace_main_function_header(const char *replace_str)
{
    if(!replace_str)
        return false;

    size_t start_pos=0;
    while((start_pos=m_code.find("void",start_pos))!=std::string::npos)
    {
        const size_t lbrace=m_code.find('(',start_pos+9); //strlen("void main")
        if(lbrace==std::string::npos)
            return false;

        std::string main;
        for(size_t i=start_pos+5;i<lbrace;++i) //strlen("void ")
        {
            if(m_code[i]>' ')
                main+=m_code[i];
        }

        if(main!="main")
        {
            start_pos+=4; //strlen("void")
            continue;
        }

        const size_t rbrace=m_code.find(')',lbrace);
        if(rbrace==std::string::npos)
            return false;

        m_code.replace(start_pos,rbrace+1-start_pos,replace_str);
        return true;
    }

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
