//https://code.google.com/p/nya-engine/

#include "material.h"

//ToDo: consistent material params/textures behavior

namespace nya_scene
{

void material_internal::set() const
{
    m_shader.internal().set();

    for(int i=0;i<(int)m_params.size();++i)
    {
        const param_proxy &p=m_params[i].p;
        if(!p.is_valid())
        {
            const param_array_proxy &a=m_params[i].a;
            if(a.is_valid() && a->get_count()>0)
            {
                m_shader.internal().set_uniform4_array(i,a->m_params[0].f,a->get_count());
                continue;
            }

            m_shader.internal().set_uniform_value(i,0,0,0,0);
            continue;
        }

        const param_proxy &m=m_params[i].m;
        if(m.is_valid())
            m_shader.internal().set_uniform_value(i,p->f[0]*m->f[0],p->f[1]*m->f[1],p->f[2]*m->f[2],p->f[3]*m->f[3]);
        else
            m_shader.internal().set_uniform_value(i,p->f[0],p->f[1],p->f[2],p->f[3]);
    }

    if(m_blend)
        nya_render::blend::enable(m_blend_src,m_blend_dst);
    else
        nya_render::blend::disable();

    if(m_color_write)
        nya_render::color_write::enable();
    else
        nya_render::color_write::disable();

    if(m_zwrite)
        nya_render::zwrite::enable();
    else
        nya_render::zwrite::disable();

    if(m_cull_face)
        nya_render::cull_face::enable(m_cull_order);
    else
        nya_render::cull_face::disable();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0)
            continue;

        if(!m_textures[i].proxy.is_valid())
        {
            nya_render::texture::unbind(m_textures[i].slot);
            continue;
        }

        m_textures[i].proxy->internal().set(m_textures[i].slot);
    }
}

void material_internal::unset() const
{
    m_shader.internal().unset();

    if(m_blend)
        nya_render::blend::disable();

    if(!m_zwrite)
        nya_render::zwrite::enable();

    if(!m_color_write)
        nya_render::color_write::enable();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0 || !m_textures[i].proxy.is_valid())
            continue;

        m_textures[i].proxy->internal().unset();
    }
}

void material::set_shader(const shader &shdr)
{
    std::vector<std::string> param_semantics;
    param_semantics.resize(internal().m_shader.internal().get_uniforms_count());
    for(int i=0;i<int(param_semantics.size());++i)
        param_semantics[i]=internal().m_shader.internal().get_uniform(i).name;

    std::vector<material_internal::param_holder> params=internal().m_params; //saves previously setted params

    m_internal.m_shader=shdr;

    for(size_t i=0;i<internal().m_textures.size();++i)
        m_internal.m_textures[i].slot=internal().m_shader.internal().get_texture_slot(internal().m_textures[i].semantics.c_str());

    m_internal.m_params.clear();
    m_internal.m_params.resize(internal().m_shader.internal().get_uniforms_count());

    for(int i=0;i<shdr.internal().get_uniforms_count();++i)
    {
        const nya_math::vec4 &v=shdr.internal().get_uniform(i).default_value;
        set_param(i,v.x,v.y,v.z,v.w);
    }

    for(int i=0;i<int(param_semantics.size());++i)
    {
        const int idx=get_param_idx(param_semantics[i].c_str());
        if(idx<0)
            continue;

        m_internal.m_params[idx]=params[i];
    }
}

void material::set_texture(const char *semantics,const texture &tex)
{
    if(!semantics)
        return;

    set_texture(semantics,texture_proxy(tex));
}

void material::set_texture(const char *semantics,const texture_proxy &proxy)
{
    if(!semantics)
        return;

    for(size_t i=0;i<internal().m_textures.size();++i)
    {
        material_internal::material_texture &t=m_internal.m_textures[i];
        if(t.semantics!=semantics)
            continue;

        t.proxy=proxy;
        t.slot=internal().m_shader.internal().get_texture_slot(semantics);
        return;
    }

    m_internal.m_textures.resize(internal().m_textures.size()+1);
    m_internal.m_textures.back().proxy=proxy;
    m_internal.m_textures.back().semantics.assign(semantics);
    m_internal.m_textures.back().slot=internal().m_shader.internal().get_texture_slot(semantics);
}

void material::set_blend(bool enabled,blend_mode src,blend_mode dst)
{
    m_internal.m_blend=enabled;
    m_internal.m_blend_src=src;
    m_internal.m_blend_dst=dst;
}

const texture_proxy &material::get_texture(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_textures.size() )
    {
        static texture_proxy invalid;
        return invalid;
    }

    return internal().m_textures[idx].proxy;
}

const char *material::get_texture_semantics(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_textures.size())
        return 0;

    return internal().m_textures[idx].semantics.c_str();
}

const texture_proxy &material::get_texture(const char *semantics) const
{
    static texture_proxy invalid;
    if(!semantics)
        return invalid;

    for(int i=0;i<int(internal().m_textures.size());++i)
    {
        if(internal().m_textures[i].semantics==semantics)
            return  internal().m_textures[i].proxy;
    }

    return invalid;
}

const char *material::get_param_name(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return 0;

    return internal().m_shader.internal().get_uniform(idx).name.c_str();
}

int material::get_param_idx(const char *name) const
{
    if(!name)
        return -1;

    for(int i=0;i<internal().m_shader.internal().get_uniforms_count();++i)
    {
        if(internal().m_shader.internal().get_uniform(i).name.compare(name)==0)
            return i;
    }

    return -1;
}

void material::set_param(int idx,float f0,float f1,float f2,float f3)
{
    set_param(idx,param(f0,f1,f2,f3));
}

void material::set_param(int idx,const param &p)
{
    set_param(idx,param_proxy(p));
}

void material::set_param(int idx,const param_proxy &p)
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return;

    m_internal.m_params[idx].p=p;
    m_internal.m_params[idx].m.free();
    m_internal.m_params[idx].a.free();
}

void material::set_param(int idx,const param_proxy &p,const param &m)
{
    set_param(idx,p,param_proxy(m));
}

void material::set_param(int idx,const param_proxy &p,const param_proxy &m)
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return;

    m_internal.m_params[idx].p=p;
    m_internal.m_params[idx].m=m;
    m_internal.m_params[idx].a.free();
}

void material::set_param_array(int idx,const param_array & a)
{
    set_param_array(idx,param_array_proxy(a));
}

void material::set_param_array(int idx,const param_array_proxy & p)
{
    if(idx<0 || idx>=(int)internal().m_params.size())
        return;

    m_internal.m_params[idx].p.free();
    m_internal.m_params[idx].m.free();
    m_internal.m_params[idx].a=p;
}

const material::param_proxy &material::get_param(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
    {
        static param_proxy invalid;
        return invalid;
    }

    return m_internal.m_params[idx].p;
}

const material::param_proxy &material::get_param_multiplier(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
    {
        static param_proxy invalid;
        return invalid;
    }

    return internal().m_params[idx].m;
}

const material::param_array_proxy &material::get_param_array(int idx) const
{
    if(idx<0 || idx>=(int)internal().m_params.size())
    {
        static param_array_proxy invalid;
        return invalid;
    }

    return internal().m_params[idx].a;
}

void material::release()
{
    for(size_t i=0;i<internal().m_textures.size();++i)
        m_internal.m_textures[i].proxy.free();

    m_internal.m_textures.clear();
    m_internal.m_params.clear();
    m_internal.m_shader.unload();
    m_internal.m_name.clear();
}

}
