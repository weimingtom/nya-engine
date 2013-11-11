//https://code.google.com/p/nya-engine/

#include "quaternion.h"
#include "constants.h"
#include <math.h>

namespace nya_math
{

quat quat::slerp(const quat &q1,const quat &q2,float t)
{
    const float eps=0.001f;
    float scale0,scale1;

    const float cosom=q1.v*q2.v+q1.w*q2.w;
    if(cosom<0.0f)
    {
        if (1.0f+cosom>eps)
        {
            const float omega=acosf(-cosom);
            const float sinom_inv=1.0f/sinf(omega);
            scale0=sinf((1.0f-t)*omega)*sinom_inv;
            scale1=-sinf(t*omega)*sinom_inv;
        }
        else
        {
            scale0=1.0f-t;
            scale1=-t;
        }
    }
    else
    {
        if (1.0f-cosom>eps)
        {
            const float omega=acosf(cosom);
            const float sinom_inv=1.0f/sinf(omega);
            scale0=sinf((1.0f-t)*omega)*sinom_inv;
            scale1=sinf(t*omega)*sinom_inv;
        }
        else
        {
            scale0=1.0f-t;
            scale1=t;
        }
    }

    return quat(scale0*q1.v.x+scale1*q2.v.x,
                scale0*q1.v.y+scale1*q2.v.y,
                scale0*q1.v.z+scale1*q2.v.z,
                scale0*q1.w+scale1*q2.w);
}

vec3 quat::get_euler() const
{
    const float x2=v.x+v.x;
    const float y2=v.y+v.y;
    const float z2=v.z+v.z;
    const float xz2=v.x*z2;
    const float wy2=w*y2;

    float temp=wy2-xz2;
    if(temp>=1.0f)
        temp=1.0f;
    else if(temp<=-1.0f)
        temp=-1.0f;

    const float ang=asinf(temp);

    const float xx2=v.x*x2;
    const float xy2=v.x*y2;
    const float zz2=v.z*z2;
    const float wz2=w*z2;

    if(ang>=constants::pi_2)
        return vec3(atan2f(xy2-wz2,1.0f-xx2+zz2),ang,0.0f);

    if(ang> -constants::pi_2)
    {
        const float yz2=v.y*z2;
        const float wx2=w*x2;
        const float yy2=v.y*y2;

        return vec3(atan2f(yz2+wx2,1.0f-xx2+yy2),ang,
                    atan2f(xy2+wz2,1.0f-yy2+zz2));
    }

    return vec3(-atan2f(xy2-wz2,1.0f-xx2+zz2),ang,0.0f);
}

quat::quat(float pitch,float yaw,float roll)
{
    pitch*=0.5f; yaw*=0.5f; roll*=0.5f;

    const float sin_x=sinf(pitch);
    const float cos_x=cosf(pitch);

    const float sin_y=sinf(yaw);
    const float cos_y=cosf(yaw);

    const float sin_z=sinf(roll);
    const float cos_z=cosf(roll);

    v.x=sin_x*cos_y*cos_z - cos_x*sin_y*sin_z;
    v.y=cos_x*sin_y*cos_z + sin_x*cos_y*sin_z;
    v.z=cos_x*cos_y*sin_z - sin_x*sin_y*cos_z;
    w  =cos_x*cos_y*cos_z + sin_x*sin_y*sin_z;
}

quat::quat(vec3 axis,float angle)
{
    angle*=0.5f;

    v=axis*sinf(angle);
    w=cosf(angle);
}

quat &quat::limit_angle(float from,float to)
{
    vec3 ang=get_euler();

    if(ang.x<from)
        ang.x=from;
    if(ang.x>to)
        ang.x=to;

    ang.y=0.0f;
    ang.z=0.0f;

    return *this=quat(ang.x,ang.y,ang.z);
}

quat &quat::normalize()
{
    const float len=sqrtf(v*v+w*w);
    if(len>0.00001f)
    {
        const float len_inv=1.0f/len;
        v*=len_inv;
        w*=len_inv;
    }

    return *this;
}

quat &quat::apply_weight(float weight)
{
    v*=weight;
    w*=weight;
    w+=1.0f-weight;

    return *this;
}

quat quat::operator * (const quat &q) const
{
    return quat(w*q.v.x + v.x*q.w   + v.y*q.v.z - v.z*q.v.y,
                w*q.v.y - v.x*q.v.z + v.y*q.w   + v.z*q.v.x,
                w*q.v.z + v.x*q.v.y - v.y*q.v.x + v.z*q.w,
                w*q.w   - v.x*q.v.x - v.y*q.v.y - v.z*q.v.z);
}

vec3 quat::rotate(const vec3 &vec) const
{
    return vec+vec3::cross(v,vec3::cross(v,vec)+vec*w)*2.0f;
}

vec3 quat::rotate_inv(const vec3 &vec) const
{
    return vec+vec3::cross(vec3::cross(vec,v)+vec*w,v)*2.0f;
}

}
