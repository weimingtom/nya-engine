#pragma once

#include "invalid_object.h"
#include <vector>
#include <map>

namespace nya_memory
{

template<class t,size_t count> class lru
{
public:
    t &access(const char *name)
    {
        if(!name)
            return get_invalid_object<t>();

        //ToDo: actual lru, lol

        static t e;
        on_access(name,e);
        return e;
    }

    void free(const char *name)
    {
        if(!name)
            return;

        map::iterator it=m_map.find(name);
        if(it==m_map.end())
            return;

        on_free(name,m_elements[it->second].second);
        m_elements.erase(m_elements.begin()+it->second);
        m_map.erase(it);
    }

    void clear()
    {
        for(size_t i=0;i<m_elements.size();++i)
            on_free(m_elements[i].first.c_str(),m_elements[i].second);
        m_elements.clear();
        m_map.clear();
    }

protected:
    virtual void on_access(const char *name,t& value) {}
    virtual void on_free(const char *name,t& value) {}

private:
    typedef std::pair<std::string,t> entry;
    std::vector<entry> m_elements;
    typedef std::map<std::string,size_t> map;
    map m_map;
};

}
