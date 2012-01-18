//https://code.google.com/p/nya-engine/

#include "plain_file_log.h"
#include <iostream>
#include <string>

namespace log
{

class plain_file_log_impl: public log
{
public:
    bool is_valid()
    {
        return !log_filename.empty();
    }

    plain_file_log_impl & operator << (int i)
    {
        if(FILE*f = fopen(log_filename.c_str(),"a+"))
        {
            fprintf(f,"%i",i);
            fclose(f);
        }
        return *this;
    }

    plain_file_log_impl & operator << (const char * str)
    {
        if(FILE*f = fopen(log_filename.c_str(),"a+"))
        {
            fprintf(f,"%s",str);
            fclose(f);
        }
        return *this;
    }

    plain_file_log_impl(const char*filename)
    {
        if(FILE*f = fopen(filename,"w+"))
        {
            fclose(f);
            log_filename.assign(filename);
            return;
        }

        log_filename.clear();
    }

    plain_file_log_impl(const plain_file_log_impl & from)
    {
        *this = from;
    } 

    plain_file_log_impl()
    {
    }

private:
    std::string log_filename;
};

bool plain_file_log::open(const char*filename)
{
    if(ref)
        delete ref;

    plain_file_log_impl *impl = new plain_file_log_impl(filename);
    if(!impl->is_valid())
    {
        delete impl;
        return false;
    }

    ref = impl;

    return true;
}

void plain_file_log::close()
{
    delete ref;
}

plain_file_log::plain_file_log(const plain_file_log & from)
{
    *this = from;
}

plain_file_log & plain_file_log::operator = (const plain_file_log & from)
{
    plain_file_log_impl *impl = static_cast<plain_file_log_impl*>(from.ref);

    if (this != &from)
    {
        if(ref)
            delete ref;

        ref = new plain_file_log_impl(*impl);
    }

    return *this;
}

}
