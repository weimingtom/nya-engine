//https://code.google.com/p/nya-engine/

#pragma once

#include "resources.h"
#include <string>
#include <vector>

namespace nya_resources
{

class file_resources_provider: public resources_provider
{
public:
    resource_data *access(const char *resource_name);
    bool has(const char *resource_name);

public:
    bool set_folder(const char*,bool recursive=true,bool ignore_nonexistent=false);

public:
    int get_resources_count();
    const char *get_resource_name(int idx);

public:
    file_resources_provider(): m_recursive(true) {}

private:
    void enumerate_folder(const char*folder_name);

private:
    std::string m_path;
    bool m_recursive;
    std::vector<std::string> m_resource_names;
};

}
