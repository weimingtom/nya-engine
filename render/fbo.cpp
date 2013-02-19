//https://code.google.com/p/nya-engine/

#include "fbo.h"
#include "platform_specific_gl.h"

namespace nya_render
{

void fbo::set_color_target(const texture &tex)
{
    if(tex.m_gl_type!=GL_TEXTURE_2D)
        return;

    if(m_color_target_idx==tex.m_tex_id)
        return;

    if(!m_fbo_idx)
        glGenFramebuffers(1,&m_fbo_idx);

    glBindFramebuffer(GL_FRAMEBUFFER,m_fbo_idx);

    if(!m_color_target_idx && m_depth_target_idx)
    {
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex.m_tex_id,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    m_color_target_idx=tex.m_tex_id;
}

void fbo::set_depth_target(const texture &tex)
{
    if(tex.m_gl_type!=GL_TEXTURE_2D)
        return;

    if(m_depth_target_idx==tex.m_tex_id)
        return;

    if(!m_fbo_idx)
        glGenFramebuffers(1,&m_fbo_idx);

    glBindFramebuffer(GL_FRAMEBUFFER,m_fbo_idx);

    if(!m_color_target_idx)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,tex.m_tex_id,0);
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    m_depth_target_idx=tex.m_tex_id;
}

void fbo::release()
{
    if(!m_fbo_idx)
        return;

    glBindFramebuffer(GL_FRAMEBUFFER,0);
    glDeleteFramebuffers(1,&m_fbo_idx);

    m_fbo_idx=0;
    m_color_target_idx=0;
    m_depth_target_idx=0;
}

void fbo::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER,m_fbo_idx);
}

void fbo::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

}
