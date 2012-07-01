//https://code.google.com/p/nya-engine/

#ifndef html_log_h
#define html_log_h

#include "log.h"
#include <string>

namespace nya_log
{

class html_log: public log
{
public:
    bool open(const char*file_name);
    void close();

public:
    log &operator << (message_type a);
    log &operator << (long int a);
    log &operator << (unsigned long int a);
    log &operator << (float a);
    log &operator << (const char *a);

public:
    void scope_inc();
    void scope_dec();

    html_log(): m_scope(0), m_block(0), m_message_type(normal) {}

private:
    std::string m_file_name;
    int m_scope;
    message_type m_message_type;
    int m_block;
};
}
#endif

