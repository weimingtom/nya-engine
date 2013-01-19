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

        m_textures[i].tex.set(m_textures[i].slot);
    }
}

void material::unset() const
{
    m_shader.unset();

    for(size_t i=0;i<m_textures.size();++i)
    {
        if(m_textures[i].slot<0)
            continue;

        m_textures[i].tex.unset();
    }
}

void material::set_shader(const shader &shdr) 
{
    m_shader.unload();

    m_shader=shdr;

    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].slot=m_shader.get_texture_slot(m_textures[i].semantics.c_str());
}

void material::set_texture(const texture &tex,const char *semantics)
{
    if(!semantics)
        return;

    for(size_t i=0;i<m_textures.size();++i)
    {
        material_texture &t=m_textures[i];
        if(t.semantics!=semantics)
            continue;

        t.tex.unload();
        t.tex=tex;
        t.slot=m_shader.get_texture_slot(semantics);
        return;
    }

    m_textures.resize(m_textures.size()+1);
    m_textures.back().tex=tex;
    m_textures.back().semantics.assign(semantics);
    m_textures.back().slot=m_shader.get_texture_slot(semantics);
}

const texture &material::get_texture(int idx) const
{
    if(idx<0 || idx>=(int)m_textures.size())
    {
        const static texture empty;
        return empty;
    }

    return m_textures[idx].tex;
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
        m_textures[i].tex.unload();

    m_textures.clear();
    m_shader.unload();
    m_name.clear();
}

}
