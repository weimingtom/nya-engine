//https://code.google.com/p/nya-engine/

#include "android_log.h"

#ifdef __ANDROID__
    #include <android/log.h>
#endif

namespace nya_log
{

void android_log::output(const char *str)
{
#ifdef __ANDROID__
    if(!str)
        return;

    m_buf.append(str);
    for(size_t i=m_buf.find("\n");i!=std::string::npos;i=m_buf.find("\n"))
    {
        std::string tmp=m_buf.substr(0,i);
        m_buf=m_buf.substr(i+1);
        __android_log_print(ANDROID_LOG_INFO,m_id.c_str(),"%s",tmp.c_str());
    }
#endif
}

}
