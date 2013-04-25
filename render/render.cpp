//https://code.google.com/p/nya-engine/

#include "render.h"
#include "transform.h"

#include <map>

namespace
{
    nya_log::log *render_log=0;

    nya_render::state current_state;
    nya_render::state applied_state;

#ifdef DIRECTX11
	float dx_clear_color[4]={0.0f};
	float dx_clear_depth=1.0f;
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
    current_state.color[0]=r;
    current_state.color[1]=g;
    current_state.color[2]=b;
    current_state.color[3]=a;
}

void set_clear_color(float r,float g,float b,float a)
{
#ifdef DIRECTX11
	dx_clear_color[0]=r;
	dx_clear_color[1]=g;
	dx_clear_color[2]=b;
	dx_clear_color[3]=a;
#else
	glClearColor(r,g,b,a);
#endif
}

void set_clear_depth(float value)
{
#ifdef DIRECTX11
	dx_clear_depth=value;
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

    ID3D11RenderTargetView* color_target=0;
    ID3D11DepthStencilView* depth_target=0;
	get_context()->OMGetRenderTargets(1,&color_target,&depth_target);

	if(color && color_target)
    {
		get_context()->ClearRenderTargetView(color_target,dx_clear_color);
        color_target->Release();
    }

	if(depth && depth_target)
    {
		get_context()->ClearDepthStencilView(depth_target,D3D11_CLEAR_DEPTH,dx_clear_depth,0);
        depth_target->Release();
    }
#else
	unsigned int mode=0;
	if(color)
		mode|=GL_COLOR_BUFFER_BIT;

	if(depth)
		mode|=GL_DEPTH_BUFFER_BIT;

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

void set_state(const state &s)
{
    current_state=s;
}

const state &get_state()
{
    return current_state;
}

const state &get_aplied_state()
{
    return applied_state;
}

#ifdef DIRECTX11
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

void dx_apply_blend_state()
{
    if(!current_state.blend)
    {
        get_context()->OMSetBlendState(0,0,current_state.color_write?0xffffffff:0);
        return;
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
            nya_render::get_device()->CreateBlendState(&blend_desc,&state);
            m_map[hsh]=state;
            return state;
        }

    private:
        typedef std::map<unsigned int,ID3D11BlendState*> cache_map;
        cache_map m_map;

    } static blend_state_cache;

    ID3D11BlendState *state=blend_state_cache.get(dx_blend_mode(current_state.blend_src),
                                                  dx_blend_mode(current_state.blend_dst));
    const float blend_factor[]={1.0f,1.0f,1.0f,1.0f};
    nya_render::get_context()->OMSetBlendState(state,blend_factor,current_state.color_write?0xffffffff:0);
}

void dx_apply_cull_face_state()
{
    if(current_state.cull_face)
    {
        static ID3D11RasterizerState *cull_enabled=0;
        if(!cull_enabled)
        {
            D3D11_RASTERIZER_DESC desc;
            ZeroMemory(&desc,sizeof(desc));
            if(current_state.cull_order==cull_face::cw)
                desc.CullMode=D3D11_CULL_BACK;
            else
                desc.CullMode=D3D11_CULL_FRONT;

            desc.FillMode=D3D11_FILL_SOLID;
            get_device()->CreateRasterizerState(&desc,&cull_enabled);

            if(cull_enabled)
                get_context()->RSSetState(cull_enabled);
        }
        else
            get_context()->RSSetState(cull_enabled);
    }
    else
    {
        static ID3D11RasterizerState *cull_disabled=0;
        if(!cull_disabled)
        {
            D3D11_RASTERIZER_DESC desc;
            ZeroMemory(&desc,sizeof(desc));
            desc.CullMode=D3D11_CULL_NONE;
            desc.FillMode=D3D11_FILL_SOLID;
            get_device()->CreateRasterizerState(&desc,&cull_disabled);

            if(cull_disabled)
                get_context()->RSSetState(cull_disabled);
        }
        else
            get_context()->RSSetState(cull_disabled);
    }
}

void dx_apply_depth_state()
{
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
            nya_render::get_device()->CreateDepthStencilState(&desc,&state);
            m_map[hsh]=state;
            return state;
        }

    private:
        typedef std::map<unsigned int,ID3D11DepthStencilState*> cache_map;
        cache_map m_map;

    } static depth_state_cache;

    D3D11_COMPARISON_FUNC dx_depth_comparsion;
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

    ID3D11DepthStencilState *state=depth_state_cache.get(current_state.depth_test,
                                                         current_state.zwrite,dx_depth_comparsion);
    nya_render::get_context()->OMSetDepthStencilState(state,1);
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
#ifdef DIRECTX11
    if(!get_context())
        return;

    if(!get_device())
        return;
#endif

    const state &c=current_state;
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
        dx_apply_cull_face_state();

    if(c.blend!=a.blend || c.blend_src!=a.blend_src || c.blend_dst!=a.blend_dst
                                || c.color_write!=a.color_write || ignore_cache)
        dx_apply_blend_state();

    if(c.depth_test!=a.depth_test || c.depth_comparsion!=a.depth_comparsion 
                                     || c.zwrite!= a.zwrite || ignore_cache)
        dx_apply_depth_state();
#else
    if(c.color[0]!=a.color[0] || c.color[1]!=a.color[1] || c.color[2]!=a.color[2]
                                        || c.color[3]!=a.color[3] || ignore_cache)
    {
#ifdef OPENGL_ES
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
    a=c;
}

}
