//https://code.google.com/p/nya-engine/

#pragma once

#include "matrix.h"

namespace nya_math
{

struct aabb
{
    vec3 origin;
    vec3 delta;
};

class frustum
{
public:
    bool test_intersect(const aabb &box) const;
    bool test_intersect(const vec3 &v) const;

public:
    frustum() {}
    frustum(const mat4 &m);

private:
    struct plane
    {
        vec3 n;
        vec3 abs_n;
        float d;

        plane(): d(0) {}
    };

    plane m_planes[6];
};

}
