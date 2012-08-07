//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_log
{

enum message_type
{
    normal,
    name,
    file,
    warning,
    error,
    error_internal
};

class log
{
public:
    virtual log &operator << (message_type) { return *this; }
    virtual log &operator << (long int) { return *this; }
    virtual log &operator << (unsigned long int) { return *this; }
    virtual log &operator << (float) { return *this; }
    virtual log &operator << (const char *) { return *this; }

    virtual log &operator << (int a) { return operator<<((long int)a); }
    virtual log &operator << (unsigned int a) { return operator<<((unsigned long int)a); }
    virtual log &operator << (short int a) { return operator<<((long int)a); }
    virtual log &operator << (unsigned short int a) { return operator<<((unsigned long int)a); }

    virtual void scope_inc() {}
    virtual void scope_dec() {}

    virtual void set_tag(const char* tag) {};
};

log &no_log();

void set_log(log *l);
log &get_log(const char *tag=0);

}
