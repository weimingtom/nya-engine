//https://code.google.com/p/nya-engine/

#include "transform.h"
#include "render/render.h"
#include "camera.h"

//ToDo: quaternions

namespace
{
    const nya_scene_internal::transform *active_transform=0;
}

namespace nya_scene_internal
{

void transform::set(const transform &tr)
{
    active_transform=&tr;
    tr.apply();
}

const transform &transform::get()
{
    if(!active_transform)
    {
        static transform invalid;
        return invalid;
    }

    return *active_transform;
}

nya_math::vec3 transform::inverse_transform(const nya_math::vec3 &vec) const
{
    const float eps=0.0001f;
    if(m_scale*m_scale<eps)
        return nya_math::vec3();

	nya_math::mat4 inv_transform;
    inv_transform.scale(1.0f/m_scale.x,1.0f/m_scale.y,1.0f/m_scale.z);
    inv_transform.rotate(-m_rot.z,0,0,1);
    inv_transform.rotate(-m_rot.x,1,0,0);
    inv_transform.rotate(-m_rot.y,0,1,0);
    inv_transform.translate(-m_pos.x,-m_pos.y,-m_pos.z);

    return inv_transform*vec;
}

nya_math::vec3 transform::inverse_rot(const nya_math::vec3 &vec) const
{
    nya_math::mat4 inv_transform;
    inv_transform.rotate(-m_rot.z,0,0,1);
    inv_transform.rotate(-m_rot.x,1,0,0);
    inv_transform.rotate(-m_rot.y,0,1,0);

    return inv_transform*vec;
}

void transform::apply() const
{
    nya_math::mat4 mat=nya_scene::get_camera().get_view_matrix();
    mat.translate(m_pos.x,m_pos.y,m_pos.z);
    mat.rotate(m_rot.y,0,1,0);
    mat.rotate(m_rot.x,1,0,0);
    mat.rotate(m_rot.z,0,0,1);
    mat.scale(m_scale.x,m_scale.y,m_scale.z);

    nya_render::set_modelview_matrix(mat);
}

}
