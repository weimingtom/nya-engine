//https://code.google.com/p/nya-engine/

#ifndef composite_resource_provider_h
#define composite_resource_provider_h

#include "resources.h"
#include <map>
#include <string>

namespace nya_resources
{

class composite_entry_info;

class composite_resources_provider: public resources_provider
{
public:
    void add_provider(resources_provider *provider);

public:
    resource_data *access(const char *resource_name);

public:
    resource_info *first_res_info();

public:
    composite_resources_provider(): m_entries(0), m_last_entry(0) {}
    ~composite_resources_provider();

private:
    std::map<std::string,resource_info*> m_resources_info;
    resource_info *m_entries;
    composite_entry_info *m_last_entry;
};

}
#endif
