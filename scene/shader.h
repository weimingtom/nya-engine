//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "render/shader.h"
#include "math/vector.h"

namespace nya_scene
{

struct shared_shader
{
    nya_render::shader shdr;
    std::string vertex;
    std::string pixel;

    typedef std::map<std::string,int> samplers_map;
    samplers_map samplers;
    int samplers_count;

	int predef_camera_local_pos;

	shared_shader(): predef_camera_local_pos(-1),samplers_count(0){}

    bool release()
    {
        vertex.clear();
        pixel.clear();
        shdr.release();
        samplers.clear();
        samplers_count=0;
        return true;
    }
};

class shader: public scene_shared<shared_shader>
{
    friend class material;

private:
    static bool load_nya_shader(shared_shader &res,resource_data &data,const char* name);

private:
    void set() const;
    void unset() const;

private:
    int get_texture_slot(const char *semantic) const;
    int get_texture_slots_count() const;

private:
	struct predefined
	{
		static nya_math::vec3 camera_local_pos;
	};

public:
    shader() { register_load_function(load_nya_shader); }
};

}
