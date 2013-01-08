//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "material.h"
#include "render/vbo.h"
#include "math/vector.h"

namespace nya_scene
{

struct shared_mesh
{
    nya_render::vbo vbo;

    struct group
    {
        material mat;
        int offset;
        int count;
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

    void set_pos(float x,float y,float z) { m_pos.x=x; m_pos.y=y; m_pos.z=z; }
    void set_rot(float yaw,float pitch,float roll) { m_rot.x=yaw; m_rot.y=pitch; m_rot.z=roll; }

private:
    static bool load_pmd(shared_mesh &res,size_t data_size,const void*data);

public:
    mesh() { register_load_function(load_pmd); }

private:
    nya_math::vec3 m_pos;
    nya_math::vec3 m_rot;
};

}
