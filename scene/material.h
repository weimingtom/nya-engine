//https://code.google.com/p/nya-engine/

#pragma once

#include "shader.h"
#include "texture.h"
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
    const texture &get_texture(int idx) const;
    const char *get_texture_semantics(int idx) const;

public:
    void release();

public:
    void set_name(const char*name) { m_name.assign(name?name:""); }
    void set_texture(const char *semantics,const texture &tex);
    void set_shader(const shader &shdr);

private:
    void set() const;
    void unset() const;

public:
    material() {}

private:
    std::string m_name;

    shader m_shader;

    struct material_texture
    {
        std::string semantics;
        int slot;
        texture tex;
    };

    std::vector<material_texture> m_textures;
};

}
