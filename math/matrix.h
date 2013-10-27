//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_math
{

struct vec3;
struct vec4;
struct quat;

struct mat4
{
    float m[4][4];

    mat4 &identity();
    mat4 &translate(float x,float y,float z);
    mat4 &translate(const nya_math::vec3 &v);
    mat4 &rotate(float angle,float x,float y,float z); //angle in degrees
    mat4 &rotate(const nya_math::quat &q);
    mat4 &scale(float sx,float sy,float sz);
    mat4 &scale(float s) { return scale(s,s,s); }

    mat4 &perspective(float fov,float aspect,float near,float far); //fov in degrees
    mat4 &frustrum(float left,float right,float bottom,float top,float near,float far);

    mat4 &invert();

    mat4 operator * (const mat4 &mat) const;

    mat4() { identity(); }
    mat4(const quat &q);
};

vec3 operator * (const mat4 &m,const vec3 &v);
vec3 operator * (const vec3 &v,const mat4 &m);
vec4 operator * (const mat4 &m,const vec4 &v);

}
