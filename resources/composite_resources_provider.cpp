//https://code.google.com/p/nya-engine/

#include "composite_resources_provider.h"
#include "memory/pool.h"
#include <set>
#include <algorithm>

namespace nya_resources
{

inline std::string fix_name(const std::string &name)
{
    std::string name_str(name);
    for(size_t i=0;i<name_str.size();++i)
    {
        if(name_str[i]=='\\')
            name_str[i]='/';
    }

    std::string out;
    char prev=0;
    for(size_t i=0;i<name_str.size();++i)
    {
        if(prev=='/' && name_str[i]=='/')
            continue;

        prev=name_str[i];
        out.push_back(prev);
    }
    
    return out;
}

void composite_resources_provider::add_provider(resources_provider *provider,const char *folder)
{
    if(!provider)
    {
        log()<<"unable to add provider: invalid provider\n";
        return;
    }

    for(size_t i=0;i<m_providers.size();++i)
    {
        if(m_providers[i].first!=provider)
            continue;

        std::swap(m_providers[i],m_providers.back());
        rebuild_cache();
        return;
    }

    m_resource_names.clear();
    m_providers.push_back(std::make_pair(provider,folder?fix_name(std::string(folder)+"/"):""));
    if(m_cache_entries)
        cache_provider((int)m_providers.size()-1);
}

void composite_resources_provider::cache_provider(int idx)
{
    if(idx<0 || idx>=(int)m_providers.size())
        return;

    resources_provider *provider=m_providers[idx].first;
    for(int i=0;i<provider->get_resources_count();++i)
    {
        const char *name=provider->get_resource_name(i);
        if(!name)
            continue;

        std::string name_str=fix_name(m_providers[idx].second+name);

        if(m_ignore_case)
            std::transform(name_str.begin(),name_str.end(),name_str.begin(),::tolower);

        entry &e=m_cached_entries[name_str];
        e.original_name.assign(name);
        e.prov_idx=idx;
    }
}

resource_data *composite_resources_provider::access(const char *resource_name)
{
    if(!resource_name)
    {
        log()<<"unable to access composite entry: invalid name\n";
        return 0;
    }

    if(!m_cache_entries)
    {
        for(size_t i=0;i<m_providers.size();++i)
        {
            const char *name=resource_name;
            if(!m_providers[i].second.empty())
            {
                if(strncmp(resource_name,m_providers[i].second.c_str(),m_providers[i].second.size())!=0)
                    continue;

                name+=m_providers[i].second.size();
            }

            if(m_providers[i].first->has(name))
                return m_providers[i].first->access(name);
        }
        return 0;
    }

    entries_map::iterator it;

    if(m_ignore_case)
    {
        std::string res_str=fix_name(resource_name);
        std::transform(res_str.begin(),res_str.end(),res_str.begin(),::tolower);

        it=m_cached_entries.find(res_str);
    }
    else
        it=m_cached_entries.find(fix_name(resource_name));

    if(it==m_cached_entries.end())
    {
        log()<<"unable to access composite entry "<<resource_name
                <<": not found\n";
        return 0;
    }

    return m_providers[it->second.prov_idx].first->access(it->second.original_name.c_str());
}

bool composite_resources_provider::has(const char *resource_name)
{
    if(!resource_name)
        return false;

    if(!m_cache_entries)
    {
        for(size_t i=0;i<m_providers.size();++i)
        {
            const char *name=resource_name;
            if(!m_providers[i].second.empty())
            {
                if(strncmp(resource_name,m_providers[i].second.c_str(),m_providers[i].second.size())!=0)
                    continue;

                name+=m_providers[i].second.size();
            }

            if(m_providers[i].first->has(name))
                return true;
        }

        return false;
    }

    if(m_ignore_case)
    {
        std::string res_str=fix_name(resource_name);
        std::transform(res_str.begin(),res_str.end(),res_str.begin(),::tolower);

        return m_cached_entries.find(res_str)!=m_cached_entries.end();
    }

    return m_cached_entries.find(fix_name(resource_name))!=m_cached_entries.end();
}

void composite_resources_provider::enable_cache()
{
    if(m_cache_entries)
        return;

    m_cache_entries=true;
    rebuild_cache();
}

void composite_resources_provider::rebuild_cache()
{
    m_resource_names.clear();

    if(!m_cache_entries)
        return;

    m_cached_entries.clear();
    for(int i=0;i<(int)m_providers.size();++i)
        cache_provider(i);
}

int composite_resources_provider::get_resources_count()
{
    if(m_resource_names.empty())
    {
        if(m_cache_entries)
        {
            for(entries_map::const_iterator it=m_cached_entries.begin();
                it!=m_cached_entries.end();++it)
                m_resource_names.push_back(it->first);
        }
        else
        {
            std::set<std::string> already_has;
            for(size_t i=0;i<m_providers.size();++i)
            {
                for(int j=0;j<m_providers[i].first->get_resources_count();++j)
                {
                    const char *name=m_providers[i].first->get_resource_name(j);
                    if(!name)
                        continue;

                    std::string name_str=fix_name(m_providers[i].second+name);

                    if(already_has.find(name_str)!=already_has.end())
                        continue;

                    m_resource_names.push_back(name_str);
                    already_has.insert(name_str);
                }
            }
        }
    }

    return (int)m_resource_names.size();
}

const char *composite_resources_provider::get_resource_name(int idx)
{
    if(idx<0 || idx>=get_resources_count())
        return 0;

    return m_resource_names[idx].c_str();
}

void composite_resources_provider::set_ignore_case(bool ignore)
{
    if(ignore==m_ignore_case)
        return;

    m_ignore_case=ignore;
    m_resource_names.clear();

    m_cached_entries.clear();
    if(m_cache_entries)
    {
        for(int i=0;i<(int)m_providers.size();++i)
            cache_provider(i);
    }

    if(ignore)
        enable_cache();
}

}
