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

void log_gl_errors(const char *place)
{
    for(int i=glGetError();i!=GL_NO_ERROR;i=glGetError())
    {
        get_log()<<"gl error: ";
        switch(i)
        {
            case GL_INVALID_ENUM: get_log()<<"invalid enum"; break;
            case GL_INVALID_VALUE: get_log()<<"invalid value"; break;
            case GL_INVALID_OPERATION: get_log()<<"invalid operation"; break;
#ifndef OPENGL_ES
            case GL_STACK_OVERFLOW: get_log()<<"stack overflow"; break;
            case GL_STACK_UNDERFLOW: get_log()<<"stack underflow"; break;
#endif
            case GL_OUT_OF_MEMORY: get_log()<<"out of memory"; break;

            default: get_log()<<"unknown"; break;
        }

        get_log()<<" ("<<i<<")";
        if(place)
            get_log()<<" at "<<place<<"\n";
        else
            get_log()<<"\n";
    }
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

void cull_face::enable(cull_face::order o)
{
    if(o==cw)
        glFrontFace(GL_CW);
    else
        glFrontFace(GL_CCW);

    glEnable(GL_CULL_FACE);
}

void cull_face::disable()
{
    glDisable(GL_CULL_FACE);
}

void zwrite::enable()
{
    glDepthMask(true);
}

void zwrite::disable()
{
    glDepthMask(false);
}

void color_write::enable()
{
    glColorMask(true,true,true,true);
}

void color_write::disable()
{
    glColorMask(false,false,false,false);
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
