//https://code.google.com/p/nya-engine/

#pragma once

#include "math/vector.h"
#include <vector>
#include <string>

namespace nya_memory { class tmp_buffer_ref; }
namespace nya_scene { class mesh; class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

struct tdcg_loader
{
public:
    static bool load_hardsave(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);
};
