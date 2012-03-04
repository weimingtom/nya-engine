//https://code.google.com/p/nya-engine/

#ifndef file_resource_provider_h
#define file_resource_provider_h

#include "resources.h"

namespace resources
{

class file_resources_provider: public resources_provider
{
public:
	resource_data *access(const char *resource_name);
	void close(resource_data *data);
};

}
#endif
