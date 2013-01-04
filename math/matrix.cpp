//https://code.google.com/p/nya-engine/

#include "matrix.h"
#include <math.h>

namespace nya_math
{

mat4 &mat4::identity()
{
    for(int i=0;i<4;++i) 
        for(int j=0;j<4;++j)
            m[i][j]=0.0f;

    m[0][0]=m[1][1]=m[2][2]=m[3][3]=1.0f;

    return *this;
}

mat4 &mat4::translate(float x,float y,float z)
{
    for(int i=0;i<4;++i)
        m[3][i]+=m[0][i]*x+m[1][i]*y+m[2][i]*z;

    return *this;
}

mat4 &mat4::rotate(float angle,float x,float y,float z)
{
    const float mag=sqrtf(x*x+y*y+z*z);
    const float ang_rad=-angle*M_PI/180.0f;
    const float sin_a=sinf(ang_rad);
    const float cos_a=cosf(ang_rad);

    if(mag < 0.001f)
        return *this;

    const float one_minus_cos=1.0f-cos_a;

    float xx, yy, zz, xy, yz, zx, xs, ys, zs;

    x/=mag; y/=mag; z/=mag;
    xx=x*x; yy=y*y; zz=z*z;
    xy=x*y; yz=y*z; zx=z*x;
    xs=x*sin_a; ys=y*sin_a; zs=z*sin_a;

    mat4 rot;

    rot.m[0][0]=(one_minus_cos*xx)+cos_a;
    rot.m[0][1]=(one_minus_cos*xy)-zs;
    rot.m[0][2]=(one_minus_cos*zx)+ys;
    rot.m[0][3]=0;

    rot.m[1][0]=(one_minus_cos*xy)+zs;
    rot.m[1][1]=(one_minus_cos*yy)+cos_a;
    rot.m[1][2]=(one_minus_cos*yz)-xs;
    rot.m[1][3]=0;

    rot.m[2][0]=(one_minus_cos*zx)-ys;
    rot.m[2][1]=(one_minus_cos*yz)+xs;
    rot.m[2][2]=(one_minus_cos*zz)+cos_a;
    rot.m[2][3]=0;

    rot.m[3][0]=0;
    rot.m[3][1]=0;
    rot.m[3][2]=0;
    rot.m[3][3]=1.0f;

    return *this=rot*(*this);
}
    
mat4 &mat4::perspective(float fov,float aspect,float near,float far)
{
    const float h=tanf(fov*M_PI/360.0f)*near;
    const float w=h*aspect;

    return frustrum(-w,w,-h,h,near,far);
}

mat4 &mat4::frustrum(float left,float right,float bottom,float top,float near,float far)
{
    if(near<=0 || far<=0)
        return identity();

    const float dx=right-left;
    const float dy=top-bottom;
    const float dz=far-near;

    if(dx<=0 || dy<=0 || dz<=0)
        return identity();

    mat4 frust;

    frust.m[0][0]=2.0f*near/dx;
    frust.m[0][1]=frust.m[0][2]=frust.m[0][3]=0;

    frust.m[1][1]=2.0f*near/dy;
    frust.m[1][0]=frust.m[1][2]=frust.m[1][3]=0;

    frust.m[2][0]=(right+left)/dx;
    frust.m[2][1]=(top+bottom)/dy;
    frust.m[2][2]= -(near+far)/dz;
    frust.m[2][3]= -1.0f;

    frust.m[3][2]= -2.0f*near*far/dz;
    frust.m[3][0]=frust.m[3][1]=frust.m[3][3]=0;

    return *this=frust*(*this);
}

mat4 mat4::operator*(const mat4 &mat) const
{
    mat4 out;

    for(int i=0;i<4;++i) 
    {
        for(int j=0;j<4;++j)
        {
            out.m[i][j]=(m[i][0]*mat.m[0][j])+
                        (m[i][1]*mat.m[1][j])+
                        (m[i][2]*mat.m[2][j])+
                        (m[i][3]*mat.m[3][j]);
        }
    }

    return out;
}

mat4 &mat4::scale(float sx,float sy,float sz)
{
    for(int i=0;i<4;++i)
    {
        m[0][i]*=sx;
        m[1][i]*=sy;
        m[2][i]*=sz;
    }

    return *this;
}

}

