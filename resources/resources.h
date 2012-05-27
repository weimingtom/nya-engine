//https://code.google.com/p/nya-engine/

#ifndef resources_h
#define resources_h

#include "log/log.h"
#include <cstddef>

namespace nya_resources
{

class resource_data
{
public:
	virtual size_t get_size() const { return 0; };

public:
	virtual bool read_all(void*data) const { return false; };
	virtual bool read_chunk(void *data,size_t size,size_t offset=0) const { return false; };

public:
	virtual void release() {}
};

class resource_info
{
public:
    virtual resource_data *access() { return 0; }
    virtual const char *get_name() const { return ""; };
    virtual bool check_extension(const char *ext) const { return false; }
    virtual resource_info *get_next() const { return 0; };
};

class resources_provider
{
public:
	virtual resource_data *access(const char *resource_name) { return 0; };
	//virtual void free_all{};

public:
    virtual resource_info *first_res_info() { return 0; }
};

void set_resources_provider(resources_provider *provider);
resources_provider &get_resources_provider();

void set_log(nya_log::log *l);
nya_log::log &get_log();

}
#endif

