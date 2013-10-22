//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_scene
{

template<typename t>
class proxy
{
public:
    bool is_valid() const { return m_ref!=0; }

    const t *operator -> () const
    {
        if(!m_ref)
            return 0;

        return &m_ref->obj;
    };

    t *operator -> ()
    {
        if(!m_ref)
            return 0;

        return &m_ref->obj;
    };

    void set(const t &obj)
    {
        if(!m_ref)
            return;

        m_ref->obj=obj;
    }

    const t *get() const
    {
        if(!m_ref)
            return 0;
        
        return &m_ref->obj;
    };

    t *get()
    {
        if(!m_ref)
            return 0;
        
        return &m_ref->obj;
    };

    void free()
    {
        if(!m_ref)
            return;

        --m_ref->count;

        if(m_ref->count<=0)
            delete m_ref;

        m_ref=0;
    }

    proxy(): m_ref(0) {}

    explicit proxy(const t &obj)
    {
        m_ref=new ref(obj);
    }

    proxy(const proxy &p)
    {
        m_ref=p.m_ref;
        if(m_ref)
            ++m_ref->count;
    }

    proxy &operator=(const proxy &p)
    {
		if(this==&p)
			return *this;

		free();

        m_ref=p.m_ref;
        if(m_ref)
            ++m_ref->count;

        return *this;
    }

    ~proxy() { free(); }

private:
    struct ref
    {
        int count;
        t obj;

		ref(const t &obj):count(1),obj(obj){}
    };

    ref *m_ref;
};

}
