//https://code.google.com/p/nya-engine/

#include "resources.h"
#include "file_resources_provider.h"
//#include "system/system.h"

namespace
{
    nya_resources::resources_provider *res_provider=0;

    nya_log::log *resources_log=0;
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

void set_log(nya_log::log *l)
{
    resources_log=l;
}

nya_log::log &get_log()
{
    static const char *resources_log_tag="resources";
    if(!resources_log)
    {
        return nya_log::get_log(resources_log_tag);
    }

    resources_log->set_tag(resources_log_tag);
    return *resources_log;
}

}
