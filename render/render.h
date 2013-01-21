//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"
#include "math/matrix.h"

namespace nya_render
{

void set_log(nya_log::log *l);
nya_log::log &get_log();

void set_projection_matrix(const nya_math::mat4 &mat);
void set_modelview_matrix(const nya_math::mat4 &mat);

struct blend
{
    enum mode
    {
        zero,
        one,
        src_color,
        src_alpha,
        inv_src_alpha

        //ToDo
    };

    static void enable(mode src,mode dst);
    static void disable();

private:
    static unsigned int gl_mode(mode &m);
};

struct zwrite
{
    static void enable();
    static void disable();
};

}
