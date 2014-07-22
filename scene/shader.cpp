//https://code.google.com/p/nya-engine/

#include "shader.h"
#include "scene.h"
#include "camera.h"
#include "memory/invalid_object.h"
#include "transform.h"
#include "render/render.h"
#include "formats/text_parser.h"
#include <string.h>
#include <cstdlib>

namespace nya_scene
{

namespace
{

struct shader_description
{
    struct predefined
    {
        std::string name;
        shared_shader::transform_type transform;
    };

    predefined predefines[shared_shader::predefines_count];

    typedef std::map<std::string,std::string> strings_map;
    strings_map samplers;
    strings_map uniforms;

    std::string vertex;
    std::string pixel;
};

shared_shader::transform_type transform_from_string(const char *str)
{
    if(!str)
        return shared_shader::none;

    if(strcmp(str,"local_rot")==0)
        return shared_shader::local_rot;

    if(strcmp(str,"local")==0)
        return shared_shader::local;

    return shared_shader::none;
}

}

bool load_nya_shader_internal(shared_shader &res,shader_description &desc,resource_data &data,const char* name,bool include)
{
    nya_formats::text_parser parser;
    parser.load_from_data((const char *)data.get_data(),data.get_size());
    for(int section_idx=0;section_idx<parser.get_sections_count();++section_idx)
    {
        const char *section_type=parser.get_section_type(section_idx);
        if(strcmp(section_type,"@include")==0)
        {
            const char *file=parser.get_section_name(section_idx);
            if(!file)
            {
                log()<<"unable to load shader include in shader "<<name<<": invalid filename\n";
                return false;
            }

            std::string path(name);
            size_t p=path.rfind("/");
            if(p==std::string::npos)
                p=path.rfind("\\");

            if(p==std::string::npos)
                path.clear();
            else
                path.resize(p+1);

            path.append(file);

            nya_resources::resource_data *file_data=nya_resources::get_resources_provider().access(path.c_str());
            if(!file_data)
            {
                log()<<"unable to load shader include resource in shader "<<name<<": unable to access resource "<<path.c_str()<<"\n";
                return false;
            }

            const size_t data_size=file_data->get_size();
            nya_memory::tmp_buffer_ref include_data(data_size);
            file_data->read_all(include_data.get_data());
            file_data->release();

            if(!load_nya_shader_internal(res,desc,include_data,path.c_str(),true))
            {
                log()<<"unable to load shader include in shader "<<name<<": unknown format in "<<path.c_str()<<"\n";
                include_data.free();
                return false;
            }

            include_data.free();
        }
        else if(strcmp(section_type,"@all")==0)
        {
            const char *text=parser.get_section_value(section_idx);
            if(text)
            {
                desc.vertex.append(text);
                desc.pixel.append(text);
            }
        }
        else if(strcmp(section_type,"@sampler")==0)
        {
            const char *name=parser.get_section_name(section_idx,0);
            const char *semantics=parser.get_section_name(section_idx,1);
            if(!name || !semantics)
            {
                log()<<"unable to load shader "<<name<<": invalid sampler syntax\n";
                return false;
            }

            desc.samplers[semantics]=name;
            res.samplers[semantics]=res.samplers_count;
            ++res.samplers_count;
        }
        else if(strcmp(section_type,"@vertex")==0)
        {
            const char *text=parser.get_section_value(section_idx);
            if(text)
                desc.vertex.append(text);
        }
        else if(strcmp(section_type,"@fragment")==0)
        {
            const char *text=parser.get_section_value(section_idx);
            if(text)
                desc.pixel.append(text);
        }
        else if(strcmp(section_type,"@predefined")==0)
        {
            const char *name=parser.get_section_name(section_idx,0);
            const char *semantics=parser.get_section_name(section_idx,1);
            if(!name || !semantics)
            {
                log()<<"unable to load shader "<<name<<": invalid predefined syntax\n";
                return false;
            }

            const char *predefined_semantics[]={"nya camera position","nya camera rotation",
                                                "nya bones pos","nya bones rot","nya viewport",
                                                "nya model pos","nya model rot","nya model scale"};

            char predefined_count_static_assert[sizeof(predefined_semantics)/sizeof(predefined_semantics[0])
                                                ==shared_shader::predefines_count?1:-1];
            predefined_count_static_assert[0]=predefined_count_static_assert[0];
            for(int i=0;i<shared_shader::predefines_count;++i)
            {
                if(strcmp(semantics,predefined_semantics[i])==0)
                {
                    desc.predefines[i].name=name;
                    desc.predefines[i].transform=transform_from_string(parser.get_section_option(section_idx));
                    break;
                }
            }
        }
        else if(strcmp(section_type,"@uniform")==0)
        {
            const char *name=parser.get_section_name(section_idx,0);
            const char *semantics=parser.get_section_name(section_idx,1);
            if(!name || !semantics)
            {
                log()<<"unable to load shader "<<name<<": invalid uniform syntax\n";
                return false;
            }

            desc.uniforms[semantics]=name;
            res.uniforms.resize(res.uniforms.size()+1);
            res.uniforms.back().name=semantics;
            res.uniforms.back().transform=transform_from_string(parser.get_section_option(section_idx));
            res.uniforms.back().default_value=parser.get_section_value_vector(section_idx);
        }
        else
            log()<<"scene shader load warning: unsupported shader tag in "<<name<<"\n";
    }

    if(include)
        return true;

    if(desc.vertex.empty())
    {
        log()<<"scene shader load error: empty vertex shader in "<<name<<"\n";
        return false;
    }

    if(desc.pixel.empty())
    {
        log()<<"scene shader load error: empty pixel shader in "<<name<<"\n";
        return false;
    }

    //log()<<"vertex <"<<res.vertex.c_str()<<">\n";
    //log()<<"pixel <"<<res.pixel.c_str()<<">\n";

    for(shared_shader::samplers_map::iterator it=res.samplers.begin();
        it!=res.samplers.end();++it)
        res.shdr.set_sampler(desc.samplers[it->first].c_str(),it->second);

    if(!res.shdr.add_program(nya_render::shader::vertex,desc.vertex.c_str()))
        return false;

    if(!res.shdr.add_program(nya_render::shader::pixel,desc.pixel.c_str()))
        return false;

    for(int i=0;i<shared_shader::predefines_count;++i)
    {
        const shader_description::predefined &p=desc.predefines[i];
        if(p.name.empty())
            continue;

        res.predefines.resize(res.predefines.size()+1);
        res.predefines.back().transform=p.transform;
        res.predefines.back().type=(shared_shader::predefined_values)i;
        res.predefines.back().location=res.shdr.get_handler(p.name.c_str());
    }

    for(int i=0;i<(int)res.uniforms.size();++i)
        res.uniforms[i].location=res.shdr.get_handler(desc.uniforms[res.uniforms[i].name].c_str());

    return true;
}

bool shader::load_nya_shader(shared_shader &res,resource_data &data,const char* name)
{
    shader_description desc;
    return load_nya_shader_internal(res,desc,data,name,false);
}

void shader_internal::set() const
{
    if(!m_shared.is_valid())
        return;

    m_shared->shdr.bind();

    for(size_t i=0;i<m_shared->predefines.size();++i)
    {
        const shared_shader::predefined &p=m_shared->predefines[i];
        switch(p.type)
        {
            case shared_shader::camera_pos:
            {
                if(p.transform==shared_shader::local)
                {
                    const nya_math::vec3 v=transform::get().inverse_transform(get_camera().get_pos());
                    m_shared->shdr.set_uniform(p.location,v.x,v.y,v.z);
                }
                else if(p.transform==shared_shader::local_rot)
                {
                    const nya_math::vec3 v=transform::get().inverse_rot(get_camera().get_pos());
                    m_shared->shdr.set_uniform(p.location,v.x,v.y,v.z);
                }
                else
                {
                    const nya_math::vec3 v=get_camera().get_pos();
                    m_shared->shdr.set_uniform(p.location,v.x,v.y,v.z);
                }
            }
            break;

            case shared_shader::camera_rot:
            {
                //if(p.transform==shared_shader::none) //ToDo
                {
                    const nya_math::quat v=get_camera().get_rot();
                    m_shared->shdr.set_uniform(p.location,v.v.x,v.v.y,v.v.z,v.w);
                }
            }

            case shared_shader::bones_pos:
            {
                if(m_skeleton && m_shared->last_skeleton_pos!=m_skeleton)
                {
                    m_shared->shdr.set_uniform3_array(p.location,m_skeleton->get_pos_buffer(),m_skeleton->get_bones_count());
                    m_shared->last_skeleton_pos=m_skeleton;
                }
            }
            break;

            case shared_shader::bones_rot:
            {
                if(m_skeleton && m_shared->last_skeleton_rot!=m_skeleton)
                {
                    m_shared->shdr.set_uniform4_array(p.location,m_skeleton->get_rot_buffer(),m_skeleton->get_bones_count());
                    m_shared->last_skeleton_rot=m_skeleton;
                }
            }
            break;

            case shared_shader::viewport:
            {
                nya_render::rect r=nya_render::get_viewport();
                m_shared->shdr.set_uniform(p.location,float(r.x),float(r.y),float(r.width),float(r.height));
            }
            break;

            case shared_shader::model_pos:
            {
                const nya_math::vec3 v=transform::get().get_pos();
                m_shared->shdr.set_uniform(p.location,v.x,v.y,v.z);
            }
            break;

            case shared_shader::model_rot:
            {
                const nya_math::quat v=transform::get().get_rot();
                m_shared->shdr.set_uniform(p.location,v.v.x,v.v.y,v.v.z,v.w);
            }
            break;

            case shared_shader::model_scale:
            {
                const nya_math::vec3 v=transform::get().get_scale();
                m_shared->shdr.set_uniform(p.location,v.x,v.y,v.z);
            }
            break;

            case shared_shader::predefines_count: break;
        }
    }
}

void shader_internal::unset() const
{
    if(!m_shared.is_valid())
        return;

    m_shared->shdr.unbind();
}

int shader_internal::get_texture_slot(const char *semantics) const
{
    if(!semantics || !m_shared.is_valid())
        return -1;

    shared_shader::samplers_map::const_iterator it=m_shared->samplers.find(semantics);
    if(it==m_shared->samplers.end())
        return -1;

    return it->second;
}

const char *shader_internal::get_texture_semantics(int slot) const
{
    for(shared_shader::samplers_map::const_iterator it=m_shared->samplers.begin();
        it!=m_shared->samplers.end();++it)
    {
        if(it->second==slot)
            return it->first.c_str();
    }

    return 0;
}

int shader_internal::get_texture_slots_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return m_shared->samplers_count;
}

