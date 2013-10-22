//https://code.google.com/p/nya-engine/

#pragma once

#include "texture.h"

namespace nya_render
{

class fbo
{
public:
    enum cubemap_side
    {
        cube_positive_x,
        cube_negative_x,
        cube_positive_y,
        cube_negative_y,
        cube_positive_z,
        cube_negative_z
    };

public:
    void set_color_target(const texture &tex);
    void set_color_target(const texture &tex,cubemap_side side);
    void set_depth_target(const texture &tex);

public:
    void release();

public:
    void bind();
    void unbind();

public:
    fbo():m_fbo_idx(0),m_color_target_idx(0),m_depth_target_idx(0){}

private:
    unsigned int m_fbo_idx;
    unsigned int m_color_target_idx;
    unsigned int m_depth_target_idx;
};

}
