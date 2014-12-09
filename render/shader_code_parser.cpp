//https://code.google.com/p/nya-engine/

#include "shader_code_parser.h"
#include <stdlib.h>

namespace nya_render
{

bool shader_code_parser::convert_to_hlsl()
{
    std::string prefix;

    parse_predefined_uniforms(m_replace_str.c_str());
    if(!m_uniforms.empty())
    {
        std::sort(m_uniforms.begin(),m_uniforms.end());

        prefix.append("cbuffer ");
        prefix.append(m_replace_str);
        prefix.append("ConstantBuffer:register(b0){");
        for(size_t i=0;i<m_uniforms.size();++i)
            prefix.append("matrix "),prefix.append(m_uniforms[i].name),prefix.append(";");
        prefix.append("}\n");
    }

    parse_uniforms(true);
    parse_varying(true);
    std::sort(m_varying.begin(),m_varying.end());

    //ToDo: replace all * with mul() and add functions for vec*vec,vec*float,etc
    //ToDo: vectors from float constructor
    replace_hlsl_types();

    //ToDo: add uniform buffer
    //ToDo: add texture uniforms
    //ToDo: add vsout
    //ToDo: replace texture sample functions
    //ToDo: replace build-in functions

    const std::string input_var=m_replace_str+"in";

    const char *gl_ps_out="gl_FragColor";
    const std::string ps_out_var=m_replace_str+std::string(gl_ps_out+3); //strlen("gl_")
    const bool is_fragment=replace_variable(gl_ps_out,ps_out_var.c_str());
    if(is_fragment)
    {
        prefix.append("static float4 ");
        prefix.append(ps_out_var);
        prefix.append(";\n");

        const std::string main=std::string("void ")+m_replace_str+"main(vsout "+input_var+")";
        replace_main_function_header(main.c_str());

        const size_t main_start=m_code.find(main);
        for(int i=0;i<(int)m_varying.size();++i)
        {
            const std::string to=input_var+"."+m_varying[i].name;
            replace_variable(m_varying[i].name.c_str(),to.c_str(),main_start);
        }

        const std::string appnd=std::string("\nfloat4 main(vsout "+input_var+"):SV_TARGET{"+
                                            m_replace_str+"main("+input_var+");return ")+ps_out_var+";}\n";
        m_code.append(appnd);
    }
    else
    {
        parse_attributes((input_var+".").c_str());
        if(!m_attributes.empty())
        {
            int idx=0;
            prefix.append("struct vsin{");
            for(int i=0;i<(int)m_attributes.size();++i)
            {
                variable &a=m_attributes[i];
                a.name=a.name.substr(input_var.size()+1);
                if(a.name=="Vertex")
                    prefix.append("float4 Vertex:POSITION;");
                else if(a.name=="Normal")
                    prefix.append("float3 Normal:NORMAL;");
                else if(a.name=="Color")
                    prefix.append("float4 Color:COLOR;");
                else
                {
                    char buf[255];
                    sprintf(buf,"float4 %s:TEXCOORD%d;",a.name.c_str(),idx++);
                    prefix.append(buf);
                }
            }
            prefix.append("};\n");
        }

        const std::string out_var=m_replace_str+"out";

        prefix.append("static vsout ");
        prefix.append(out_var);
        prefix.append(";\n");

        const std::string main=std::string("void ")+m_replace_str+"main(vsin "+input_var+")";
        replace_main_function_header(main.c_str());

        const size_t main_start=m_code.find(main);

        const char *gl_vs_out="gl_Position";
        const std::string vs_out_var=out_var+"."+m_replace_str+std::string(gl_vs_out+3); //strlen("gl_")
        replace_variable(gl_vs_out,vs_out_var.c_str(),main_start);

        for(int i=0;i<(int)m_varying.size();++i)
        {
            const std::string to=out_var+"."+m_varying[i].name;
            replace_variable(m_varying[i].name.c_str(),to.c_str(),main_start);
        }

        const std::string appnd=std::string("\nvsout main(vsin "+input_var+"){"+
                                            m_replace_str+"main("+input_var+");return ")+out_var+";}\n";
        m_code.append(appnd);
    }

    m_code.insert(0,prefix);
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

namespace
{

template<typename t> bool parse_vars(std::string &code,t& vars,const char *str,bool remove)
{
    const size_t str_len=strlen(str);
    for(size_t i=code.find(str);i!=std::string::npos;i=code.find(str,i))
    {
        size_t type_from=i+str_len+1;
        while(code[type_from]<=' ') if(++type_from>=code.length()) return false;
        size_t type_to=type_from;
        while(code[type_to]>' ') if(++type_to>=code.length()) return false;

        size_t name_from=type_to+1;
        while(code[name_from]<=' ') if(++name_from>=code.length()) return false;
        size_t name_to=name_from;
        while(code[name_to]>' ' && code[name_to]!=';' && code[name_to]!='[') if(++name_to>=code.length()) return false;

        const std::string name=code.substr(name_from,name_to-name_from);
        const std::string type_name=code.substr(type_from,type_to-type_from);

        size_t last=name_to;
        while(code[last]!=';') if(++last>=code.length()) return false;

        int count=1;
        size_t array_from=code.find('[',name_to);
        if(array_from<last)
            count=atoi(&code[array_from+1]);

        if(count<=0)
            return false;

        if(type_name.compare(0,3,"vec")==0)
        {
            char dim=(type_name.length()==4)?type_name[3]:'\0';
            switch(dim)
            {
                case '2': vars.push_back(shader_code_parser::variable(shader_code_parser::type_vec2,name.c_str(),count)); break;
                case '3': vars.push_back(shader_code_parser::variable(shader_code_parser::type_vec3,name.c_str(),count)); break;
                case '4': vars.push_back(shader_code_parser::variable(shader_code_parser::type_vec4,name.c_str(),count)); break;
                default: return false;
            };
        }
        else if(type_name=="float")
            vars.push_back(shader_code_parser::variable(shader_code_parser::type_float,name.c_str(),count));
        else if(type_name=="sampler2D")
            vars.push_back(shader_code_parser::variable(shader_code_parser::type_sampler2d,name.c_str(),count));
        else if(type_name=="samplerCube")
            vars.push_back(shader_code_parser::variable(shader_code_parser::type_sampler_cube,name.c_str(),count));
        else
            return false;

        if(remove)
            code.erase(i,last-i+1);
        else
            i=last+1;
    }

    return true;
}

}

bool shader_code_parser::parse_uniforms(bool remove) { return parse_vars(m_code,m_uniforms,"uniform",remove); }
bool shader_code_parser::parse_varying(bool remove) { return parse_vars(m_code,m_varying,"varying",remove); }

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
        if(replace_variable(gl_matrix_names[i],to.c_str()))
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
        if(replace_variable(gl_attr_names[i],to.c_str()))
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
        sprintf(buf,"%s%s%d",replace_prefix_str,tc_atr_name+3,idx);
        push_unique_to_vec(m_attributes,variable(type_vec4,buf,idx));
    }

