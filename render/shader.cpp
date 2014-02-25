//https://code.google.com/p/nya-engine/

/*
    ToDo: more log, cleanup ARB
*/

#include "shader.h"
#include "platform_specific_gl.h"
#include "render.h"

//#define CACHE_UNIFORM_CHANGES

#ifdef OPENGL_ES
    #define GLhandleARB GLuint
#endif

#ifdef DIRECTX11
    #include <map>

  #ifndef WINDOWS_PHONE8
    #include <D3Dcompiler.h>
  #endif
#endif

#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    #define SUPPORT_OLD_SHADERS
#endif

#if defined DIRECTX11 || defined SUPPORT_OLD_SHADERS
    #include "transform.h"
#endif

namespace nya_render
{

namespace
{
    compiled_shaders_provider *render_csp=0;

    int current_shader=-1;
    int active_shader=-1;

    struct shader_obj
    {
#ifdef DIRECTX11
    public:
        shader_obj(): vertex_program(0),pixel_program(0) {}

    public:
        compiled_shader compiled[shader::program_types_count];
        ID3D11VertexShader *vertex_program;
        ID3D11PixelShader *pixel_program;

        typedef std::map<int,ID3D11InputLayout*> layouts_map;
        shader_obj::layouts_map shader_obj::layouts;

        struct constants_buffer
        {
            mutable std::vector<float> buffer;
            bool mvp_matrix;
            bool mv_matrix;
            bool p_matrix;

            ID3D11Buffer *dx_buffer;

            constants_buffer(): mvp_matrix(false),mv_matrix(false),p_matrix(false),dx_buffer(0) {}
        };

        constants_buffer constants;

        struct uniforms_buffer
        {
            std::vector<float> buffer;
            ID3D11Buffer *dx_buffer;
            bool changed;

            uniforms_buffer(): dx_buffer(0),changed(false) {}
        };

        uniforms_buffer vertex_uniforms;
        uniforms_buffer pixel_uniforms;

        struct uniform
        {
            std::string name;
            int vs_offset;
            int ps_offset;
            int dimension;
            int array_size;

            uniform(): vs_offset(-1), ps_offset(-1) {}
        };

        std::vector<uniform> uniforms;

        uniform &add_uniform(const std::string &name)
        {
            for(int i=0;i<int(uniforms.size());++i)
            {
                if(uniforms[i].name==name)
                {
                    uniforms.push_back(uniforms[i]);
                    uniforms.erase(uniforms.begin()+i);
                    return uniforms.back();
                }
            }

            uniforms.resize(uniforms.size()+1);
            uniforms.back().name=name;
            return uniforms.back();
        }
#else
    public:
        shader_obj(): program(0)
        {
            for(int i = 0;i<shader::program_types_count;++i)
                objects[i]=0;

#ifdef SUPPORT_OLD_SHADERS
            mat_mvp=-1;
            mat_mv=-1;
            mat_p=-1;
#endif
        }

    public:
        GLhandleARB program;
        GLhandleARB objects[shader::program_types_count];

#ifdef SUPPORT_OLD_SHADERS
        int mat_mvp;
        int mat_mv;
        int mat_p;
#endif
#endif
    public:
        static int add() { return get_shader_objs().add(); }
        static shader_obj &get(int idx) { return get_shader_objs().get(idx); }
        static void remove(int idx) { return get_shader_objs().remove(idx); }
        static void invalidate() { return get_shader_objs().invalidate(); DIRECTX11_ONLY(remove_layouts()); }

#ifdef DIRECTX11
        static void remove_layout(int mesh_idx)
        {
            struct remover
            {
                int mesh_idx;

                void apply(shader_obj &obj)
                {
                    layouts_map::iterator it=obj.layouts.find(mesh_idx);
                    if(it!=obj.layouts.end())
                    {
                        if(it->second)
                            it->second->Release();

                        obj.layouts.erase(it);
                    }
                }
            };

            remover r;
            r.mesh_idx=mesh_idx;
            get_shader_objs().apply_to_all(r);
        }

        static void remove_layouts()
        {
            struct remover{ void apply(shader_obj &obj) { obj.layouts.clear(); } };
            remover r; get_shader_objs().apply_to_all(r);
        }
#endif

    private:
        typedef render_objects<shader_obj> shader_objs;
        static shader_objs &get_shader_objs()
        {
            static shader_objs objs;
            return objs;
        }
    };

#ifndef DIRECTX11
  #ifndef NO_EXTENSIONS_INIT
    PFNGLGENPROGRAMSARBPROC glGenProgramsARB = NULL;
    PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = NULL;
    PFNGLBINDPROGRAMARBPROC glBindProgramARB = NULL;
    PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = NULL;
    PFNGLDETACHOBJECTARBPROC glDetachObjectARB = NULL;
    PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
    PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
    PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
    PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
    PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
    PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
    PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = NULL;
    PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB = NULL;
    PFNGLUNIFORM4FARBPROC glUniform4fARB = NULL;
    PFNGLUNIFORM1IARBPROC glUniform1iARB = NULL;
    PFNGLUNIFORM3FVARBPROC glUniform3fvARB = NULL;
    PFNGLUNIFORM4FVARBPROC glUniform4fvARB = NULL;
    PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB = NULL;
    PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
    PFNGLGETINFOLOGARBPROC glGetInfoLogARB = NULL;
    PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = NULL;
#endif

