//https://code.google.com/p/nya-engine/

#pragma once

#include "memory/shared_ptr.h"

namespace nya_scene
{

template<typename t>
class proxy: public nya_memory::shared_ptr<t>
{
public:
    proxy &create() { return *this=proxy(t()); }
    proxy &create(const t &obj) { return *this=proxy(obj); }

    proxy &set(const t &obj)
    {
        if(!nya_memory::shared_ptr<t>::m_ref)
            return *this;

        *nya_memory::shared_ptr<t>::m_ref=obj;
        return *this;
    }

    proxy(): nya_memory::shared_ptr<t>() {}

    explicit proxy(const t &obj): nya_memory::shared_ptr<t>(obj) {}

    proxy(const proxy &p): nya_memory::shared_ptr<t>(p) {}
};

}
