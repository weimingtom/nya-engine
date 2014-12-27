#pragma once

#include "invalid_object.h"
#include <vector>
#include <map>

namespace nya_memory
{

template<class t> class tag_list
{
public:
    t &add() { return add(0,0); }

    t &add(const char **tags,size_t tags_count)
    {
        const int idx=(int)m_elements.size();
        if(tags)
        {
            for(size_t i=0;i<tags_count;++i)
                m_tags[tags[i]].push_back(idx);
        }

        m_elements.resize(idx+1);
        return m_elements.back();
    }

public:
    int get_count(const char *tag) const
    {
        if(!tag)
            return 0;

        map::const_iterator it=m_tags.find(tag);
        if(it==m_tags.end())
            return 0;

        return (int)it->second.size();
    }

    int get_idx(const char *tag,int idx) const
    {
        if(!tag || idx<0)
            return -1;

        map::const_iterator it=m_tags.find(tag);
        if(it==m_tags.end())
            return -1;

        if(idx>=(int)it->second.size())
            return -1;

        return it->second[idx];
    }

    const t &get(const char *tag,int idx) const { return get(get_idx(tag,idx)); }
    t &get(const char *tag,int idx) { return get(get_idx(tag,idx)); }

public:
    int get_count() const { return (int)m_elements.size(); }

    const t &get(int idx) const
    {
        if(idx<0 || idx>=(int)m_elements.size())
            get_invalid_object<t>();

        return m_elements[idx];
    }

    t &get(int idx)
    {
        if(idx<0 || idx>=(int)m_elements.size())
            get_invalid_object<t>();

        return m_elements[idx];
    }
/*
    void remove(int idx)
    {
        if(idx<0 || idx>=(int)m_elements.size())
            return;

        //ToDo: remove from tags

        m_elements.erase(idx);
    }
*/
    void clear() { m_elements.clear(),m_tags.clear(); }

private:
    std::vector<t> m_elements;
    typedef std::map<std::string,std::vector<int> > map;
    map m_tags;
};

}
