//https://code.google.com/p/nya-engine/

#pragma once

#include "math/matrix.h"
#include "math/vector.h"

namespace nya_scene_internal
{

class transform
{
public:
    static void set(const transform &tr);
    static const transform &get();

public:
    void set_pos(float x,float y,float z) { m_pos.x=x; m_pos.y=y; m_pos.z=z; }
    void set_rot(float yaw,float pitch,float roll) { m_rot.y=yaw; m_rot.x=pitch; m_rot.z=roll; }
    void set_scale(float sx,float sy,float sz) { m_scale.x=sx; m_scale.y=sy; m_scale.z=sz; }

public:
    nya_math::vec3 inverse_transform(const nya_math::vec3 &vec) const;
    nya_math::vec3 inverse_rot(const nya_math::vec3 &vec) const;

public:
    transform():m_scale(nya_math::vec3(1.0f,1.0f,1.0f)){}

private:
    void apply() const;

private:
    nya_math::vec3 m_pos;
    nya_math::vec3 m_rot;
    nya_math::vec3 m_scale;
};

}
