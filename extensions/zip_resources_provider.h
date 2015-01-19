//https://code.google.com/p/nya-engine/

#pragma once

#include "resources/resources.h"
#include <vector>
#include <string>

namespace nya_resources
{

class zip_resources_provider: public resources_provider
{
public:
    bool open_archive(const char *archive_name);
    bool open_archive(nya_resources::resource_data *data);
    void close_archive() { if(m_res) m_res->release(); m_entries.clear(); }

public:
    resource_data *access(const char *resource_name);
    bool has(const char *resource_name);

public:
    int get_resources_count();
    const char *get_resource_name(int idx);

public:
    zip_resources_provider(): m_res(0) {}
    ~zip_resources_provider() { close_archive(); }

private:
    nya_resources::resource_data *m_res;

    struct zip_entry
    {
        std::string name;
        unsigned int offset;
        unsigned int packed_size;
        unsigned int unpacked_size;
    };

    std::vector<zip_entry> m_entries;
};

}
