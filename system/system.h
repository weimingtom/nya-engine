//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"

namespace nya_system
{

void set_log(nya_log::log_base *l);
nya_log::log_base &log();

const char *get_app_path();
unsigned long get_time();

}
