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

        prefix.append("cbuffer "+m_replace_str+"constant_buffer:register(b0){");
        for(size_t i=0;i<m_uniforms.size();++i)
            prefix.append("matrix "+m_uniforms[i].name+";");
        prefix.append("}\n");
    }

    const size_t predefined_count=m_uniforms.size();

    parse_uniforms(true);
    parse_varying(true);
    std::sort(m_varying.begin(),m_varying.end());

    //ToDo: vectors from float constructor

    replace_hlsl_mul();
    replace_hlsl_types();

    replace_variable("mix","lerp");

    bool has_samplers=false;
    for(size_t i=predefined_count;i<m_uniforms.size();++i)
    {
        const variable &v=m_uniforms[i];
        if(v.type!=type_sampler2d && v.type!=type_sampler_cube)
            continue;

        const char *types[]={"Texture2D","TextureCube"};

        unsigned int reg=0;
        std::map<std::string,unsigned int>::iterator it=m_samplers.find(v.name);
        if(it==m_samplers.end())
        {
            for(it=m_samplers.begin();it!=m_samplers.end();++it) if(reg<=it->second) reg=it->second+1;
            m_samplers[v.name]=reg;
        }
        else
            reg=it->second;

        char buf[512];
        sprintf(buf,"%s %s: register(t%d); SamplerState %s_nya_st: register(s%d);\n",
                types[v.type-type_sampler2d],v.name.c_str(),reg,v.name.c_str(),reg);
        prefix.append(buf);
        has_samplers=true;
    }

    if(has_samplers)
    {
        prefix.append("#define texture2D(a,b) a.Sample(a##"+m_replace_str+"st,(b))\n");
        prefix.append("#define textureCube(a,b) a.Sample(a##"+m_replace_str+"st,(b))\n");
    }

    const char *gl_vs_out="gl_Position",*gl_ps_out="gl_FragColor";
    const char *type_names[]={"float","float2","float3","float4","float4x4"};

    prefix.append("struct "+m_replace_str+"vsout{float4 "+m_replace_str+std::string(gl_vs_out+3)+":POSITION;");
    for(int i=0;i<(int)m_varying.size();++i)
    {
        const variable &v=m_varying[i];
        if(v.type==type_invalid)
            return false;

        if(v.type-1>=sizeof(type_names)/sizeof(type_names[0]))
            continue;

        char buf[255];
        sprintf(buf,"%s %s:TEXCOORD%d;",type_names[v.type-1],m_varying[i].name.c_str(),i);
        prefix.append(buf);
    }
    prefix.append("};\n");

    const std::string input_var=m_replace_str+"in";
    const std::string ps_out_var=m_replace_str+std::string(gl_ps_out+3); //strlen("gl_")
    const bool is_fragment=replace_variable(gl_ps_out,ps_out_var.c_str());
    if(is_fragment)
    {
        prefix.append("static float4 "+ps_out_var+";\n");

        const std::string main=std::string("void ")+m_replace_str+"main("+m_replace_str+"vsout "+input_var+")";
        replace_main_function_header(main.c_str());

        const size_t main_start=m_code.find(main);
        for(int i=0;i<(int)m_varying.size();++i)
        {
            const std::string to=input_var+"."+m_varying[i].name;
            replace_variable(m_varying[i].name.c_str(),to.c_str(),main_start);
        }

        m_code.append("\nfloat4 main("+m_replace_str+"vsout "+input_var+"):SV_TARGET{"+
                      m_replace_str+"main("+input_var+");return "+ps_out_var+";}\n");
    }
    else
    {
        parse_attributes((input_var+".").c_str());
        if(!m_attributes.empty())
        {
            prefix.append("struct "+m_replace_str+"vsin{");
            for(int i=0,idx=0;i<(int)m_attributes.size();++i)
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
        prefix.append("static "+m_replace_str+"vsout "+out_var+";\n");

        const std::string main=std::string("void ")+m_replace_str+"main("+m_replace_str+"vsin "+input_var+")";
        replace_main_function_header(main.c_str());

        const size_t main_start=m_code.find(main);
        const std::string vs_out_var=out_var+"."+m_replace_str+std::string(gl_vs_out+3); //strlen("gl_")
        replace_variable(gl_vs_out,vs_out_var.c_str(),main_start);

        for(int i=0;i<(int)m_varying.size();++i)
        {
            const std::string to=out_var+"."+m_varying[i].name;
            replace_variable(m_varying[i].name.c_str(),to.c_str(),main_start);
        }

        m_code.append("\n"+m_replace_str+"vsout main("+m_replace_str+"vsin "+input_var+"){"+
                      m_replace_str+"main("+input_var+");return "+out_var+";}\n");
    }

    if(m_uniforms.size()>predefined_count)
    {
        prefix.append("cbuffer "+m_replace_str+"uniforms_buffer:register(b"),
        prefix.append(is_fragment?"2":"1"),prefix.append("){");

        for(size_t i=predefined_count;i<m_uniforms.size();++i)
        {
            const variable &v=m_uniforms[i];
            if(v.type==type_invalid)
                return false;

            if(v.type-1>=sizeof(type_names)/sizeof(type_names[0]))
                continue;

            prefix.append(type_names[v.type-1]),prefix.append(" "+v.name);
            if(v.array_size>1)
            {
                char buf[255];
                sprintf(buf,"[%d];",v.array_size);
                prefix.append(buf);
            }
            else
                prefix.append(";");
        }
        prefix.append("}\n");
    }

    m_code.insert(0,prefix);
    m_varying.clear();
    return true;
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
        prefix.append(m_attributes[i].name+";\n");
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

template<typename t> static bool parse_vars(std::string &code,t& vars,const char *str,bool remove)
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

bool shader_code_parser::parse_uniforms(bool remove) { return parse_vars(m_code,m_uniforms,"uniform",remove); }
bool shader_code_parser::parse_varying(bool remove) { return parse_vars(m_code,m_varying,"varying",remove); }

template<typename t> static void push_unique_to_vec(std::vector<t> &v,const t &e)
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

static size_t get_var_pos(const std::string &code,size_t pos,int add)
{
    int brace_count=0;
    char lbrace=add>0?'(':')';
    char rbrace=add>0?')':'(';
    bool first_spaces=true;

    for(size_t i=pos+add;i<code.size() && i>0;i+=add)
    {
        const char c=code[i];
        if(first_spaces && c<=' ')
            continue;

        first_spaces=false;

        if(c==lbrace)
        {
            ++brace_count;
            continue;
        }

        if(c==rbrace)
        {
            --brace_count;
            if(brace_count<0)
                return add>0?i:i-add;

            continue;
        }

        if(brace_count)
            continue;

        if(strchr(";+-=*/,<>%?&|:{} \t\n\r",c))
            return add>0?i:i-add;
    }

    return pos;
}

static bool is_numeric_only_var(const std::string &s)
{
    for(size_t i=0;i<s.size();++i) if(isalpha(s[i]) || s[i]=='_') return false;
    return true;
}

static bool is_space_only_var(const std::string &s)
{
    for(size_t i=0;i<s.size();++i) if(s[i]>' ') return false;
    return true;
}

bool shader_code_parser::replace_hlsl_mul()
{
    size_t start_pos=0;
    while((start_pos=m_code.find('*',start_pos))!=std::string::npos)
    {
        const size_t left=get_var_pos(m_code,start_pos,-1);
        const size_t right=get_var_pos(m_code,start_pos,1);
        if(left==start_pos || right==start_pos)
            return false;

        const std::string left_var=m_code.substr(left,start_pos-left);
        const std::string right_var=m_code.substr(start_pos+1,right-start_pos-1);

        if(is_space_only_var(right_var)) // *=
        {
            ++start_pos;
            continue;
        }

        //ToDo: not sure if matrix*scalar don't need mul, in case of fail replace || with &&
        if(is_numeric_only_var(left_var) || is_numeric_only_var(right_var))
        {
            ++start_pos;
            continue;
        }

        const std::string replace="mul("+left_var+","+right_var+")";
        m_code.replace(left,right-left,replace);
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

static bool is_name_char(char c) { return isalnum(c) || c=='_'; }

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
