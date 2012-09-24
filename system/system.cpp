//https://code.google.com/p/nya-engine/

#include "system.h"

#ifdef __APPLE__
    #include <mach-o/dyld.h>
    #include <string>
#elif defined _WIN32
    #include <windows.h>
    #include <string.h>
#else
    #include <unistd.h>
#endif

namespace
{
    nya_log::log *system_log=0;
}

namespace nya_system
{

void set_log(nya_log::log *l)
{
    system_log=l;
}

nya_log::log &get_log()
{
    static const char *system_log_tag="system";
    if(!system_log)
    {
        return nya_log::get_log(system_log_tag);
    }

    system_log->set_tag(system_log_tag);
    return *system_log;
}

const char *get_app_path()
{
    const size_t max_path=4096;
    static char path[max_path]="";
    static bool has_path=false;
    if(!has_path)
    {

#ifdef __APPLE__
        uint32_t path_length=max_path;
        _NSGetExecutablePath(path,&path_length);

        std::string path_str(path);
        size_t p=path_str.rfind(".app");
        size_t p2=path_str.rfind("/",p);
        if(p2!=std::string::npos)
            path[p2+1]='\0';
        else
            path[0]='\0';
#elif defined _WIN32
        GetModuleFileName(0,path,max_path);

        char *last_slash = strrchr(path,'\\');
        if(last_slash)
        *last_slash = 0;
#else
        readlink("/proc/self/exe",path,max_path);

        int last_slash=0;
        for(int i=0;i<max_path-1 && path[i];++i)
        {
            if(path[i]=='/')
                last_slash=i;
        }

        path[last_slash+1]=0;
#endif
        has_path=true;
    }

    return path;
}

#ifdef _WIN32

#include "time.h"

#pragma comment ( lib, "WINMM.LIB"  )

unsigned long get_time()
{
    return timeGetTime();
}

#else

#include <sys/time.h>

unsigned long get_time()
{
    timeval tim;
    gettimeofday(&tim, 0);
    return (tim.tv_sec*1000+(tim.tv_usec/1000));
}

#endif

}

