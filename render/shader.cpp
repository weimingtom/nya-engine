//https://code.google.com/p/nya-engine/

/*
    ToDo: check if this program already set (less state changes)
		  check if this program on uniform sets
		  log
*/

#include "shader.h"
#include "platform_specific_gl.h"
#include "render.h"

namespace render
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
	if(!code)
		return;

	if(!m_program)
		m_program = glCreateProgramObjectARB();

	if(!m_program)
		return;

	if(m_objects[type])
	{
		glDetachObjectARB(m_program,m_objects[type]);
		m_objects[type]=0;
	}

	GLenum gl_type=GL_VERTEX_SHADER_ARB;
	if(type==pixel)									//ToDo: switch and all cases
		gl_type=GL_FRAGMENT_SHADER_ARB;

	GLhandleARB object = glCreateShaderObjectARB(gl_type);
	glShaderSourceARB(object,1,&code,0);
	glCompileShaderARB(object);
	GLint compiled;
	glGetObjectParameterivARB(object,GL_OBJECT_COMPILE_STATUS_ARB,&compiled);

	const static char type_str[][12]={"vertex","pixel","geometry","tesselation"};

	if(!compiled)
	{
		GLint length;
		glGetObjectParameterivARB(object,GL_OBJECT_INFO_LOG_LENGTH_ARB,&length);
		unsigned char *buf = new unsigned char[length];
		glGetInfoLogARB(object, length, &length, (GLcharARB*)buf);
		//LogN(LOG_ERROR,"Can`t compile %s shader (AddShader func)\n Shader error: \n%s\n",
			//type_str[type],buf);
		delete []buf;
		m_program=0;
		return;
	}
	glAttachObjectARB(m_program,object);
	glDeleteObjectARB(object);

	if(m_program)
	{
		int result=0;

		glLinkProgramARB(m_program);
		glGetObjectParameterivARB(m_program,GL_OBJECT_LINK_STATUS_ARB,&result);

		if(!result)
		{
//			Log("Can`t link %s %s shader",Name.c_str(),type_str[type]);
			m_program=0; //??
			return;
		}

		glValidateProgramARB(m_program);
		glGetObjectParameterivARB(m_program,GL_OBJECT_VALIDATE_STATUS_ARB, &result);

		if(!result)
		{
//			Log("Can`t validate %s %s shader",Name.c_str(),type_str[type]);
			m_program=0; //??
			return;
		}
	}

	m_objects[type]=object;

	//glUseProgramObjectARB(0);
}

void shader::bind()
{
	if(!m_program)
		return;

    glUseProgramObjectARB(m_program);
}

void shader::unbind()
{
	if(!check_init_shaders())
		return;

    glUseProgramObjectARB(0);
}

void shader::set_sampler(const char*name,unsigned int layer)
{
	if(!m_program||!name)
		return;

	glUniform1iARB(glGetUniformLocationARB(m_program,name),layer);
}

unsigned int shader::get_handler(const char *name)
{
	if(!m_program||!name)
		return 0;

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
	glUniform4fvARB(i,count,f);
}

void shader::release()
{
    if(m_program==0)
        return;

	glUseProgramObjectARB(0);

	for(int i = 0;i<program_types_count;++i)
	{
		if(!m_objects[i])
			continue;

		glDetachObjectARB(m_program,m_objects[i]);
		m_objects[i]=0;
	}

    glDeleteObjectARB(m_program);
	m_program=0;
}

}

