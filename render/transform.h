//https://code.google.com/p/nya-engine/

#pragma once

#include "math/matrix.h"
#include "platform_specific_gl.h"

#ifdef OPENGL_ES
    #define MANUAL_MATRICES_ASSIGN
#endif

namespace nya_render
{

class transform
{
public:
    void set_projection_matrix(const nya_math::mat4 &mat);
    void set_modelview_matrix(const nya_math::mat4 &mat);

#ifdef MANUAL_MATRICES_ASSIGN
public:
    const nya_math::mat4 &get_projection_matrix() { return m_projection; }
    const nya_math::mat4 &get_modelview_matrix() { return m_modelview; }
    const nya_math::mat4 &get_modelviewprojection_matrix();
#endif

public:
    static transform &get()
    {
        static transform tr;
        return tr;
    }

#ifdef MANUAL_MATRICES_ASSIGN
private:
    nya_math::mat4 m_projection;
    nya_math::mat4 m_modelview;

    bool m_recalc_mvp;
    nya_math::mat4 m_modelviewproj;
#endif

};

}
