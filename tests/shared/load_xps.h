//https://code.google.com/p/nya-engine/

#pragma once

#include "math/vector.h"

namespace nya_memory { class memory_reader; class tmp_buffer_ref; }
namespace nya_scene { class mesh; class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

struct xps_loader
{
public:
    struct vert
    {
        nya_math::vec3 pos;
        nya_math::vec3 normal;
        nya_math::vec2 tc;
        //float bone_idx[4];
        //float bone_weight[4];
    };

public:
    static bool load_xps(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name) { return false; } //ToDo
    static bool load_mesh(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);
    static bool load_mesh_ascii(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);
};
