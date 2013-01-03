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
    float mag=sqrtf(x*x+y*y+z*z);
    float ang_rad=-angle*M_PI/180.0f;
    float sin_a=sinf(ang_rad);
    float cos_a=cosf(ang_rad);

    if(mag < 0.001f)
        return *this;
    
    float one_minus_cos=1.0-cos_a;

    float xx, yy, zz, xy, yz, zx, xs, ys, zs;

    x/=mag; y/=mag; z/=mag;
    xx=x*x; yy=y*y; zz=z*z;
    xy=x*y; yz=y*z; zx=z*x;
    xs=x*sin_a; ys=y*sin_a; zs=z*sin_a;

    mat4 rot;

    rot.m[0][0]=(one_minus_cos*xx)+cos_a;
    rot.m[0][1]=(one_minus_cos*xy)-zs;
    rot.m[0][2]=(one_minus_cos*zx)+ys;
    rot.m[0][3]=0.0;

    rot.m[1][0]=(one_minus_cos*xy)+zs;
    rot.m[1][1]=(one_minus_cos*yy)+cos_a;
    rot.m[1][2]=(one_minus_cos*yz)-xs;
    rot.m[1][3]=0.0;

    rot.m[2][0]=(one_minus_cos*zx)-ys;
    rot.m[2][1]=(one_minus_cos*yz)+xs;
    rot.m[2][2]=(one_minus_cos*zz)+cos_a;
    rot.m[2][3]=0.0;

    rot.m[3][0]=0.0;
    rot.m[3][1]=0.0;
    rot.m[3][2]=0.0;
    rot.m[3][3]=1.0;

    *this=rot*(*this);

    return *this;
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

