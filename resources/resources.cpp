//https://code.google.com/p/nya-engine/

#include "resources.h"
#include "file_resources_provider.h"

namespace
{

resources::file_resources_provider file_res_provider;
resources::resources_provider *res_provider = &file_res_provider;

}

namespace resources
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

}
