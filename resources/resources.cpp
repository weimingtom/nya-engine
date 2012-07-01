//https://code.google.com/p/nya-engine/

#include "resources.h"
#include "file_resources_provider.h"

namespace
{
    nya_resources::file_resources_provider file_res_provider;
    nya_resources::resources_provider *res_provider = &file_res_provider;

    nya_log::log *resources_log;
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
        return file_res_provider;

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
