//https://code.google.com/p/nya-engine/

#include "render.h"
#include "texture.h"
#include "shader.h"
#include "transform.h"
#include "platform_specific_gl.h"

#include <map>

namespace nya_render
{

namespace
{
    nya_log::log_base *render_log=0;

    nya_render::state current_state;
    nya_render::state applied_state;

    nya_render::state_override override_state;
    bool has_state_override=false;

    nya_render::rect viewport_rect;

	float clear_color[4]={0.0f};
	float clear_depth=1.0f;
}

void set_log(nya_log::log_base *l)
{
    render_log = l;
}

nya_log::log_base &log()
{
    static const char *render_log_tag="render";
    if(!render_log)
    {
        return nya_log::log(render_log_tag);
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
        log()<<"gl error: ";
        switch(i)
        {
            case GL_INVALID_ENUM: log()<<"invalid enum"; break;
            case GL_INVALID_VALUE: log()<<"invalid value"; break;
            case GL_INVALID_OPERATION: log()<<"invalid operation"; break;
#ifndef OPENGL_ES
            case GL_STACK_OVERFLOW: log()<<"stack overflow"; break;
            case GL_STACK_UNDERFLOW: log()<<"stack underflow"; break;
#endif
            case GL_OUT_OF_MEMORY: log()<<"out of memory"; break;

            default: log()<<"unknown"; break;
        }

        log()<<" ("<<i<<")";
        if(place)
            log()<<" at "<<place<<"\n";
        else
            log()<<"\n";
    }
#endif
}

void set_viewport(int x,int y,int w,int h,bool ignore_cache)
{
    if(!(viewport_rect.width!=w || viewport_rect.height!=h || viewport_rect.x!=x || viewport_rect.y!=y || ignore_cache))
        return;

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

    viewport_rect.x=x;
    viewport_rect.y=y;
    viewport_rect.width=w;
    viewport_rect.height=h;
}

const rect &get_viewport()
{
    if(viewport_rect.width==viewport_rect.height)
    {
#ifdef DIRECTX11
        //ToDo
#else
        int vport[4];
        glGetIntegerv(GL_VIEWPORT,vport);
        viewport_rect.x=vport[0];
        viewport_rect.y=vport[1];
        viewport_rect.width=vport[2];
        viewport_rect.height=vport[3];
#endif
    }

    return viewport_rect;
}

void set_projection_matrix(const nya_math::mat4 &mat) { transform::get().set_projection_matrix(mat); }
void set_modelview_matrix(const nya_math::mat4 &mat) { transform::get().set_modelview_matrix(mat); }
void set_orientation_matrix(const nya_math::mat4 &mat) { transform::get().set_orientation_matrix(mat); }

void set_color(float r,float g,float b,float a)
{
    current_state.color[0]=r;
    current_state.color[1]=g;
    current_state.color[2]=b;
    current_state.color[3]=a;
}

void set_clear_color(float r,float g,float b,float a,bool ignore_cache)
{
#ifndef DIRECTX11
    if(!(clear_color[0]!=r || clear_color[1]!=g || clear_color[2]!=b || clear_color[3]!=a || ignore_cache))
        return;
#endif

	clear_color[0]=r;
	clear_color[1]=g;
	clear_color[2]=b;
	clear_color[3]=a;

#ifndef DIRECTX11
	glClearColor(r,g,b,a);
#endif
}

void set_clear_depth(float value,bool ignore_cache)
{
#ifndef DIRECTX11
    if(clear_depth==value && !ignore_cache)
        return;
#endif

	clear_depth=value;

#ifndef DIRECTX11
  #if defined OPENGL_ES
	glClearDepthf(value);
  #else
	glClearDepth(value);
  #endif
#endif
}

void clear(bool color,bool depth)
{
#ifdef DIRECTX11
	if(!get_context())
		return;

    dx_target target=get_target();

    if(color && target.color)
		get_context()->ClearRenderTargetView(target.color,clear_color);

    if(depth && target.depth)
        get_context()->ClearDepthStencilView(target.depth,D3D11_CLEAR_DEPTH,clear_depth,0);
#else
	unsigned int mode=0;
	if(color)
    {
		mode|=GL_COLOR_BUFFER_BIT;
        if(!applied_state.color_write)
        {
            glColorMask(true,true,true,true);
            applied_state.color_write=true;
        }
    }

	if(depth)
    {
		mode|=GL_DEPTH_BUFFER_BIT;
        if(!applied_state.zwrite)
        {
            glDepthMask(true);
            applied_state.zwrite=true;
        }
    }

	glClear(mode);
#endif
}

void blend::enable(blend::mode src,blend::mode dst)
{
    current_state.blend=true;
    current_state.blend_src=src;
    current_state.blend_dst=dst;
}

void blend::disable()
{
    current_state.blend=false;
}

void cull_face::enable(cull_face::order o)
{
    current_state.cull_face=true;
    current_state.cull_order=o;
}

void cull_face::disable()
{
    current_state.cull_face=false;
}

void depth_test::enable(comparsion mode)
{
    current_state.depth_test=true;
    current_state.depth_comparsion=mode;
}

void depth_test::disable()
{
    current_state.depth_test=false;
}

void zwrite::enable()
{
    current_state.zwrite=true;
}

void zwrite::disable()
{
    current_state.zwrite=false;
}

void color_write::enable()
{
    current_state.color_write=true;
}

void color_write::disable()
{
    current_state.color_write=false;
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

void set_state(const state &s) { current_state=s; }
const state &get_state() { return current_state; }
const state &get_aplied_state() { return applied_state; }

void set_state_override(const state_override &s)
{
    override_state=s;
    has_state_override=s.override_blend || s.override_cull_face;
}

const state_override &get_state_override() { return override_state; }

state get_overriden_state(const state &s)
{
    state r=s;

    if(override_state.override_blend)
    {
        r.blend=override_state.blend;
        r.blend_src=override_state.blend_src;
        r.blend_dst=override_state.blend_dst;
    }

    if(override_state.override_cull_face)
    {
        r.cull_face=override_state.cull_face;
        r.cull_order=override_state.cull_order;
    }

    return r;
}

#ifdef DIRECTX11

class cull_face_state_class
{
public:
    void apply()
    {
        if(!get_device() || !get_context())
            return;

        if(!m_cull_enabled)
        {
            D3D11_RASTERIZER_DESC desc;
            ZeroMemory(&desc,sizeof(desc));
            if(current_state.cull_order==cull_face::cw)
                desc.CullMode=D3D11_CULL_BACK;
            else
                desc.CullMode=D3D11_CULL_FRONT;

            desc.FillMode=D3D11_FILL_SOLID;
            desc.DepthClipEnable=true;
            get_device()->CreateRasterizerState(&desc,&m_cull_enabled);
        }

        if(!m_cull_disabled)
        {
            D3D11_RASTERIZER_DESC desc;
            ZeroMemory(&desc,sizeof(desc));
            desc.CullMode=D3D11_CULL_NONE;
            desc.FillMode=D3D11_FILL_SOLID;
            desc.DepthClipEnable=true;
            get_device()->CreateRasterizerState(&desc,&m_cull_disabled);
        }

        if(current_state.cull_face)
        {
            if(m_cull_enabled)
                get_context()->RSSetState(m_cull_enabled);
        }
        else
        {
            if(m_cull_disabled)
                get_context()->RSSetState(m_cull_disabled);
        }
    }

    cull_face_state_class(): m_cull_enabled(0), m_cull_disabled(0) {}

    void release()
    {
        if(m_cull_enabled)
            m_cull_enabled->Release();

        if(m_cull_disabled)
            m_cull_disabled->Release();

        m_cull_enabled=m_cull_disabled=0;
    }

private:
    ID3D11RasterizerState *m_cull_enabled;
    ID3D11RasterizerState *m_cull_disabled;

} cull_face_state;

namespace
{

D3D11_BLEND dx_blend_mode(blend::mode m)
{
    switch(m)
    {
        case blend::zero: return D3D11_BLEND_ZERO;
        case blend::one: return D3D11_BLEND_ONE;
        case blend::src_color: return D3D11_BLEND_SRC_COLOR;
        case blend::inv_src_color: return D3D11_BLEND_INV_SRC_COLOR;
        case blend::src_alpha: return D3D11_BLEND_SRC_ALPHA;
        case blend::inv_src_alpha: return D3D11_BLEND_INV_SRC_ALPHA;
        case blend::dst_color: return D3D11_BLEND_DEST_COLOR;
        case blend::inv_dst_color: return D3D11_BLEND_INV_DEST_COLOR;
        case blend::dst_alpha: return D3D11_BLEND_DEST_ALPHA;
        case blend::inv_dst_alpha: return D3D11_BLEND_INV_DEST_ALPHA;
    }

    return D3D11_BLEND_ONE;
}

class
{
public:
    ID3D11BlendState *get(D3D11_BLEND src,D3D11_BLEND dst)
    {
        unsigned int hsh=src;
        hsh+=128*dst;

        cache_map::iterator it=m_map.find(hsh);
        if(it!=m_map.end())
            return it->second;

        const D3D11_RENDER_TARGET_BLEND_DESC desc=
        {
            true,
            src,
            dst,
            D3D11_BLEND_OP_ADD,
            D3D11_BLEND_ONE,
            D3D11_BLEND_ZERO,
            D3D11_BLEND_OP_ADD,
            D3D11_COLOR_WRITE_ENABLE_ALL
        };

        D3D11_BLEND_DESC blend_desc;
        blend_desc.AlphaToCoverageEnable=false;
        blend_desc.IndependentBlendEnable=false;
        blend_desc.RenderTarget[0]=desc;

        ID3D11BlendState *state;
        get_device()->CreateBlendState(&blend_desc,&state);
        m_map[hsh]=state;
        return state;
    }

    void apply()
    {
        if(!get_device() || !get_context())
            return;

        if(!current_state.blend)
        {
            get_context()->OMSetBlendState(0,0,current_state.color_write?0xffffffff:0);
            return;
        }

        ID3D11BlendState *state=get(dx_blend_mode(current_state.blend_src),
                                                        dx_blend_mode(current_state.blend_dst));
        const float blend_factor[]={1.0f,1.0f,1.0f,1.0f};
        nya_render::get_context()->OMSetBlendState(state,blend_factor,current_state.color_write?0xffffffff:0);
    }

    void release()
    {
        if(get_context())
            get_context()->OMSetBlendState(0,0,0xffffffff);

        for(auto &s:m_map) if(s.second) s.second->Release();
        m_map.clear();
    }

private:
    typedef std::map<unsigned int,ID3D11BlendState*> cache_map;
    cache_map m_map;

} blend_state;

class
{
public:
    ID3D11DepthStencilState *get(bool test,bool write,D3D11_COMPARISON_FUNC comparsion)
    {
        unsigned int hsh=0;
        if(test) hsh=1;
        if(write) hsh+=2;
        hsh+=4*comparsion;

        cache_map::iterator it=m_map.find(hsh);
        if(it!=m_map.end())
            return it->second;

        D3D11_DEPTH_STENCIL_DESC desc;
        desc.DepthEnable=test;
        desc.DepthWriteMask=write?D3D11_DEPTH_WRITE_MASK_ALL:D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc=comparsion;
        desc.StencilEnable=true;
        desc.StencilReadMask=0xFF;
        desc.StencilWriteMask=0xFF;

        desc.FrontFace.StencilFailOp=D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilDepthFailOp=D3D11_STENCIL_OP_INCR;
        desc.FrontFace.StencilPassOp=D3D11_STENCIL_OP_KEEP;
        desc.FrontFace.StencilFunc=D3D11_COMPARISON_ALWAYS;

        desc.BackFace.StencilFailOp=D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilDepthFailOp=D3D11_STENCIL_OP_DECR;
        desc.BackFace.StencilPassOp=D3D11_STENCIL_OP_KEEP;
        desc.BackFace.StencilFunc=D3D11_COMPARISON_ALWAYS;

        ID3D11DepthStencilState *state;
        get_device()->CreateDepthStencilState(&desc,&state);
        m_map[hsh]=state;
        return state;
    }

    void apply()
    {
        if(!get_device() || !get_context())
            return;

        D3D11_COMPARISON_FUNC dx_depth_comparsion=D3D11_COMPARISON_ALWAYS;
        switch(current_state.depth_comparsion)
        {
            case depth_test::never: dx_depth_comparsion=D3D11_COMPARISON_NEVER; break;
            case depth_test::less: dx_depth_comparsion=D3D11_COMPARISON_LESS; break;
            case depth_test::equal: dx_depth_comparsion=D3D11_COMPARISON_EQUAL; break;
            case depth_test::greater: dx_depth_comparsion=D3D11_COMPARISON_GREATER; break;
            case depth_test::not_less: dx_depth_comparsion=D3D11_COMPARISON_GREATER_EQUAL; break;
            case depth_test::not_equal: dx_depth_comparsion=D3D11_COMPARISON_NOT_EQUAL; break;
            case depth_test::not_greater: dx_depth_comparsion=D3D11_COMPARISON_LESS_EQUAL; break;
            case depth_test::allways: dx_depth_comparsion=D3D11_COMPARISON_ALWAYS; break;
        }

        ID3D11DepthStencilState *state=get(current_state.depth_test,current_state.zwrite,dx_depth_comparsion);
        nya_render::get_context()->OMSetDepthStencilState(state,1);
    }

    void release()
    {
        if(get_context())
           get_context()->OMSetDepthStencilState(0,0);

        for(auto &s:m_map) if(s.second) s.second->Release();
        m_map.clear();
    }

private:
    typedef std::map<unsigned int,ID3D11DepthStencilState*> cache_map;
    cache_map m_map;

} depth_state;

}

#else
unsigned int gl_blend_mode(blend::mode m)
{
    switch(m)
    {
        case blend::zero: return GL_ZERO;
        case blend::one: return GL_ONE;
        case blend::src_color: return GL_SRC_COLOR;
        case blend::inv_src_color: return GL_ONE_MINUS_SRC_COLOR;
        case blend::src_alpha: return GL_SRC_ALPHA;
        case blend::inv_src_alpha: return GL_ONE_MINUS_SRC_ALPHA;
        case blend::dst_color: return GL_DST_COLOR;
        case blend::inv_dst_color: return GL_ONE_MINUS_DST_COLOR;
        case blend::dst_alpha: return GL_DST_ALPHA;
        case blend::inv_dst_alpha: return GL_ONE_MINUS_DST_ALPHA;
    }

    return GL_ONE;
}
#endif

void apply_state(bool ignore_cache)
{
    DIRECTX11_ONLY(if(!get_context() || !get_device()) return);

    const state &c=has_state_override?get_overriden_state(current_state):current_state;
    state &a=applied_state;

    static bool first_time=true;
    if(first_time)
    {
        first_time=false;
        ignore_cache=true;
    }

#ifdef DIRECTX11
    //ToDo: color
    
    if(c.cull_order!=a.cull_order || c.cull_face!=a.cull_face || ignore_cache)
        cull_face_state.apply();

    if(c.blend!=a.blend || c.blend_src!=a.blend_src || c.blend_dst!=a.blend_dst
                                || c.color_write!=a.color_write || ignore_cache)
        blend_state.apply();

    if(c.depth_test!=a.depth_test || c.depth_comparsion!=a.depth_comparsion 
                                     || c.zwrite!= a.zwrite || ignore_cache)
        depth_state.apply();
#else
    if(c.color[0]!=a.color[0] || c.color[1]!=a.color[1] || c.color[2]!=a.color[2]
                                        || c.color[3]!=a.color[3] || ignore_cache)
    {
#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
        glVertexAttrib4f(color_attribute,c.color[0],c.color[1],c.color[2],c.color[3]);
#else
        glColor4f(c.color[0],c.color[1],c.color[2],c.color[3]);
#endif
    }

    if(c.blend!=a.blend || ignore_cache)
    {
        if(c.blend)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
    }

    if(c.blend_src!=a.blend_src || c.blend_dst!=a.blend_dst || ignore_cache)
        glBlendFunc(gl_blend_mode(c.blend_src),gl_blend_mode(c.blend_dst));

    if(c.cull_face!=a.cull_face || ignore_cache)
    {
        if(c.cull_face)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }

    if(c.cull_order!=a.cull_order || ignore_cache)
    {
        if(c.cull_order==cull_face::cw)
            glFrontFace(GL_CW);
        else
            glFrontFace(GL_CCW);
    }

    if(c.depth_test!=a.depth_test || ignore_cache)
    {
        if(c.depth_test)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    if(c.depth_comparsion!=a.depth_comparsion || ignore_cache)
    {
        switch(c.depth_comparsion)
        {
            case depth_test::never: glDepthFunc(GL_NEVER); break;
            case depth_test::less: glDepthFunc(GL_LESS); break;
            case depth_test::equal: glDepthFunc(GL_EQUAL); break;
            case depth_test::greater: glDepthFunc(GL_GREATER); break;
            case depth_test::not_less: glDepthFunc(GL_GEQUAL); break;
            case depth_test::not_equal: glDepthFunc(GL_NOTEQUAL); break;
            case depth_test::not_greater: glDepthFunc(GL_LEQUAL); break;
            case depth_test::allways: glDepthFunc(GL_ALWAYS); break;
        }
    }

    if(c.zwrite!=a.zwrite || ignore_cache)
        glDepthMask(c.zwrite);

    if(c.color_write!=a.color_write || ignore_cache)
        glColorMask(c.color_write,c.color_write,c.color_write,c.color_write);
#endif

    if(ignore_cache)
    {
        set_viewport(viewport_rect,true);
        texture::apply(true);
        shader::apply(true);
        reset_vbo_state();

#ifndef DIRECTX11
        set_clear_color(clear_color[0],clear_color[1],clear_color[2],clear_color[3],true);
        set_clear_depth(clear_depth,true);
#endif
    }

    a=c;
}


#ifdef DIRECTX11
namespace
{
    ID3D11Device *render_device=0;
    ID3D11DeviceContext *render_context=0;

void discard_state()
{
    invalidate_resources();
    applied_state=current_state=nya_render::state();

    cull_face_state=cull_face_state_class();
    depth_state=decltype(depth_state)();
    blend_state=decltype(blend_state)();
}

}

void release_states()
{
    depth_state.release();
    cull_face_state.release();
    blend_state.release();
}

ID3D11Device *get_device() { return render_device; }
void set_device(ID3D11Device *device)
{
    if(render_device==device)
        return;

    discard_state();
    render_device=device;
}

ID3D11DeviceContext *get_context() { return render_context; }
void set_context(ID3D11DeviceContext *context)
{
    if(render_context==context)
        return;

    discard_state();
    render_context=context;
}
#endif


}
