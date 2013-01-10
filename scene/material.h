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
    const char *get_name() { return m_name.c_str(); }
    int get_textures_count() { return (int)m_textures.size(); }
    texture &get_texture();
    const char *get_texture_semantic();

private:
    void release();

public:
    void set_name(const char*name) { m_name.assign(name?name:""); }
    void set_texture(const texture &tex,const char *semantic);
    void set_shader(const shader &shdr);

private:
    void set();
    void unset();

public:
    material() {}
    //material(const char *name,const shader &shader);

private:
    std::string m_name;

    shader m_shader;

    struct material_texture
    {
        std::string semantic;
        int slot;
        texture tex;
    };

    std::vector<material_texture> m_textures;
};

}