  #ifdef OPENGL_ES
    #define glCreateProgramObjectARB glCreateProgram
    #define glUseProgramObjectARB glUseProgram
    #define glLinkProgramARB glLinkProgram
    #define glValidateProgramARB glValidateProgram

    #define glCreateShaderObjectARB glCreateShader
    #define glDeleteObjectARB glDeleteShader
    #define glAttachObjectARB glAttachShader
    #define glDetachObjectARB glDetachShader
    #define glShaderSourceARB glShaderSource
    #define glCompileShaderARB glCompileShader
    #define glGetInfoLogARB glGetShaderInfoLog

    #define glGetUniformLocationARB glGetUniformLocation
    #define glUniform1iARB glUniform1i
    #define glUniform4fARB glUniform4f
    #define glUniform3fvARB glUniform3fv
    #define glUniform4fvARB glUniform4fv
    #define glUniformMatrix4fvARB glUniformMatrix4fv

    #ifndef GL_VERTEX_SHADER_ARB
        #define GL_VERTEX_SHADER_ARB GL_VERTEX_SHADER
    #endif

    #ifndef GL_FRAGMENT_SHADER_ARB
        #define GL_FRAGMENT_SHADER_ARB GL_FRAGMENT_SHADER
    #endif

    #ifndef GL_ARB_shader_objects
        #define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
        #define GL_OBJECT_INFO_LOG_LENGTH_ARB GL_INFO_LOG_LENGTH
        #define GL_OBJECT_LINK_STATUS_ARB GL_LINK_STATUS
        #define GL_OBJECT_VALIDATE_STATUS_ARB GL_VALIDATE_STATUS
    #endif

