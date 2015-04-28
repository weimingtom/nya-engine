//https://code.google.com/p/nya-engine/

#include "shader_code_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <algorithm>

namespace nya_render
{

bool shader_code_parser::convert_to_hlsl()
{
    m_uniforms.clear();
    m_attributes.clear();

    std::string prefix="#define DIRECTX11 1\n";

    parse_predefined_uniforms(m_replace_str.c_str(),true);
    if(!m_uniforms.empty())
    {
        std::sort(m_uniforms.begin(),m_uniforms.end());

        prefix.append("cbuffer "+m_replace_str+"constant_buffer:register(b0){");
        for(size_t i=0;i<m_uniforms.size();++i)
            prefix.append("matrix "+m_uniforms[i].name+";");
        prefix.append("}\n");
    }

    const size_t predefined_count=m_uniforms.size();

    if(!parse_uniforms(true))
    {
        m_error.append("unable to parse uniforms\n");
        return false;
    }

    if(!parse_varying(true))
    {
        m_error.append("unable to parse varying\n");
        return false;
    }

    std::sort(m_varying.begin(),m_varying.end());

    const std::string replace_constructor=m_replace_str+"cast_float";
    if(replace_vec_from_float(replace_constructor.c_str()))
    {
        prefix.append("float2 "+replace_constructor+"2(float a){return float2(a,a);} ");
        prefix.append("float2 "+replace_constructor+"2(float2 a){return a;}\n");
        prefix.append("float3 "+replace_constructor+"3(float a){return float3(a,a,a);} ");
        prefix.append("float3 "+replace_constructor+"3(float3 a){return a;}\n");
        prefix.append("float4 "+replace_constructor+"4(float a){return float4(a,a,a,a);} ");
        prefix.append("float4 "+replace_constructor+"4(float4 a){return a;}\n");
    }

    replace_hlsl_mul("mul");
    replace_hlsl_types();

    replace_variable("mix","lerp");
    replace_variable("fract","frac");
    replace_variable("inversesqrt","rsqrt");
    if(replace_variable("pow",(m_replace_str+"pow").c_str()))
        prefix.append("#define "+m_replace_str+"pow(f,e) pow(abs(f),e)\n");

    int samplers_count=0;
    for(size_t i=predefined_count;i<m_uniforms.size();++i)
    {
        const variable &v=m_uniforms[i];
        if(v.type!=type_sampler2d && v.type!=type_sampler_cube)
            continue;

        const char *types[]={"Texture2D","TextureCube"};

        char buf[512];
        sprintf(buf,"%s %s: register(t%d); SamplerState %s_nya_st: register(s%d);\n",
                types[v.type-type_sampler2d],v.name.c_str(),samplers_count,v.name.c_str(),samplers_count);
        prefix.append(buf);
        ++samplers_count;
    }

    if(samplers_count>0)
    {
        if(find_variable("texture2D")) prefix.append("#define texture2D(a,b) a.Sample(a##"+m_replace_str+"st,(b))\n");
        if(find_variable("textureCube")) prefix.append("#define textureCube(a,b) a.Sample(a##"+m_replace_str+"st,(b))\n");
        if(find_variable("texture2DProj"))
        {
            prefix.append("float2 "+m_replace_str+"tc_proj(float3 tc){return tc.xy/tc.z;}\n");
            prefix.append("float2 "+m_replace_str+"tc_proj(float4 tc){return tc.xy/tc.w;}\n");
            prefix.append("#define texture2DProj(a,b) a.Sample(a##"+m_replace_str+"st,"+m_replace_str+"tc_proj(b))\n");
        }
    }

    const char *gl_vs_out="gl_Position",*gl_ps_out="gl_FragColor";
    const char *type_names[]={"float","float2","float3","float4","float4x4"};

    std::string vs_pos_out=m_replace_str+std::string(gl_vs_out+3);
    prefix.append("struct "+m_replace_str+"vsout{float4 "+vs_pos_out+":SV_POSITION;");
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

        const std::string main=std::string("void ")+m_replace_str+"main()";
        if(replace_main_function_header(main.c_str()))
        {
            m_error.append("main function not found\n");
            return false;
        }

        std::string in_var_assign;
        for(int i=0;i<(int)m_varying.size();++i)
        {
            const variable &v=m_varying[i];
            if(v.type==type_invalid)
            {
                m_error.append("invalid variable \'"+v.name+"\' type\n");
                return false;
            }

            if(v.type-1>=sizeof(type_names)/sizeof(type_names[0]))
                continue;

            prefix.append("static "+std::string(type_names[v.type-1])+" "+m_varying[i].name+";");
            in_var_assign.append(m_varying[i].name+"="+input_var+"."+m_varying[i].name+";");
        }
        prefix.append("\n");

        m_code.append("\nfloat4 main("+m_replace_str+"vsout "+input_var+"):SV_TARGET{"+in_var_assign+
                      m_replace_str+"main();return "+ps_out_var+";}\n");
    }
    else
    {
        if(!parse_attributes(m_replace_str.c_str(),(input_var+".").c_str()))
        {
            m_error.append("unable to parse attributes\n");
            return false;
        }

        if(!m_attributes.empty())
        {
            prefix.append("struct "+m_replace_str+"vsin{");
            for(int i=0,idx=0;i<(int)m_attributes.size();++i)
            {
                variable &a=m_attributes[i];
                a.name=a.name.substr(m_replace_str.size());
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

            std::string instance_var=m_replace_str+"InstanceID";
            if(replace_variable("gl_InstanceID",(input_var+"."+instance_var).c_str()))
                prefix.append("uint "+instance_var+":SV_InstanceID;");

            prefix.append("};\n");
        }

        const std::string out_var=m_replace_str+"out";
        prefix.append("static "+m_replace_str+"vsin "+input_var+";\n");

        const std::string main=std::string("void ")+m_replace_str+"main()";
        replace_main_function_header(main.c_str());
        const std::string vs_out_var=out_var+"."+m_replace_str+std::string(gl_vs_out+3); //strlen("gl_")

        replace_variable(gl_vs_out,vs_pos_out.c_str());

        std::string out_var_assign;
        prefix.append("static float4 "+vs_pos_out+";");
        out_var_assign.append(out_var+"."+vs_pos_out+"="+vs_pos_out+";");
        for(int i=0;i<(int)m_varying.size();++i)
        {
            const variable &v=m_varying[i];
            if(v.type==type_invalid)
                return false;

            if(v.type-1>=sizeof(type_names)/sizeof(type_names[0]))
                continue;

            prefix.append("static "+std::string(type_names[v.type-1])+" "+m_varying[i].name+";");
            out_var_assign.append(out_var+"."+m_varying[i].name+"="+m_varying[i].name+";");
        }
        prefix.append("\n");

        m_code.append("\n"+m_replace_str+"vsout main("+m_replace_str+"vsin "+input_var+"_){"+input_var+"="+input_var+"_;"+
                      m_replace_str+"main();"+m_replace_str+"vsout "+out_var+";"+out_var_assign+"return "+out_var+";}\n");
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

bool shader_code_parser::convert_to_glsl()
{
    if(replace_variable("gl_InstanceID","gl_InstanceIDARB"))
        m_code.insert(0,"#extension GL_ARB_draw_instanced : enable\n");

    return true;
}

bool shader_code_parser::convert_to_glsl_es2(const char *precision)
{
    m_uniforms.clear();
    m_attributes.clear();

    if(!parse_predefined_uniforms(m_replace_str.c_str(),true))
    {
        m_error.append("unable to parse predefined uniforms\n");
        return false;
    }

    if(!parse_attributes(m_replace_str.c_str(),m_replace_str.c_str()))
    {
        m_error.append("unable to parse predefined attributes\n");
        return false;
    }

    std::string prefix="#define OPENGL_ES2 1\n";

    if(replace_variable("gl_InstanceID","gl_InstanceIDEXT"))
        prefix.append("#extension GL_EXT_draw_instanced : enable\n");

    for(int i=0;i<(int)m_uniforms.size();++i)
        prefix.append("uniform mat4 "+m_uniforms[i].name+";\n");

    for(int i=0;i<(int)m_attributes.size();++i)
    {
        prefix.append("attribute ");
        prefix.append(m_attributes[i].type==type_vec3?"vec3 ":"vec4 ");
        prefix.append(m_attributes[i].name+";\n");
    }

    prefix.append("precision "+std::string(precision)+" float;\n");
    m_code.insert(0,prefix);
    return true;
}

bool shader_code_parser::convert_to_glsl3()
{
    m_uniforms.clear();
    m_attributes.clear();

    if(!parse_predefined_uniforms(m_replace_str.c_str(),true))
    {
        m_error.append("unable to parse predefined uniforms\n");
        return false;
    }

    if(!parse_attributes(m_replace_str.c_str(),m_replace_str.c_str()))
    {
        m_error.append("unable to parse predefined attributes\n");
        return false;
    }

    std::string prefix="#version 330\n#define OPENGL3 1\n";

    for(int i=0;i<(int)m_uniforms.size();++i)
        prefix.append("uniform mat4 "+m_uniforms[i].name+";\n");

    parse_uniforms(false);

    for(int i=0;i<(int)m_attributes.size();++i)
    {
        prefix.append("in ");
        prefix.append(m_attributes[i].type==type_vec3?"vec3 ":"vec4 ");
        prefix.append(m_attributes[i].name+";\n");
    }

    //prefix.append("precision highp float;\n");

    replace_variable("texture2D","texture");
    replace_variable("texture2DProj","textureProj");
    replace_variable("textureCube","texture");

    const char *gl_ps_out="gl_FragColor";
    std::string ps_out_var=m_replace_str+std::string(gl_ps_out+3);
    const bool is_fragment=replace_variable(gl_ps_out,ps_out_var.c_str());
    if(is_fragment)
    {
        prefix.append("layout(location=0) out vec4 "+ps_out_var+";\n");
        replace_variable("varying","in");
    }
    else
    {
        //replace_variable("attribute","in");
        replace_variable("varying","out");
    }
    
    m_code.insert(0,prefix);
    return true;
}

int shader_code_parser::get_uniforms_count()
{
    if(m_uniforms.empty())
    {
        parse_predefined_uniforms(m_replace_str.c_str(),false);
        parse_uniforms(false);
    }

    return (int)m_uniforms.size();
}

shader_code_parser::variable shader_code_parser::get_uniform(int idx) const
{
    if(idx<0 || idx>=(int)m_uniforms.size())
        return shader_code_parser::variable();

    return m_uniforms[idx];
}

int shader_code_parser::get_attributes_count()
{
    if(m_attributes.empty())
        parse_attributes(m_replace_str.c_str(),0);

    return (int)m_attributes.size();
}

shader_code_parser::variable shader_code_parser::get_attribute(int idx) const
{
    if(idx<0 || idx>=(int)m_attributes.size())
        return shader_code_parser::variable();

    return m_attributes[idx];
}

void shader_code_parser::remove_comments()
{
    while(m_code.find("//")!=std::string::npos)
    {
        const size_t from=m_code.find("//");
        m_code.erase(from,m_code.find_first_of("\n\r",from+1)-from);
    }
    while(m_code.find("/*")!=std::string::npos)
    {
        const size_t from=m_code.find("/*");
        m_code.erase(from,(m_code.find("*/",from+2)-from)+2);
    }
}

template<typename t> static bool parse_vars(std::string &code,std::string &error,t& vars,const char *str,bool remove)
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
        while(code[last]!=';')
        {
            if(++last>=code.length())
            {
                error.append("unclosed ; on variable declaration\n");
                return false;
            }
        }

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

bool shader_code_parser::parse_uniforms(bool remove) { return parse_vars(m_code,m_error,m_uniforms,"uniform",remove); }
bool shader_code_parser::parse_varying(bool remove) { return parse_vars(m_code,m_error,m_varying,"varying",remove); }

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

bool shader_code_parser::parse_predefined_uniforms(const char *replace_prefix_str,bool replace)
{
    if(!replace_prefix_str)
        return false;

    const char *gl_matrix_names[]={"gl_ModelViewMatrix","gl_ModelViewProjectionMatrix","gl_ProjectionMatrix"};

    for(size_t i=0;i<sizeof(gl_matrix_names)/sizeof(gl_matrix_names[0]);++i)
    {
        std::string to=std::string(replace_prefix_str)+std::string(gl_matrix_names[i]+3); //strlen("gl_")
        if(replace)
        {
            if(replace_variable(gl_matrix_names[i],to.c_str()))
                push_unique_to_vec(m_uniforms,variable(type_mat4,to.c_str(),1));
        }
        else if(find_variable(gl_matrix_names[i]))
            push_unique_to_vec(m_uniforms,variable(type_mat4,to.c_str(),1));
    }

    return true;
}

bool shader_code_parser::parse_attributes(const char *info_replace_str,const char *code_replace_str)
{
    if(!info_replace_str)
        return false;

    const char *gl_attr_names[]={"gl_Vertex","gl_Normal","gl_Color"};
    variable_type gl_attr_types[]={type_vec4,type_vec3,type_vec4};

    for(size_t i=0;i<sizeof(gl_attr_names)/sizeof(gl_attr_names[0]);++i)
    {
        const std::string info=std::string(info_replace_str)+std::string(gl_attr_names[i]+3); //strlen("gl_")
        if(code_replace_str)
        {
            const std::string replace=std::string(code_replace_str)+std::string(gl_attr_names[i]+3); //strlen("gl_")
            if(replace_variable(gl_attr_names[i],replace.c_str()))
                push_unique_to_vec(m_attributes,variable(gl_attr_types[i],info.c_str(),0));
        }
        else
        {
            if(m_code.find(gl_attr_names[i],0)!=std::string::npos)
                push_unique_to_vec(m_attributes,variable(gl_attr_types[i],info.c_str(),0));
        }
    }

    const char *tc_atr_name="gl_MultiTexCoord";

    size_t start_pos=0;
    while((start_pos=m_code.find(tc_atr_name,start_pos))!=std::string::npos)
    {
        const size_t replace_start=start_pos;
        start_pos+=strlen(tc_atr_name);
        const int idx=atoi(&m_code[start_pos]);

        if(code_replace_str)
            m_code.replace(replace_start,3,code_replace_str); //strlen("gl_")

        char buf[255];
        sprintf(buf,"%s%s%d",info_replace_str,tc_atr_name+3,idx);
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

static void remove_var_spaces(std::string &s)
{
    if(s.empty())
        return;

    size_t f,t=0;
    for(f=0;f<s.size();++f) if(s[f]>' ') break;
    for(t=s.size();t>0;--t) if(s[t-1]>' ') break;

    s=s.substr(f,t);
}

static bool is_name_char(char c) { return isalnum(c) || c=='_'; }

bool shader_code_parser::replace_hlsl_mul(const char *func_name)
{
    if(!func_name)
        return false;

    typedef std::pair<size_t,size_t> scope;
    scope global_scope=scope(0,std::string::npos);
    typedef std::map<std::string,scope> mat_map;
    mat_map matrices;
    for(int i=0;i<(int)m_varying.size();++i) if(m_varying[i].type==type_mat4) matrices[m_varying[i].name]=global_scope;
    for(int i=0;i<(int)m_uniforms.size();++i) if(m_uniforms[i].type==type_mat4) matrices[m_uniforms[i].name]=global_scope;

    size_t start_pos=0;
    while((start_pos=m_code.find("mat",start_pos))!=std::string::npos)
    {
        if(start_pos!=0 && is_name_char(m_code[start_pos-1]))
        {
            start_pos+=3;//strlen("mat")
            continue;
        }

        if(start_pos+5>=m_code.size()) //strlen("mat4 ")
            break;

        start_pos+=3;//strlen("mat")
        if(!strchr("234",m_code[start_pos]))
            continue;

        if(is_name_char(m_code[++start_pos]))
           continue;

        const size_t end_pos=m_code.find(';',start_pos);
        if(end_pos==std::string::npos)
            break;

        std::string var=m_code.substr(start_pos,end_pos-start_pos);
        const size_t end_pos2=var.find('=');
        if(end_pos2<end_pos)
            var.resize(end_pos2);
        remove_var_spaces(var);
        matrices[var]=std::make_pair(end_pos,std::string::npos); //ToDo: find right scope border
    }

    bool result=false;
    start_pos=0;
    while((start_pos=m_code.find('*',start_pos))!=std::string::npos)
    {
        const size_t left=get_var_pos(m_code,start_pos,-1);
        const size_t right=get_var_pos(m_code,start_pos,1);
        if(left==start_pos || right==start_pos)
        {
            m_error.append("unable to parse variables in '*' to 'mul' replacement\n");
            return false;
        }

        std::string left_var=m_code.substr(left,start_pos-left);
        std::string right_var=m_code.substr(start_pos+1,right-start_pos-1);

        remove_var_spaces(right_var);
        if(right_var.empty()) // *=
        {
            ++start_pos;
            continue;
        }

        remove_var_spaces(left_var);

        //ToDo: not sure if matrix*scalar don't need mul, in case of fail replace || with &&
        if(is_numeric_only_var(left_var) || is_numeric_only_var(right_var))
        {
            ++start_pos;
            continue;
        }

        mat_map::const_iterator left_scope=matrices.find(left_var);
        mat_map::const_iterator right_scope=matrices.find(right_var);
        if((left_scope==matrices.end() || left_scope->second.first>start_pos || left_scope->second.second<start_pos) &&
           (right_scope==matrices.end() || right_scope->second.first>start_pos || right_scope->second.second<start_pos))
        {
            //ToDo: detect mul(mat,mat) as mat
            //ToDo: detect mat() constructors

            ++start_pos;
            continue;
        }

        const std::string replace=std::string(func_name)+"("+left_var+","+right_var+")";
        m_code.replace(left,right-left,replace);
        result=true;
    }

    return result;
}

bool shader_code_parser::replace_vec_from_float(const char *func_name)
{
    if(!func_name)
        return false;

    bool result=false;
    size_t start_pos=0;
    while((start_pos=m_code.find("vec",start_pos))!=std::string::npos)
    {
        if(start_pos>0 && is_name_char(m_code[start_pos-1]))
        {
            start_pos+=3;
            continue;
        }

        if(start_pos+4>m_code.size()) //strlen("vec")+1
        {
            m_error.append("incomplite vec declaration: code end spotted\n");
            return false;
        }

        const char dim=m_code[start_pos+3];
        if(!strchr("234",dim))
        {
            start_pos+=3;
            continue;
        }

        size_t brace_start=start_pos+4;

        while(m_code[brace_start]<=' ') if(++brace_start>=m_code.length()) return false;
        if(m_code[brace_start]!='(')
        {
            start_pos+=3;
            continue;
        }

        bool has_comma=false;
        int brace_count=0;
        size_t brace_end=brace_start;
        while(++brace_end<m_code.length())
        {
            char c=m_code[brace_end];
            if(c=='(')
            {
                ++brace_count;
                continue;
            }

            if(c==')')
            {
                if(--brace_count<0) break;
                continue;
            }

            if(!brace_count && c==',')
            {
                has_comma=true;
                break;
            }
        }

        if(!has_comma)
        {
            std::string replace=func_name+std::string(1,dim)+"("+m_code.substr(brace_start+1,brace_end-brace_start-1)+")";
            m_code.replace(start_pos,brace_end+1-start_pos,replace);
            result=true;
        }

        start_pos+=3;
    }

    return result;
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
        {
            m_error.append("can't find '(' in void function declaration\n");
            return false;
        }

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
        {
            m_error.append("can't find ')' in void function declaration\n");
            return false;
        }

        m_code.replace(start_pos,rbrace+1-start_pos,replace_str);
        return true;
    }

    return false;
}

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

bool shader_code_parser::find_variable(const char *str,size_t start_pos)
{
    if(!str)
        return false;

    const size_t str_len=strlen(str);
    while((start_pos=m_code.find(str,start_pos))!=std::string::npos)
    {
        if((start_pos!=0 && is_name_char(m_code[start_pos-1])) ||
           (start_pos+str_len<m_code.size() && is_name_char(m_code[start_pos+str_len])))
        {
            start_pos+=str_len;
            continue;
        }

        return true;
    }

    return false;
}

}
