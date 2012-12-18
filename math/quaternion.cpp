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

}

