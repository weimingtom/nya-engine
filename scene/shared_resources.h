//https://code.google.com/p/nya-engine/

#pragma once

#include "resources/shared_resources.h"
#include "memory/tmp_buffer.h"

namespace nya_scene
{

template<typename t>
class scene_shared
{
public:
    virtual bool load(const char *name)
    {
        unload();

        if(!name)
            return false;

        if(get_resources_prefix().empty())
            m_shared=get_shared_resources().access(name);
        else
            m_shared=get_shared_resources().access((get_resources_prefix()+name).c_str());

        return m_shared.is_valid();
    }

    virtual void unload()
    {
        if(m_shared.is_valid())
            m_shared.free();
    }

    const char get_name() const { return m_shared.get_name(); }

public:
    static void set_resources_prefix(const char *prefix) { get_resources_prefix().assign(prefix?prefix:""); }

public:
    typedef bool (*load_function)(t &sh,size_t data_size,const void*data);

    static void register_load_function(load_function function)
    {
        if(!function)
            return;

        for(size_t i=0;i<get_load_functions().size();++i)
            if(get_load_functions()[i]==function)
                return;

        get_load_functions().push_back(function);
    }

protected:
    typedef nya_resources::shared_resources<t,8> shared_resources;
    typedef typename shared_resources::shared_resource_ref shared_resource_ref;

    class shared_resources_manager: public shared_resources
    {
        bool fill_resource(const char *name,t &res)
        {
            if(!name)
            {
                nya_resources::get_log()<<"unable to load scene resource: invalid name\n";
                return false;
            }

            nya_resources::resource_data *file_data=nya_resources::get_resources_provider().access(name);
            if(!file_data)
            {
                nya_resources::get_log()<<"unable to load scene resource: unable to access resource\n";
                return false;
            }

            const size_t data_size=file_data->get_size();
            nya_memory::tmp_buffer_scoped res_data(data_size);
            file_data->read_all(res_data.get_data());
            file_data->release();

            for(size_t i=0;i<m_loader.get_load_functions().size();++i)
            {
                if(m_loader.get_load_functions()[i](res,data_size,res_data.get_data()))
                    return true;
            }

            nya_resources::get_log()<<"unable to load scene resource: unknown format\n";

            return false;
        }

        bool release_resource(t &res)
        {
            return res.release();
        }

    public:
        shared_resources_manager(scene_shared &loader):m_loader(loader){}

    private:
        scene_shared &m_loader;
    };

    shared_resources_manager &get_shared_resources()
    {
        static shared_resources_manager manager(*this);
        return manager;
    }

private:
    typedef std::vector<load_function> load_functions;
    static load_functions &get_load_functions()
    {
        static load_functions functions;
        return functions;
    }

private:
    static std::string &get_resources_prefix()
    {
        static std::string prefix;
        return prefix;
    }

protected:
    shared_resource_ref m_shared;
};

}
