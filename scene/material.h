//https://code.google.com/p/nya-engine/

#pragma once

#include "shader.h"
#include "texture.h"
#include "texture_proxy.h"
#include "render/render.h"
#include <string>
#include <vector>

namespace nya_scene
{

class material
{
    friend class mesh;
    friend struct shared_mesh;

public:
    const char *get_name() const { return m_name.c_str(); }
    int get_textures_count() const { return (int)m_textures.size(); }
    const char *get_texture_name(int idx) const;
    const char *get_texture_semantics(int idx) const;

public:
    void release();

public:
    typedef nya_render::blend::mode blend_mode;

    void set_name(const char*name) { m_name.assign(name?name:""); }
    void set_texture(const char *semantics,const texture &tex);
    void set_texture(const char *semantics,const texture_proxy &proxy);
    void set_shader(const shader &shdr);
    void set_zwrite(bool enabled) { m_zwrite=enabled; }
    void set_blend(bool enabled,blend_mode src,blend_mode dst);
    void set_blend(bool enabled) { m_blend=enabled; }

private:
    void set() const;
    void unset() const;

public:
    material(): m_zwrite(true),m_blend(false)
        ,m_blend_src(nya_render::blend::one),m_blend_dst(nya_render::blend::zero) {}

private:
    std::string m_name;

    shader m_shader;

    bool m_zwrite;
    bool m_blend;
    blend_mode m_blend_src;
    blend_mode m_blend_dst;

    struct material_texture
    {
        std::string semantics;
        int slot;
        texture_proxy proxy;
    };

    std::vector<material_texture> m_textures;
    typedef std::map<int,bool> invalid_slots;
    invalid_slots m_invalid_slots;
};

}
