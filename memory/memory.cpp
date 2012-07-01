//https://code.google.com/p/nya-engine/

#include "memory.h"

namespace
{
    nya_log::log *memory_log=0;
}

namespace nya_memory
{

void set_log(nya_log::log *l)
{
    memory_log=l;
}

nya_log::log &get_log()
{
    static const char *memory_log_tag="memory";
    if(!memory_log)
    {
        return nya_log::get_log(memory_log_tag);
    }

    memory_log->set_tag(memory_log_tag);
    return *memory_log;
}

}

