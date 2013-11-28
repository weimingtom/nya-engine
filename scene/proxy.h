//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_scene
{

template<typename t>
class proxy
{
public:
    bool is_valid() const { return m_ref!=0; }

    const t *operator -> () const { return m_ref; };
    t *operator -> () { return m_ref; };

    void set(const t &obj)
    {
        if(!m_ref)
            return;

        *m_ref=obj;
    }

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

    proxy(): m_ref(0) {}

    explicit proxy(const t &obj)
    {
        m_ref=new t(obj);
        m_ref_count=new int(1);
    }

    proxy(const proxy &p)
    {
        m_ref=p.m_ref;
        m_ref_count=p.m_ref_count;
        if(m_ref)
            ++(*m_ref_count);
    }

    proxy &operator=(const proxy &p)
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

    ~proxy() { free(); }

private:
    t *m_ref;
    int *m_ref_count;
};

}
