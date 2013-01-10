//https://code.google.com/p/nya-engine/

#include "material.h"

namespace nya_scene
{

void material::set()
{
    m_shader.set();

    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].tex.set(m_textures[i].slot);
}

void material::unset()
{
    m_shader.unset();

    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].tex.unset();
}

void material::set_shader(const shader &shdr) 
{
    m_shader.unload();

    m_shader=shdr;

    for(size_t i=0;i<m_textures.size();++i)
        m_textures[i].slot=m_shader.get_texture_slot(m_textures[i].semantic.c_str());
}

void material::set_texture(const texture &tex,const char *semantic)
{
    if(!semantic)
        return;

    for(size_t i=0;i<m_textures.size();++i)
    {
        material_texture &t=m_textures[i];
        if(t.semantic!=semantic)
            continue;

        t.tex.unload();
        t.tex=tex;
        return;
    }

    m_textures.resize(m_textures.size()+1);
    m_textures.back().tex=tex;
    m_textures.back().semantic.assign(semantic);
    m_textures.back().slot=m_shader.get_texture_slot(semantic);
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
