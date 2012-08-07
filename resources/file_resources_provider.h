//https://code.google.com/p/nya-engine/

#pragma once

#include "resources.h"

namespace nya_resources
{
class file_resource_info;

class file_resources_provider: public resources_provider
{
public:
    resource_data *access(const char *resource_name);

public:
    bool set_folder(const char*);

public:
    resource_info *first_res_info();

public:
    file_resources_provider(): m_entries(0) {}
    ~file_resources_provider() { clear_entries(); }

private:
    void clear_entries();

private:
    file_resource_info *m_entries;
};

}
