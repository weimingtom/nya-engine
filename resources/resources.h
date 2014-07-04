//https://code.google.com/p/nya-engine/

#pragma once

#include "log/log.h"
#include <cstddef>

namespace nya_resources
{

class resource_data
{
public:
    virtual size_t get_size() { return 0; }

public:
    virtual bool read_all(void*data) { return false; }
    virtual bool read_chunk(void *data,size_t size,size_t offset=0) { return false; }

public:
    virtual void release() {}
};

class resource_info
{
public:
    virtual resource_data *access() { return 0; }
    virtual const char *get_name() const { return ""; }
    virtual bool check_extension(const char *ext) const { return false; }
    virtual resource_info *get_next() const { return 0; }
};

class resources_provider
{
public:
    virtual resource_data *access(const char *resource_name) { return 0; }
    virtual bool has(const char *resource_name) { return false; }
    //virtual void free_all{};

public:
    virtual resource_info *first_res_info() { return 0; }
};

void set_resources_provider(resources_provider *provider);
resources_provider &get_resources_provider();

void set_log(nya_log::log_base *l);
nya_log::log_base &log();

}
