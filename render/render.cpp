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

    state current_state;
    state applied_state;

    state_override override_state;
    bool has_state_override=false;

    rect viewport_rect;
    DIRECTX11_ONLY(rect viewport_applied_rect);
    rect scissor_rect;
    bool scissor_test=false,applied_scissor_test=false;

	float clear_color[4]={0.0f},clear_depth=1.0f;
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
#if !defined OPENGL_ES && !defined OPENGL3
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
#ifndef DIRECTX11
    if(!(viewport_rect.width!=w || viewport_rect.height!=h || viewport_rect.x!=x || viewport_rect.y!=y || ignore_cache))
        return;

	glViewport(x,y,w,h);
#endif

    viewport_rect.x=x;
    viewport_rect.y=y;
    viewport_rect.width=w;
    viewport_rect.height=h;
}

const rect &get_viewport()
{
    if(viewport_rect.width==0 && viewport_rect.height==0)
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

const nya_math::mat4 &get_projection_matrix() { return transform::get().get_projection_matrix(); }
const nya_math::mat4 &get_modelview_matrix() { return transform::get().get_modelview_matrix(); }
const nya_math::mat4 &get_orientation_matrix() { return transform::get().get_orientation_matrix(); }

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

void apply_viewport_scissor(bool ignore_cache);

void clear(bool color,bool depth)
{
    apply_viewport_scissor(false);

#ifdef DIRECTX11
	if(!get_context())
		return;

    dx_target target=dx_get_target();

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

void scissor::enable(int x,int y,int w,int h,bool ignore_cache)
{
    scissor_test=true;

#ifndef DIRECTX11
    if(!ignore_cache &&
       x==scissor_rect.x && y==scissor_rect.y &&
       w==scissor_rect.width && h==scissor_rect.height )
        return;

    glScissor(x,y,w,h);
#endif
    scissor_rect.x=x,scissor_rect.y=y,scissor_rect.width=w,scissor_rect.height=h;
}

void scissor::disable()
{
    scissor_test=false;
}

bool scissor::is_enabled() { return scissor_test; }
const rect &scissor::get() { return scissor_rect; }

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

inline const state &get_current_state()
{
#ifdef DIRECTX11
    static state s;
    s=has_state_override?get_overriden_state(current_state):current_state;

    //flip cull order when flipping y
    if(!dx_is_default_target())
        s.cull_order=(s.cull_order==cull_face::ccw ? cull_face::cw : cull_face::ccw);
#else
    if(!has_state_override)
        return current_state;

    static state s;
    s=get_overriden_state(current_state);
#endif

    return s;
}

#ifdef DIRECTX11

class rasterizer_state_class
{
    struct rdesc;

    ID3D11RasterizerState *get(const rdesc &d)
    {
        const unsigned int hsh=(d.cull?1:0) + (d.scissor?2:0) + d.cull_order*4;
        cache_map::iterator it=m_map.find(hsh);
        if(it!=m_map.end())
            return it->second;

        D3D11_RASTERIZER_DESC desc;
        ZeroMemory(&desc,sizeof(desc));
        if(d.cull)
        {
            if(d.cull_order==cull_face::cw)
                desc.CullMode=D3D11_CULL_BACK;
            else
                desc.CullMode=D3D11_CULL_FRONT;
        }
        else 
            desc.CullMode=D3D11_CULL_NONE;

        desc.FillMode=D3D11_FILL_SOLID;
        desc.DepthClipEnable=true;
        desc.ScissorEnable=d.scissor;
        ID3D11RasterizerState *state;
        get_device()->CreateRasterizerState(&desc,&state);
        m_map[hsh]=state;
        return state;
    }

public:
    void apply()
    {
        if(!get_device() || !get_context())
            return;

        auto &c=get_current_state();

        rdesc d;
        d.cull=c.cull_face;
        d.cull_order=c.cull_order;
        d.scissor=scissor_test;

        get_context()->RSSetState(get(d));
    }

    void release()
    {
        if(get_context())
            get_context()->RSSetState(0);

        for(auto &s:m_map) if(s.second) s.second->Release();
        m_map.clear();
    }

private:
    struct rdesc
    {
        bool cull;
        cull_face::order cull_order;
        bool scissor;
    };

    typedef std::map<unsigned int,ID3D11RasterizerState*> cache_map;
    cache_map m_map;

} rasterizer_state;

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

        auto &c=get_current_state();

        if(!c.blend)
        {
            get_context()->OMSetBlendState(0,0,c.color_write?0xffffffff:0);
            return;
        }

        ID3D11BlendState *state=get(dx_blend_mode(c.blend_src),
                                                        dx_blend_mode(c.blend_dst));
        const float blend_factor[]={1.0f,1.0f,1.0f,1.0f};
        get_context()->OMSetBlendState(state,blend_factor,c.color_write?0xffffffff:0);
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

        auto &c=get_current_state();

        D3D11_COMPARISON_FUNC dx_depth_comparsion=D3D11_COMPARISON_ALWAYS;
        switch(c.depth_comparsion)
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

        ID3D11DepthStencilState *state=get(c.depth_test,c.zwrite,dx_depth_comparsion);
        get_context()->OMSetDepthStencilState(state,1);
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

    const state &c=get_current_state();
    state &a=applied_state;

    static bool first_time=true;
    if(first_time)
    {
        first_time=false;
        ignore_cache=true;
    }

    apply_viewport_scissor(ignore_cache);

#ifdef DIRECTX11
    //ToDo: color

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
    #ifndef NO_EXTENSIONS_INIT
        static PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4f=NULL;
        if(!glVertexAttrib4f)
            glVertexAttrib4f=(PFNGLVERTEXATTRIB4FARBPROC)get_extension("glVertexAttrib4fARB");
    #endif
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

void apply_viewport_scissor(bool ignore_cache)
{
#ifdef DIRECTX11
    if(!get_context() || !get_device())
        return;

    const state &c=get_current_state();
    state &a=applied_state;

    const int h=dx_get_target_height();
    const int vp_y=dx_is_default_target()?(h-viewport_rect.y-viewport_rect.height):viewport_rect.y;
    if(viewport_rect.x!=viewport_applied_rect.x || vp_y!=viewport_applied_rect.y
       || viewport_rect.width!=viewport_applied_rect.width || viewport_rect.height!=viewport_applied_rect.height
       || ignore_cache)
    {
        D3D11_VIEWPORT vp;
        vp.Width=FLOAT(viewport_applied_rect.width=viewport_rect.width);
        viewport_applied_rect.height = int(vp.Height = (FLOAT)viewport_rect.height);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        viewport_applied_rect.x = int(vp.TopLeftX = (FLOAT)viewport_rect.x);
        viewport_applied_rect.y = int(vp.TopLeftY = (FLOAT)vp_y);
        get_context()->RSSetViewports(1,&vp);
    }

    if(scissor_test || ignore_cache) //ToDo: cache
    {
        D3D11_RECT r;
        r.left=scissor_rect.x;
        r.right=scissor_rect.x+scissor_rect.width;
        r.top=dx_is_default_target()?(h-scissor_rect.y-scissor_rect.height):scissor_rect.y;
        r.bottom=h-scissor_rect.y;
        get_context()->RSSetScissorRects(1,&r);
    }

    if(c.cull_order!=a.cull_order || c.cull_face!=a.cull_face
       || scissor_test!=applied_scissor_test || ignore_cache)
    {
        rasterizer_state.apply();
        a.cull_order=c.cull_order;
        a.cull_face=c.cull_face;
        applied_scissor_test=scissor_test;
    }

#else
    if(scissor_test!=applied_scissor_test || ignore_cache)
    {
        if(scissor_test)
            glEnable(GL_SCISSOR_TEST);
        else
            glDisable(GL_SCISSOR_TEST);

        applied_scissor_test=scissor_test;
    }
#endif
}

#ifdef DIRECTX11
namespace
{
    ID3D11Device *render_device=0;
    ID3D11DeviceContext *render_context=0;

void discard_state()
{
    invalidate_resources();
    applied_state=current_state=state();

    rasterizer_state=rasterizer_state_class();
    depth_state=decltype(depth_state)();
    blend_state=decltype(blend_state)();
}

}

void release_states()
{
    depth_state.release();
    rasterizer_state.release();
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

render_api get_render_api()
{
#if defined DIRECTX11
    return render_api_directx11;
#elif defined OPENGL_ES
    return render_api_opengl_es2;
#elif defined OPENGL3
    return render_api_opengl3;
#else
    return render_api_opengl;
#endif
}

unsigned int release_resources()
{
    unsigned int count=0;
    count+=release_textures();
    count+=release_shaders();
    count+=release_vbos();
    count+=release_fbos();
    DIRECTX11_ONLY(release_states());
    return count;
}

unsigned int invalidate_resources()
{
    unsigned int count=0;
    count+=texture_obj::invalidate_all();
    count+=invalidate_shaders();
    count+=invalidate_vbos();
    count+=invalidate_fbos();
    return count;
}

void set_default_target(ID3D11RenderTargetView *color,ID3D11DepthStencilView *depth)
{
    DIRECTX11_ONLY(dx_set_target(color,depth,true));
}

}
