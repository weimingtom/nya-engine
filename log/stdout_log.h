//https://code.google.com/p/nya-engine/

#pragma once

#include "log.h"

namespace nya_log
{

class stdout_log: public log_base
{
private:
    virtual void output(const char *str);
};

}
