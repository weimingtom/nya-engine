//https://code.google.com/p/nya-engine/

#include "system.h"
#include "render/platform_specific_gl.h"

#ifdef __APPLE__
    #include <mach-o/dyld.h>
    #include "TargetConditionals.h"
    #include <string>
#elif defined _WIN32
    #include <windows.h>
    #include <string.h>
#else
    #include <unistd.h>
#endif

namespace
{
    nya_log::log_base *system_log=0;
}

namespace nya_system
{

void set_log(nya_log::log_base *l)
{
    system_log=l;
}

nya_log::log_base &log()
{
    static const char *system_log_tag="system";
    if(!system_log)
    {
        return nya_log::log(system_log_tag);
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

    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        size_t p2=path_str.find("/",p);
    #else
        size_t p2=path_str.rfind("/",p);
    #endif

        if(p2!=std::string::npos)
            path[p2+1]='\0';
        else
            path[0]='\0';
#elif defined _WIN32
	#ifdef WINDOWS_RT
        auto current=Windows::ApplicationModel::Package::Current;
        if(!current)
            return 0;

		auto local = current->InstalledLocation;//Windows::Storage::ApplicationData::Current->LocalFolder;
        if(!local)
            return 0;

        const wchar_t *wp=local->Path->Data();
        const char *p=path;
        for(int i=0;i<local->Path->Length();++i) //ToDo
            path[i]=(char)wp[i];

        for(int i=0;i<max_path;++i)
        {
            if(path[i]=='\\')
                path[i]='/';
        }

        path[local->Path->Length()]='/';
        path[local->Path->Length()+1]=0;

        //char *last_slash = strrchr(path,'\\');
        //if(last_slash)
        //    *(last_slash+1) = 0;
	#else
        GetModuleFileNameA(0,path,max_path);
        for(int i=0;i<max_path;++i)
        {
            if(path[i]=='\\')
                path[i]='/';
        }

        char *last_slash = strrchr(path,'/');
        if(last_slash)
            *(last_slash+1) = 0;
	#endif
#else
        readlink("/proc/self/exe",path,max_path);

        int last_slash=0;
        for(unsigned int i=0;i<max_path-1 && path[i];++i)
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
  #ifdef WINDOWS_RT
    unsigned long get_time()
    {
        static LARGE_INTEGER freq;
        static bool initialised=false;
        if(!initialised)
        {
            QueryPerformanceFrequency(&freq);
            initialised=true;
        }

        LARGE_INTEGER time;
        QueryPerformanceCounter(&time);

        return unsigned long(time.QuadPart*1000/freq.QuadPart);
    }
  #else
    #include "time.h"

    #pragma comment ( lib, "WINMM.LIB"  )

    unsigned long get_time()
    {
        return timeGetTime();
    }
  #endif
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

