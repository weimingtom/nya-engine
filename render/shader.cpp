//https://code.google.com/p/nya-engine/

/*
    ToDo: more log
*/

#include "shader.h"
#include "platform_specific_gl.h"
#include "render.h"

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

namespace
{
    nya_render::compiled_shaders_provider *render_csp=0;

    int current_shader=-1;
    int active_shader=-1;
}

namespace nya_render
{
    void set_compiled_shaders_provider(compiled_shaders_provider *csp) { render_csp=csp; }

    struct shader_obj
    {
#ifdef DIRECTX11
    public:
        shader_obj(): vertex_program(0),pixel_program(0) {}

    public:
        compiled_shader compiled[shader::program_types_count];
        ID3D11VertexShader *vertex_program;
        ID3D11PixelShader *pixel_program;

        typedef std::map<std::string,ID3D11InputLayout*> layouts_map;
        static layouts_map layouts;

        struct constants_buffer
        {
            mutable std::vector<float> buffer;
            bool mvp_matrix;
            bool mv_matrix;
            bool p_matrix;

            ID3D11Buffer *dx_buffer;

            constants_buffer():mvp_matrix(false),mv_matrix(false),p_matrix(false),dx_buffer(0){}
        };

        constants_buffer constants;
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

    private:
        typedef render_objects<shader_obj> shader_objs;
        static shader_objs &get_shader_objs()
        {
            static shader_objs objs;
            return objs;
        }
    };

#ifdef DIRECTX11
    shader_obj::layouts_map shader_obj::layouts;
#endif

#ifndef DIRECTX11
  #ifndef NO_EXTENSIONS_INIT
    PFNGLGENPROGRAMSARBPROC glGenProgramsARB = NULL;
    PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB = NULL;
    PFNGLBINDPROGRAMARBPROC glBindProgramARB = NULL;
    PFNGLISPROGRAMARBPROC glIsProgramARB = NULL;

    PFNGLPROGRAMSTRINGARBPROC glProgramStringARB = NULL;
    PFNGLGETPROGRAMIVARBPROC glGetProgramivARB = NULL;

    PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4fARB = NULL;
    PFNGLVERTEXATTRIB4FVARBPROC glVertexAttrib4fvARB = NULL;
    PFNGLVERTEXATTRIB3FARBPROC glVertexAttrib3fARB = NULL;
    PFNGLVERTEXATTRIB3FVARBPROC glVertexAttrib3fvARB = NULL;

    PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointerARB = NULL;
    PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB = NULL;
    PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArrayARB = NULL;

    PFNGLPROGRAMLOCALPARAMETER4FARBPROC glProgramLocalParameter4fARB = NULL;
    PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB = NULL;

    PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC glGetProgramLocalParameterfvARB = NULL;

    PFNGLPROGRAMENVPARAMETER4FARBPROC glProgramEnvParameter4fARB = NULL;
    PFNGLPROGRAMENVPARAMETER4FVARBPROC glProgramEnvParameter4fvARB = NULL;

    PFNGLGETPROGRAMENVPARAMETERFVARBPROC glGetProgramEnvParameterfvARB = NULL;

    PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = NULL;
    PFNGLGETHANDLEARBPROC glGetHandleARB = NULL;
    PFNGLDETACHOBJECTARBPROC glDetachObjectARB = NULL;
    PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = NULL;
    PFNGLSHADERSOURCEARBPROC glShaderSourceARB = NULL;
    PFNGLCOMPILESHADERARBPROC glCompileShaderARB = NULL;
    PFNGLCREATEPROGRAMOBJECTARBPROC glCreateProgramObjectARB = NULL;
    PFNGLATTACHOBJECTARBPROC glAttachObjectARB = NULL;
    PFNGLLINKPROGRAMARBPROC glLinkProgramARB = NULL;
    PFNGLUSEPROGRAMOBJECTARBPROC glUseProgramObjectARB = NULL;
    PFNGLVALIDATEPROGRAMARBPROC glValidateProgramARB = NULL;
    PFNGLUNIFORM1FARBPROC glUniform1fARB = NULL;
    PFNGLUNIFORM2FARBPROC glUniform2fARB = NULL;
    PFNGLUNIFORM3FARBPROC glUniform3fARB = NULL;
    PFNGLUNIFORM4FARBPROC glUniform4fARB = NULL;
    PFNGLUNIFORM1IARBPROC glUniform1iARB = NULL;
    PFNGLUNIFORM2IARBPROC glUniform2iARB = NULL;
    PFNGLUNIFORM3IARBPROC glUniform3iARB = NULL;
    PFNGLUNIFORM4IARBPROC glUniform4iARB = NULL;
    PFNGLUNIFORM1FVARBPROC glUniform1fvARB = NULL;
    PFNGLUNIFORM2FVARBPROC glUniform2fvARB = NULL;
    PFNGLUNIFORM3FVARBPROC glUniform3fvARB = NULL;
    PFNGLUNIFORM4FVARBPROC glUniform4fvARB = NULL;
    PFNGLUNIFORM1IVARBPROC glUniform1ivARB = NULL;
    PFNGLUNIFORM2IVARBPROC glUniform2ivARB = NULL;
    PFNGLUNIFORM3IVARBPROC glUniform3ivARB = NULL;
    PFNGLUNIFORM4IVARBPROC glUniform4ivARB = NULL;
    PFNGLUNIFORMMATRIX2FVARBPROC glUniformMatrix2fvARB = NULL;
    PFNGLUNIFORMMATRIX3FVARBPROC glUniformMatrix3fvARB = NULL;
    PFNGLUNIFORMMATRIX4FVARBPROC glUniformMatrix4fvARB = NULL;
    PFNGLGETOBJECTPARAMETERFVARBPROC glGetObjectParameterfvARB = NULL;
    PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB = NULL;
    PFNGLGETINFOLOGARBPROC glGetInfoLogARB = NULL;
    PFNGLGETATTACHEDOBJECTSARBPROC glGetAttachedObjectsARB = NULL;
    PFNGLGETUNIFORMLOCATIONARBPROC glGetUniformLocationARB = NULL;
    PFNGLGETACTIVEUNIFORMARBPROC glGetActiveUniformARB = NULL;
    PFNGLGETUNIFORMFVARBPROC glGetUniformfvARB = NULL;
    PFNGLGETUNIFORMIVARBPROC glGetUniformivARB = NULL;
    PFNGLGETSHADERSOURCEARBPROC glGetShaderSourceARB = NULL;

    PFNGLBINDATTRIBLOCATIONARBPROC glBindAttribLocationARB = NULL;
    PFNGLGETACTIVEATTRIBARBPROC glGetActiveAttribARB = NULL;
    PFNGLGETATTRIBLOCATIONARBPROC glGetAttribLocationARB = NULL;
    PFNGLGETVERTEXATTRIBFVARBPROC glGetVertexAttribfvARB = NULL;
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
    glGenProgramsARB                = (PFNGLGENPROGRAMSARBPROC)               get_extension ( "glGenProgramsARB" );
    if(!glGenProgramsARB)
    {
        return false;
    }

