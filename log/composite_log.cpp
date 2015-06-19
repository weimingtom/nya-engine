//https://code.google.com/p/nya-engine/

#include "composite_log.h"
#include <stdio.h>

namespace nya_log
{

void composite_log::add_log(nya_log::log_base *l) { if(l) m_logs.push_back(l); }

void composite_log::output(const char *string)
{
    for(size_t i=0;i<m_logs.size();++i)
        *m_logs[i]<<string;
}

}
