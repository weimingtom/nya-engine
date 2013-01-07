//https://code.google.com/p/nya-engine/

#pragma once

#include "math/matrix.h"
#include "math/vector.h"

namespace nya_scene
{

class camera
{
public:
    void set_proj(float fov,float aspect,float near,float far);
    void set_pos(float x,float y,float z);
    void set_rot(float yaw,float pitch,float roll);

public:
    nya_math::mat4 &get_proj_matrix();
    nya_math::mat4 &get_view_matrix();

public:
    camera(): m_recalc_view(true) {}

private:
    nya_math::vec3 m_pos;
    nya_math::vec3 m_rot;

    nya_math::mat4 m_proj;
    nya_math::mat4 m_view;

    bool m_recalc_view;
};

void set_camera(const camera &cam);
const camera &get_camera();

}
