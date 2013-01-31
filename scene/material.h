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
    typedef nya_render::blend::mode blend_mode;

public:
    const char *get_name() const { return m_name.c_str(); }
    int get_textures_count() const { return (int)m_textures.size(); }
    const char *get_texture_name(int idx) const;
    const char *get_texture_semantics(int idx) const;
    bool get_zwrite() const { return m_zwrite; }
    bool get_blend(blend_mode &src,blend_mode &dst) const { 
                   src=m_blend_src; dst=m_blend_dst; return m_blend; }
    bool get_blend() const { return m_blend; }

public:
    void release();

public:
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

private:
	static void set_camera_local_pos(const nya_math::vec3 &pos) 
	{ shader::predefined::camera_local_pos=pos; }

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
