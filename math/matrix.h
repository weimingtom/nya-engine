//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_math
{

struct mat4
{
    float m[4][4];

    mat4 &identity();
    mat4 &translate(float x,float y,float z);
    mat4 &rotate(float angle,float x,float y,float z);
    mat4 &scale(float sx,float sy,float sz);
    mat4 &scale(float s) { return scale(s,s,s); }
    
    mat4 &perspective(float fov,float aspect,float near,float far);
    mat4 &frustrum(float left,float right,float bottom,float top,float near,float far);

    mat4 operator * (const mat4 &mat) const;

    mat4() { identity(); }
};

}
