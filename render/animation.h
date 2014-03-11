//https://code.google.com/p/nya-engine/

#pragma once

#include "math/vector.h"
#include "math/quaternion.h"
#include "math/bezier.h"
#include <vector>
#include <map>
#include <string>

namespace nya_render
{

class animation
{
public:
    unsigned int get_duration() const { return m_duration; }
    int get_bone_idx(const char *name) const; //< 0 if invalid
    nya_math::vec3 get_bone_pos(int idx,unsigned int time,bool looped=true) const;
    nya_math::quat get_bone_rot(int idx,unsigned int time,bool looped=true) const;
    int get_bones_count() const { return (int)m_bone_names.size(); }
    const char *get_bone_name(int idx) const;

    int get_curve_idx(const char *name) const; //< 0 if invalid
    float get_curve(int idx,unsigned int time,bool looped=true) const;
    int get_cuves_count() const { return (int)m_curves.size(); }
    const char *get_curve_name(int idx) const;

public:
    int add_bone(const char *name); //create or return existing
    void add_bone_pos_frame(int bone_idx,unsigned int time,const nya_math::vec3 &pos) { pos_interpolation i; add_bone_pos_frame(bone_idx,time,pos,i); }
    void add_bone_rot_frame(int bone_idx,unsigned int time,const nya_math::quat &rot) { nya_math::bezier i; add_bone_rot_frame(bone_idx,time,rot,i); }

    struct pos_interpolation
    {
        nya_math::bezier x;
        nya_math::bezier y;
        nya_math::bezier z;
    };

    void add_bone_pos_frame(int bone_idx,unsigned int time,const nya_math::vec3 &pos,const pos_interpolation &interpolation);
    void add_bone_rot_frame(int bone_idx,unsigned int time,const nya_math::quat &rot,const nya_math::bezier &interpolation);

public:
    int add_curve(const char *name); //create or return existing
    void add_curve_frame(int idx,unsigned int time,float value);

public:
    void release() { *this=animation(); }

public:
    animation(): m_duration(0) {}

private:
    template<typename t,typename interpolation>struct frame
    {
        unsigned int time;
        t value;
        interpolation inter;

        frame(): time(0) {}
    };

    struct pos_frame: public frame<nya_math::vec3,pos_interpolation> { nya_math::vec3 interpolate(const pos_frame &prev,float k) const; };
    struct rot_frame: public frame<nya_math::quat,nya_math::bezier> { nya_math::quat interpolate(const rot_frame &prev,float k) const; };

    typedef std::vector<pos_frame> pos_sequence;
    typedef std::vector<rot_frame> rot_sequence;


    typedef std::map<std::string,unsigned int> index_map;
    index_map m_bones_map;
    std::vector<std::string> m_bone_names;
    std::vector<pos_sequence> m_pos_sequences;
    std::vector<rot_sequence> m_rot_sequences;

    struct curve_frame
    {
        unsigned int time;
        float value;

        float interpolate(const curve_frame &prev,float k) const;

        curve_frame(): time(0),value(0.0f) {}
    };

    typedef std::vector<curve_frame> curve_sequence;

    index_map m_curves_map;
    std::vector<curve_sequence> m_curves;
    std::vector<std::string> m_curve_names;

    unsigned int m_duration;
};

}
