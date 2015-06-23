//https://code.google.com/p/nya-engine/

#include "shader.h"
#include "shader_code_parser.h"
#include "platform_specific_gl.h"
#include "render.h"

//#define CACHE_UNIFORM_CHANGES
//#define CACHE_MATRIX_CHANGES
//#define CACHE_UNIFORM_ARRAY_CHANGES

#if defined CACHE_UNIFORM_ARRAY_CHANGES || defined CACHE_MATRIX_CHANGES
    #define CACHE_UNIFORM_CHANGES
#endif

#ifdef CACHE_UNIFORM_CHANGES
    #include "math/vector.h"
#endif

#ifdef OPENGL_ES
    #define GLhandleARB GLuint
#endif

#ifdef DIRECTX11
    #include <map>

  #ifndef WINDOWS_METRO
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
    int current_shader= -1,active_shader= -1;
    bool shaders_validation=false;

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
            int mvp_matrix,mv_matrix,p_matrix;
            ID3D11Buffer *dx_buffer;

            constants_buffer(): mvp_matrix(-1),mv_matrix(-1),p_matrix(-1),dx_buffer(0) {}
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
            int array_size;
            shader::uniform_type type;

            uniform(): vs_offset(-1), ps_offset(-1) {}
        };
#else
    public:
        shader_obj(): program(0)
        {
            for(int i = 0;i<shader::program_types_count;++i)
                objects[i]=0;

#ifdef SUPPORT_OLD_SHADERS
            mat_mvp=mat_mv=mat_p= -1;
#endif
        }

    public:
        GLhandleARB program;
        GLhandleARB objects[shader::program_types_count];

        struct uniform
        {
            std::string name;
            int array_size;

            shader::uniform_type type;

            uniform(): array_size(1) {}
        };

#ifdef SUPPORT_OLD_SHADERS
        int mat_mvp,mat_mv,mat_p;
#endif
#endif
    public:
        std::vector<uniform> uniforms;

#ifdef CACHE_UNIFORM_CHANGES
        std::vector<nya_math::vec4> uniforms_cache;
