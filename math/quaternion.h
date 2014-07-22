//https://code.google.com/p/nya-engine/

#pragma once

#include "vector.h"

namespace nya_math
{

struct quat
{
    vec3 v;
    float w;

    quat(): w(1.0f) {}

    quat(float x,float y,float z,float w)
    {
        v.x=x; v.y=y;
        v.z=z; this->w=w;
    }

    quat(float pitch,float yaw,float roll); //angles in radians

    quat(const vec3 &axis,float angle); //angle in radians

    quat(const vec3 &from,const vec3 &to);

    explicit quat(const float *q) { v.x=q[0]; v.y=q[1]; v.z=q[2]; w=q[3]; }

    quat operator - () const { return quat(-v.x,-v.y,-v.z,-w); }

    quat operator * (const quat &q) const;

    vec3 get_euler() const; //pitch,yaw,roll

    quat &limit_pitch(float from,float to);

    quat &normalize();

    quat &apply_weight(float weight);

    vec3 rotate(const vec3 &v) const;
    vec3 rotate_inv(const vec3 &v) const;

    static quat slerp(const quat &from,const quat &to,float t);
};

}
