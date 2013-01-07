//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "material.h"
#include "render/vbo.h"

namespace nya_scene
{

struct shared_mesh
{
    nya_render::vbo vbo;

    struct group
    {
        material mat;
    
    };

    std::vector<group> groups;

    bool release()
    {
        vbo.release();
        return true;
    }
};

class mesh: public scene_shared<shared_mesh>
{
public:
    void draw();

private:
    static bool load_pmd(shared_mesh &res,size_t data_size,const void*data);

public:
    mesh() { register_load_function(load_pmd); }
};

}
