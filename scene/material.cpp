//https://code.google.com/p/nya-engine/

#include "material.h"

namespace nya_scene
{

void material::set() const
{
    m_shader.set();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0)
            continue;

        if(!m_textures[i].proxy.is_valid())
        {
            nya_render::texture::select_multitex_slot(m_textures[i].slot);
            nya_render::texture::unbind();
            continue;
        }

        m_textures[i].proxy->set(m_textures[i].slot);
    }

    for(invalid_slots::const_iterator it=m_invalid_slots.begin();
        it!=m_invalid_slots.end();++it)
    {
        nya_render::texture::select_multitex_slot(it->first);
        nya_render::texture::unbind();
    }
}

void material::unset() const
{
    m_shader.unset();

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

    for(size_t i=0;i<m_shader.get_texture_slots_count();++i)
        m_invalid_slots[i]=true;

    for(size_t i=0;i<m_textures.size();++i)
    {
        m_textures[i].slot=m_shader.get_texture_slot(m_textures[i].semantics.c_str());
        invalid_slots::iterator it=m_invalid_slots.find(m_textures[i].slot);
        if(it!=m_invalid_slots.end())
            m_invalid_slots.erase(it);
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
    invalid_slots::iterator it=m_invalid_slots.find(m_textures.back().slot);
    if(it!=m_invalid_slots.end())
        m_invalid_slots.erase(it);
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
