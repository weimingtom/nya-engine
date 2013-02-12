//https://code.google.com/p/nya-engine/

#pragma once

#include "texture.h"

namespace nya_scene
{

class texture_proxy
{
public:
    bool is_valid() const { return m_ref!=0; }

    const texture *operator -> () const
    {
        if(!m_ref)
            return 0;
        
        return &m_ref->tex;
    };

    texture *operator -> ()
    {
        if(!m_ref)
            return 0;

        return &m_ref->tex;
    };

    void set(const texture &tex)
    {
        if(!m_ref)
            return;

        m_ref->tex=tex;
    }

    void free()
    {
        if(!m_ref)
            return;

        --m_ref->count;

        if(m_ref->count<=0)
            delete m_ref;

        m_ref=0;
    }

    texture_proxy(): m_ref(0) {}

    explicit texture_proxy(const texture &tex)
    {
        m_ref=new ref(tex);
    }

    texture_proxy(const texture_proxy &proxy)
    {
        m_ref=proxy.m_ref;
        if(m_ref)
            ++m_ref->count;
    }

    texture_proxy &operator=(const texture_proxy &proxy) 
    {
		if(this==&proxy)
			return *this;

		free();

        m_ref=proxy.m_ref;
        if(m_ref)
            ++m_ref->count;

        return *this;
    }

    ~texture_proxy() { free(); }

private:
    struct ref
    {
        int count;
        texture tex;

		ref(const texture &tex):count(1),tex(tex){}
    };

    ref *m_ref;
};

}
