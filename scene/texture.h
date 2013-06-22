//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "render/texture.h"

namespace nya_scene
{

struct shared_texture
{
    nya_render::texture tex;

    bool release()
    {
        tex.release();
        return true;
    }
};

class texture: public scene_shared<shared_texture>
{
    friend class material;

public:
    static bool load_tga(shared_texture &res,resource_data &data,const char* name);

private:
    void set(int slot=0) const;
    void unset() const;

public:
    unsigned int get_width() const;
    unsigned int get_height() const;

public:
    typedef nya_render::texture::color_format color_format;
    void build(const void *data,unsigned int width,unsigned int height,color_format format);

public:
    texture():m_last_slot(0) { default_load_function(load_tga); }

private:
    mutable int m_last_slot;
};

}
