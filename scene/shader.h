//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "render/shader.h"

namespace nya_scene
{

struct shared_shader
{
    nya_render::shader shdr;

    bool release()
    {
        shdr.release();
        return true;
    }
};

class shader: public scene_shared<shared_shader>
{
    friend class material;

private:
    static bool load_nya_shader(shared_shader &res,size_t data_size,const void*data);

private:
    void set();
    void unset();

public:
    shader() { register_load_function(load_nya_shader); }
};

}
