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

}
