//https://code.google.com/p/nya-engine/

#include "transform.h"

namespace nya_render
{

void transform::set_projection_matrix(const nya_math::mat4 &mat)
{
#ifdef MANUAL_MATRICES_ASSIGN
    m_projection=mat;
    m_recalc_mvp=true;
#else
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(mat.m[0]);
#endif
}

void transform::set_modelview_matrix(const nya_math::mat4 &mat)
{
#ifdef MANUAL_MATRICES_ASSIGN
    m_modelview=mat;
    m_recalc_mvp=true;
#else
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(mat.m[0]);
#endif
}

#ifdef MANUAL_MATRICES_ASSIGN
const nya_math::mat4 &transform::get_modelviewprojection_matrix()
{
    if(!m_recalc_mvp)
        return m_modelviewproj;

    m_modelviewproj=m_modelviewproj*m_projection;
    m_recalc_mvp=false;

    return m_modelviewproj;
}
#endif

}
