//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_memory
{

template<typename t>
class shared_ptr
{
    template<typename tt,typename tf> friend shared_ptr<tt> shared_ptr_cast(shared_ptr<tf>& f);

public:
    bool is_valid() const { return m_ref!=0; }

    const t *operator -> () const { return m_ref; };
    t *operator -> () { return m_ref; };

    void free()
    {
        if(!m_ref)
            return;

        if(--(*m_ref_count)<=0)
        {
            delete m_ref;
            delete m_ref_count;
        }

        m_ref=0;
    }

    shared_ptr(): m_ref(0) {}

    explicit shared_ptr(const t &obj)
    {
        m_ref=new t(obj);
        m_ref_count=new int(1);
    }

    shared_ptr(const shared_ptr &p)
    {
        m_ref=p.m_ref;
        m_ref_count=p.m_ref_count;
        if(m_ref)
            ++(*m_ref_count);
    }

    shared_ptr &operator=(const shared_ptr &p)
    {
        if(this==&p)
            return *this;

        free();
        m_ref=p.m_ref;
        if(m_ref)
        {
            m_ref_count=p.m_ref_count;
            ++(*m_ref_count);
        }

        return *this;
    }

    ~shared_ptr() { free(); }

protected:
    t *m_ref;
    int *m_ref_count;
};

template<typename to,typename from> shared_ptr<to> shared_ptr_cast(shared_ptr<from>& f)
{
    shared_ptr<to> t;
    t.m_ref=static_cast<to*>(f.m_ref);
    if(f.m_ref)
    {
        t.m_ref_count=f.m_ref_count;
        ++(*t.m_ref_count);
    }

    return t;
}

}
