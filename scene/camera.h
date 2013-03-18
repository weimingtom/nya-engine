//https://code.google.com/p/nya-engine/

#pragma once

#include "math/matrix.h"
#include "math/vector.h"
#include "math/frustrum.h"

namespace nya_scene
{

class camera
{
public:
    void set_proj(float fov,float aspect,float near,float far);
    void set_proj(float left,float right,float bottom,float top,float near,float far);

    void set_pos(float x,float y,float z);
    void set_rot(float yaw,float pitch,float roll);

public:
    const nya_math::mat4 &get_proj_matrix() const { return m_proj; }
    const nya_math::mat4 &get_view_matrix() const;

    const nya_math::frustrum &get_frustrum() const;

public:
	const nya_math::vec3 &get_pos() const { return m_pos; }

public:
    camera(): m_recalc_view(true), m_recalc_frustrum(true) {}

private:
    nya_math::vec3 m_pos;
    nya_math::vec3 m_rot;

    mutable nya_math::mat4 m_proj;
    mutable nya_math::mat4 m_view;

    mutable nya_math::frustrum m_frustrum;

    mutable bool m_recalc_view;
    mutable bool m_recalc_frustrum;
};

void set_camera(camera &cam);
camera &get_camera();

}
