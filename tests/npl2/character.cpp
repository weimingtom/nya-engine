//https://code.google.com/p/nya-engine/

#include "character.h"
#include "attributes.h"
#include "tsb_anim.h"

#include "string.h"

void character::set_attrib(const char *key,const char *value,int num)
{
    const int max_models_per_part=10;
    
    if(!key || num>=max_models_per_part)
    {
        nya_log::get_log()<<"Unable to set character attribute: invalid input params\n";
        return;
    }

    if(!value || strcasecmp(value,"nil")==0)
    {
        if(strcmp(key,"BODY")==0)
            value="imo_bodyA_00";
        else if(strcmp(key,"EYE")==0)
            value="imo_eye_00";
        else if(strcmp(key,"HAIR")==0)
            value="imo_hairA_00";
        else
            return;
    }
    
    std::string value_str(value);
    const char last=value_str[value_str.length()-1];
    if(last>='A' && last<='D')
    {
        value_str.resize(value_str.length()-1);
        num=last-'A';
    }

      // ToDo: special case - COORDINATE - a whole set

      // ToDo: ignore replace in cases of num>=0
    
    if(strcmp(key,"COORDINATE")==0)
    {
        reset_attrib();
        
        attribute *a=get_attribute_manager().get(key,value);
        if(!a)
        {
            nya_log::get_log()<<"Invalid attribute "<<value<<" of type "<<key<<"\n";
            return;
        }

        set_attrib("BODY",a->get_value("COORD_00"));
        set_attrib("EYE",a->get_value("COORD_01")); 
        set_attrib("UNDER",a->get_value("COORD_02"),0);
        set_attrib("UNDER",a->get_value("COORD_03"),1);
        set_attrib("SOCKS",a->get_value("COORD_04"));
        set_attrib("COSTUME",a->get_value("COORD_05"),0);
        set_attrib("COSTUME",a->get_value("COORD_06"),1);
        set_attrib("HEAD",a->get_value("COORD_07"));
        set_attrib("FACE",a->get_value("COORD_08"));
        set_attrib("NECK",a->get_value("COORD_9"));
        set_attrib("ARM",a->get_value("COORD_10"));
        set_attrib("SHOES",a->get_value("COORD_11"));
        set_attrib("HAIR",a->get_value("COORD_12")); 

        return;
    }

    parts_map::iterator it=m_parts_map.find(key);
    part *p=0;
    if(it!=m_parts_map.end() && it->second>=0 && it->second<m_parts.size())
    {
        p=&m_parts[it->second];
        if(p->value.compare(value)==0)
            return;
    }
    else
    {
        m_parts_map[key]=(int)m_parts.size();
        m_parts.push_back(part());
        p=&m_parts.back();
    }

    if(!p)
    {
        nya_log::get_log()<<"Unable to set character attribute\n";
        return;
    }

    if(!value[0] || strcasecmp(value,"nil")==0)
    {
        p->value.assign("nil");
        if(num<0)
            p->free_models();
        else if(num<max_models_per_part)
            p->models[num].free();
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
        value=value_str.c_str();
        a=get_attribute_manager().get(key,value);
        if(!a)
        {
            nya_log::get_log()<<"Invalid attribute "<<value<<" of type "<<key<<"\n";
            return;
        }
    }

    if(num<0)
    {
        int max_models=max_models_per_part;
        if(strcmp(key,"UNDER")==0)
            max_models=2;

        for(int i=0;i<max_models;++i)
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

            model_ref ref=get_shared_models().access(model_name);
            if(!ref.is_valid())
            {
                nya_log::get_log()<<"Unable to set character attribute::Invalid model ref\n";
                return;
            }

            p->models.push_back(ref);
        }
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

        ref=get_shared_models().access(model_name);
        if(!ref.is_valid())
            nya_log::get_log()<<"Unable to set character attribute::Invalid model ref\n";
    }
    
    if(strcmp(key,"BODY")==0)
        set_anim(m_anim_name.c_str());
}

const char *character::get_attrib(const char *key)
{
    parts_map::iterator it=m_parts_map.find(key);
    if(it!=m_parts_map.end() && it->second>=0 && it->second<m_parts.size())
        return m_parts[it->second].value.c_str();
    
    return 0;
}

void character::copy_attributes(const character &from)
{
    for(parts_map::const_iterator it=from.m_parts_map.begin();
        it!=from.m_parts_map.end();++it)
    {
        if(it->second>=0 && it->second<from.m_parts.size())
            set_attrib(it->first.c_str(),from.m_parts[it->second].value.c_str());
    }
}

void character::reset_attrib()
{
    for(parts_vector::iterator it=m_parts.begin();
        it!=m_parts.end();++it)
    {
        for(parts_vector::iterator it=m_parts.begin();
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
    
    m_anim_name.assign(anim_name);

    nya_log::get_log()<<"Set anim: "<<anim_name<<"\n";

    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
    {
        nya_log::get_log()<<"Unable to set character anim: invalid base part\n";
        return;
    }

    if(m_parts[it->second].models.empty())
        return;

    model_ref m=m_parts[it->second].models.front();
    if(!m.is_valid())
    {
        nya_log::get_log()<<"Unable to set character anim: invalid base part model ref\n";
        return;
    }

    std::string name(anim_name);
    name.append(".tsb");

    anim_ref a = get_shared_anims().access(name.c_str());
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
    for(parts_vector::iterator it=m_parts.begin();
        it!=m_parts.end();++it)
    {
        for(unsigned int i=it->models.size();i>0;--i)
        {
            model_ref &m=it->models[i-1];
            if(m.is_valid())
                m->draw(use_materials);
        }
    }
}

void character::release()
{
    for(parts_vector::iterator it=m_parts.begin();
        it!=m_parts.end();++it)
    {
        for(parts_vector::iterator it=m_parts.begin();
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

    if(m_parts[it->second].models.empty())
        return 0;

    model_ref m=m_parts[it->second].models.front();
    if(!m.is_valid())
        return 0;

    return m->get_buffer(frame);
}

unsigned int character::get_frames_count()
{
    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
        return 0;

    if(m_parts[it->second].models.empty())
        return 0;

    model_ref m=m_parts[it->second].models.front();
    if(!m.is_valid())
        return 0;

    return m->get_frames_count();
}

unsigned int character::get_bones_count()
{
    parts_map::iterator it=m_parts_map.find("BODY");
    if(it==m_parts_map.end())
        return 0;

    if(m_parts[it->second].models.empty())
        return 0;

    model_ref m=m_parts[it->second].models.front();
    if(!m.is_valid())
        return 0;

    return m->get_bones_count();
}
