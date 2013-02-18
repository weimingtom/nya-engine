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
    void set_frustrum(float left,float right,float bottom,float top,float near,float far);

    void set_pos(float x,float y,float z);
    void set_rot(float yaw,float pitch,float roll);

public:
    const nya_math::mat4 &get_proj_matrix() const { return m_proj; }
    const nya_math::mat4 &get_view_matrix() const;

public:
	const nya_math::vec3 &get_pos() const { return m_pos; }

public:
    camera(): m_recalc_view(true) {}

private:
    nya_math::vec3 m_pos;
    nya_math::vec3 m_rot;

    mutable nya_math::mat4 m_proj;
    mutable nya_math::mat4 m_view;

    mutable bool m_recalc_view;
};

void set_camera(camera &cam);
camera &get_camera();

}
