//https://code.google.com/p/nya-engine/

#include "log.h"
#include "stdout_log.h"

namespace nya_log
{

namespace { log_base *default_log=0; }

log_base &no_log()
{
    static log_base *l=new log_base();
    return *l;
}

void set_log(log_base *l) { default_log=l; }

log_base &log(const char *tag)
{
    if(!tag)
        tag="general";

    if(!default_log)
    {
        static stdout_log stdlog;
        stdlog.set_tag(tag);
        return stdlog;
    }

    default_log->set_tag(tag);
    return *default_log;
}

}
