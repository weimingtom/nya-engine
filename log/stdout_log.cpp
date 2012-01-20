//https://code.google.com/p/nya-engine/

#include "stdout_log.h"
#include <stdio.h>

namespace log
{

log & stdout_log::operator << (int a)
{
	for(int i=0;i<m_scope;++i)
		printf("%s",m_scope_tab);

    printf("%i",a);
    return *this;
}

log & stdout_log::operator << (float a)
{
	for(int i=0;i<m_scope;++i)
		printf("%s",m_scope_tab);

    printf("%f",a);
    return *this;
}

log & stdout_log::operator << (const char * a)
{
	for(int i=0;i<m_scope;++i)
		printf("%s",m_scope_tab);

    printf("%s",a);
    return *this;
}


}
