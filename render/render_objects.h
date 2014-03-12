//https://code.google.com/p/nya-engine/

#pragma once

#include <vector>
#include <list>

namespace nya_render
{

template<typename t>
class render_objects
{
public:
    t &get(int idx) { return m_objects[idx].data; }

    void remove(int idx)
    {
        m_objects[idx].free=true;
        m_objects[idx].data.release();
        m_free.push_back(idx);
    }

    int add()
    {
        if(!m_free.empty())
        {
            const int idx=m_free.back();
            m_free.pop_back();
            m_objects[idx].free=false;
            //m_objects[idx].data=t(); //not counting on release

            return idx;
        }

        const int idx=(int)m_objects.size();
        m_objects.resize(m_objects.size()+1);
        m_objects[idx].free=false;

        return idx;
    }

    template<typename ta>
    void apply_to_all(ta &applier)
    {
        for(int i=0;i<(int)m_objects.size();++i)
        {
            if(m_objects[i].free)
                continue;

            applier.apply(m_objects[i].data);
        }
    }

    void release_all()
    {
        for(int i=0;i<(int)m_objects.size();++i)
        {
            if(m_objects[i].free)
                continue;

            m_objects[i].data.release();
        }
    }

    void invalidate_all()
    {
        for(int i=0;i<(int)m_objects.size();++i)
        {
            if(m_objects[i].free)
                continue;

            m_objects[i].data=t();
        }
    }

private:
    struct object
    {
        bool free;
        t data;
    };

    std::vector<object> m_objects;
    std::list<int> m_free;
};

}
