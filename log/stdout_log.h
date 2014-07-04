//https://code.google.com/p/nya-engine/

#pragma once

#include "log.h"
#include <string>

namespace nya_log
{

class stdout_log: public log_base
{
public:
    ostream_base &operator << (long int a);
    ostream_base &operator << (unsigned long int a);
    ostream_base &operator << (float a);
    ostream_base &operator << (const char *a);

public:
    void scope_inc() { ++m_scope; }
    void scope_dec() { if(m_scope>0) --m_scope; }

    stdout_log(): m_scope(0), m_scope_tab("  ") {}
    stdout_log(const char *scope_tab): m_scope(0), m_scope_tab(scope_tab) {}

private:
    int m_scope;
    std::string m_scope_tab;
};

}
