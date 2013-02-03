//https://code.google.com/p/nya-engine/

#include "material.h"

namespace nya_scene
{

void material::set() const
{
    m_shader.set();

    if(m_blend)
        nya_render::blend::enable(m_blend_src,m_blend_dst);
    else
        nya_render::blend::disable();

    if(m_zwrite)
        nya_render::zwrite::enable();
    else
        nya_render::zwrite::disable();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0)
            continue;

        if(!m_textures[i].proxy.is_valid())
        {
            nya_render::texture::select_multitex_slot(m_textures[i].slot);
            nya_render::texture::unbind_all();
            continue;
        }

        m_textures[i].proxy->set(m_textures[i].slot);
    }
}

void material::unset() const
{
    m_shader.unset();

    if(m_blend)
        nya_render::blend::disable();

    if(!m_zwrite)
        nya_render::zwrite::enable();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0 || !m_textures[i].proxy.is_valid())
            continue;

        m_textures[i].proxy->unset();
    }
}

void material::set_shader(const shader &shdr) 
{
    m_shader.unload();

    m_shader=shdr;

    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].slot=m_shader.get_texture_slot(m_textures[i].semantics.c_str());
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

    for(size_t i=0;i<m_textures.size();++i)
    {
        material_texture &t=m_textures[i];
        if(t.semantics!=semantics)
            continue;

        t.proxy=proxy;
        t.slot=m_shader.get_texture_slot(semantics);
        return;
    }

    m_textures.resize(m_textures.size()+1);
    m_textures.back().proxy=proxy;
    m_textures.back().semantics.assign(semantics);
    m_textures.back().slot=m_shader.get_texture_slot(semantics);
}

void material::set_blend(bool enabled,blend_mode src,blend_mode dst)
{
    m_blend=enabled;
    m_blend_src=src;
    m_blend_dst=dst;
}

const char *material::get_texture_name(int idx) const
{
    if(idx<0 || idx>=(int)m_textures.size())
        return 0;
    
    if(!m_textures[idx].proxy.is_valid())
        return 0;

    return m_textures[idx].proxy->get_name();
}

const char *material::get_texture_semantics(int idx) const
{
    if(idx<0 || idx>=(int)m_textures.size())
        return 0;

    return m_textures[idx].semantics.c_str();
}

void material::release()
{
    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].proxy.free();

    m_textures.clear();
    m_shader.unload();
    m_name.clear();
}

}
