//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "material.h"
#include "render/vbo.h"
#include "render/skeleton.h"
#include "math/vector.h"
#include "transform.h"

namespace nya_scene
{

struct shared_mesh
{
    nya_render::vbo vbo;

    struct texture_info
    {
        std::string name;
        std::string semantic;
    };

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

    nya_render::skeleton skeleton;
};

class mesh: public scene_shared<shared_mesh>
{
public:
    virtual void unload();

    void draw(int group_idx=-1);

    void set_pos(float x,float y,float z) { m_transform.set_pos(x,y,z); }
    void set_rot(float yaw,float pitch,float roll) { m_transform.set_rot(yaw,pitch,roll); }
    void set_scale(float sx,float sy,float sz) { m_transform.set_scale(sx,sy,sz); }

public:
    int get_materials_count();
    const material &get_material(int idx);
    void set_material(int idx,const material &mat);

public:
    const nya_render::skeleton &get_skeleton();

public:
    static bool load_pmd(shared_mesh &res,resource_data &data,const char* name);

public:
    mesh() { register_load_function(load_pmd); }

private:
    nya_scene_internal::transform m_transform;

private:
    std::vector<int> m_replaced_materials_idx;
    std::vector<material> m_replaced_materials;
};

}
