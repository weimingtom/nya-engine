//https://code.google.com/p/nya-engine/

#include "log.h"
#include "stdout_log.h"

namespace
{
    nya_log::log *default_log=0;
}

namespace nya_log
{

log &no_log()
{
	static log l;
	return l;
}

void set_log(log *l)
{
    default_log=l;
}

log &get_log(const char *tag)
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
