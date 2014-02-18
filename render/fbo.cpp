//https://code.google.com/p/nya-engine/

#include "fbo.h"
#include "render.h"
#include "platform_specific_gl.h"

namespace nya_render
{

namespace
{
#ifdef DIRECTX11
    nya_render::dx_target default_target;
    nya_render::dx_target target;
#else
    int default_fbo_idx=0;
#endif

#ifdef DIRECTX11
void init_default_target()
{
    //ToDo
//#ifdef WINDOWS_PHONE8
    if(default_target.color)
        return;
//#endif
    ID3D11RenderTargetView *color=0;
    ID3D11DepthStencilView *depth=0;
    get_context()->OMGetRenderTargets(1,&color,&depth);
    if(color)
        target.color=default_target.color=color;

    if(depth)
        target.depth=default_target.depth=depth;
}

dx_target get_default_target() { init_default_target(); return default_target; }
dx_target get_target() { init_default_target(); return target; }

void set_target(ID3D11RenderTargetView *color,ID3D11DepthStencilView *depth,bool default)
{
    if(!get_context())
        return;

    init_default_target();

    target.color=color;
    target.depth=depth;
    if(default)
        default_target=target;

    if(color)
        get_context()->OMSetRenderTargets(1,&color,depth);
    else
        get_context()->OMSetRenderTargets(0,0,depth);
}
#endif

struct fbo_obj
{
    int color_tex_idx;
    int depth_tex_idx;

#ifdef DIRECTX11
    int cubemap_side;

    fbo_obj(): color_tex_idx(-1),depth_tex_idx(-1),cubemap_side(-1) {}
#else
    unsigned int fbo_idx;

    unsigned int color_target_idx;
    unsigned int color_gl_target;
    unsigned int depth_target_idx;

    fbo_obj(): color_tex_idx(-1),depth_tex_idx(-1),fbo_idx(0),color_target_idx(0),color_gl_target(GL_TEXTURE_2D),depth_target_idx(0) {}
#endif

public:
    static int add() { return get_fbo_objs().add(); }
    static fbo_obj &get(int idx) { return get_fbo_objs().get(idx); }
    static void remove(int idx) { return get_fbo_objs().remove(idx); }

private:
    typedef render_objects<fbo_obj> fbo_objs;
    static fbo_objs &get_fbo_objs()
    {
        static fbo_objs objs;
        return objs;
    }
};

#ifndef DIRECTX11
  #ifndef NO_EXTENSIONS_INIT
    PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
  #endif

bool check_init_fbo()
{
    static bool initialised=false;
    static bool failed=true;
    if(initialised)
        return !failed;

    //if(!has_extension("GL_EXT_framebuffer_object"))
    //    return false;

  #ifndef NO_EXTENSIONS_INIT
    if(!(glGenFramebuffers=(PFNGLGENFRAMEBUFFERSPROC)get_extension("glGenFramebuffers"))) return false;
	if(!(glBindFramebuffer=(PFNGLBINDFRAMEBUFFERPROC)get_extension("glBindFramebuffer"))) return false;
	if(!(glDeleteFramebuffers=(PFNGLDELETEFRAMEBUFFERSPROC)get_extension("glDeleteFramebuffers"))) return false;
	if(!(glFramebufferTexture2D=(PFNGLFRAMEBUFFERTEXTURE2DPROC)get_extension("glFramebufferTexture2D"))) return false;
  #endif

    glGetIntegerv(GL_FRAMEBUFFER_BINDING,&default_fbo_idx);
    initialised=true,failed=false;
    return true;
}
#endif
}

void fbo::set_color_target(const texture &tex,cubemap_side side)
{
    if(m_fbo_idx<0)
        m_fbo_idx=fbo_obj::add();

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);
    fbo.color_tex_idx=tex.m_tex;

#ifdef DIRECTX11
    fbo.cubemap_side=side;
#else
    if(!fbo.fbo_idx)
    {
        if(!check_init_fbo())
            return;

        glGenFramebuffers(1,&fbo.fbo_idx);
    }

    if(!fbo.fbo_idx)
        return;

    fbo.color_gl_target=GL_TEXTURE_2D;
    switch(side)
    {
        case cube_positive_x: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_POSITIVE_X; break;
        case cube_negative_x: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_NEGATIVE_X; break;
        case cube_positive_y: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_POSITIVE_Y; break;
        case cube_negative_y: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_NEGATIVE_Y; break;
        case cube_positive_z: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_POSITIVE_Z; break;
        case cube_negative_z: fbo.color_gl_target=GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; break;
        default: break;
    }
#endif
}

void fbo::set_color_target(const texture &tex) { set_color_target(tex,cubemap_side(-1)); }

