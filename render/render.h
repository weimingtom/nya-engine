//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"
#include "math/matrix.h"

namespace nya_render
{

void set_log(nya_log::log *l);
nya_log::log &get_log();
void log_gl_errors(const char *place=0);

void set_projection_matrix(const nya_math::mat4 &mat);
void set_modelview_matrix(const nya_math::mat4 &mat);

void set_color(float r,float g,float b,float a);

struct blend
{
    enum mode
    {
        zero,
        one,
        src_color,
        inv_src_color,
        src_alpha,
        inv_src_alpha,
        dst_color,
        inv_dst_color,
        dst_alpha,
        inv_dst_alpha
    };

    static void enable(mode src,mode dst);
    static void disable();

private:
    static unsigned int gl_mode(mode m);
};

struct cull_face
{
    enum order
    {
        cw,
        ccw
    };

    static void enable(order o);
    static void disable();
};

struct zwrite
{
    static void enable();
    static void disable();
};

struct scissor
{
    static void enable(int x,int y,int w,int h);
    static void disable();
};

}
