//https://code.google.com/p/nya-engine/

#include "camera.h"
#include "render/render.h"
#include "math/quaternion.h"
#include "math/constants.h"
#include "render/transform.h"

namespace
{
    nya_scene::camera default_camera;
    nya_scene::camera_proxy active_camera(default_camera);
}

namespace nya_scene
{

void camera::set_proj(float fov,float aspect,float near,float far)
{
    m_proj.identity();
    m_proj.perspective(fov,aspect,near,far);
    if(this==active_camera.operator->())
        nya_render::set_projection_matrix(m_proj);

    m_recalc_frustum=true;
}

void camera::set_proj(float left,float right,float bottom,float top,float near,float far)
{
    m_proj.identity();
    m_proj.frustrum(left,right,bottom,top,near,far);
    if(this==active_camera.operator->())
        nya_render::set_projection_matrix(m_proj);

    m_recalc_frustum=true;
}

void camera::set_proj(const nya_math::mat4 &mat)
{
    m_proj=mat;
    if(this==active_camera.operator->())
        nya_render::set_projection_matrix(m_proj);

    m_recalc_frustum=true;
}

void camera::set_pos(const nya_math::vec3 &pos)
{
    m_pos=pos;

    m_recalc_view=true;
    m_recalc_frustum=true;
}

void camera::set_rot(const nya_math::quat &rot)
{
    m_rot=rot;
    m_recalc_view=true;
    m_recalc_frustum=true;
}

void camera::set_rot(float yaw,float pitch,float roll)
{
    const float a2r=nya_math::constants::pi/180.0f;
    set_rot(nya_math::quat(pitch*a2r,yaw*a2r,roll*a2r));
}

const nya_math::mat4 &camera::get_view_matrix() const
{
    if(m_recalc_view)
    {
        m_recalc_view=false;

        nya_math::quat rot=m_rot;
        rot.v.x=-rot.v.x;
        rot.v.y=-rot.v.y;
        m_view=nya_math::mat4(rot);
        m_view.transpose();
        m_view.translate(-m_pos);
    }

    return m_view;
}

const nya_math::frustum &camera::get_frustum() const
{
    if(m_recalc_frustum)
    {
        m_recalc_frustum=false;

        if(nya_render::transform::get().has_orientation_matrix())
            m_frustum=nya_math::frustum(get_view_matrix()*get_proj_matrix()*nya_render::transform::get().get_orientation_matrix());
        else
            m_frustum=nya_math::frustum(get_view_matrix()*get_proj_matrix());
    }

    return m_frustum;
}

void set_camera(const camera_proxy &cam)
{
    active_camera=cam;
    if(cam.is_valid())
        nya_render::set_projection_matrix(cam->get_proj_matrix());
}

const camera_proxy &get_camera() { return active_camera; }

}
