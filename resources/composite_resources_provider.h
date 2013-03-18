//https://code.google.com/p/nya-engine/

#pragma once

#include "resources.h"
#include <map>
#include <string>
#include <vector>

namespace nya_resources
{

class composite_entry_info;

class composite_resources_provider: public resources_provider
{
public:
    void add_provider(resources_provider *provider);
    void enable_cache();
    void set_ignore_case(bool ignore); //only for cached

public:
    resource_data *access(const char *resource_name);
    bool has(const char *resource_name);

public:
    resource_info *first_res_info(); //enables cache

private:
    void cache_provider(resources_provider *provider);

public:
    composite_resources_provider(): m_entries(0),m_last_entry(0),
                                 m_ignore_case(false),m_cache_entries(false) {}
    ~composite_resources_provider();

private:
    std::vector<resources_provider*> m_providers;
    std::map<std::string,resource_info*> m_resources_info;
    resource_info *m_entries;
    composite_entry_info *m_last_entry;
    bool m_ignore_case;
    bool m_cache_entries;
};

}
