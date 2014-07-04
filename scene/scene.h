//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"
#include "log/warning.h"

namespace nya_scene
{

void set_log(nya_log::log_base *l);
nya_log::log_base &log();

}
