//https://code.google.com/p/nya-engine/

#ifndef shared_resources_h
#define shared_resources_h

#include "memory/pool.h"
#include <map>
#include <string>

namespace nya_resources
{

template<typename t_res,int block_count> class shared_resources
{

typedef shared_resources<t_res,block_count> t_creator;

struct res_holder;

public:
    class shared_resource_ref
    {
        template<typename,int> friend class shared_resources;

    public:
        bool is_valid() const { return m_res!=0; }

        t_res *get() { return m_res; }
        const t_res *const_get() const { return m_res; }

        t_res *operator -> () { return m_res; }
        const t_res *operator -> () const { return m_res; };

        void free()
        {
            if(m_creator)
                m_creator->free(*this);

            m_res=0;
            m_res_holder=0;
            m_creator=0;
        }

    public:
        shared_resource_ref(): m_res(0), m_res_holder(0), m_creator(0) {}

    private:
        shared_resource_ref(t_res*res,res_holder*holder,t_creator *creator):
                                    m_res(res),m_res_holder(holder),m_creator(creator) {}

    private:
        t_res *m_res;
        res_holder *m_res_holder;
        t_creator *m_creator;
    };

public:
    shared_resource_ref access(const char*name)
    {
        std::pair<resources_map_iterator,bool> ir = m_res_map.insert(std::make_pair(std::string(name),(res_holder*)0));
        if(ir.second) 
        {
            res_holder *holder=m_res_pool.allocate();
            if(!holder)
                return shared_resource_ref();

            ir.first->second = holder;
            if(!fill_resource(name,holder->res))
            {
                m_res_map.erase(ir.first);
                return shared_resource_ref();
            }
            holder->ref_count=1;
            holder->map_it=ir.first;
            
            return shared_resource_ref(&(holder->res),holder,this);
        }
        else
        {
            res_holder *holder=ir.first->second;
            if (holder)
            {
                ++holder->ref_count;
                return shared_resource_ref(&(holder->res),holder,this);
            }
        }

        return shared_resource_ref();
    }

    void free(shared_resource_ref&ref)
    {
        if(!ref.m_res_holder)
            return;

        --ref.m_res_holder->ref_count;

        if(ref.m_res_holder->ref_count>0)
            return;

        if(!m_should_unload_unused)
            return;

        if(ref.m_res)
            release_resource(*ref.m_res);

        m_res_map.erase(ref.m_res_holder->map_it);
        m_res_pool.free(ref.m_res_holder);
    }

    void should_unload_unused(bool unload)
    {
        if(unload && unload!=m_should_unload_unused)
            free_unused();
        
        m_should_unload_unused=unload;
    }

    void free_unused()
    {
        resources_map_iterator it=m_res_map.begin();

        while(it!=m_res_map.end())
        {
            if(it->second)
            {
                if(it->second->ref_count>0)
                {
                    ++it;
                    continue;
                }

                release_resource(it->second->res);
                m_res_pool.free(it->second);
            }

            resources_map_iterator er = it;
            ++it;

            m_res_map.erase(er);
        }
    }

    void free_all()
    {
        resources_map_iterator it;
        for(it=m_res_map.begin();it!=m_res_map.end();++it)
        {
            if(it->second)
                release_resource(it->second->res);
        }
        
        m_res_map.clear();
        m_res_pool.clear();
    }

    ~shared_resources() 
    {
        unsigned int unreleased_count=0;
        resources_map_iterator it;
        for(it=m_res_map.begin();it!=m_res_map.end();++it)
        {
            if(it->second)
                ++unreleased_count;
        }

        m_res_map.clear();
        m_res_pool.clear();

        //if(unreleased_count>0)
        //log unreleased_count
    }

private:
    virtual bool fill_resource(const char *name,t_res &res) { return false; }
    virtual bool release_resource(t_res &res) { return false; }
    //virtual t_res *get_resource(const res_holder *holder) { return &holder->res; }

public:
    shared_resources(): m_should_unload_unused(true) {}

    //non copiable
private:
    shared_resources(const shared_resources &);
    void operator = (const shared_resources &);

private:
    
    typedef std::map<std::string,res_holder*> resources_map;
    typedef typename resources_map::iterator resources_map_iterator;
    
private:
    struct res_holder
    {
        t_res res;
        unsigned int ref_count;
        resources_map_iterator map_it;
        
        res_holder(): ref_count(0) {}
    };
    
    resources_map m_res_map;
    nya_memory::pool<res_holder,block_count> m_res_pool;

private:
    bool m_should_unload_unused;    
};

}

#endif
