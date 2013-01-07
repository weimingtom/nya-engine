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
public:
    const char *get_name() { return m_name.c_str(); }
    int get_textures_count() { return m_textures.size(); }
    texture &get_texture();
    const char *get_texture_semantic();

public:
    bool set_texture(const texture &tex,const char *semantic);

public:
    material() {}
    material(const char *name,const shader &shader);

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
