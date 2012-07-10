//https://code.google.com/p/nya-engine/

#include "character.h"
#include "attributes.h"
#include "tsb_anim.h"

#include "string.h"

void character::set_attrib(const char *key,const char *value,int num)
{
    const int max_models_per_part=10;

    if(!key||!value||num>=max_models_per_part)
    {
        nya_log::get_log()<<"Unable to set character attribute: invalid input params\n";
        return;
    }

      // ToDo: special case - COORDINATE - a whole set

      // ToDo: ignore replace in cases of num>=0

    parts_map::iterator it=m_parts_map.find(key);
    part *p=0;
    if(it!=m_parts_map.end() && it->second)
    {
        if(it->second->value.compare(value)==0)
            return;

        p=it->second;
    }
    else
    {
        m_parts.push_back(part());
        p=m_parts_map[key]=&m_parts.back();
    }

    if(!p)
    {
        nya_log::get_log()<<"Unable to set character attribute\n";
        return;
    }

    if(num<0)
    {
        p->value.assign(value);
        p->free_models();
    }
    else
        p->models.resize(max_models_per_part);

    attribute *a=get_attribute_manager().get(key,value);
    if(!a)
    {
        nya_log::get_log()<<"Invalid attribute "<<value<<" of type "<<key<<"\n";
        return;
    }

    if(num<0)
    for(int i=0;i<max_models_per_part;++i)
    {
        char key[7]="FILE_";
        key[5]=i+'0';
        const char *model_name=a->get_value(key);
        if(!model_name)
        {
            break;
        }

        if(strcmp(model_name,"nil")==0)
        {
            p->models.push_back(model_ref());
            continue;
        }

        model_ref ref=get_models_manager().access(model_name);
        if(!ref.is_valid())
        {
            nya_log::get_log()<<"Unable to set character attribute::Invalid model ref\n";
            return;
        }

        p->models.push_back(ref);
    }
    else
    {
        char key[7]="FILE_";
        key[5]=num+'0';
        model_ref &ref=p->models[num];
        ref.free();
        const char *model_name=a->get_value(key);
        if(!model_name||strcmp(model_name,"nil")==0)
            return;

        ref=get_models_manager().access(model_name);
        if(!ref.is_valid())
            nya_log::get_log()<<"Unable to set character attribute::Invalid model ref\n";
    }
}

void character::reset_attrib()
{
    for(parts_list::iterator it=m_parts.begin();
        it!=m_parts.end();++it)
    {
        for(parts_list::iterator it=m_parts.begin();
            it!=m_parts.end();++it)
            it->free_models();
    }

    m_parts.clear();
    m_parts_map.clear();
}

void character::set_anim(const char *anim_name)
{
    if(!anim_name)
        return;

    nya_log::get_log()<<"Set anim: "<<anim_name<<"\n";

    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
    {
        nya_log::get_log()<<"Unable to set character anim: invalid base part\n";
        return;
    }

    if(it->second->models.empty())
        return;

    model_ref m=it->second->models.front();
    if(!m.is_valid())
    {
        nya_log::get_log()<<"Unable to set character anim: invalid base part model ref\n";
        return;
    }

    std::string name(anim_name);
    name.append(".tsb");

    anim_ref a = get_anims_manager().access(name.c_str());
    if(!a.is_valid())
    {
        nya_log::get_log()<<"Unable to set character anim: invalid animation\n";
        return;
    }

    m->apply_anim(a.get());

    a.free();
}

void character::draw(bool use_materials)
{
    for(parts_list::iterator it=m_parts.begin();
        it!=m_parts.end();++it)
    {
        for(unsigned int i=0;i<it->models.size();++i)
        {
            model_ref &m=it->models[i];
            if(m.is_valid())
                m->draw(use_materials);
        }
    }
}

void character::release()
{
    for(parts_list::iterator it=m_parts.begin();
        it!=m_parts.end();++it)
    {
        for(parts_list::iterator it=m_parts.begin();
            it!=m_parts.end();++it)
            it->free_models();
    }

    m_parts.clear();
    m_parts_map.clear();
}

float *character::get_buffer(unsigned int frame)
{
    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
        return 0;

    if(it->second->models.empty())
        return 0;

    model_ref m=it->second->models.front();
    if(!m.is_valid())
        return 0;

    return m->get_buffer(frame);
}

unsigned int character::get_frames_count()
{
    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
        return 0;

    if(it->second->models.empty())
        return 0;

    model_ref m=it->second->models.front();
    if(!m.is_valid())
        return 0;

    return m->get_frames_count();
}

unsigned int character::get_bones_count()
{
    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
        return 0;

    if(it->second->models.empty())
        return 0;

    model_ref m=it->second->models.front();
    if(!m.is_valid())
        return 0;

    return m->get_bones_count();
}
