//https://code.google.com/p/nya-engine/

#include "transform.h"
#include "platform_specific_gl.h"

#if defined ATTRIBUTES_INSTEAD_OF_CLIENTSTATES || defined DIRECTX11
    #define MANUAL_MATRICES_ASSIGN
#endif

namespace nya_render
{

void transform::set_orientation_matrix(const nya_math::mat4 &mat)
{
    m_orientation=mat;
    m_orientated_proj=m_projection*m_orientation;
    m_has_orientation=true;
}

void transform::set_projection_matrix(const nya_math::mat4 &mat)
{
    m_projection=mat, m_recalc_mvp=true;
    if(m_has_orientation)
        m_orientated_proj=m_projection*m_orientation;

#ifndef MANUAL_MATRICES_ASSIGN
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(get_projection_matrix().m[0]);
#endif
}

void transform::set_modelview_matrix(const nya_math::mat4 &mat)
{
    m_modelview=mat, m_recalc_mvp=true;

#ifndef MANUAL_MATRICES_ASSIGN
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mat.m[0]);
#endif
}

const nya_math::mat4 &transform::get_modelviewprojection_matrix()
{
    if(!m_recalc_mvp)
        return m_modelviewproj;

    m_modelviewproj=m_modelview*get_projection_matrix();
    m_recalc_mvp=false;

    return m_modelviewproj;
}

}
