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

private:
    static bool load_tga(shared_texture &res,size_t data_size,const void*data);

private:
    void set(int slot=0);
    void unset();

public:
    texture():m_last_slot(0) { register_load_function(load_tga); }

private:
    int m_last_slot;
};

}