void fbo::set_depth_target(const texture &tex)
{
    if(m_fbo_idx<0)
        m_fbo_idx=fbo_obj::add();

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);
    fbo.depth_tex_idx=tex.m_tex;

#ifndef DIRECTX11
    if(!fbo.fbo_idx)
    {
        if(!check_init_fbo())
            return;

        glGenFramebuffers(1,&fbo.fbo_idx);
    }
#endif
}

void fbo::release()
{
	if(m_fbo_idx<0)
		return;

    const fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

#ifndef DIRECTX11
    if(fbo.fbo_idx)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
        glDeleteFramebuffers(1,&fbo.fbo_idx);
    }
#endif

    fbo_obj::remove(m_fbo_idx);
    m_fbo_idx=-1;
}

void fbo::bind()
{
	if(m_fbo_idx<0)
		return;

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

#ifdef DIRECTX11
    if(!get_device())
        return;

    ID3D11RenderTargetView *color=0;
    ID3D11DepthStencilView *depth=0;

    if(fbo.color_tex_idx>=0)
    {
        texture_obj &tex=texture_obj::get(fbo.color_tex_idx);
        ID3D11RenderTargetView *&tex_color=tex.color_targets[fbo.cubemap_side>=0?fbo.cubemap_side:0];
        if(!tex_color && tex.tex)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvd;
            rtvd.Format=tex.dx_format;
            if(fbo.cubemap_side<0)
            {
	            rtvd.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
	            rtvd.Texture2D.MipSlice=0;
            }
            else
            {
                rtvd.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvd.Texture2DArray.FirstArraySlice=fbo.cubemap_side;
                rtvd.Texture2DArray.ArraySize=1;
                rtvd.Texture2DArray.MipSlice=0;
            }

            get_device()->CreateRenderTargetView(tex.tex,&rtvd,&tex_color);
        }
        color=tex_color;
    }

    if(fbo.depth_tex_idx>=0)
    {
        texture_obj &tex=texture_obj::get(fbo.depth_tex_idx);
        if(!tex.depth_target && tex.tex)
        {
            CD3D11_DEPTH_STENCIL_VIEW_DESC dsvd(D3D11_DSV_DIMENSION_TEXTURE2D);
            dsvd.Format=tex.dx_format;
            dsvd.Texture2D.MipSlice=0;
            get_device()->CreateDepthStencilView(tex.tex,&dsvd,&tex.depth_target);
        }
        depth=tex.depth_target;
    }

    set_target(color,depth);
#else
    if(!fbo.fbo_idx)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER,fbo.fbo_idx);

    if(fbo.color_tex_idx>=0)
    {
        const texture_obj &tex=texture_obj::get(fbo.color_tex_idx);

        const bool color_target_was_invalid=fbo.color_target_idx==0;

        if(fbo.color_target_idx!=tex.tex_id)
        {
            if(fbo.color_gl_target==GL_TEXTURE_2D)
            {
                if(tex.gl_type==GL_TEXTURE_2D)
                    fbo.color_target_idx=tex.tex_id;
                else
                    fbo.color_target_idx=0;
            }
            else
            {
                if(tex.gl_type!=GL_TEXTURE_2D)
                    fbo.color_target_idx=tex.tex_id;
                else
                    fbo.color_target_idx=0;
            }
        }
        else
        {
            //ToDo: check if tex.gl_type changed
        }

        if(color_target_was_invalid && fbo.color_target_idx && fbo.depth_target_idx)
        {
#ifndef OPENGL_ES
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
#endif
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,fbo.color_gl_target,fbo.color_target_idx,0);
    }
    else if(fbo.color_target_idx)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,0,0);
        fbo.depth_tex_idx=0;
    }

    if(fbo.depth_tex_idx>=0)
    {
        const texture_obj &tex=texture_obj::get(fbo.depth_tex_idx);
        if(fbo.depth_target_idx!=tex.tex_id)
        {
            if(tex.gl_type!=GL_TEXTURE_2D)
            {
                if(fbo.depth_target_idx)
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,tex.tex_id,0);
                    fbo.depth_target_idx=0;
                }
            }
            else
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,tex.tex_id,0);
                fbo.depth_target_idx=tex.tex_id;
#ifndef OPENGL_ES
                if(!fbo.color_target_idx && fbo.depth_target_idx)
                {
                    glDrawBuffer(GL_NONE);
                    glReadBuffer(GL_NONE);
                }
#endif
            }
        }
    }
    else if(fbo.depth_target_idx)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,0,0);
        fbo.depth_target_idx=0;
    }

#endif
}

void fbo::unbind()
{
#ifdef DIRECTX11
    dx_target target=get_default_target();
    set_target(target.color,target.depth);
#else
    glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
#endif
}

}
