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
}

void camera::set_pos(float x,float y,float z)
{
    m_pos.x=x;
    m_pos.y=y;
    m_pos.z=z;
    
    m_recalc_view=true;
}

void camera::set_rot(float yaw,float pitch,float roll)
{
    m_rot.y=yaw;
    m_rot.x=pitch;
    m_rot.z=roll;

    m_recalc_view=true;
}

nya_math::mat4 &camera::get_view_matrix()
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
        return default_camera;
    }

    return *active_camera;
}

}
