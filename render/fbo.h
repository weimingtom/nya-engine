//https://code.google.com/p/nya-engine/

#pragma once

#include "texture.h"

namespace nya_render
{

class fbo
{
public:
    void set_color_target(const texture &tex);
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
