//https://code.google.com/p/nya-engine/

#pragma once

#include "math/vector.h"
#include "math/quaternion.h"

#include <string>
#include <map>
#include <vector>

namespace nya_render
{

class skeleton
{
public:
    int get_bone_idx(const char *name) const; //< 0 if invalid
    const char *get_bone_name(int idx) const;
    int get_bone_parent_idx(int idx) const;
    nya_math::vec3 transform(int bone_idx,nya_math::vec3 point) const;
    int get_bones_count() const { return (int)m_bones.size(); }

    nya_math::vec3 get_bone_pos(int idx) const;
    nya_math::quat get_bone_rot(int idx) const;

    void set_bone_transform(int bone_idx,const nya_math::vec3 &pos,
                                                const nya_math::quat &rot);
    void update();

public:
    const float *get_pos_buffer() const;
    const float *get_rot_buffer() const;

public:
    int add_bone(const char *name,const nya_math::vec3 &pos,int parent_bone_idx=-1);

public:
    int add_ik(int target_bone_idx,int effect_bone_idx,int count,float fact);
    void add_ik_link(int ik_idx,int bone_idx);
    void add_ik_link(int ik_idx,int bone_idx,float limit_from,float limit_to);

private:
    void update_bone(int idx);

private:
    typedef std::map<std::string,unsigned int> index_map;

    struct bone
    {
        nya_math::vec3 pos_org;
        nya_math::vec3 offset;

        nya_math::vec3	pos;
        nya_math::quat	rot;

        int parent;

        index_map::const_iterator map_it;
    };

    index_map m_bones_map;
    std::vector<bone> m_bones;

    std::vector<nya_math::vec3> m_pos_tr;
    std::vector<nya_math::quat> m_rot_tr;

    struct ik_link
    {
        int idx;
        bool limit;
        float limit_from;
        float limit_to;
    };

    struct ik
    {
        int target;
        int eff;

        int count;
        float fact;

        std::vector<ik_link> links;
    };

    std::vector<ik> m_iks;
};

}
