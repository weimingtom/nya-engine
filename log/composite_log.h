//https://code.google.com/p/nya-engine/

#pragma once

#include "log.h"
#include <vector>

namespace nya_log
{

class composite_log: public log_base
{
public:
    void add_log(log_base *l);

private:
    virtual void output(const char *string);

private:
    std::vector<log_base *> m_logs;
};

}