#endif
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

    public:
        static int add() { return get_shader_objs().add(); }
        static shader_obj &get(int idx) { return get_shader_objs().get(idx); }
        static void remove(int idx) { return get_shader_objs().remove(idx); }
        static int invalidate_all() { return get_shader_objs().invalidate_all(); }
        static int release_all() { return get_shader_objs().release_all(); }

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
#endif
    public:
        void release();

    private:
        typedef render_objects<shader_obj> shader_objs;
        static shader_objs &get_shader_objs()
        {
            static shader_objs objs;
            return objs;
        }
    };

    shader::uniform_type convert_uniform_type(shader_code_parser::variable_type type)
    {
        switch(type)
        {
            case shader_code_parser::type_float: return shader::uniform_float;
            case shader_code_parser::type_vec2: return shader::uniform_vec2;
            case shader_code_parser::type_vec3: return shader::uniform_vec3;
            case shader_code_parser::type_vec4: return shader::uniform_vec4;
            case shader_code_parser::type_mat4: return shader::uniform_mat4;
            case shader_code_parser::type_sampler2d: return shader::uniform_sampler2d;
            case shader_code_parser::type_sampler_cube: return shader::uniform_sampler_cube;

            default:break;
        }

        return shader::uniform_not_found;
    }

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
    #ifdef OPENGL3
        PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB = NULL;
    #endif
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

    #define glBindAttribLocationARB glBindAttribLocation

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
    if(!(glGenProgramsARB          =(PFNGLGENPROGRAMSARBPROC)          get_extension("glGenProgramsARB"))) return false;
    if(!(glDeleteProgramsARB       =(PFNGLDELETEPROGRAMSARBPROC)       get_extension("glDeleteProgramsARB"))) return false;
    if(!(glBindProgramARB          =(PFNGLBINDPROGRAMARBPROC)          get_extension("glBindProgramARB"))) return false;
    if(!(glDeleteObjectARB         =(PFNGLDELETEOBJECTARBPROC)         get_extension("glDeleteObjectARB"))) return false;
    if(!(glDetachObjectARB         =(PFNGLDETACHOBJECTARBPROC)         get_extension("glDetachObjectARB"))) return false;
    if(!(glCreateShaderObjectARB   =(PFNGLCREATESHADEROBJECTARBPROC)   get_extension("glCreateShaderObjectARB"))) return false;
    if(!(glShaderSourceARB         =(PFNGLSHADERSOURCEARBPROC)         get_extension("glShaderSourceARB"))) return false;
    if(!(glCompileShaderARB        =(PFNGLCOMPILESHADERARBPROC)        get_extension("glCompileShaderARB"))) return false;
    if(!(glCreateProgramObjectARB  =(PFNGLCREATEPROGRAMOBJECTARBPROC)  get_extension("glCreateProgramObjectARB"))) return false;
    if(!(glAttachObjectARB         =(PFNGLATTACHOBJECTARBPROC)         get_extension("glAttachObjectARB"))) return false;
    if(!(glLinkProgramARB          =(PFNGLLINKPROGRAMARBPROC)          get_extension("glLinkProgramARB"))) return false;
    if(!(glUseProgramObjectARB     =(PFNGLUSEPROGRAMOBJECTARBPROC)     get_extension("glUseProgramObjectARB" ))) return false;
    if(!(glValidateProgramARB      =(PFNGLVALIDATEPROGRAMARBPROC)      get_extension("glValidateProgramARB" ))) return false;
    if(!(glUniform4fARB            =(PFNGLUNIFORM4FARBPROC)            get_extension("glUniform4fARB"))) return false;
    if(!(glUniform1iARB            =(PFNGLUNIFORM1IARBPROC)            get_extension("glUniform1iARB"))) return false;
    if(!(glUniform3fvARB           =(PFNGLUNIFORM3FVARBPROC)           get_extension("glUniform3fvARB"))) return false;
    if(!(glUniform4fvARB           =(PFNGLUNIFORM4FVARBPROC)           get_extension("glUniform4fvARB"))) return false;
    if(!(glUniformMatrix4fvARB     =(PFNGLUNIFORMMATRIX4FVARBPROC)     get_extension("glUniformMatrix4fvARB"))) return false;
    if(!(glGetObjectParameterivARB =(PFNGLGETOBJECTPARAMETERIVARBPROC) get_extension("glGetObjectParameterivARB"))) return false;
    if(!(glGetInfoLogARB           =(PFNGLGETINFOLOGARBPROC)           get_extension("glGetInfoLogARB"))) return false;
    if(!(glGetUniformLocationARB   =(PFNGLGETUNIFORMLOCATIONARBPROC)   get_extension("glGetUniformLocationARB"))) return false;
    #ifdef OPENGL3
      if(!(glBindAttribLocationARB =(PFNGLBINDATTRIBLOCATIONARBPROC)   get_extension("glBindAttribLocationARB"))) return false;
    #endif
  #endif
    failed=false;
    initialised=true;
    return true;
}

void gl_set_matrix(shader_obj &shdr,int idx,const float *m)
{
#ifdef CACHE_MATRIX_CHANGES
    if(idx+4>(int)shdr.uniforms_cache.size())
        shdr.uniforms_cache.resize(idx + 4 + 1);

    if(memcmp(&shdr.uniforms_cache[idx],m,4*16)==0)
        return;

    memcpy(&shdr.uniforms_cache[idx],m,4*16);
#endif
    glUniformMatrix4fvARB(idx,1,false,m);
}

#endif
}

int invalidate_shaders() { return shader_obj::invalidate_all(); }
int release_shaders() { return shader_obj::release_all(); current_shader=active_shader= -1; }

void shader_obj::release()
{
#ifdef DIRECTX11
    if(vertex_program) vertex_program->Release();
    if(pixel_program) pixel_program->Release();

    for(layouts_map::iterator it=layouts.begin();it!=layouts.end();++it)
        if(it->second)
            it->second->Release();

    layouts.clear();

    if(constants.dx_buffer) constants.dx_buffer->Release();
    if(vertex_uniforms.dx_buffer) vertex_uniforms.dx_buffer->Release();
    if(pixel_uniforms.dx_buffer) pixel_uniforms.dx_buffer->Release();
#else
    for(int i=0;i<shader::program_types_count;++i)
    {
        if(!objects[i])
            continue;

        glDetachObjectARB(program,objects[i]);
        glDeleteObjectARB(objects[i]);
    }

    if( program )
        glDeleteObjectARB(program);
#endif
    *this=shader_obj();
}

