//https://code.google.com/p/nya-engine/

#pragma once

#include "output_stream.h"

namespace nya_log
{

class log_base: public ostream_base
{
protected:
    virtual void output(const char *string) {}

public:
    virtual void set_tag(const char* tag) {};

public:
    virtual ~log_base() {}
};

log_base &no_log();

void set_log(log_base *l);
log_base &log(const char *tag=0);

}
