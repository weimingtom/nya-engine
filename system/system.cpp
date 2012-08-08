//https://code.google.com/p/nya-engine/

#include "system.h"

namespace
{
    nya_log::log *system_log=0;
}

namespace nya_system
{

void set_log(nya_log::log *l)
{
    system_log=l;
}

nya_log::log &get_log()
{
    static const char *system_log_tag="system";
    if(!system_log)
    {
        return nya_log::get_log(system_log_tag);
    }

    system_log->set_tag(system_log_tag);
    return *system_log;
}

}

