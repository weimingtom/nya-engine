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
    void set_color_target(const texture &tex,unsigned int attachment_idx=0);
    void set_color_target(const texture &tex,cubemap_side side,unsigned int attachment_idx=0);
    void set_depth_target(const texture &tex);

public:
    static unsigned int get_max_color_attachments();

public:
    void release();

public:
    void bind() const;
    static void unbind();

public:
    fbo(): m_fbo_idx(-1) {}

private:
    int m_fbo_idx;
};

}
