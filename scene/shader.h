//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "render/shader.h"
#include "render/skeleton.h"
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

class shader_internal: public scene_shared<shared_shader>
{
public:
    void set() const;
    void unset() const;

    static void set_skeleton(const nya_render::skeleton *skeleton) { m_skeleton=skeleton; }
    void reset_skeleton() { m_last_skeleton_pos=0; m_last_skeleton_rot=0; }
    void skeleton_changed(const nya_render::skeleton *skeleton) const
    {
        if(skeleton==m_last_skeleton_pos)
            m_last_skeleton_pos=0;

        if(skeleton==m_last_skeleton_rot)
            m_last_skeleton_rot=0;
    }

    shader_internal(): m_last_skeleton_pos(0),m_last_skeleton_rot(0) {}

public:
    int get_texture_slot(const char *semantic) const;
    int get_texture_slots_count() const;

public:
    const shared_shader::uniform &get_uniform(int idx) const;
    void set_uniform_value(int idx,float f0,float f1,float f2,float f3) const;
    void set_uniform4_array(int idx,const float *array,int size) const;
    int get_uniforms_count() const;

private:
    static const nya_render::skeleton *m_skeleton;
    const mutable nya_render::skeleton *m_last_skeleton_pos;
    const mutable nya_render::skeleton *m_last_skeleton_rot;
};

class shader
{
public:
    bool load(const char *name) { m_internal.reset_skeleton(); return m_internal.load(name); }
    void unload() { return m_internal.unload(); }

public:
    void create(const shared_shader &res) { m_internal.reset_skeleton(); m_internal.create(res); }

public:
    static void set_resources_prefix(const char *prefix) { shader_internal::set_resources_prefix(prefix); }
    static void register_load_function(shader_internal::load_function function) { shader_internal::register_load_function(function); }

public:
    const char *get_name() const { return m_internal.get_name(); }

public:
    shader() { m_internal.default_load_function(load_nya_shader); }

public:
    static bool load_nya_shader(shared_shader &res,resource_data &data,const char* name);

public:
    const shader_internal &internal() const { return m_internal; }

private:
    shader_internal m_internal;
};

}
