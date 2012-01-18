//https://code.google.com/p/nya-engine/

#ifndef plain_file_log_h
#define plain_file_log_h

#include "log.h"

namespace log
{

class plain_file_log: public log_ref
{
public:
    bool open(const char*filename);
    void close();

public:    
    plain_file_log(const plain_file_log &);
    plain_file_log &operator = (const plain_file_log &);

    plain_file_log() {}
};
    
}
#endif
