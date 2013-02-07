//https://code.google.com/p/nya-engine/

#include "composite_resources_provider.h"
#include "memory/pool.h"
#include <algorithm>

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

        if(m_ignore_case)
            return m_lowcase_name.c_str();

        return m_info->get_name();
    }

    bool check_extension(const char *ext) const
    {
        if(!m_info)
        {
            get_log()<<"unable to check entry extension: invalid info\n";
            return 0;
        }

        if(m_ignore_case)
        {
            std::string ext_str(ext);

            std::transform(ext_str.begin(),ext_str.end(),ext_str.begin(),::tolower);

            return m_info->check_extension(ext_str.c_str());
        }

        return m_info->check_extension(ext);
    }

    resource_info *get_next() const { return m_next; };

public:
    composite_entry_info(): m_info(0), m_next(0), m_ignore_case(false) {}

private:
    resource_info *m_info;
    composite_entry_info *m_next;

public:
    std::string m_lowcase_name;
    bool m_ignore_case;
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

    m_providers.push_back(provider);

    if(m_cache_entries)
        cache_provider(provider);
}

void composite_resources_provider::cache_provider(resources_provider *provider)
{
    if(!provider)
        return;

    resource_info *entry=provider->first_res_info();
    while(entry)
    {
        const char *name=entry->get_name();
        if(!name)
        {
            entry=entry->get_next();
            continue;
        }

        std::string name_str(name);

        if(m_ignore_case)
            std::transform(name_str.begin(),name_str.end(),name_str.begin(),::tolower);

        std::pair<res_info_iterator,bool> ir=m_resources_info.insert(std::make_pair(name_str,entry));
        if(!ir.second)
        {
            //get_log()<<"unable to add composite provider entry "<<entry->get_name()
            //        <<": already exist\n";
        }
        else
        {
            composite_entry_info *last_entry=entries.allocate();

            last_entry->set_info(entry);
            if(m_ignore_case)
            {
                last_entry->m_ignore_case=true;
                last_entry->m_lowcase_name=name_str;
            }

            if(m_last_entry)
                m_last_entry->set_next(last_entry);
            else
                m_entries=last_entry;

            m_last_entry=last_entry;
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

    if(!m_cache_entries)
    {
        for(size_t i=0;i<m_providers.size();++i)
        {
            if(m_providers[i]->has(resource_name))
                return m_providers[i]->access(resource_name);
        }

        return 0;
    }

    res_info_iterator it;

    if(m_ignore_case)
    {
        std::string res_str(resource_name);
        std::transform(res_str.begin(),res_str.end(),res_str.begin(),::tolower);

        it=m_resources_info.find(res_str.c_str());
    }
    else
        it=m_resources_info.find(resource_name);

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

bool composite_resources_provider::has(const char *resource_name)
{
    if(!resource_name)
        return false;

    if(!m_cache_entries)
    {
        for(size_t i=0;i<m_providers.size();++i)
        {
            if(m_providers[i]->has(resource_name))
                return true;
        }

        return false;
    }

    res_info_iterator it;

    if(m_ignore_case)
    {
        std::string res_str(resource_name);
        std::transform(res_str.begin(),res_str.end(),res_str.begin(),::tolower);

        it=m_resources_info.find(res_str.c_str());
    }
    else
        it=m_resources_info.find(resource_name);

    return it!=m_resources_info.end();
}

void composite_resources_provider::enable_cache()
{
    if(m_cache_entries)
        return;

    for(size_t i=0;i<m_providers.size();++i)
        cache_provider(m_providers[i]);

    m_cache_entries=true;
}

resource_info *composite_resources_provider::first_res_info()
{
    enable_cache();

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

void composite_resources_provider::set_ignore_case(bool ignore)
{
    if(ignore && !m_ignore_case)
    {
        //todo: convert all to lowcase
        //and set all entries m_ignore_case to true
    }
    else if(!ignore && m_ignore_case)
    {
        //todo: restore original case
        //and set all entries m_ignore_case to false
    }

    m_ignore_case=ignore;
}

}
