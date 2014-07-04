//https://code.google.com/p/nya-engine/

#pragma once

#include "output_stream.h"

namespace nya_log
{

class log_base: public ostream_base
{
public:
    virtual void scope_inc() {}
    virtual void scope_dec() {}

    virtual void set_tag(const char* tag) {};
    
    ~log_base() {}
};

log_base &no_log();

void set_log(log_base *l);
log_base &log(const char *tag=0);

}
