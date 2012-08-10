//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"

namespace nya_system
{

void set_log(nya_log::log *l);
nya_log::log &get_log();

const char *get_app_path();

}