void set_compiled_shaders_provider(compiled_shaders_provider *csp) { render_csp=csp; }

#ifdef DIRECTX11
ID3D11InputLayout *dx_get_layout(int mesh_idx)
{
    if(current_shader<0)
        return 0;

    shader_obj::layouts_map::iterator it=shader_obj::get(current_shader).layouts.find(mesh_idx);
    if(it==shader_obj::get(current_shader).layouts.end())
        return 0;

    return it->second;
}

ID3D11InputLayout *dx_add_layout(int mesh_idx,const D3D11_INPUT_ELEMENT_DESC*desc,size_t desc_size)
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

void dx_remove_layout(int mesh_idx) { shader_obj::remove_layout(mesh_idx); }
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
            active_shader= -1;
            return;
        }

        shader_obj &shdr=shader_obj::get(idx);
        glUseProgramObjectARB(shdr.program);
        if(!shdr.program)
            active_shader= -1;
        else
            active_shader=idx;
    }
#endif

bool shader::add_program(program_type type,const char*code)
{
    if(type>pixel)
    {
        log()<<"Unable to add shader program: invalid shader type\n";
        return false;
    }

    if(!code || !code[0])
    {
        log()<<"Unable to add shader program: invalid code\n";
        return false;
    }

    const static char type_str[][12]={"vertex","pixel","geometry","tesselation"};

    shader_code_parser parser(code);

    //ToDo: release or reuse if already exists

#ifdef DIRECTX11
    if(!get_device())
    {
        log()<<"Unable to add shader program: invalid directx device, use nya_render::set_device()\n";
        return false;
    }

    if(m_shdr<0)
        m_shdr=shader_obj::add();

    shader_obj &shdr=shader_obj::get(m_shdr);

    if(render_csp) render_csp->get(code,shdr.compiled[type]);
    if(!shdr.compiled[type].get_data())
    {
#ifdef WINDOWS_METRO
        log()<<"Can`t compile "<<type_str[type]<<" shader: Windows metro apps are not allowed to compile shaders, please,"
                                                 "set compiled_shaders_provider and add compiled shaders cache\n";
        return false;
#else
        if(!parser.convert_to_hlsl())
        {
            log()<<"Unable to add shader program: cannot convert shader code to hlsl\n";
            log()<<parser.get_error()<<"\n";
            return false;
        }

        const char *profile=type==pixel?"ps_4_0_level_9_3":"vs_4_0_level_9_3";
        ID3D10Blob *compiled=0, *error=0;
        D3DCompile(parser.get_code(),strlen(parser.get_code()),0,0,0,"main",profile,0,0,&compiled,&error);
        if(error)
        {
            log()<<"Can`t compile "<<type_str[type]<<" shader: \n";
            std::string error_text((const char *)error->GetBufferPointer(),error->GetBufferSize());
            log()<<error_text.c_str()<<"\n";
            log()<<"Shader code:\n"<<parser.get_code()<<"\n\n";

            error->Release();
            return false;
        }

        shdr.compiled[type]=compiled_shader(compiled->GetBufferSize());
        memcpy(shdr.compiled[type].get_data(),compiled->GetBufferPointer(),compiled->GetBufferSize());
        compiled->Release();

        if(render_csp) render_csp->set(code,shdr.compiled[type]);
#endif
    }

    if(type==vertex && get_device()->CreateVertexShader(shdr.compiled[type].get_data(),
                                            shdr.compiled[type].get_size(),nullptr,&shdr.vertex_program)<0)
    {
        log()<<"Can`t create "<<type_str[type]<<" shader\n";
        return false;
    }

    if(type==pixel && get_device()->CreatePixelShader(shdr.compiled[type].get_data(),
                                           shdr.compiled[type].get_size(),nullptr,&shdr.pixel_program)<0)
    {
        log()<<"Can`t create "<<type_str[type]<<" shader\n";
        return false;
    }

    if(type==vertex)
    {
        shdr.layouts.clear();

        int size=0;
        for(int i=0;i<parser.get_uniforms_count();++i)
        {
            const shader_code_parser::variable v=parser.get_uniform(i);
            if(v.type!=shader_code_parser::type_mat4)
                continue;

            if(v.name=="_nya_ModelViewProjectionMatrix") shdr.constants.mvp_matrix=size,size+=16;
            else if(v.name=="_nya_ModelViewMatrix") shdr.constants.mv_matrix=size,size+=16;
            else if(v.name=="_nya_ProjectionMatrix") shdr.constants.p_matrix=size,size+=16;
        }

        if(shdr.constants.dx_buffer)
            shdr.constants.dx_buffer->Release(),shdr.constants.dx_buffer=0;

        if(size>0)
        {
            CD3D11_BUFFER_DESC desc(size*sizeof(float),D3D11_BIND_CONSTANT_BUFFER);
            get_device()->CreateBuffer(&desc,nullptr,&shdr.constants.dx_buffer);
        }
        shdr.constants.buffer.resize(size,0.0f);
    }

    shader_obj::uniforms_buffer &buf=(type==vertex)?shdr.vertex_uniforms:shdr.pixel_uniforms;

    if(buf.dx_buffer)
        buf.dx_buffer->Release(),buf.dx_buffer=0;

    int uniforms_buf_size=0;
    for(int i=0;i<parser.get_uniforms_count();++i)
    {
        const shader_code_parser::variable v=parser.get_uniform(i);
        if(v.type==shader_code_parser::type_mat4)
        {
            if(v.name=="_nya_ModelViewProjectionMatrix") continue;
            if(v.name=="_nya_ModelViewMatrix") continue;
            if(v.name=="_nya_ProjectionMatrix") continue;
        }

        if(v.type==shader_code_parser::type_sampler2d || v.type==shader_code_parser::type_sampler_cube)
            continue;

        shader_obj::uniform &u=shdr.add_uniform(v.name);
        if(type==vertex)
            u.vs_offset=uniforms_buf_size;
        else
            u.ps_offset=uniforms_buf_size;

        if(u.vs_offset>=0 && u.ps_offset>=0)
        {
            if(u.array_size!=v.array_size)
            {
                log()<<"Unable to add shader program: uniform variable declared in both vs and ps has different array size\n";
                return false;
            }

            if(u.type!=convert_uniform_type(v.type))
            {
                log()<<"Unable to add shader program: uniform variable declared in both vs and ps has different type\n";
                return false;
            }
        }

        u.array_size=v.array_size;
        u.type=convert_uniform_type(v.type);
        uniforms_buf_size+=(v.type==shader_code_parser::type_mat4?16:4)*v.array_size;
    }
    buf.buffer.resize(uniforms_buf_size,0.0f);
    if(uniforms_buf_size>0)
    {
        CD3D11_BUFFER_DESC desc(uniforms_buf_size*sizeof(float),D3D11_BIND_CONSTANT_BUFFER);
        get_device()->CreateBuffer(&desc,nullptr,&buf.dx_buffer);
        buf.changed=true;
    }

#else
    if(!check_init_shaders())
        return false;

    if(m_shdr<0)
        m_shdr=shader_obj::add();

    shader_obj &shdr=shader_obj::get(m_shdr);

#ifdef CACHE_UNIFORM_CHANGES
    shdr.uniforms_cache.clear();
#endif

    if(!shdr.program)
        shdr.program=glCreateProgramObjectARB();

    if(!shdr.program)
    {
        log()<<"Unable to create shader program object\n";
        return false;
    }

    if(shdr.objects[type])
    {
        glDetachObjectARB(shdr.program,shdr.objects[type]);
        glDeleteObjectARB(shdr.objects[type]);
        shdr.objects[type]=0;
    }

#ifdef SUPPORT_OLD_SHADERS
  #ifdef OPENGL_ES
   #ifndef __APPLE__
    parser.fix_per_component_functions(); //some droids despise glsl specs
   #endif

    if(!parser.convert_to_glsl_es2())
    {
        log()<<"Unable to add shader program: cannot convert shader code to glsl for es2\n";
        log()<<parser.get_error()<<"\n";
        return false;
    }

  #else
    if(!parser.convert_to_glsl3())
    {
        log()<<"Unable to add shader program: cannot convert shader code to glsl3\n";
        log()<<parser.get_error()<<"\n";
        return false;
    }

  #endif
#else
    parser.convert_to_glsl();
#endif
    code=parser.get_code();

    GLenum gl_type=type==pixel?GL_FRAGMENT_SHADER_ARB:GL_VERTEX_SHADER_ARB; //ToDo: switch all cases

    GLhandleARB object = glCreateShaderObjectARB(gl_type);
    glShaderSourceARB(object,1,&code,0);
    glCompileShaderARB(object);
    GLint compiled;
    glGetShaderParam(object,GL_OBJECT_COMPILE_STATUS_ARB,&compiled);
    if(!compiled)
    {
        GLint log_len=0;
        log()<<"Can`t compile "<<type_str[type]<<" shader: \n";
        glGetShaderParam(object,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
        if(log_len>0)
        {
            std::string log_text(log_len,0);
            glGetInfoLogARB(object,log_len,&log_len,&log_text[0]);
            log()<<log_text.c_str()<<"\n";
        }

        shdr.program=0;
        return false;
    }
    glAttachObjectARB(shdr.program,object);
#endif

    for(int i=0;i<parser.get_uniforms_count();++i)
    {
        const shader_code_parser::variable from=parser.get_uniform(i);
        shader_obj::uniform &to=shdr.add_uniform(from.name.c_str());
        to.array_size=from.array_size;
        to.type=convert_uniform_type(from.type);
    }

#ifndef DIRECTX11
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
            log()<<"Can`t link shader\n";

            GLint log_len=0;
            glGetObjectParam(shdr.program,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
            if(log_len>0)
            {
                std::string log_text(log_len,0);
                glGetInfoLogARB(shdr.program,log_len,&log_len,&log_text[0]);
                log()<<log_text.c_str()<<"\n";
            }

            shdr.program=0; //??
            return false;
        }

        if(type==pixel)
        {
            set_shader(m_shdr);

            for(size_t i=0,layer=0;i<shdr.uniforms.size();++i)
            {
                const shader_obj::uniform &u=shdr.uniforms[i];
                if(u.type!=uniform_sampler2d && u.type!=uniform_sampler_cube)
                    continue;

                int handler=glGetUniformLocationARB(shdr.program,u.name.c_str());
                if(handler>=0)
                    glUniform1iARB(handler,(int)layer);
                else
                    log()<<"Unable to set shader sampler \'"<<u.name.c_str()<<"\': probably not found\n";

                ++layer;
            }

            set_shader(-1);
        }

        if(shaders_validation)
        {
            result=0;
            glValidateProgramARB(shdr.program);
            glGetObjectParam(shdr.program,GL_OBJECT_VALIDATE_STATUS_ARB,&result);
            if(!result)
            {
                log()<<"Can`t validate shader\n";

                GLint log_len=0;
                glGetObjectParam(shdr.program,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
                if(log_len>0)
                {
                    std::string log_text(log_len,0);
                    glGetInfoLogARB(shdr.program,log_len,&log_len,&log_text[0]);
                    log()<<log_text.c_str()<<"\n";
                }
                //shdr.program=0; //??
                //return false;
            }
        }

  #ifdef SUPPORT_OLD_SHADERS
        for(int i=0;i<(int)shdr.uniforms.size();++i)
        {
            if(shdr.uniforms[i].type!=uniform_mat4)
                continue;

            const std::string &name=shdr.uniforms[i].name;
            if(name=="_nya_ModelViewProjectionMatrix") shdr.mat_mvp=get_handler(name.c_str());
            else if(name=="_nya_ModelViewMatrix") shdr.mat_mv=get_handler(name.c_str());
            else if(name=="_nya_ProjectionMatrix") shdr.mat_p=get_handler(name.c_str());
        }
  #endif
    }

  #ifdef SUPPORT_OLD_SHADERS
    for(int i=0;i<parser.get_attributes_count();++i)
    {
        const shader_code_parser::variable a=parser.get_attribute(i);
        if(a.name=="_nya_Vertex") glBindAttribLocationARB(shdr.program,vertex_attribute,a.name.c_str());
        else if(a.name=="_nya_Normal") glBindAttribLocationARB(shdr.program,normal_attribute,a.name.c_str());
        else if(a.name=="_nya_Color") glBindAttribLocationARB(shdr.program,color_attribute,a.name.c_str());
        else if(a.name.find("_nya_MultiTexCoord")==0) glBindAttribLocationARB(shdr.program,tc0_attribute+a.idx,a.name.c_str());
    }
  #endif

    shdr.objects[type]=object;
#endif

    return true;
}

void shader::bind() const { current_shader=m_shdr; }
void shader::unbind() { current_shader= -1; }

void shader::apply(bool ignore_cache)
{
#ifdef DIRECTX11
    if(current_shader<0)
    {
        if(ignore_cache)
            active_shader= -1;

        return;
    }

    if(!get_context())
        return;

    shader_obj &shdr=shader_obj::get(current_shader);

    if(shdr.vertex_program)
    {
        if(shdr.constants.dx_buffer)
        {
            if(shdr.constants.mv_matrix>=0)
                memcpy(&shdr.constants.buffer[shdr.constants.mv_matrix],
                       transform::get().get_modelview_matrix().m[0],16*sizeof(float));

            if(shdr.constants.mvp_matrix>=0)
                memcpy(&shdr.constants.buffer[shdr.constants.mvp_matrix],
                       transform::get().get_modelviewprojection_matrix().m[0],16*sizeof(float));

            if(shdr.constants.p_matrix>=0)
                memcpy(&shdr.constants.buffer[shdr.constants.p_matrix],
                       transform::get().get_projection_matrix().m[0],16*sizeof(float));

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

    shader_obj &shdr=shader_obj::get(current_shader);
    if(shdr.mat_mvp>=0)
        gl_set_matrix(shdr,shdr.mat_mvp,transform::get().get_modelviewprojection_matrix().m[0]);
    if(shdr.mat_mv>=0)
        gl_set_matrix(shdr,shdr.mat_mv,transform::get().get_modelview_matrix().m[0]);
    if(shdr.mat_p>=0)
        gl_set_matrix(shdr,shdr.mat_p,transform::get().get_projection_matrix().m[0]);
  #endif
#endif
}

int shader::get_sampler_layer(const char *name) const
{
    if(!name)
    {
        log()<<"Unable to get sampler layer: invalid name\n";
        return -1;
    }

    if(m_shdr<0)
        return -1;

    const shader_obj &shdr=shader_obj::get(m_shdr);
    for(size_t i=0,layer=0;i<shdr.uniforms.size();++i)
    {
        const shader_obj::uniform &u=shdr.uniforms[i];
        if(u.type!=uniform_sampler2d && u.type!=uniform_sampler_cube)
            continue;

        if(u.name==name)
            return (int)layer;

        ++layer;
    }

    return -1;
}

int shader::get_handler(const char *name) const
{
    if(!name || !name[0])
    {
        log()<<"Unable to get shader handler: invalid name\n";
        return -1;
    }

    if(m_shdr<0)
        return -1;

    const shader_obj &shdr=shader_obj::get(m_shdr);
#ifdef DIRECTX11
    for(int i=0;i<(int)shdr.uniforms.size();++i)
    {
        if(shdr.uniforms[i].name==name)
            return i;
    }

    return -1;
#else
    set_shader(m_shdr);

    if(!shdr.program)
    {
        log()<<"Unable to get shader handler \'"<<name<<"\': invalid program\n";
        return -1;
    }

    return glGetUniformLocationARB(shdr.program,name);
#endif
}

void shader::set_uniform(int i,float f0,float f1,float f2,float f3) const
{
    if(m_shdr<0 || i<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);

#ifdef DIRECTX11
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
    if(!shdr.program)
        return;

  #ifdef CACHE_UNIFORM_CHANGES
    if(i>=(int)shdr.uniforms_cache.size())
        shdr.uniforms_cache.resize(i+1);

    nya_math::vec4 &v=shdr.uniforms_cache[i];
    if(v.x==f0 && v.y==f1 && v.z==f2 &&v.w==f3)
        return;

    v.x=f0,v.y=f1,v.z=f2,v.w=f3;
  #endif

    set_shader(m_shdr);
    glUniform4fARB(i,f0,f1,f2,f3);
#endif
}

void shader::set_uniform3_array(int i,const float *f,unsigned int count) const
{
    if(m_shdr<0 || i<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);

#ifdef DIRECTX11
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
#ifdef CACHE_UNIFORM_ARRAY_CHANGES
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
#ifdef CACHE_UNIFORM_ARRAY_CHANGES
            if(memcmp(&shdr.pixel_uniforms.buffer[o],&f[o2],size)==0)
                continue;
#endif
            memcpy(&shdr.pixel_uniforms.buffer[o],&f[o2],size);
            shdr.pixel_uniforms.changed=true;
        }
    }

#else
    if(!shdr.program || !f)
        return;

    set_shader(m_shdr);
    glUniform3fvARB(i,count,f);
#endif
}

void shader::set_uniform4_array(int i,const float *f,unsigned int count) const
{
    if(m_shdr<0 || i<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);

#ifdef DIRECTX11
    if(i>=(int)shdr.uniforms.size())
        return;

    const shader_obj::uniform &u=shdr.uniforms[i];
    if(int(count)>u.array_size)
        count=u.array_size;

    if(u.vs_offset>=0)
    {
        const size_t size=sizeof(float)*4*count;
#ifdef CACHE_UNIFORM_ARRAY_CHANGES
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
#ifdef CACHE_UNIFORM_ARRAY_CHANGES
        if(memcmp(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size)!=0)
#endif
        {
            memcpy(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size);
            shdr.pixel_uniforms.changed=true;
        }
    }

#else
    if(!shdr.program || !f)
        return;

    set_shader(m_shdr);
    glUniform4fvARB(i,count,f);
#endif
}

void shader::set_uniform16_array(int i,const float *f,unsigned int count,bool transpose) const
{
    if(m_shdr<0 || i<0)
        return;

    shader_obj &shdr=shader_obj::get(m_shdr);

#ifdef DIRECTX11
    if(i>=(int)shdr.uniforms.size())
        return;

    const shader_obj::uniform &u=shdr.uniforms[i];
    if(int(count)>u.array_size)
        count=u.array_size;

    if(u.vs_offset>=0)
    {
        const size_t size=sizeof(float)*16*count;
#ifdef CACHE_UNIFORM_ARRAY_CHANGES
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
#ifdef CACHE_UNIFORM_ARRAY_CHANGES
        if(memcmp(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size)!=0)
#endif
        {
            memcpy(&shdr.pixel_uniforms.buffer[u.ps_offset],f,size);
            shdr.pixel_uniforms.changed=true;
        }
    }

#else
    if(!shdr.program || !f)
        return;

    set_shader(m_shdr);
    glUniformMatrix4fvARB(i,count,transpose,f);
#endif
}

int shader::get_uniforms_count() const
{
    if(m_shdr<0)
        return 0;

    return (int)shader_obj::get(m_shdr).uniforms.size();
}

const char *shader::get_uniform_name(int idx) const
{
    if(idx<0 || idx>=get_uniforms_count())
        return 0;

    return shader_obj::get(m_shdr).uniforms[idx].name.c_str();
}

shader::uniform_type shader::get_uniform_type(int idx) const
{
    if(idx<0 || idx>=get_uniforms_count())
        return uniform_not_found;

    return shader_obj::get(m_shdr).uniforms[idx].type;
}

unsigned int shader::get_uniform_array_size(int idx) const
{
    if(idx<0 || idx>=get_uniforms_count())
        return uniform_not_found;

    return shader_obj::get(m_shdr).uniforms[idx].array_size;
}

void shader::release()
{
    if(m_shdr<0)
        return;

    OPENGL_ONLY(if(m_shdr==active_shader) set_shader(-1));
    if(m_shdr==active_shader) active_shader= -1;
    if(m_shdr==current_shader) current_shader= -1;

    shader_obj::remove(m_shdr);
    m_shdr= -1;
}

void shader::set_shaders_validation(bool enable) { shaders_validation=enable; }

}
