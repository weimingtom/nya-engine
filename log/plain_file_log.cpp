//https://code.google.com/p/nya-engine/

#include "plain_file_log.h"
#include <stdio.h>

namespace nya_log
{

log & plain_file_log::operator << (long int a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        for(int i=0;i<m_scope;++i)
            fprintf(f,"%s",m_scope_tab.c_str());

        fprintf(f,"%ld",a);
        fclose(f);
    }
    return *this;
}

log & plain_file_log::operator << (unsigned long int a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        for(int i=0;i<m_scope;++i)
            fprintf(f,"%s",m_scope_tab.c_str());

        fprintf(f,"%ld",a);
        fclose(f);
    }
    return *this;
}

log & plain_file_log::operator << (float a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        for(int i=0;i<m_scope;++i)
            fprintf(f,"%s",m_scope_tab.c_str());

        fprintf(f,"%f",a);
        fclose(f);
    }
    return *this;
}

log & plain_file_log::operator << (const char * a)
{
    if(FILE*f = fopen(m_file_name.c_str(),"a+"))
    {
        for(int i=0;i<m_scope;++i)
            fprintf(f,"%s",m_scope_tab.c_str());

        fprintf(f,"%s",a);
        fclose(f);
    }
    return *this;
}

bool plain_file_log::open(const char*file_name)
{
    if(FILE*f = fopen(file_name,"w+"))
    {
        fclose(f);
        m_file_name.assign(file_name);
        return true;
    }

    m_file_name.clear();
    return false;
}

void plain_file_log::close()
{
    m_file_name.clear();
}

}
