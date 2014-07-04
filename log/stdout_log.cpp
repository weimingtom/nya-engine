//https://code.google.com/p/nya-engine/

#include "stdout_log.h"
#include <stdio.h>

namespace nya_log
{

ostream_base & stdout_log::operator << (long int a)
{
    for(int i=0;i<m_scope;++i)
        printf("%s",m_scope_tab.c_str());

    printf("%ld",a);
    return *this;
}

ostream_base & stdout_log::operator << (unsigned long int a)
{
    for(int i=0;i<m_scope;++i)
        printf("%s",m_scope_tab.c_str());

    printf("%ld",a);
    return *this;
}

ostream_base & stdout_log::operator << (float a)
{
    for(int i=0;i<m_scope;++i)
        printf("%s",m_scope_tab.c_str());

    printf("%f",a);
    return *this;
}

ostream_base & stdout_log::operator << (const char * a)
{
    for(int i=0;i<m_scope;++i)
        printf("%s",m_scope_tab.c_str());

    printf("%s", a? a:"NULL");
    return *this;
}


}
