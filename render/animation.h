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
    struct bone
    {
        nya_math::vec3 pos;
        nya_math::quat rot;
    };

public:
    unsigned int get_duration() const { return m_duration; }
    int get_bone_idx(const char *name) const; //< 0 if invalid
    bone get_bone(int idx,unsigned int time,bool looped=true) const;
    int get_bones_count() const { return (int)m_bones.size(); }
    const char *get_bone_name(int idx) const;

    int get_curve_idx(const char *name) const; //< 0 if invalid
    float get_curve(int idx,unsigned int time,bool looped=true) const;
    int get_cuves_count() const { return (int)m_curves.size(); }
    const char *get_curve_name(int idx) const;

public:
    int add_bone(const char *name); //create or return existing
    void add_bone_frame(int idx,unsigned int time,bone &b);

    struct interpolation
    {
        nya_math::bezier pos_x;
        nya_math::bezier pos_y;
        nya_math::bezier pos_z;
        nya_math::bezier rot;
    };

    void add_bone_frame(int idx,unsigned int time,bone &b,interpolation &i);

public:
    int add_curve(const char *name); //create or return existing
    void add_curve_frame(int idx,unsigned int time,float value);

public:
    void release();

public:
    animation(): m_duration(0) {}

private:
    struct frame
    {
        unsigned int time;
        bone value;
        interpolation inter;

        bone interpolate(const frame &prev,float k) const;

        frame(): time(0) {}
    };

    typedef std::map<std::string,unsigned int> index_map;

    struct sequence
    {
        std::string name;
        std::vector<frame> frames;
    };

    index_map m_bones_map;
    std::vector<sequence> m_bones;

    struct curve_frame
    {
        unsigned int time;
        float value;

        float interpolate(const curve_frame &prev,float k) const;

        curve_frame(): time(0),value(0.0f) {}
    };

    struct curve_sequence
    {
        std::string name;
        std::vector<curve_frame> frames;
    };

    index_map m_curves_map;
    std::vector<curve_sequence> m_curves;

    unsigned int m_duration;
};

}
