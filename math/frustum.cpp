//https://code.google.com/p/nya-engine/

#include "frustum.h"
#include "quaternion.h"

namespace nya_math
{

aabb::aabb(const aabb &b,const vec3 &p,const quat &q,const vec3 &s)
{
    delta.x=s.x*b.delta.x;
    delta.y=s.y*b.delta.y;
    delta.z=s.z*b.delta.z;

    const nya_math::vec3 v[4]=
    {
        nya_math::vec3(delta.x,delta.y,delta.z),
        nya_math::vec3(delta.x,-delta.y,delta.z),
        nya_math::vec3(delta.x,delta.y,-delta.z),
        nya_math::vec3(delta.x,-delta.y,-delta.z)
    };

    delta=nya_math::vec3();

    for(int i=0;i<4;++i)
    {
        const nya_math::vec3 t=q.rotate(v[i]).abs();
        if(t.x>delta.x) delta.x=t.x;
        if(t.y>delta.y) delta.y=t.y;
        if(t.z>delta.z) delta.z=t.z;
    }

    origin.x=b.origin.x*s.x;
    origin.y=b.origin.y*s.y;
    origin.z=b.origin.z*s.z;
    origin=q.rotate(origin)+p;
}

bool frustum::test_intersect(const aabb &box) const
{
	for(int i=0;i<6;++i)
	{
        const plane &p=m_planes[i];
		if(box.origin*p.n+(box.delta*p.abs_n+p.d)<0.0f)
			return false;
	}

	return true;
}

bool frustum::test_intersect(const vec3 &v) const
{
    const float eps = 0.001f;
	for(int i=0;i<6;++i)
    {
        const plane &p=m_planes[i];
        if((p.n*(p.n*p.d+v) )< -eps)
            return false;
    }

    return true;
}

frustum::frustum(const mat4 &m)
{
    for(int i=0;i<3;++i)
    {
        const int idx=2*i;
        plane &p=m_planes[idx];
        p.n.x=m[0][3]-m[0][i];
        p.n.y=m[1][3]-m[1][i];
        p.n.z=m[2][3]-m[2][i];
        p.d=m[3][3]-m[3][i];

        plane &p2=m_planes[idx+1];
        p2.n.x=m[0][3]+m[0][i];
        p2.n.y=m[1][3]+m[1][i];
        p2.n.z=m[2][3]+m[2][i];
        p2.d=m[3][3]+m[3][i];
    }

	for(int i=0;i<6;++i)
	{
        plane &p=m_planes[i];
        float len=p.n.length();
        if(len>0.0001f)
        {
            p.n/=len;
            p.d/=len;
        }

        p.abs_n=nya_math::vec3::abs(p.n);
    }
}

}