    glDeleteProgramsARB             = (PFNGLDELETEPROGRAMSARBPROC)            get_extension ( "glDeleteProgramsARB" );
    glBindProgramARB                = (PFNGLBINDPROGRAMARBPROC)               get_extension ( "glBindProgramARB" );
    glIsProgramARB                  = (PFNGLISPROGRAMARBPROC)                 get_extension ( "glIsProgramARB" );
    glProgramStringARB              = (PFNGLPROGRAMSTRINGARBPROC)             get_extension ( "glProgramStringARB" );
    glGetProgramivARB               = (PFNGLGETPROGRAMIVARBPROC)              get_extension ( "glGetProgramivARB" );
    glVertexAttrib4fARB             = (PFNGLVERTEXATTRIB4FARBPROC)            get_extension ( "glVertexAttrib4fARB" );
    glVertexAttrib4fvARB            = (PFNGLVERTEXATTRIB4FVARBPROC)           get_extension ( "glVertexAttrib4fvARB" );
    glVertexAttrib3fARB             = (PFNGLVERTEXATTRIB3FARBPROC)            get_extension ( "glVertexAttrib3fARB" );
    glVertexAttrib3fvARB            = (PFNGLVERTEXATTRIB3FVARBPROC)           get_extension ( "glVertexAttrib3fvARB" );
    glVertexAttribPointerARB        = (PFNGLVERTEXATTRIBPOINTERARBPROC)       get_extension ( "glVertexAttribPointerARB" );
    glEnableVertexAttribArrayARB    = (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)   get_extension ( "glEnableVertexAttribArrayARB" );
    glDisableVertexAttribArrayARB   = (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)  get_extension ( "glDisableVertexAttribArrayARB" );
    glProgramLocalParameter4fARB    = (PFNGLPROGRAMLOCALPARAMETER4FARBPROC)   get_extension ( "glProgramLocalParameter4fARB" );
    glProgramLocalParameter4fvARB   = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC)  get_extension ( "glProgramLocalParameter4fvARB" );
    glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARBPROC)get_extension ( "glGetProgramLocalParameterfvARB" );
    glProgramEnvParameter4fARB      = (PFNGLPROGRAMENVPARAMETER4FARBPROC)     get_extension ( "glProgramEnvParameter4fARB" );
    glProgramEnvParameter4fvARB     = (PFNGLPROGRAMENVPARAMETER4FVARBPROC)    get_extension ( "glProgramEnvParameter4fvARB" );
    glGetProgramEnvParameterfvARB   = (PFNGLGETPROGRAMENVPARAMETERFVARBPROC)  get_extension ( "glGetProgramEnvParameterfvARB" );
    glDeleteObjectARB         = (PFNGLDELETEOBJECTARBPROC)         get_extension ( "glDeleteObjectARB" );
    glGetHandleARB            = (PFNGLGETHANDLEARBPROC)            get_extension ( "glGetHandleARB" );
    glDetachObjectARB         = (PFNGLDETACHOBJECTARBPROC)         get_extension ( "glDetachObjectARB" );
    glCreateShaderObjectARB   = (PFNGLCREATESHADEROBJECTARBPROC)   get_extension ( "glCreateShaderObjectARB" );
    glShaderSourceARB         = (PFNGLSHADERSOURCEARBPROC)         get_extension ( "glShaderSourceARB" );
    glCompileShaderARB        = (PFNGLCOMPILESHADERARBPROC)        get_extension ( "glCompileShaderARB" );
    glCreateProgramObjectARB  = (PFNGLCREATEPROGRAMOBJECTARBPROC)  get_extension ( "glCreateProgramObjectARB" );
    glAttachObjectARB         = (PFNGLATTACHOBJECTARBPROC)         get_extension ( "glAttachObjectARB" );
    glLinkProgramARB          = (PFNGLLINKPROGRAMARBPROC)          get_extension ( "glLinkProgramARB" );
    glUseProgramObjectARB     = (PFNGLUSEPROGRAMOBJECTARBPROC)     get_extension ( "glUseProgramObjectARB" );
    glValidateProgramARB      = (PFNGLVALIDATEPROGRAMARBPROC)      get_extension ( "glValidateProgramARB" );
    glUniform1fARB            = (PFNGLUNIFORM1FARBPROC)            get_extension ( "glUniform1fARB" );
    glUniform2fARB            = (PFNGLUNIFORM2FARBPROC)            get_extension ( "glUniform2fARB" );
    glUniform3fARB            = (PFNGLUNIFORM3FARBPROC)            get_extension ( "glUniform3fARB" );
    glUniform4fARB            = (PFNGLUNIFORM4FARBPROC)            get_extension ( "glUniform4fARB" );
    glUniform1iARB            = (PFNGLUNIFORM1IARBPROC)            get_extension ( "glUniform1iARB" );
    glUniform2iARB            = (PFNGLUNIFORM2IARBPROC)            get_extension ( "glUniform2iARB" );
    glUniform3iARB            = (PFNGLUNIFORM3IARBPROC)            get_extension ( "glUniform3iARB" );
    glUniform4iARB            = (PFNGLUNIFORM4IARBPROC)            get_extension ( "glUniform4iARB" );
    glUniform1fvARB           = (PFNGLUNIFORM1FVARBPROC)           get_extension ( "glUniform1fvARB" );
    glUniform2fvARB           = (PFNGLUNIFORM2FVARBPROC)           get_extension ( "glUniform2fvARB" );
    glUniform3fvARB           = (PFNGLUNIFORM3FVARBPROC)           get_extension ( "glUniform3fvARB" );
    glUniform4fvARB           = (PFNGLUNIFORM4FVARBPROC)           get_extension ( "glUniform4fvARB" );
    glUniform1ivARB           = (PFNGLUNIFORM1IVARBPROC)           get_extension ( "glUniform1ivARB" );
    glUniform2ivARB           = (PFNGLUNIFORM2IVARBPROC)           get_extension ( "glUniform2ivARB" );
    glUniform3ivARB           = (PFNGLUNIFORM3IVARBPROC)           get_extension ( "glUniform3ivARB" );
    glUniform4ivARB           = (PFNGLUNIFORM4IVARBPROC)           get_extension ( "glUniform4ivARB" );
    glUniformMatrix2fvARB     = (PFNGLUNIFORMMATRIX2FVARBPROC)     get_extension ( "glUniformMatrix2fvARB" );
    glUniformMatrix3fvARB     = (PFNGLUNIFORMMATRIX3FVARBPROC)     get_extension ( "glUniformMatrix3fvARB" );
    glUniformMatrix4fvARB     = (PFNGLUNIFORMMATRIX4FVARBPROC)     get_extension ( "glUniformMatrix4fvARB" );
    glGetObjectParameterfvARB = (PFNGLGETOBJECTPARAMETERFVARBPROC) get_extension ( "glGetObjectParameterfvARB" );
    glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC) get_extension ( "glGetObjectParameterivARB" );
    glGetInfoLogARB           = (PFNGLGETINFOLOGARBPROC)           get_extension ( "glGetInfoLogARB" );
    glGetAttachedObjectsARB   = (PFNGLGETATTACHEDOBJECTSARBPROC)   get_extension ( "glGetAttachedObjectsARB" );
    glGetUniformLocationARB   = (PFNGLGETUNIFORMLOCATIONARBPROC)   get_extension ( "glGetUniformLocationARB" );
    glGetActiveUniformARB     = (PFNGLGETACTIVEUNIFORMARBPROC)     get_extension ( "glGetActiveUniformARB" );
    glGetUniformfvARB         = (PFNGLGETUNIFORMFVARBPROC)         get_extension ( "glGetUniformfvARB" );
    glGetUniformivARB         = (PFNGLGETUNIFORMIVARBPROC)         get_extension ( "glGetUniformivARB" );
    glGetShaderSourceARB      = (PFNGLGETSHADERSOURCEARBPROC)      get_extension ( "glGetShaderSourceARB" );
    glBindAttribLocationARB   = (PFNGLBINDATTRIBLOCATIONARBPROC)   get_extension ( "glBindAttribLocationARB" );
    glGetActiveAttribARB      = (PFNGLGETACTIVEATTRIBARBPROC)      get_extension ( "glGetActiveAttribARB"    );
    glGetAttribLocationARB    = (PFNGLGETATTRIBLOCATIONARBPROC)    get_extension ( "glGetAttribLocationARB"  );
    glGetVertexAttribfvARB    = (PFNGLGETVERTEXATTRIBFVARBPROC)    get_extension ( "glGetVertexAttribfvARB"  );
  #endif
    failed=false;
    initialised=true;
    return true;
}
#endif

