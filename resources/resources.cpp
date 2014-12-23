//https://code.google.com/p/nya-engine/

#include "resources.h"
#include "file_resources_provider.h"
//#include "system/system.h"
#include <string.h>

#if defined(_WIN32)
    #define strcasecmp _stricmp
#endif

namespace
{
    nya_resources::resources_provider *res_provider=0;
    nya_log::log_base *resources_log=0;
}

namespace nya_resources
{

void set_resources_provider(resources_provider *provider)
{
    res_provider = provider;
}

resources_provider &get_resources_provider()
{
    if(!res_provider)
    {
        static nya_resources::file_resources_provider fprov;
        //fprov.set_folder(nya_system::get_app_path(),true);
        res_provider= &fprov;
    }

    return *res_provider;
}

void set_log(nya_log::log_base *l)
{
    resources_log=l;
}

nya_log::log_base &log()
{
    static const char *resources_log_tag="resources";
    if(!resources_log)
    {
        return nya_log::log(resources_log_tag);
    }

    resources_log->set_tag(resources_log_tag);
    return *resources_log;
}

bool check_extension(const char *name,const char *ext)
{
    if(!name || !ext)
        return false;

    const size_t name_len=strlen(name),ext_len=strlen(ext);
    if(ext_len>name_len)
        return false;

    return strcasecmp(name+name_len-ext_len,ext)==0;
}

}
