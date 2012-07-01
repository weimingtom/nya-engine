//https://code.google.com/p/nya-engine/

#include "composite_resources_provider.h"
#include "memory/pool.h"

namespace nya_resources
{

class composite_entry_info: public resource_info
{
public:
    void set_info(resource_info *info) { m_info=info; }
    void set_next(composite_entry_info *next) { m_next=next; }

public:
    resource_data *access()
    {
        if(!m_info)
        {
            get_log()<<"unable to acess entry: invalid info\n";
            return 0;
        }

        return m_info->access();
    }

    const char *get_name() const
    {
        if(!m_info)
        {
            get_log()<<"unable to get entry name: invalid info\n";
            return 0;
        }

        return m_info->get_name();
    }
    
    bool check_extension(const char *ext) const
    {
        if(!m_info)
        {
            get_log()<<"unable to check entry extension: invalid info\n";
            return 0;
        }
        
        return m_info->check_extension(ext);
    }

    resource_info *get_next() const { return m_next; };

public:
    composite_entry_info(): m_info(0), m_next(0) {}

private:
    resource_info *m_info;
    composite_entry_info *m_next;
};

}

namespace
{
    nya_memory::pool<nya_resources::composite_entry_info,32> entries;
}

namespace nya_resources
{

typedef std::map<std::string,resource_info*> res_info_map;
typedef res_info_map::iterator res_info_iterator;

void composite_resources_provider::add_provider(resources_provider *provider)
{
    if(!provider)
    {
        get_log()<<"unable to add provider: invalid provider\n";
        return;
    }

    resource_info *entry=provider->first_res_info();
    composite_entry_info *prev_entry=0;
    while(entry)
    {
        composite_entry_info *last_entry=entries.allocate();

        std::pair<res_info_iterator,bool> ir=m_resources_info.insert(std::make_pair(std::string(entry->get_name()),entry));
        if(!ir.second)
        {
            get_log()<<"unable to add composite provider entry "<<entry->get_name()
                    <<": already exist\n";
            entries.free(last_entry);
        }
        else
        {
            last_entry->set_info(entry);

            if(prev_entry)
                prev_entry->set_next(last_entry);
            else
                m_entries=last_entry;

            prev_entry=last_entry;
        }

        entry=entry->get_next();
    }
}

resource_data *composite_resources_provider::access(const char *resource_name)
{
    if(!resource_name)
    {
        get_log()<<"unable to access composite entry: invalid name\n";
        return 0;
    }

    res_info_iterator it=m_resources_info.find(resource_name);
    if(it==m_resources_info.end())
    {
        get_log()<<"unable to access composite entry "<<resource_name
                <<": not found\n";
        return 0;
    }

    resource_info *entry=it->second;
    if(!entry)
    {
        get_log()<<"unable to access composite entry "<<resource_name
                <<": invalid entry\n";
        return 0;
    }

    return entry->access();
}

resource_info *composite_resources_provider::first_res_info()
{
    return m_entries;
}
    
composite_resources_provider::~composite_resources_provider()
{
    resource_info *entry=m_entries;
    while(entry)
    {
        resource_info *next_entry=entry->get_next();
        entries.free((composite_entry_info*)entry);
        entry=next_entry;
    }
}

}