#ifdef DIRECTX11
ID3D11InputLayout *get_layout(const std::string &layout)
{
    if(current_shader<0)
        return 0;

    shader_obj::layouts_map::iterator it=shader_obj::get(current_shader).layouts.find(layout);
    if(it==shader_obj::get(current_shader).layouts.end())
        return 0;

    return it->second;
}

ID3D11InputLayout *add_layout(const std::string &layout,
                              const D3D11_INPUT_ELEMENT_DESC*desc,size_t desc_size)
{
    if(current_shader<0)
        return 0;

    shader_obj &shdr=shader_obj::get(current_shader);

    ID3D11InputLayout *out=0;
	get_device()->CreateInputLayout(desc,desc_size,shdr.compiled[shader::vertex].get_data(),
                                    shdr.compiled[shader::vertex].get_size(),&out);
    if(!out)
        return 0;

    shdr.layouts[layout]=out;

    return out;
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
            code_final.append("}");

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
                GLchar *log = new GLchar[log_len];
                glGetInfoLogARB(shdr.program,log_len,&log_len,log);
                get_log()<<log<<"\n";
                delete(log);
            }

            shdr.program=0; //??
            return false;
        }

        if(type==pixel)
        {
            glUseProgramObjectARB(shdr.program);

            for(size_t i=0;i<m_samplers.size();++i)
            {
                const sampler &s=m_samplers[i];
                int handler=glGetUniformLocationARB(shdr.program,s.name.c_str());
                if(handler>=0)
                    glUniform1iARB(handler,s.layer);
                else
                    get_log()<<"Unable to set shader sampler \'"<<s.name.c_str()<<"\': probably not found\n";
            }

            glUseProgramObjectARB(0);
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
                GLchar *log = new GLchar[log_len];
                glGetInfoLogARB(shdr.program,log_len,&log_len,log);
                get_log()<<log<<"\n";
                delete(log);
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

#ifndef DIRECTX11
void set_shader(int idx)
{
    if(idx==active_shader)
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

void shader::apply()
{
#ifdef DIRECTX11
    if(current_shader<0)
        return;

    if(!get_context())
        return;

    const shader_obj &shdr=shader_obj::get(current_shader);

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
            get_context()->VSSetConstantBuffers(0,1,&shdr.constants.dx_buffer);
        }

        if(current_shader!=active_shader)
            get_context()->VSSetShader(shdr.vertex_program,nullptr,0);
    }

    if(shdr.pixel_program && current_shader!=active_shader)
        get_context()->PSSetShader(shdr.pixel_program,nullptr,0);

    active_shader=current_shader;
#else
    set_shader(current_shader);

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

    shdr.layouts.clear();

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
