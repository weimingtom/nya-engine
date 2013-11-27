//https://code.google.com/p/nya-engine/

#include "camera.h"
#include "render/render.h"

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

void camera::set_pos(float x,float y,float z)
{
    m_pos.x=x;
    m_pos.y=y;
    m_pos.z=z;

    m_recalc_view=true;
    m_recalc_frustum=true;
}

void camera::set_rot(float yaw,float pitch,float roll)
{
    m_rot.y=yaw;
    m_rot.x=pitch;
    m_rot.z=roll;

    m_recalc_view=true;
    m_recalc_frustum=true;
}

const nya_math::mat4 &camera::get_view_matrix() const
{
    if(m_recalc_view)
    {
        m_recalc_view=false;

        m_view.identity();
        m_view.rotate(m_rot.z,0,0,1);
        m_view.rotate(m_rot.x,1,0,0);
        m_view.rotate(m_rot.y,0,1,0);
        m_view.translate(-m_pos.x,-m_pos.y,-m_pos.z);
    }

    return m_view;
}

const nya_math::frustum &camera::get_frustum() const
{
    if(m_recalc_frustum)
    {
        m_recalc_frustum=false;

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
