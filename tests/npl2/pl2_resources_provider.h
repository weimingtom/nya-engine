//https://code.google.com/p/nya-engine/

#ifndef pl2_resource_provider_h
#define pl2_resource_provider_h

#include "resources.h"

namespace nya_resources
{
class pl2_entry_info;

class pl2_resources_provider: public resources_provider
{
public:
	bool open_archieve(resource_data *archieve_data);

	bool open_archieve(const char *archieve_name,resources_provider &provider)
    {
        resource_data *archieve_data=provider.access(archieve_name);
        if(!archieve_data)
            return false;

        return open_archieve(archieve_data);
    }

    bool open_archieve(const char *archieve_name)
    {
        return open_archieve(archieve_name,get_resources_provider());
    }

	void close_archieve();

	resource_data *access_attribute();

public:
    resource_info *first_res_info();

public:
	resource_data *access(const char *resource_name);

public:
    pl2_resources_provider(): m_archieve_data(0), m_entries(0), m_attribute(0) {}
    ~pl2_resources_provider() { close_archieve(); }

private:
    resource_data *m_archieve_data;
    pl2_entry_info *m_entries;
    pl2_entry_info *m_attribute;
};

}
#endif
