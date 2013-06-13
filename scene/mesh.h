//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "material.h"
#include "animation.h"
#include "render/vbo.h"
#include "render/skeleton.h"
#include "math/vector.h"
#include "math/frustum.h"
#include "transform.h"

namespace nya_scene
{

struct shared_mesh
{
    nya_math::aabb aabb;
    nya_render::vbo vbo;

    struct group
    {
        unsigned int material_idx;
        unsigned int offset;
        unsigned int count;
    };

    std::vector<group> groups;

    std::vector<material> materials;
    nya_render::skeleton skeleton;

    bool release()
    {
        aabb=nya_math::aabb();
        vbo.release();
        groups.clear();
        skeleton=nya_render::skeleton();

        return true;
    }
};

typedef proxy<animation> animation_proxy;

class mesh: public scene_shared<shared_mesh>
{
public:
    virtual bool load(const char *name);
    virtual void unload();

    void draw(int group_idx=-1) const;

    void set_pos(float x,float y,float z) { m_transform.set_pos(x,y,z); m_recalc_aabb=true; }
    void set_pos(const nya_math::vec3 &pos) { m_transform.set_pos(pos.x,pos.y,pos.z); m_recalc_aabb=true; }
    void set_rot(float yaw,float pitch,float roll) { m_transform.set_rot(yaw,pitch,roll); m_recalc_aabb=true; }
    void set_rot(const nya_math::quat &rot) { m_transform.set_rot(rot); m_recalc_aabb=true; }
    void set_scale(float sx,float sy,float sz) { m_transform.set_scale(sx,sy,sz); m_recalc_aabb=true; }
    void set_scale(float s) { set_scale(s,s,s); }

public:
    int get_groups_count() const;
    int get_material_idx(int group_idx) const;

    int get_materials_count() const;
    const material &get_material(int material_idx) const;
    material &modify_material(int material_idx);
    void set_material(int material_idx,const material &mat);

public:
    void set_bone_pos(int bone_idx,const nya_math::vec3 &pos,bool additive);
    void set_bone_rot(int bone_idx,const nya_math::quat &rot,bool additive);

public:
    const nya_render::skeleton &get_skeleton() const;

public:
    void set_anim(const animation & anim,int layer=0) { set_anim(animation_proxy(anim)); }
    void set_anim(const animation_proxy & anim,int layer=0);

    const animation_proxy & get_anim(int layer=0) const;
    unsigned int get_anim_time(int layer=0) const;
    void set_anim_time(unsigned int time,int layer=0);

    //float get_anim_weight(int layer=0) const; //mesh own anim weight

public:
    void update(unsigned int dt);

public:
    const nya_math::aabb &get_aabb() const;

public:
    static bool load_nms_mesh_section(shared_mesh &res,const void *data,size_t size);
    static bool load_nms_skeleton_section(shared_mesh &res,const void *data,size_t size);
    static bool load_nms_material_section(shared_mesh &res,const void *data,size_t size);

public:
    static bool load_nms(shared_mesh &res,resource_data &data,const char* name);

public:
    mesh(): m_recalc_aabb(true), m_has_aabb(false) { default_load_function(load_nms); }

private:
    nya_scene_internal::transform m_transform;

private:
    void draw_group(int idx) const;

private:
    std::vector<int> m_replaced_materials_idx;
    std::vector<material> m_replaced_materials;

private:
    struct applied_anim
    {
        int layer;
        float time;
        std::vector<int> bones_map;
        animation_proxy anim;
        unsigned int version;
        bool full_weight;

        void set_time(float time);
        void update_mapping(const nya_render::skeleton &skeleton);

        applied_anim(): layer(0),time(0),version(0) {}
    };

    nya_render::skeleton m_skeleton;
    std::vector<applied_anim> m_anims;

    enum bone_control_mode
    {
        bone_free,
        bone_additive,
        bone_override
    };

    struct bone_control
    {
        nya_math::vec3 pos;
        bone_control_mode pos_ctrl;
        nya_math::quat rot;
        bone_control_mode rot_ctrl;

        bone_control(): pos_ctrl(bone_free),rot_ctrl(bone_free) {}
    };

    typedef std::map<int,bone_control> bone_control_map;
    bone_control_map m_bone_controls;

    mutable bool m_recalc_aabb;
    mutable nya_math::aabb m_aabb;
    bool m_has_aabb;
};

}
