//https://code.google.com/p/nya-engine/

#ifndef plain_file_log_h
#define plain_file_log_h

#include "log.h"

namespace log
{

class plain_file_log_impl;
    
class plain_file_log: public log
{
public:
    bool open(const char*filename);
    void close();

public:
    //log &operator << (message_type a);
    log &operator << (int a);
    log &operator << (const char *a);

public:    
    plain_file_log(const plain_file_log &);
    plain_file_log &operator = (const plain_file_log &);

    plain_file_log();
    
private:
    plain_file_log_impl *ref;
};
    
}
#endif