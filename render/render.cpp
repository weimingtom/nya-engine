//https://code.google.com/p/nya-engine/

#include "render.h"
#include "transform.h"
//#include "platform_specific_gl.h"

namespace
{
    nya_log::log *render_log=0;

#ifdef DIRECTX11
	float clear_color[4]={0.0f};
	float clear_depth=0.0f;
#endif
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
#ifdef DIRECTX11
#else
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
#endif
}

void set_viewport(int x,int y,int w,int h)
{
#ifdef DIRECTX11
	if(!get_context())
		return;

    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)w;
    vp.Height = (FLOAT)h;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = (FLOAT)x;
    vp.TopLeftY = (FLOAT)y;
    get_context()->RSSetViewports(1,&vp);
#else
	glViewport(x,y,w,h);
#endif
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
#ifdef DIRECTX11
#elif defined OPENGL_ES
    glVertexAttrib4f(color_attribute,r,g,b,a);
#else
    glColor4f(r,g,b,a);
#endif
}

void set_clear_color(float r,float g,float b,float a)
{
#ifdef DIRECTX11
	clear_color[0]=r;
	clear_color[1]=g;
	clear_color[2]=b;
	clear_color[3]=a;
#else
	glClearColor(r,g,b,a);
#endif
}

void set_clear_depth(float value)
{
#ifdef DIRECTX11
	clear_depth=value;
#elif defined OPENGL_ES
	glClearDepthf(value);
#else
	glClearDepth(value);
#endif
}

void clear(bool color,bool depth)
{
#ifdef DIRECTX11
	if(!get_context())
		return;

	//temporary here, will move to fbo.cpp
	static ID3D11RenderTargetView* color_target=0;
	static ID3D11DepthStencilView* depth_target=0;
	if(!color_target)
		get_context()->OMGetRenderTargets(1,&color_target,&depth_target);

	if(color && color_target)
		get_context()->ClearRenderTargetView(color_target,clear_color);

	if(depth && depth_target)
		get_context()->ClearDepthStencilView(depth_target,D3D11_CLEAR_DEPTH,clear_depth,0);
#else
	unsigned int mode=0;
	if(color)
		mode|=GL_COLOR_BUFFER_BIT;

	if(depth)
		mode|=GL_DEPTH_BUFFER_BIT;

	glClear(mode);
#endif
}

#ifdef DIRECTX11
#else
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
#endif

void blend::enable(blend::mode src,blend::mode dst)
{
#ifdef DIRECTX11
#else
    glEnable(GL_BLEND);
    glBlendFunc(gl_mode(src),gl_mode(dst));
#endif
}

void blend::disable()
{
#ifdef DIRECTX11
#else
    glDisable(GL_BLEND);
#endif
}

void cull_face::enable(cull_face::order o)
{
#ifdef DIRECTX11
#else
    if(o==cw)
        glFrontFace(GL_CW);
    else
        glFrontFace(GL_CCW);

    glEnable(GL_CULL_FACE);
#endif
}

void cull_face::disable()
{
#ifdef DIRECTX11
#else
    glDisable(GL_CULL_FACE);
#endif
}

void depth_test::enable()
{
#ifdef DIRECTX11
#else
	glEnable(GL_DEPTH_TEST);
#endif
}

void depth_test::disable()
{
#ifdef DIRECTX11
#else
	glDisable(GL_DEPTH_TEST);
#endif
}

void zwrite::enable()
{
#ifdef DIRECTX11
#else
    glDepthMask(true);
#endif
}

void zwrite::disable()
{
#ifdef DIRECTX11
#else
    glDepthMask(false);
#endif
}

void color_write::enable()
{
#ifdef DIRECTX11
#else
    glColorMask(true,true,true,true);
#endif
}

void color_write::disable()
{
#ifdef DIRECTX11
#else
    glColorMask(false,false,false,false);
#endif
}

void scissor::enable(int x,int y,int w,int h)
{
#ifdef DIRECTX11
#else
    glEnable(GL_SCISSOR_TEST);
    glScissor(x,y,w,h);
#endif
}

void scissor::disable()
{
#ifdef DIRECTX11
#else
    glDisable(GL_SCISSOR_TEST);
#endif
}

}
