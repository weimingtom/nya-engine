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

    quat(vec3 euler);

    quat(vec3 axis,float angle);

    quat(const float *q) { v.x=q[0]; v.y=q[1]; v.z=q[2]; w=q[3]; }

    quat operator - () const { return quat(-v.x,-v.y,-v.z,-w); }

    quat operator * (const quat &q) const;

    vec3 get_euler() const;

    quat limit_angle(float from,float to);

    quat normalize();

    vec3 rotate(const vec3 &v) const;
    vec3 rotate_inv(const vec3 &v) const;

    static quat slerp(const quat &from,const quat &to,float t);
};

}
