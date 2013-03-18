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

    enum predefined_values
    {
        camera_pos=0,
        bones_pos,
        bones_rot,

        predefines_count
    };

    enum transform_type
    {
        none,
        local,
        local_rot
    };

    struct predefined
    {
        predefined_values type;
        int location;
        transform_type transform;

        predefined(): location(-1), transform(none) {}
    };

    std::vector<predefined> predefines;

    struct uniform
    {
        std::string name;
        int location;
        transform_type transform;

        uniform(): location(-1), transform(none) {}
    };

    std::vector<uniform> uniforms;

	shared_shader():samplers_count(0){}

    bool release()
    {
        vertex.clear();
        pixel.clear();
        shdr.release();
        predefines.clear();
        uniforms.clear();
        samplers.clear();
        samplers_count=0;
        return true;
    }
};

class shader: public scene_shared<shared_shader>
{
    friend class material;

public:
    static bool load_nya_shader(shared_shader &res,resource_data &data,const char* name);

private:
    void set() const;
    void unset() const;

private:
    int get_texture_slot(const char *semantic) const;
    int get_texture_slots_count() const;

private:
    const shared_shader::uniform &get_uniform(int idx) const;
    void set_uniform_value(int idx,float f0,float f1,float f2,float f3) const;
    int get_uniforms_count() const;

public:
    shader() { register_load_function(load_nya_shader); }
};

}