    #define glGetObjectParam glGetProgramiv
    #define glGetShaderParam glGetShaderiv
  #else
    #define glGetObjectParam glGetObjectParameterivARB
    #define glGetShaderParam glGetObjectParameterivARB
  #endif

bool check_init_shaders()
{
    static bool initialised=false;
    static bool failed=true;
    if(initialised)
        return !failed;

  #ifndef NO_EXTENSIONS_INIT
    if(!(glGenProgramsARB          =(PFNGLGENPROGRAMSARBPROC)          get_extension("glGenPrograms"))) return false;
    if(!(glDeleteProgramsARB       =(PFNGLDELETEPROGRAMSARBPROC)       get_extension("glDeletePrograms"))) return false;
    if(!(glBindProgramARB          =(PFNGLBINDPROGRAMARBPROC)          get_extension("glBindProgram"))) return false;
    if(!(glDeleteObjectARB         =(PFNGLDELETEOBJECTARBPROC)         get_extension("glDeleteObject"))) return false;
    if(!(glDetachObjectARB         =(PFNGLDETACHOBJECTARBPROC)         get_extension("glDetachObject"))) return false;
    if(!(glCreateShaderObjectARB   =(PFNGLCREATESHADEROBJECTARBPROC)   get_extension("glCreateShaderObject"))) return false;
    if(!(glShaderSourceARB         =(PFNGLSHADERSOURCEARBPROC)         get_extension("glShaderSource"))) return false;
    if(!(glCompileShaderARB        =(PFNGLCOMPILESHADERARBPROC)        get_extension("glCompileShader"))) return false;
    if(!(glCreateProgramObjectARB  =(PFNGLCREATEPROGRAMOBJECTARBPROC)  get_extension("glCreateProgramObject"))) return false;
    if(!(glAttachObjectARB         =(PFNGLATTACHOBJECTARBPROC)         get_extension("glAttachObject"))) return false;
    if(!(glLinkProgramARB          =(PFNGLLINKPROGRAMARBPROC)          get_extension("glLinkProgram"))) return false;
    if(!(glUseProgramObjectARB     =(PFNGLUSEPROGRAMOBJECTARBPROC)     get_extension("glUseProgramObject" ))) return false;
    if(!(glValidateProgramARB      =(PFNGLVALIDATEPROGRAMARBPROC)      get_extension("glValidateProgram" ))) return false;
    if(!(glUniform4fARB            =(PFNGLUNIFORM4FARBPROC)            get_extension("glUniform4f"))) return false;
    if(!(glUniform1iARB            =(PFNGLUNIFORM1IARBPROC)            get_extension("glUniform1i"))) return false;
    if(!(glUniform3fvARB           =(PFNGLUNIFORM3FVARBPROC)           get_extension("glUniform3fv"))) return false;
    if(!(glUniform4fvARB           =(PFNGLUNIFORM4FVARBPROC)           get_extension("glUniform4fv"))) return false;
    if(!(glUniformMatrix4fvARB     =(PFNGLUNIFORMMATRIX4FVARBPROC)     get_extension("glUniformMatrix4fv"))) return false;
    if(!(glGetObjectParameterivARB =(PFNGLGETOBJECTPARAMETERIVARBPROC) get_extension("glGetObjectParameteriv"))) return false;
    if(!(glGetInfoLogARB           =(PFNGLGETINFOLOGARBPROC)           get_extension("glGetInfoLog"))) return false;
    if(!(glGetUniformLocationARB   =(PFNGLGETUNIFORMLOCATIONARBPROC)   get_extension("glGetUniformLocation"))) return false;
  #endif
    failed=false;
    initialised=true;
    return true;
}
#endif
}

void invalidate_shaders() { shader_obj::invalidate(); }

void set_compiled_shaders_provider(compiled_shaders_provider *csp) { render_csp=csp; }

#ifdef DIRECTX11
ID3D11InputLayout *get_layout(int mesh_idx)
{
    if(current_shader<0)
        return 0;

    shader_obj::layouts_map::iterator it=shader_obj::get(current_shader).layouts.find(mesh_idx);
    if(it==shader_obj::get(current_shader).layouts.end())
        return 0;

    return it->second;
}

ID3D11InputLayout *add_layout(int mesh_idx,
                              const D3D11_INPUT_ELEMENT_DESC*desc,size_t desc_size)
{
    if(current_shader<0)
        return 0;

    shader_obj &shdr=shader_obj::get(current_shader);

	if(!shdr.compiled[shader::vertex].get_data())
		return 0;

    ID3D11InputLayout *out=0;
	get_device()->CreateInputLayout(desc,desc_size,shdr.compiled[shader::vertex].get_data(),
                                    shdr.compiled[shader::vertex].get_size(),&out);
    if(!out)
        return 0;

    shdr.layouts[mesh_idx]=out;

    return out;
}

void remove_layout(int mesh_idx) { shader_obj::remove_layout(mesh_idx); }
#else
    void set_shader(int idx,bool ignore_cache=false)
    {
        if(idx==active_shader && !ignore_cache)
            return;

        if(!check_init_shaders())
            return;

        if(idx<0)
        {
            glUseProgramObjectARB(0);
            active_shader=-1;
            return;
        }

        shader_obj &shdr=shader_obj::get(idx);
        glUseProgramObjectARB(shdr.program);
        if(!shdr.program)
            active_shader=-1;
        else
            active_shader=idx;
    }
#endif

bool shader::add_program(program_type type,const char*code)
{
    if(type>pixel)
        return false;

    if(!code || !code[0])
    {
        get_log()<<"Unable to add shader program: invalid code\n";
        return false;
    }

    const static char type_str[][12]={"vertex","pixel","geometry","tesselation"};

    //ToDo: release or reuse if already exists

#ifdef DIRECTX11
    if(!get_device())
        return false;

    if(m_shdr<0)
        m_shdr=shader_obj::add();

    shader_obj &shdr=shader_obj::get(m_shdr);

    std::string code_str(code);
    std::string code_final;

    shader_obj::uniforms_buffer &buf=(type==vertex)?shdr.vertex_uniforms:shdr.pixel_uniforms;
    for(size_t i=code_str.find("uniform");i!=std::string::npos;i=code_str.find("uniform",i+7))
    {
        size_t prevln=code_str.rfind('\n',i);
        if(prevln!=std::string::npos)
        {
            size_t comment=code_str.find("//",prevln);
            if(comment<i)
                continue;
        }

        size_t type_from=i+8;
        while(code_str[type_from]<=' ') if(++type_from>=code_str.length()) return false;
        size_t type_to=type_from;
        while(code_str[type_to]>' ') if(++type_to>=code_str.length()) return false;

        size_t name_from=type_to+1;
        while(code_str[name_from]<=' ') if(++name_from>=code_str.length()) return false;
        size_t name_to=name_from;
        while(code_str[name_to]>' ' && code_str[name_to]!=';' && code_str[name_to]!='[') if(++name_to>=code_str.length()) return false;

        const std::string name=code_str.substr(name_from,name_to-name_from);
        const std::string type_name=code_str.substr(type_from,type_to-type_from);

        size_t last=name_to;
        while(code_str[last]!=';') if(++last>=code_str.length()) return false;

        int count=1;
        size_t array_from=code_str.find('[',name_to);
        if(array_from<last)
            count=atoi(&code_str[array_from+1]);

        if(count<=0)
            return false;

        for(size_t c=i;c<=last;++c)
            code_str[c]=' ';

		if(type_name.compare(0,5,"float")==0)
        {
            shader_obj::uniform &u=shdr.add_uniform(name);
            if(type==vertex)
                u.vs_offset=(int)buf.buffer.size();
            else
                u.ps_offset=(int)buf.buffer.size();

            char dim=(type_name.length()==6)?type_name[5]:'\0';

            int dimension=1;
            switch(dim)
            {
            case '2':
                dimension=2;
                buf.buffer.resize(buf.buffer.size()+4*count,0.0f);
                break;

            case '3':
                dimension=3;
                buf.buffer.resize(buf.buffer.size()+4*count,0.0f);
                break;

            case '4':
                dimension=4;
                buf.buffer.resize(buf.buffer.size()+4*count,0.0f);
                break;

            default:
                dimension=1;
                buf.buffer.resize(buf.buffer.size()+4,0.0f);
                break;
            };

            if(u.vs_offset>=0 && u.ps_offset>=0) //ToDo: log error
            {
                if(u.array_size!=count)
                    return false;

                if(u.dimension!=dimension)
                    return false;
            }

            u.array_size=count;
            u.dimension=dimension;

            buf.changed=true;
        }
        else if(type_name=="sampler2D")
        {
            int reg=-1;
            for(int i=0;i<int(m_samplers.size());++i)
            {
				if(m_samplers[i].name==name)
                {
					reg=m_samplers[i].layer;
                    break;
                }
            }

            if(reg<0) //ToDo: log error
                return false;

            char buf[512];
            sprintf(buf,"Texture2D %s: register(t%d); SamplerState %s_nya_st: register(s%d);\n",name.c_str(),reg,name.c_str(),reg);
            code_final.append(buf);
        }
    }

    if(!buf.buffer.empty())
    {
        code_final.append("cbuffer NyaUniformsBuffer : register");
        if(type==vertex)
            code_final.append("(b1){");
        else
            code_final.append("(b2){");

        for(size_t i=0;i<shdr.uniforms.size();++i)
        {
            const shader_obj::uniform &u=shdr.uniforms[i];

            if(type==vertex)
            {
                if(u.vs_offset<0)
                    continue;
            }
            else if(type==pixel)
            {
                if(u.ps_offset<0)
                    continue;
            }

            switch(u.dimension)
            {
            case 2:
            case 3:
            case 4:
                code_final.append("float");
                code_final.push_back('0'+u.dimension);
                code_final.push_back(' ');
                code_final.append(u.name);
                if(u.array_size>1)
                {
                    char buf[255];
                    sprintf(buf,"[%d]",u.array_size);
                    code_final.append(buf);
                }
                code_final.push_back(';');
                break;

            default: return false;
            }
        }

        code_final.append("}\n");

        CD3D11_BUFFER_DESC desc(buf.buffer.size()*sizeof(float),D3D11_BIND_CONSTANT_BUFFER);
        get_device()->CreateBuffer(&desc,nullptr,&buf.dx_buffer);
    }

    if(type==vertex)
    {
        shdr.layouts.clear();

        shdr.constants.mvp_matrix=code_str.find("ModelViewProjectionMatrix")!=std::string::npos;
        shdr.constants.mv_matrix=code_str.find("ModelViewMatrix")!=std::string::npos;
        for(size_t i=code_str.find("ProjectionMatrix");i!=std::string::npos;
                                                       i=code_str.find("ProjectionMatrix",i+16))
        {
            if(code_str[i-1]!='w')
            {
                shdr.constants.p_matrix=true;
                break;
            }
        }

        int size=0;
        if(shdr.constants.mvp_matrix) size+=16;
        if(shdr.constants.mv_matrix) size+=16;
        if(shdr.constants.p_matrix) size+=16;

        if(shdr.constants.dx_buffer)
            shdr.constants.dx_buffer->Release();

        shdr.constants.dx_buffer=0;

        if(size>0)
        {
            code_final.append("cbuffer NyaConstantBuffer : register(b0){");
            if(shdr.constants.mvp_matrix) code_final.append("matrix ModelViewProjectionMatrix;");
            if(shdr.constants.mv_matrix) code_final.append("matrix ModelViewMatrix;");
            if(shdr.constants.p_matrix) code_final.append("matrix ProjectionMatrix;");
            code_final.append("}\n");

            CD3D11_BUFFER_DESC desc(size*sizeof(float),D3D11_BIND_CONSTANT_BUFFER);
            get_device()->CreateBuffer(&desc,nullptr,&shdr.constants.dx_buffer);
        }

        shdr.constants.buffer.resize(size);
    }

    code_final.append(code_str);

    if(render_csp)
        render_csp->get(code_final.c_str(),shdr.compiled[type]);

    if(!shdr.compiled[type].get_data())
    {
#ifdef WINDOWS_PHONE8
        get_log()<<"Can`t compile "<<type_str[type]<<" shader: Windows phone 8 not allowed to compile shaders, please, set compiled_shaders_provider and add compiled shaders cache\n";
        return false;
#else
        ID3D10Blob *compiled=0;
	    ID3D10Blob *error=0;
        if(type==vertex)
    	    D3DCompile(code_final.c_str(),code_final.size(),0,0,0,"main","vs_4_0_level_9_3",0,0,&compiled,&error);
        else if(type==pixel)
            D3DCompile(code_final.c_str(),code_final.size(),0,0,0,"main","ps_4_0_level_9_3",0,0,&compiled,&error);

        if(error)
        {
            get_log()<<"Can`t compile "<<type_str[type]<<" shader: \n";
            std::string error_text((const char *)error->GetBufferPointer(),error->GetBufferSize());
            get_log()<<error_text.c_str()<<"\n";

            error->Release();
            return false;
        }

        shdr.compiled[type]=compiled_shader(compiled->GetBufferSize());
        memcpy(shdr.compiled[type].get_data(),compiled->GetBufferPointer(),compiled->GetBufferSize());
        compiled->Release();

        if(render_csp)
            render_csp->set(code_final.c_str(),shdr.compiled[type]);
#endif
    }

    if(type==vertex)
    {
        if(get_device()->CreateVertexShader(shdr.compiled[type].get_data(),
                                            shdr.compiled[type].get_size(),nullptr,&shdr.vertex_program)<0)
        {
            get_log()<<"Can`t create "<<type_str[type]<<" shader\n";
            return false;
        }
    }

    if(type==pixel)
    {
        if(get_device()->CreatePixelShader(shdr.compiled[type].get_data(),
                                           shdr.compiled[type].get_size(),nullptr,&shdr.pixel_program)<0)
        {
            get_log()<<"Can`t create "<<type_str[type]<<" shader\n";
            return false;
        }
    }

#else
    if(!check_init_shaders())
        return false;

    if(m_shdr<0)
        m_shdr=shader_obj::add();

    shader_obj &shdr=shader_obj::get(m_shdr);

    if(!shdr.program)
        shdr.program=glCreateProgramObjectARB();

    if(!shdr.program)
    {
        get_log()<<"Unable to create shader program object\n";
        return false;
    }

    if(shdr.objects[type])
    {
        glDetachObjectARB(shdr.program,shdr.objects[type]);
        glDeleteObjectARB(shdr.objects[type]);
        shdr.objects[type]=0;
    }

#ifdef SUPPORT_OLD_SHADERS
    std::string code_str(code);
    std::string code_final;

    if(type==vertex)
    {
        const char *attribute_names[]={"nyaVertex","nyaNormal","nyaColor","nyaMultiTexCoord"};

        bool used_attribs[max_attributes]={false};
        shdr.mat_mvp=-1;
        shdr.mat_mv=-1;
        shdr.mat_p=-1;

        for(size_t gl=code_str.find("gl_");gl!=std::string::npos;gl=code_str.find("gl_",gl+8))
        {
            if(code_str.size()<=gl+8)
                break;

            bool replace=false;

            switch(code_str[gl+3])
            {
                case 'V':
                    if(code_str.compare(gl+3,6,"Vertex")==0)
                    {
                        used_attribs[vertex_attribute]=true;
                        replace=true;
                    }
                    break;

                case 'N':
                    if(code_str.compare(gl+3,6,"Normal")==0)
                    {
                        used_attribs[normal_attribute]=true;
                        replace=true;
                    }
                    break;

                case 'C':
                    if(code_str.compare(gl+3,5,"Color")==0)
                    {
                        used_attribs[color_attribute]=true;
                        replace=true;
                    }
                    break;

                case 'M':
                    if(code_str.compare(gl+3,13,"MultiTexCoord")==0)
                    {
                        if(code_str.size()<=gl+17)
                            break;

                        char n0=code_str[gl+16];
                        char n1=code_str[gl+17];

                        if(n0<'0' || n0>'9')
                            break;

                        int idx=n0-'0';
                        if(n1>'0' && n1<='9')
                            idx=idx*10+n1-'0';

                        if(tc0_attribute+idx>=max_attributes)
                            break;

                        used_attribs[tc0_attribute+idx]=true;
                        replace=true;
                    }
                    else if(code_str.size()>gl+15 && code_str.compare(gl+3,15,"ModelViewMatrix")==0)
                    {
                        shdr.mat_mv=1;
                        replace=true;
                    }
                    else if(code_str.size()>gl+25 && code_str.compare(gl+3,25,"ModelViewProjectionMatrix")==0)
                    {
                        shdr.mat_mvp=1;
                        replace=true;
                    }
                    break;

                case 'P':
                    if(code_str.size()>gl+16 && code_str.compare(gl+3,16,"ProjectionMatrix")==0)
                    {
                        shdr.mat_p=1;
                        replace=true;
                    }
                    break;

                //case 'T':     //ToDo: gl_TexCoord[] variables
                //break;
            }

            if(replace)
            {
                code_str[gl]='n';
                code_str[gl+1]='y';
                code_str[gl+2]='a';
            }
        }

        if(shdr.mat_mvp>0)
            code_final.append("uniform mat4 nyaModelViewProjectionMatrix;");
        if(shdr.mat_mv>0)
            code_final.append("uniform mat4 nyaModelViewMatrix;");
        if(shdr.mat_p>0)
            code_final.append("uniform mat4 nyaProjectionMatrix;");

        for(int i=0;i<tc0_attribute;++i)
        {
            if(!used_attribs[i])
                continue;

            code_final.append("attribute vec4 ");
            code_final.append(attribute_names[i]);
            code_final.append(";\n");

            glBindAttribLocation(shdr.program,i,attribute_names[i]);
        }

        for(int i=tc0_attribute;i<max_attributes;++i)
        {
            if(!used_attribs[i])
                continue;

            std::string attrib_name(attribute_names[tc0_attribute]);
            int idx=i-tc0_attribute;
            if(idx>=10)
            {
                int h=idx/10;
                attrib_name+=char('0'+h);
                idx-=h*10;
            }
            attrib_name+=char('0'+idx);

            code_final.append("attribute vec4 ");
            code_final.append(attrib_name);
            code_final.append(";\n");

            glBindAttribLocation(shdr.program,i,attrib_name.c_str());
        }
    }

    code_final.append("precision mediump float;\n");
    code_final.append(code_str);

    code=code_final.c_str();

    //get_log()<<code<<"\n";
#endif

    GLenum gl_type=GL_VERTEX_SHADER_ARB;
    if(type==pixel)                                    //ToDo: switch and all cases
        gl_type=GL_FRAGMENT_SHADER_ARB;

    GLhandleARB object = glCreateShaderObjectARB(gl_type);
    glShaderSourceARB(object,1,&code,0);
    glCompileShaderARB(object);
    GLint compiled;
    glGetShaderParam(object,GL_OBJECT_COMPILE_STATUS_ARB,&compiled);

    if(!compiled)
    {
        GLint log_len=0;
        get_log()<<"Can`t compile "<<type_str[type]<<" shader: \n";
        glGetShaderParam(object,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
        if(log_len>0)
        {
            GLchar *buf=new GLchar[log_len];
            glGetInfoLogARB(object,log_len,&log_len,buf);
            get_log()<<buf<<"\n";
            delete []buf;
        }

        shdr.program=0;
        return false;
    }
    glAttachObjectARB(shdr.program,object);

#ifdef OPENGL_ES
    shdr.objects[type]=object;

    if(shdr.program && shdr.objects[vertex] && shdr.objects[pixel])
#else
    if(shdr.program)
#endif
    {
        GLint result=0;
        glLinkProgramARB(shdr.program);
        glGetObjectParam(shdr.program,GL_OBJECT_LINK_STATUS_ARB,&result);
        if(!result)
        {
#ifdef OPENGL_ES
            get_log()<<"Can`t link shader\n";
#else
            get_log()<<"Can`t link "<<type_str[type]<<" shader\n";
#endif
            GLint log_len=0;
            glGetObjectParam(shdr.program,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
            if (log_len>0)
            {
                std::string log(log_len,0);
                glGetInfoLogARB(shdr.program,log_len,&log_len,&log[0]);
                get_log()<<log.c_str()<<"\n";
            }

            shdr.program=0; //??
            return false;
        }

        if(type==pixel)
        {
            set_shader(m_shdr);

            for(size_t i=0;i<m_samplers.size();++i)
            {
                const sampler &s=m_samplers[i];
                int handler=glGetUniformLocationARB(shdr.program,s.name.c_str());
                if(handler>=0)
                    glUniform1iARB(handler,s.layer);
                else
                    get_log()<<"Unable to set shader sampler \'"<<s.name.c_str()<<"\': probably not found\n";
            }

            set_shader(-1);
        }

        result=0;
        glValidateProgramARB(shdr.program);
        glGetObjectParam(shdr.program,GL_OBJECT_VALIDATE_STATUS_ARB,&result);
        if(!result)
        {
#ifdef OPENGL_ES
            get_log()<<"Can`t validate shader\n";
#else
            get_log()<<"Can`t validate "<<type_str[type]<<" shader\n";
#endif
            GLint log_len=0;
            glGetObjectParam(shdr.program,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
            if (log_len>0)
            {
                std::string log(log_len,0);
                glGetInfoLogARB(shdr.program,log_len,&log_len,&log[0]);
                get_log()<<log.c_str()<<"\n";
            }
            shdr.program=0; //??
            return false;
        }

#ifdef SUPPORT_OLD_SHADERS

        if(shdr.mat_mvp>=0)
            shdr.mat_mvp=get_handler("nyaModelViewProjectionMatrix");
        if(shdr.mat_mv>=0)
            shdr.mat_mv=get_handler("nyaModelViewMatrix");
        if(shdr.mat_p>=0)
            shdr.mat_p=get_handler("nyaProjectionMatrix");
#endif
    }

    shdr.objects[type]=object;
#endif

    return true;
}

void shader::bind() const { current_shader=m_shdr; }
void shader::unbind() { current_shader=-1; }

void shader::apply(bool ignore_cache)
{
#ifdef DIRECTX11
    if(current_shader<0)
    {
        if(ignore_cache)
            active_shader=-1;

        return;
    }

    if(!get_context())
        return;

    shader_obj &shdr=shader_obj::get(current_shader);

    if(shdr.vertex_program)
    {
        if(shdr.constants.dx_buffer)
        {
            int offset=0;
            if(shdr.constants.mvp_matrix)
            {
                memcpy(&shdr.constants.buffer[0],
                       transform::get().get_modelviewprojection_matrix().m[0],16*sizeof(float));
                offset+=16;
            }

            if(shdr.constants.mv_matrix)
            {
                memcpy(&shdr.constants.buffer[0]+offset,
                       transform::get().get_modelview_matrix().m[0],16*sizeof(float));
                offset+=16;
            }

            if(shdr.constants.p_matrix)
            {
                memcpy(&shdr.constants.buffer[0]+offset,
                       transform::get().get_projection_matrix().m[0],16*sizeof(float));
                offset+=16;
            }

            get_context()->UpdateSubresource(shdr.constants.dx_buffer,0,NULL,&shdr.constants.buffer[0],0,0);
        }

        if(shdr.vertex_uniforms.changed)
        {
            shdr.vertex_uniforms.changed=false;
            get_context()->UpdateSubresource(shdr.vertex_uniforms.dx_buffer,0,NULL,&shdr.vertex_uniforms.buffer[0],0,0);
        }

        if(current_shader!=active_shader || ignore_cache)
        {
            get_context()->VSSetConstantBuffers(0,1,&shdr.constants.dx_buffer);
            get_context()->VSSetConstantBuffers(1,1,&shdr.vertex_uniforms.dx_buffer);
            get_context()->VSSetShader(shdr.vertex_program,nullptr,0);
        }
    }

    if(shdr.pixel_program)
    {
        if(shdr.pixel_uniforms.changed)
        {
            shdr.pixel_uniforms.changed=false;
            get_context()->UpdateSubresource(shdr.pixel_uniforms.dx_buffer,0,NULL,&shdr.pixel_uniforms.buffer[0],0,0);
        }

        if(current_shader!=active_shader || ignore_cache)
        {
            get_context()->PSSetConstantBuffers(2,1,&shdr.pixel_uniforms.dx_buffer);
            get_context()->PSSetShader(shdr.pixel_program,nullptr,0);
        }
    }

    active_shader=current_shader;
#else
    set_shader(current_shader,ignore_cache);

  #ifdef SUPPORT_OLD_SHADERS
    if(current_shader<0)
        return;

    const shader_obj &shdr=shader_obj::get(current_shader);

    if(shdr.mat_mvp>=0)
        glUniformMatrix4fvARB(shdr.mat_mvp,1,false,transform::get().get_modelviewprojection_matrix().m[0]);
    if(shdr.mat_mv>=0)
        glUniformMatrix4fvARB(shdr.mat_mv,1,false,transform::get().get_modelview_matrix().m[0]);
    if(shdr.mat_p>=0)
        glUniformMatrix4fvARB(shdr.mat_p,1,false,transform::get().get_projection_matrix().m[0]);
  #endif
#endif
}

void shader::set_sampler(const char*name,unsigned int layer)
{
    if(!name || !name[0])
    {
        get_log()<<"Unable to set shader sampler: invalid name\n";
        return;
    }

    for(size_t i=0;i<m_samplers.size();++i)
    {
        if(m_samplers[i].layer!=layer)
            continue;

        m_samplers[i].name.assign(name);
        return;
    }

    m_samplers.push_back(sampler(name,layer));
}

int shader::get_handler(const char *name) const
{
    if(!name || !name[0])
    {
        get_log()<<"Unable to set shader handler: invalid name\n";
        return -1;
    }

#ifdef DIRECTX11
    if(m_shdr<0)
        return -1;

    shader_obj &shdr=shader_obj::get(m_shdr);
    for(int i=0;i<(int)shdr.uniforms.size();++i)
    {
        if(shdr.uniforms[i].name==name)
            return i;
    }

    return -1;
#else
    if(m_shdr<0)
        return -1;

    set_shader(m_shdr);

    if(!shader_obj::get(m_shdr).program)
    {
        get_log()<<"Unable to get shader handler \'"<<name<<"\': invalid program\n";
        return -1;
    }

    return glGetUniformLocationARB(shader_obj::get(m_shdr).program,name);
#endif
}

void shader::set_uniform(unsigned int i,float f0,float f1,float f2,float f3) const
{
#ifdef DIRECTX11
    if(m_shdr<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);
    if(i>=(int)shdr.uniforms.size())
        return;

    const shader_obj::uniform &u=shdr.uniforms[i];
    if(u.vs_offset>=0)
    {
        float *f=&shdr.vertex_uniforms.buffer[u.vs_offset];
#ifdef CACHE_UNIFORM_CHANGES
        if(f[0]!=f0 || f[1]!=f1 || f[2]!=f2 || f[3]!=f3)
#endif
            f[0]=f0,f[1]=f1,f[2]=f2,f[3]=f3,shdr.vertex_uniforms.changed=true;
    }
    if(u.ps_offset>=0)
    {
        float *f=&shdr.pixel_uniforms.buffer[u.ps_offset];
#ifdef CACHE_UNIFORM_CHANGES
        if(f[0]!=f0 || f[1]!=f1 || f[2]!=f2 || f[3]!=f3)
#endif
            f[0]=f0,f[1]=f1,f[2]=f2,f[3]=f3,shdr.pixel_uniforms.changed=true;
    }

#else
    if(m_shdr<0)
        return;

    set_shader(m_shdr);

    if(!shader_obj::get(m_shdr).program)
        return;

    glUniform4fARB(i,f0,f1,f2,f3);
#endif
}

void shader::set_uniform3_array(unsigned int i,const float *f,unsigned int count) const
{
#ifdef DIRECTX11
    if(m_shdr<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);
    if(i>=(int)shdr.uniforms.size())
        return;

    const shader_obj::uniform &u=shdr.uniforms[i];
    if(int(count)>u.array_size)
        count=u.array_size;

    if(u.vs_offset>=0)
    {
        const int size=sizeof(float)*3;
        for(int i=0,o=u.vs_offset,o2=0;i<int(count);++i,o+=4,o2+=3)
        {
#ifdef CACHE_UNIFORM_CHANGES
            if(memcmp(&shdr.vertex_uniforms.buffer[o],&f[o2],size)==0)
                continue;
#endif
            memcpy(&shdr.vertex_uniforms.buffer[o],&f[o2],size);
            shdr.vertex_uniforms.changed=true;
        }
    }
    if(u.ps_offset>=0)
    {
        const int size=sizeof(float)*3;
        for(int i=0,o=u.ps_offset,o2=0;i<int(count);++i,o+=4,o2+=3)
        {
#ifdef CACHE_UNIFORM_CHANGES
            if(memcmp(&shdr.pixel_uniforms.buffer[o],&f[o2],size)==0)
                continue;
#endif
            memcpy(&shdr.pixel_uniforms.buffer[o],&f[o2],size);
            shdr.pixel_uniforms.changed=true;
        }
    }

#else
    if(m_shdr<0)
        return;

    set_shader(m_shdr);

    if(!shader_obj::get(m_shdr).program || !f)
        return;

    glUniform3fvARB(i,count,f);
#endif
}

void shader::set_uniform4_array(unsigned int i,const float *f,unsigned int count) const
{
#ifdef DIRECTX11
    if(m_shdr<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);
    if(i>=(int)shdr.uniforms.size())
        return;

    const shader_obj::uniform &u=shdr.uniforms[i];
    if(int(count)>u.array_size)
        count=u.array_size;

    if(u.vs_offset>=0)
    {
        const size_t size=sizeof(float)*4*count;
#ifdef CACHE_UNIFORM_CHANGES
        if(memcmp(&shdr.vertex_uniforms.buffer[u.vs_offset],f,size)!=0)
#endif
        {
            memcpy(&shdr.vertex_uniforms.buffer[u.vs_offset],f,size);
            shdr.vertex_uniforms.changed=true;
        }
    }
    if(u.ps_offset>=0)
    {
        const size_t size=sizeof(float)*4*count;
#ifdef CACHE_UNIFORM_CHANGES
        if(memcmp(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size)!=0)
#endif
        {
            memcpy(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size);
            shdr.pixel_uniforms.changed=true;
        }
    }

#else
    if(m_shdr<0)
        return;

    set_shader(m_shdr);

    if(!shader_obj::get(m_shdr).program || !f)
        return;

    glUniform4fvARB(i,count,f);
#endif
}

void shader::set_uniform16_array(unsigned int i,const float *f,unsigned int count,bool transpose) const
{
#ifdef DIRECTX11
    if(m_shdr<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);
    if(i>=(int)shdr.uniforms.size())
        return;

    const shader_obj::uniform &u=shdr.uniforms[i];
    if(int(count)>u.array_size)
        count=u.array_size;

    if(u.vs_offset>=0)
    {
        const size_t size=sizeof(float)*16*count;
#ifdef CACHE_UNIFORM_CHANGES
        if(memcmp(&shdr.vertex_uniforms.buffer[u.vs_offset],f,size)!=0)
#endif
        {
            memcpy(&shdr.vertex_uniforms.buffer[u.vs_offset],f,size);
            shdr.vertex_uniforms.changed=true;
        }
    }
    if(u.ps_offset>=0)
    {
        const size_t size=sizeof(float)*16*count;
#ifdef CACHE_UNIFORM_CHANGES
        if(memcmp(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size)!=0)
#endif
        {
            memcpy(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size);
            shdr.pixel_uniforms.changed=true;
        }
    }

#else
    if(m_shdr<0)
        return;

    set_shader(m_shdr);

    if(!shader_obj::get(m_shdr).program || !f)
        return;

    glUniformMatrix4fvARB(i,count,transpose,f);
#endif
}

void shader::release()
{
    m_samplers.clear();

    if(m_shdr<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);

#ifdef DIRECTX11
    if(shdr.vertex_program)
        shdr.vertex_program->Release();

    if(shdr.pixel_program)
        shdr.pixel_program->Release();

    for(shader_obj::layouts_map::iterator it=shdr.layouts.begin();
        it!=shdr.layouts.end();++it)
        if(it->second)
            it->second->Release();

    shdr.layouts.clear();

    if(shdr.constants.dx_buffer)
        shdr.constants.dx_buffer->Release();

    if(shdr.vertex_uniforms.dx_buffer)
        shdr.vertex_uniforms.dx_buffer->Release();

    if(shdr.pixel_uniforms.dx_buffer)
        shdr.pixel_uniforms.dx_buffer->Release();

    shdr.constants = shader_obj::constants_buffer();
    shdr.vertex_uniforms = shader_obj::uniforms_buffer();
    shdr.pixel_uniforms = shader_obj::uniforms_buffer();

    if(m_shdr==active_shader)
        active_shader=-1;
#else
    if(m_shdr==active_shader)
        set_shader(-1);

    if(!shdr.program)
        return;

    for(int i=0;i<program_types_count;++i)
    {
        if(!shdr.objects[i])
            continue;

        glDetachObjectARB(shdr.program,shdr.objects[i]);
        glDeleteObjectARB(shdr.objects[i]);
    }

    glDeleteObjectARB(shdr.program);
#endif

    if(m_shdr==current_shader)
        current_shader=-1;

    shader_obj::remove(m_shdr);
    m_shdr=-1;
}

}
