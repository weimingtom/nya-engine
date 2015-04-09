//https://code.google.com/p/nya-engine/

#include "fbo.h"
#include "render.h"
#include "platform_specific_gl.h"

//ToDo: mrt on dx11

namespace nya_render
{

namespace
{
    DIRECTX11_ONLY(dx_target default_target,target);
    OPENGL_ONLY(int default_fbo_idx=0);
}

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

void set_target(ID3D11RenderTargetView *color,ID3D11DepthStencilView *depth,bool is_default)
{
    if(!get_context())
        return;

    init_default_target();

    target.color=color;
    target.depth=depth;
    if(is_default)
        default_target=target;

    if(color)
        get_context()->OMSetRenderTargets(1,&color,depth);
    else
        get_context()->OMSetRenderTargets(0,0,depth);
}
#endif

namespace
{

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

#ifndef GL_MAX_COLOR_ATTACHMENTS
    #define GL_MAX_COLOR_ATTACHMENTS 0x8CDF
#endif

int gl_target(int side)
{
    switch(side)
    {
        case fbo::cube_positive_x: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        case fbo::cube_negative_x: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case fbo::cube_positive_y: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case fbo::cube_negative_y: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case fbo::cube_positive_z: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case fbo::cube_negative_z: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    }

    return GL_TEXTURE_2D;
}

#endif

struct fbo_obj
{
    struct attachment
    {
        int tex_idx;
        int cubemap_side;
        OPENGL_ONLY(unsigned int last_target_idx);
        OPENGL_ONLY(int last_gl_type);

        attachment(): tex_idx(-1),cubemap_side(-1)
        {
            OPENGL_ONLY(last_target_idx=0);
            OPENGL_ONLY(last_gl_type= -1);
        }
    };

    std::vector<attachment> color_attachments;
    int depth_tex_idx;
    OPENGL_ONLY(unsigned int fbo_idx);
    OPENGL_ONLY(int depth_target_idx);

    fbo_obj(): depth_tex_idx(-1)
    {
        OPENGL_ONLY(fbo_idx=depth_target_idx=0);
    }

public:
    static int add() { return get_fbo_objs().add(); }
    static fbo_obj &get(int idx) { return get_fbo_objs().get(idx); }
    static void remove(int idx) { return get_fbo_objs().remove(idx); }
    static int release_all() { return get_fbo_objs().release_all(); }
    static int invalidate_all() { return get_fbo_objs().invalidate_all(); }

public:
    void release() { OPENGL_ONLY(if(fbo_idx) glDeleteFramebuffers(1,&fbo_idx)); *this=fbo_obj(); }

private:
    typedef render_objects<fbo_obj> fbo_objs;
    static fbo_objs &get_fbo_objs()
    {
        static fbo_objs objs;
        return objs;
    }
};

}

int release_fbos() { return fbo_obj::release_all(); }
int invalidate_fbos() { return fbo_obj::invalidate_all(); }

void fbo::set_color_target(const texture &tex,cubemap_side side,unsigned int attachment_idx)
{
    if(attachment_idx>=get_max_color_attachments())
        return;

    if(m_fbo_idx<0)
        m_fbo_idx=fbo_obj::add();

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

    if(attachment_idx>=fbo.color_attachments.size())
        fbo.color_attachments.resize(attachment_idx+1);

    fbo_obj::attachment &a=fbo.color_attachments[attachment_idx];
    a.tex_idx=tex.m_tex;
    a.cubemap_side=side;

#ifndef DIRECTX11
    if(!fbo.fbo_idx)
    {
        if(!check_init_fbo())
            return;

        glGenFramebuffers(1,&fbo.fbo_idx);
    }
#endif
}

void fbo::set_color_target(const texture &tex,unsigned int attachment_idx)
{
    set_color_target(tex,cubemap_side(-1),attachment_idx);
}

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

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

#ifndef DIRECTX11
    if(fbo.fbo_idx)
    {
        glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
        glDeleteFramebuffers(1,&fbo.fbo_idx);
    }
#endif

    fbo_obj::remove(m_fbo_idx);
    m_fbo_idx= -1;
}

void fbo::bind() const
{
	if(m_fbo_idx<0)
    {
        unbind();
		return;
    }

    fbo_obj &fbo=fbo_obj::get(m_fbo_idx);

#ifdef DIRECTX11
    if(!get_device())
        return;

    ID3D11RenderTargetView *color=0;
    ID3D11DepthStencilView *depth=0;

    if(!fbo.color_attachments.empty() && fbo.color_attachments[0].tex_idx>=0)
    {
        fbo_obj::attachment &a=fbo.color_attachments[0];

        texture_obj &tex=texture_obj::get(a.tex_idx);
        ID3D11RenderTargetView *&tex_color=tex.color_targets[a.cubemap_side>=0?a.cubemap_side:0];
        if(!tex_color && tex.tex)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvd;
            rtvd.Format=tex.dx_format;
            if(a.cubemap_side<0)
            {
	            rtvd.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
	            rtvd.Texture2D.MipSlice=0;
            }
            else
            {
                rtvd.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvd.Texture2DArray.FirstArraySlice=a.cubemap_side;
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

    bool has_color_target=false;

    for(size_t i=0;i<fbo.color_attachments.size();++i)
    {
        fbo_obj::attachment &a=fbo.color_attachments[i];

#ifndef OPENGL_ES
        const bool color_target_was_invalid=a.last_target_idx==0;
#endif
        const texture_obj &tex=texture_obj::get(a.tex_idx);

        const int gl_type=a.cubemap_side<0?tex.gl_type:gl_target(a.cubemap_side);
        if(gl_type!=a.last_gl_type && a.last_gl_type>=0)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+int(i),a.last_gl_type,0,0);
            a.last_gl_type=tex.tex_id?gl_type:-1;
        }

        if(tex.tex_id!=a.last_target_idx)
        {
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0+int(i),gl_type,tex.tex_id,0);
            a.last_target_idx=tex.tex_id;
        }

#ifndef OPENGL_ES
        if(color_target_was_invalid && tex.tex_id && fbo.depth_target_idx)
        {
            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
        }
#endif
        if(tex.tex_id)
            has_color_target=true;
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
                if(!has_color_target && fbo.depth_target_idx)
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
    if(!check_init_fbo())
        return;

    glBindFramebuffer(GL_FRAMEBUFFER,default_fbo_idx);
#endif
}

unsigned int fbo::get_max_color_attachments()
{
#ifdef DIRECTX11
    return 1; //D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; //ToDo
#else
    static int max_attachments= -1;
    if(max_attachments<0)
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);

    return max_attachments;
#endif
}

}