const shared_shader::uniform &shader_internal::get_uniform(int idx) const
{
    if(!m_shared.is_valid() || idx<0 || idx >=(int)m_shared->uniforms.size())
    {
        return nya_memory::get_invalid_object<shared_shader::uniform>();
    }

    return m_shared->uniforms[idx];
}

void shader_internal::set_uniform_value(int idx,float f0,float f1,float f2,float f3) const
{
    if(!m_shared.is_valid() || idx<0 || idx >=(int)m_shared->uniforms.size())
        return;

    if(m_shared->uniforms[idx].location<0)
        return;

    if(m_shared->uniforms[idx].transform==shared_shader::local)
    {
        nya_math::vec3 v=transform::get().inverse_transform(nya_math::vec3(f0,f1,f2));
        m_shared->shdr.set_uniform(m_shared->uniforms[idx].location,v.x,v.y,v.z,f3);
    }
    else if(m_shared->uniforms[idx].transform==shared_shader::local_rot)
    {
        nya_math::vec3 v=transform::get().inverse_rot(nya_math::vec3(f0,f1,f2));
        m_shared->shdr.set_uniform(m_shared->uniforms[idx].location,v.x,v.y,v.z,f3);
    }
    else
        m_shared->shdr.set_uniform(m_shared->uniforms[idx].location,f0,f1,f2,f3);
}

void shader_internal::set_uniform4_array(int idx,const float *array,int size) const
{
    if(!m_shared.is_valid() || idx<0 || idx >=(int)m_shared->uniforms.size())
        return;

    m_shared->shdr.set_uniform4_array(m_shared->uniforms[idx].location,array,size);
}

int shader_internal::get_uniforms_count() const
{
    if(!m_shared.is_valid())
        return 0;

    return (int)m_shared->uniforms.size();
}

const nya_render::skeleton *shader_internal::m_skeleton=0;

}
