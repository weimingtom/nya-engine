//https://code.google.com/p/nya-engine/

#include "viewer_camera.h"
#include "scene/camera.h"

void viewer_camera::add_rot(float dx,float dy)
{
    m_rot_x+=dx;
    m_rot_y+=dy;

    const float max_angle=360.0f;

    if(m_rot_x>max_angle)
        m_rot_x-=max_angle;

    if(m_rot_x< -max_angle)
        m_rot_x+=max_angle;

    if(m_rot_y>max_angle)
        m_rot_y-=max_angle;

    if(m_rot_y< -max_angle)
        m_rot_y+=max_angle;

    update();
}

void viewer_camera::add_pos(float dx,float dy,float dz)
{
    m_pos.x-=dx;
    m_pos.y-=dy;
    m_pos.z-=dz;
    if(m_pos.z<2.0f)
        m_pos.z=2.0f;

    if(m_pos.z>200.0f)
        m_pos.z=200.0f;

    update();
}

void viewer_camera::set_aspect(float aspect)
{
    nya_scene::get_camera().set_proj(55.0,aspect,1.0,2000.0);
    update();
}

void viewer_camera::update()
{
    nya_scene::get_camera().set_rot(m_rot_x,m_rot_y,0.0);

    nya_math::quat rot(-m_rot_y*3.14f/180.0f,-m_rot_x*3.14f/180.0f,0.0f);
    nya_math::vec3 pos=rot.rotate(m_pos);

    nya_scene::get_camera().set_pos(pos.x,pos.y+10.0f,pos.z);
}
