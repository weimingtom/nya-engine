//https://code.google.com/p/nya-engine/

#include "fbo.h"
#include "platform_specific_gl.h"

namespace
{
    int default_fbo_idx=0;
}

namespace nya_render
{

#ifndef DIRECTX11
  #ifdef NO_EXTENSIONS_INIT
    #define fbo_glGenFramebuffers glGenFramebuffers
    #define fbo_glBindFramebuffer glBindFramebuffer
	#define fbo_glDeleteFramebuffers glDeleteFramebuffers
	#define fbo_glFramebufferTexture2D glFramebufferTexture2D
  #else
    PFNGLGENFRAMEBUFFERSPROC fbo_glGenFramebuffers;
	PFNGLBINDFRAMEBUFFERPROC fbo_glBindFramebuffer;
	PFNGLDELETEFRAMEBUFFERSPROC fbo_glDeleteFramebuffers;
	PFNGLFRAMEBUFFERTEXTURE2DPROC fbo_glFramebufferTexture2D;
  #endif

bool check_init_fbo()
{
    static bool initialised=false;
    static bool failed=true;
    if(initialised)
        return !failed;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&default_fbo_idx);

    //if(!has_extension("GL_EXT_framebuffer_object"))
    //    return false;

  #ifndef NO_EXTENSIONS_INIT
    fbo_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)get_extension("glGenFramebuffers");
    if(!fbo_glGenFramebuffers)
        return false;

	fbo_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)get_extension("glBindFramebuffer");
    if(!fbo_glBindFramebuffer)
        return false;

	fbo_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)get_extension("glDeleteFramebuffers");
    if(!fbo_glDeleteFramebuffers)
        return false;

	fbo_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)get_extension("glFramebufferTexture2D");
    if(!fbo_glFramebufferTexture2D)
        return false;
  #endif

    initialised=true;
    failed=false;

    return true;
}
#endif

void fbo::set_color_target(const texture &tex)
{
#ifdef DIRECTX11
#else
	if(!check_init_fbo())
		return;

    if(tex.m_tex<0)
        return;

    const texture_obj &tex_obj=texture_obj::get(tex.m_tex);

    if(tex_obj.gl_type!=GL_TEXTURE_2D)
        return;

    if(m_color_target_idx==tex_obj.tex_id)
        return;

    if(!m_fbo_idx)
        fbo_glGenFramebuffers(1,&m_fbo_idx);

    fbo_glBindFramebuffer(GL_FRAMEBUFFER,m_fbo_idx);

    if(!m_color_target_idx && m_depth_target_idx)
    {
  #ifndef OPENGL_ES
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
  #endif
    }

    fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex_obj.tex_id,0);
    fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);

    m_color_target_idx=tex_obj.tex_id;

#endif
}

void fbo::set_depth_target(const texture &tex)
{
#ifdef DIRECTX11
#else
	if(!check_init_fbo())
		return;
    //ToDo
    
    if(tex.m_tex<0)
        return;
    
    const texture_obj &tex_obj=texture_obj::get(tex.m_tex);

    if(tex_obj.gl_type!=GL_TEXTURE_2D)
        return;

    if(m_depth_target_idx==tex_obj.tex_id)
        return;

    if(!m_fbo_idx)
        fbo_glGenFramebuffers(1,&m_fbo_idx);

    fbo_glBindFramebuffer(GL_FRAMEBUFFER,m_fbo_idx);

    if(!m_color_target_idx)
    {
  #ifndef OPENGL_ES
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
  #endif
    }

    fbo_glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,tex_obj.tex_id,0);
    fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);

    m_depth_target_idx=tex_obj.tex_id;
#endif
}

void fbo::release()
{
#ifdef DIRECTX11
#else
    if(!m_fbo_idx)
        return;

    fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
    fbo_glDeleteFramebuffers(1,&m_fbo_idx);

    m_fbo_idx=0;
    m_color_target_idx=0;
    m_depth_target_idx=0;
#endif
}

void fbo::bind()
{
#ifdef DIRECTX11
#else
	if(!m_fbo_idx)
		return;

    fbo_glBindFramebuffer(GL_FRAMEBUFFER,m_fbo_idx);
#endif
}

void fbo::unbind()
{
#ifdef DIRECTX11
#else
	if(!m_fbo_idx)
		return;

    fbo_glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
#endif
}

}
