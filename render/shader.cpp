//https://code.google.com/p/nya-engine/

/*
    ToDo: check if this program already set (less state changes)
          check if this program on uniform and handler sets
          more log
*/

#include "shader.h"
#include "platform_specific_gl.h"
#include "render.h"

#include <string>

#ifdef SUPPORT_OLD_SHADERS
    #include "transform.h"
#endif

namespace nya_render
{

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
    static bool initialised = false;
    if(initialised)
        return true;

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
    initialised=true;
    return true;
}

void shader::add_program(program_type type,const char*code)
{
    if(!check_init_shaders())
        return;

    if(!code || !code[0])
    {
        get_log()<<"Unable to add shader program: invalid code\n";
        return;
    }

    if(!m_program)
        m_program=glCreateProgramObjectARB();

    if(!m_program)
    {
        get_log()<<"Unable to create shader program object\n";
        return;
    }

    if(m_objects[type])
    {
        glDetachObjectARB(m_program,m_objects[type]);
        glDeleteObjectARB(m_objects[type]);
        m_objects[type]=0;
    }

#ifdef SUPPORT_OLD_SHADERS
    std::string code_str(code);
    std::string code_final;

    if(type==vertex)
    {
        const char *attribute_names[]={"nyaVertex","nyaNormal","nyaColor","nyaMultiTexCoord"};

        bool used_attribs[max_attributes]={false};
        m_mat_mvp=-1;
        m_mat_mv=-1;
        m_mat_p=-1;

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
                    else if(code_str.size()>gl+16 && code_str.compare(gl+3,16,"ProjectionMatrix")==0)
                    {
                        m_mat_p=1;
                        replace=true;
                    }
                    else if(code_str.size()>gl+15 && code_str.compare(gl+3,15,"ModelViewMatrix")==0)
                    {
                        m_mat_mv=1;
                        replace=true;
                    }
                    else if(code_str.size()>gl+25 && code_str.compare(gl+3,25,"ModelViewProjectionMatrix")==0)
                    {
                        m_mat_mvp=1;
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

        if(m_mat_mvp>0)
            code_final.append("uniform mat4 nyaModelViewProjectionMatrix;");
        if(m_mat_mv>0)
            code_final.append("uniform mat4 nyaModelViewMatrix;");
        if(m_mat_p>0)
            code_final.append("uniform mat4 nyaProjectionMatrix;");

        for(int i=0;i<tc0_attribute;++i)
        {
            if(!used_attribs[i])
                continue;

            code_final.append("attribute vec4 ");
            code_final.append(attribute_names[i]);
            code_final.append(";\n");

            glBindAttribLocation(m_program,i,attribute_names[i]);
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

            glBindAttribLocation(m_program,i,attrib_name.c_str());
        }
    }

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

    const static char type_str[][12]={"vertex","pixel","geometry","tesselation"};

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

        m_program=0;
        return;
    }
    glAttachObjectARB(m_program,object);

#ifdef OPENGL_ES
    m_objects[type]=object;

    if(m_program && m_objects[vertex] && m_objects[pixel])
#else
    if(m_program)
#endif
    {
        GLint result=0;
        glLinkProgramARB(m_program);
        glGetObjectParam(m_program,GL_OBJECT_LINK_STATUS_ARB,&result);
        if(!result)
        {
#ifdef OPENGL_ES
            get_log()<<"Can`t link shader\n";
#else
            get_log()<<"Can`t link "<<type_str[type]<<" shader\n";
#endif
            GLint log_len=0;
            glGetObjectParam(m_program,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
            if (log_len>0)
            {
                GLchar *log = new GLchar[log_len];
                glGetInfoLogARB(m_program,log_len,&log_len,log);
                get_log()<<log<<"\n";
                delete(log);
            }

            m_program=0; //??
            return;
        }

        result=0;
        glValidateProgramARB(m_program);
        glGetObjectParam(m_program,GL_OBJECT_VALIDATE_STATUS_ARB,&result);
        if(!result)
        {
#ifdef OPENGL_ES
            get_log()<<"Can`t validate shader\n";
#else
            get_log()<<"Can`t validate "<<type_str[type]<<" shader\n";
#endif
            GLint log_len=0;
            glGetObjectParam(m_program,GL_OBJECT_INFO_LOG_LENGTH_ARB,&log_len);
            if (log_len>0)
            {
                GLchar *log = new GLchar[log_len];
                glGetInfoLogARB(m_program,log_len,&log_len,log);
                get_log()<<log<<"\n";
                delete(log);
            }
            m_program=0; //??
            return;
        }

#ifdef SUPPORT_OLD_SHADERS

        if(m_mat_mvp>=0)
            m_mat_mvp=get_handler("nyaModelViewProjectionMatrix");
        if(m_mat_mv>=0)
            m_mat_mv=get_handler("nyaModelViewMatrix");
        if(m_mat_p>=0)
            m_mat_p=get_handler("nyaProjectionMatrix");
#endif
    }

    m_objects[type]=object;

    //glUseProgramObjectARB(0);
}

void shader::bind()
{
    if(!m_program)
        return;

    glUseProgramObjectARB(m_program);

#ifdef SUPPORT_OLD_SHADERS
    if(m_mat_mvp>=0)
        glUniformMatrix4fvARB(m_mat_mvp,1,false,transform::get().get_modelviewprojection_matrix().m[0]);
    if(m_mat_mv>=0)
        glUniformMatrix4fvARB(m_mat_mv,1,false,transform::get().get_modelview_matrix().m[0]);
    if(m_mat_p>=0)
        glUniformMatrix4fvARB(m_mat_p,1,false,transform::get().get_projection_matrix().m[0]);
#endif
}

void shader::unbind()
{
    if(!check_init_shaders())
        return;

    glUseProgramObjectARB(0);
}

void shader::set_sampler(const char*name,unsigned int layer)
{
    if(!name || !name[0])
    {
        get_log()<<"Unable to set shader sampler: invalid name\n";
        return;
    }

    if(!m_program)
    {
        get_log()<<"Unable to set shader sampler \'"<<name<<"\': invalid program\n";
        return;
    }

    int handler=glGetUniformLocationARB(m_program,name);
    if(handler<0)
    {
        get_log()<<"Unable to set shader sampler \'"<<name<<"\': probably not found\n";
        return;
    }

    glUniform1iARB(handler,layer);
}

int shader::get_handler(const char *name)
{
    if(!check_init_shaders())
        return 0;

    if(!name || !name[0])
    {
        get_log()<<"Unable to set shader handler: invalid name\n";
        return 0;
    }

    return glGetUniformLocationARB(m_program,name);
}

void shader::set_uniform(unsigned int i,float f0,float f1,float f2,float f3)
{
    if(!check_init_shaders())
        return;

    glUniform4fARB(i,f0,f1,f2,f3);
}

void shader::set_uniform4_array(unsigned int i,const float *f,unsigned int count)
{
    if(!check_init_shaders() || !f)
        return;

    glUniform4fvARB(i,count,f);
}

void shader::set_uniform16_array(unsigned int i,const float *f,unsigned int count,bool transpose)
{
    if(!check_init_shaders() || !f)
        return;

    glUniformMatrix4fvARB(i,count,transpose,f);
}

void shader::release()
{
    if(m_program==0)
        return;

    glUseProgramObjectARB(0);

    for(int i=0;i<program_types_count;++i)
    {
        if(!m_objects[i])
            continue;

        glDetachObjectARB(m_program,m_objects[i]);
        glDeleteObjectARB(m_objects[i]);
        m_objects[i]=0;
    }

    glDeleteObjectARB(m_program);
    m_program=0;
}

}

