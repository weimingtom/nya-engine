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
        std::string name;
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

class mesh_internal: public scene_shared<shared_mesh>
{
    friend class mesh;

    mesh_internal(): m_recalc_aabb(true), m_has_aabb(false) {}

private:
    nya_scene_internal::transform m_transform;

private:
    void draw_group(int idx) const;
    bool init_form_shared();

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
        
        applied_anim(): layer(0),time(0),version(0) {}
    };
    
private:
    void anim_update_mapping(applied_anim &anim);
    void anim_set_time(applied_anim &anim,float time);
    bool is_anim_finished(int layer=0) const;

private:
    void update(unsigned int dt);

private:
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

class mesh
{
public:
    virtual bool load(const char *name);
    virtual void unload();

public:
    void create(const shared_mesh &res);

public:
    void draw(int group_idx=-1) const;

public:
    void set_pos(float x,float y,float z) { m_internal.m_transform.set_pos(x,y,z); m_internal.m_recalc_aabb=true; }
    void set_pos(const nya_math::vec3 &pos) { m_internal.m_transform.set_pos(pos.x,pos.y,pos.z); m_internal.m_recalc_aabb=true; }
    void set_rot(float yaw,float pitch,float roll) { m_internal.m_transform.set_rot(yaw,pitch,roll); m_internal.m_recalc_aabb=true; }
    void set_rot(const nya_math::quat &rot) { m_internal.m_transform.set_rot(rot); m_internal.m_recalc_aabb=true; }
    void set_scale(float sx,float sy,float sz) { m_internal.m_transform.set_scale(sx,sy,sz); m_internal.m_recalc_aabb=true; }
    void set_scale(float s) { set_scale(s,s,s); }

public:
    const nya_math::vec3 &get_pos() const { return internal().m_transform.get_pos(); }
    const nya_math::quat &get_rot() const { return internal().m_transform.get_rot(); }
    const nya_math::vec3 &get_scale() const { return internal().m_transform.get_scale(); }

public:
    const char *get_name() const { return internal().get_name(); }

public:
    static void set_resources_prefix(const char *prefix) { mesh_internal::set_resources_prefix(prefix); }
    static void register_load_function(mesh_internal::load_function function) { mesh_internal::register_load_function(function); }

public:
    int get_groups_count() const;
    const char *get_group_name(int group_idx) const;
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
    void set_anim(const animation & anim,int layer=0) { set_anim(animation_proxy(anim),layer); }
    void set_anim(const animation_proxy & anim,int layer=0);

    const animation_proxy & get_anim(int layer=0) const;
    unsigned int get_anim_time(int layer=0) const;
    bool is_anim_finished(int layer=0) const;
    void set_anim_time(unsigned int time,int layer=0);

    //float get_anim_weight(int layer=0) const; //mesh own anim weight

public:
    void update(unsigned int dt);

public:
    const nya_math::aabb &get_aabb() const;

public:
    mesh() { internal().default_load_function(load_nms); }

public:
    static bool load_nms_mesh_section(shared_mesh &res,const void *data,size_t size);
    static bool load_nms_skeleton_section(shared_mesh &res,const void *data,size_t size);
    static bool load_nms_material_section(shared_mesh &res,const void *data,size_t size);

public:
    static bool load_nms(shared_mesh &res,resource_data &data,const char* name);

public:
    const mesh_internal &internal() const { return m_internal; }

private:
    mesh_internal m_internal;
};

}
