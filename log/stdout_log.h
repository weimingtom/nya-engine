//https://code.google.com/p/nya-engine/

#ifndef stdout_log_h
#define stdout_log_h

#include "log.h"

namespace log
{

class stdout_log: public log
{
public:
    log &operator << (int a);
    log &operator << (float a);
    log &operator << (const char *a);

public:
	void scope_inc() { ++m_scope; }
	void scope_dec() { --m_scope; if(m_scope<0) m_scope=0; }

	stdout_log(): m_scope(0), m_scope_tab("  ") {}
	stdout_log(const char *scope_tab): m_scope(0), m_scope_tab(scope_tab) {}

private:
	int m_scope;
	const char *m_scope_tab;
};
    
}
#endif

