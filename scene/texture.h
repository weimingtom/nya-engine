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
private:
    static bool load_tga(shared_texture &res,size_t data_size,const void*data) { return true; }

public:
    texture() { register_load_function(load_tga); }
};

}
