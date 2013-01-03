//https://code.google.com/p/nya-engine/

#pragma once

#include "resources/shared_resources.h"
#include "render/vbo.h"

namespace nya_scene
{

class mesh
{
public:
    bool load(const char *name);
    void draw();
    void unload();

public:
    struct shared_mesh
    {
        nya_render::vbo vbo;
    };

private:
    typedef nya_resources::shared_resources<shared_mesh,8> shared_meshes;
    typedef shared_meshes::shared_resource_ref shared_mesh_ref;

    class shared_meshes_manager: public shared_meshes
    {
        bool fill_resource(const char *name,shared_mesh &res);
        bool release_resource(shared_mesh &res);
    };

    shared_meshes_manager &get_shared_meshes()
    {
        static shared_meshes_manager manager;
        return manager;
    }

private:
    shared_mesh_ref m_shared_mesh_ref;
};

}
