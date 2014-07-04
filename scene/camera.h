//https://code.google.com/p/nya-engine/

#pragma once

#include "proxy.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "math/quaternion.h"
#include "math/frustum.h"

namespace nya_scene
{

class camera
{
public:
    void set_proj(float fov,float aspect,float near,float far);
    void set_proj(float left,float right,float bottom,float top,float near,float far);
    void set_proj(const nya_math::mat4 &mat);

    void set_pos(float x,float y,float z) { set_pos(nya_math::vec3(x,y,z)); }
    void set_pos(const nya_math::vec3 &pos);
    void set_rot(float yaw,float pitch,float roll);
    void set_rot(const nya_math::quat &rot);
    void set_rot(const nya_math::vec3 &direction);

public:
    const nya_math::mat4 &get_proj_matrix() const { return m_proj; }
    const nya_math::mat4 &get_view_matrix() const;

    const nya_math::frustum &get_frustum() const;

public:
	const nya_math::vec3 &get_pos() const { return m_pos; }
	const nya_math::quat &get_rot() const { return m_rot; }

public:
    camera(): m_recalc_view(true), m_recalc_frustum(true) {}

private:
    nya_math::vec3 m_pos;
    nya_math::quat m_rot;

    mutable nya_math::mat4 m_proj;
    mutable nya_math::mat4 m_view;

    mutable nya_math::frustum m_frustum;

    mutable bool m_recalc_view;
    mutable bool m_recalc_frustum;
};

typedef proxy<camera> camera_proxy;

void set_camera(const camera_proxy &cam);
const camera_proxy &get_camera();

}
