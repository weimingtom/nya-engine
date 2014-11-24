//https://code.google.com/p/nya-engine/

#pragma once

#include "resources.h"
#include <string>

namespace nya_resources
{
class file_resource_info;

class file_resources_provider: public resources_provider
{
public:
    resource_data *access(const char *resource_name);
    bool has(const char *resource_name);

public:
    bool set_folder(const char*,bool recursive=true,bool ignore_nonexistent=false);

public:
    resource_info *first_res_info();

public:
    file_resources_provider(): m_entries(0), m_recursive(true) {}
    ~file_resources_provider() { clear_entries(); }

private:
    void enumerate_folder(const char*folder_name,file_resource_info **last);
    void clear_entries();

private:
    file_resource_info *m_entries;
    std::string m_path;
    bool m_recursive;
};

}