    return true;
}

bool shader_code_parser::replace_hlsl_types()
{
    replace_variable("vec2","float2");
    replace_variable("vec3","float3");
    replace_variable("vec4","float4");
    replace_variable("mat2","float2x2");
    replace_variable("mat3","float3x3");
    replace_variable("mat4","float4x4");
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

bool shader_code_parser::replace_string(const char *from,const char *to,size_t start_pos)
{
    if(!from || !from[0] || !to)
        return false;

    bool result=false;
    while((start_pos=m_code.find(from,start_pos))!=std::string::npos)
    {
        m_code.replace(start_pos,strlen(from),to);
        start_pos+=strlen(to);
        result=true;
    }

    return result;
}

namespace { bool is_name_char(char c) { return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_'; } }

bool shader_code_parser::replace_variable(const char *from,const char *to,size_t start_pos)
{
    if(!from || !from[0] || !to)
        return false;

    bool result=false;
    const size_t from_len=strlen(from);
    while((start_pos=m_code.find(from,start_pos))!=std::string::npos)
    {
        if((start_pos!=0 && is_name_char(m_code[start_pos-1])) ||
           (start_pos+from_len<m_code.size() && is_name_char(m_code[start_pos+from_len])))
        {
            start_pos+=from_len;
            continue;
        }

        m_code.replace(start_pos,from_len,to);
        start_pos+=strlen(to);
        result=true;
    }
    
    return result;
}

}
