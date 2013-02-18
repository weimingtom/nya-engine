//https://code.google.com/p/nya-engine/

#include "render.h"
#include "transform.h"
//#include "platform_specific_gl.h"

namespace
{
    nya_log::log *render_log=0;
}

namespace nya_render
{

void set_log(nya_log::log *l)
{
    render_log = l;
}

nya_log::log &get_log()
{
    static const char *render_log_tag="render";
    if(!render_log)
    {
        return nya_log::get_log(render_log_tag);
    }

    render_log->set_tag(render_log_tag);
    return *render_log;
}

void set_projection_matrix(const nya_math::mat4 &mat)
{
    transform::get().set_projection_matrix(mat);
}

void set_modelview_matrix(const nya_math::mat4 &mat)
{
    transform::get().set_modelview_matrix(mat);
}

void set_color(float r,float g,float b,float a)
{
#ifdef OPENGL_ES
    glVertexAttrib4f(color_attribute,r,g,b,a);
#else
    glColor4f(r,g,b,a);
#endif
}

unsigned int blend::gl_mode(mode m)
{
    switch(m)
    {
        case zero: return GL_ZERO;
        case one: return GL_ONE;
        case src_color: return GL_SRC_COLOR;
        case inv_src_color: return GL_ONE_MINUS_SRC_COLOR;
        case src_alpha: return GL_SRC_ALPHA;
        case inv_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
        case dst_color: return GL_DST_COLOR;
        case inv_dst_color: return GL_ONE_MINUS_DST_COLOR;
        case dst_alpha: return GL_DST_ALPHA;
        case inv_dst_alpha: return GL_ONE_MINUS_DST_ALPHA;
    }

    return GL_ONE;
}

void blend::enable(blend::mode src,blend::mode dst)
{
    glEnable(GL_BLEND);
    glBlendFunc(gl_mode(src),gl_mode(dst));
}

void blend::disable()
{
    glDisable(GL_BLEND);
}

void zwrite::enable()
{
    glDepthMask(true);
}

void zwrite::disable()
{
    glDepthMask(false);
}

void scissor::enable(int x,int y,int w,int h)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(x,y,w,h);
}

void scissor::disable()
{
    glDisable(GL_SCISSOR_TEST);
}

}
