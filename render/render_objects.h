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
        m_objects[idx].data=t();
        m_free.push_back(idx);
    }

    int add()
    {
        if(!m_free.empty())
        {
            const int idx=m_free.back();
            m_free.pop_back();
            m_objects[idx].free=false;

            return idx;
        }

        const int idx=(int)m_objects.size();
        m_objects.resize(m_objects.size()+1);
        m_objects[idx].free=false;

        return idx;
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
