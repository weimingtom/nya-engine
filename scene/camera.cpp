//https://code.google.com/p/nya-engine/

#include "camera.h"
#include "render/render.h"

namespace
{
    nya_scene::camera *active_camera=0;
}

namespace nya_scene
{

void camera::set_proj(float fov,float aspect,float near,float far)
{
    m_proj.identity();
    m_proj.perspective(fov,aspect,near,far);
    if(this==active_camera)
        nya_render::set_projection_matrix(m_proj);

    m_recalc_frustrum=true;
}

void camera::set_proj(float left,float right,float bottom,float top,float near,float far)
{
    m_proj.identity();
    m_proj.frustrum(left,right,bottom,top,near,far);
    if(this==active_camera)
        nya_render::set_projection_matrix(m_proj);

    m_recalc_frustrum=true;
}

void camera::set_pos(float x,float y,float z)
{
    m_pos.x=x;
    m_pos.y=y;
    m_pos.z=z;

    m_recalc_view=true;
    m_recalc_frustrum=true;
}

void camera::set_rot(float yaw,float pitch,float roll)
{
    m_rot.y=yaw;
    m_rot.x=pitch;
    m_rot.z=roll;

    m_recalc_view=true;
    m_recalc_frustrum=true;
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

const nya_math::frustrum &camera::get_frustrum() const
{
    if(m_recalc_frustrum)
    {
        m_recalc_frustrum=false;

        m_frustrum=nya_math::frustrum(get_view_matrix()*get_proj_matrix());
    }

    return m_frustrum;
}

void set_camera(camera &cam)
{
    active_camera=&cam;
    nya_render::set_projection_matrix(cam.get_proj_matrix());
}

camera &get_camera()
{
    if(!active_camera)
    {
        static camera default_camera;
        active_camera=&default_camera;
        return default_camera;
    }

    return *active_camera;
}

}
