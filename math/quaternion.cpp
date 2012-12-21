//https://code.google.com/p/nya-engine/

#include "quaternion.h"
#include <math.h>

namespace nya_math
{

quat quat::slerp(const quat &q1,const quat &q2,float t)
{
    float p1[4];
    float omega,cosom,sinom,scale0,scale1;

    cosom=q1.x*q2.x+q1.y*q2.y+q1.z*q2.z+q1.w*q2.w;

    if(cosom<0.0)
    {
        cosom= -cosom;
        p1[0]= -q2.x;
        p1[1]= -q2.y;
        p1[2]= -q2.z;
        p1[3]= -q2.w;
    }
    else
    {
        p1[0]=q2.x;
        p1[1]=q2.y;
        p1[2]=q2.z;
        p1[3]=q2.w;
    }

    const float eps=0.001f;
    if ((1.0-cosom)>eps)
    {
        omega=acosf(cosom);
        sinom=sinf(omega);
        scale0=sinf((1.0f-t)*omega)/sinom;
        scale1=sinf(t*omega)/sinom;
    }
    else
    {
        scale0=1.0f-t;
        scale1=t;
    }

    return quat(scale0*q1.x+scale1*p1[0],
                scale0*q1.y+scale1*p1[1],
                scale0*q1.z+scale1*p1[2],
                scale0*q1.w+scale1*p1[3]);
}

vec3 quat::get_euler()
{
    const float x2=x+x;
    const float y2=y+y;
    const float z2=z+z;
    const float xz2=x*z2;
    const float wy2=w*y2;

    float temp=wy2-xz2;
    if(temp>=1.0f)
        temp=1.0f;
    else if(temp<=-1.0f)
        temp=-1.f;

    const float ang=asinf(temp);

    const float xx2=x*x2;
    const float xy2=x*y2;
    const float zz2=z*z2;
    const float wz2=w*z2;

    if(ang>=M_PI_2)
    {
		return vec3(atan2f(xy2-wz2,1.0f-xx2+zz2),ang,0.0f);
    }

    if(ang> -M_PI_2)
    {
        const float yz2=y*z2;
        const float wx2=w*x2;
        const float yy2=y*y2;

        return vec3(atan2f(yz2+wx2,1.0f-xx2+yy2),ang,
                    atan2f(xy2+wz2,1.0f-yy2+zz2));
    }

    return vec3(-atan2f(xy2-wz2,1.0f-xx2+zz2),ang,0.0f);
}


quat::quat(vec3 euler)
{
    euler*=0.5f;

    const float sin_x=sinf(euler.x);
    const float cos_x=cosf(euler.x);

    const float sin_y=sinf(euler.y);
    const float cos_y=cosf(euler.y);

    const float sin_z=sinf(euler.z);
    const float cos_z=cosf(euler.z);

	x=sin_x*cos_y*cos_z - cos_x*sin_y*sin_z;
	y=cos_x*sin_y*cos_z + sin_x*cos_y*sin_z;
	z=cos_x*cos_y*sin_z - sin_x*sin_y*cos_z;
	w=cos_x*cos_y*cos_z + sin_x*sin_y*sin_z;
}


}

