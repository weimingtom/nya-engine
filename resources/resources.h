//https://code.google.com/p/nya-engine/

#ifndef resources_h
#define resources_h

#include <cstddef>

namespace resources
{

class resource_data
{
public:
	virtual size_t get_size() { return 0; };

	virtual bool read_all(void*data) { return false; };
	virtual bool read_chunk(void *data,size_t size,size_t offset=0) { return false; };
};

class resources_provider
{
public:
	virtual resource_data *access(const char *resource_name) { return 0; };
	virtual void close(resource_data *res) {};
};

void set_resources_provider(resources_provider *provider);
resources_provider &get_resources_provider();

}
#endif
