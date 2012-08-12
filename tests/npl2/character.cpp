//https://code.google.com/p/nya-engine/

#include "character.h"
#include "attributes.h"
#include "tsb_anim.h"

#include "string.h"

void character::set_attrib(const char *key,const char *value,int num)
{
    if(!key || num>=part::max_models_per_part)
    {
        nya_log::get_log()<<"Unable to set character attribute: invalid input params\n";
        return;
    }

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
        set_attrib("NECK",a->get_value("COORD_09"));
        set_attrib("ARM",a->get_value("COORD_10"));
        set_attrib("SHOES",a->get_value("COORD_11"));
        set_attrib("HAIR",a->get_value("COORD_12")); 
        
        return;
    }

    part_id id=get_part_id(key);
    if(id==invalid_part)
    {
        nya_log::get_log()<<"Unable to set character attribute: unknown key\n";
        return;
    }
    
    part &p=m_parts[id];

    if(!value || strcasecmp(value,"nil")==0)
    {
        if(id==body)
            value="imo_bodyA_00";
        else if(id==eye)
            value="imo_eye_00";
        else if(id==hair)
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
    
    attribute *a=get_attribute_manager().get(key,value);
    if(!a)
    {
        value=value_str.c_str();
        a=get_attribute_manager().get(key,value);
        if(!a)
        {
            //nya_log::get_log()<<"Invalid attribute "<<value<<" of type "<<key<<"\n";
            value="nil";
        }
    }

    if(!value[0] || strcasecmp(value,"nil")==0)
    {
        if(num<0)
            p.free_models();
        else if(num<part::max_models_per_part)
        {
            p.subparts[num].model.free();
            p.subparts[num].value="nil";
        }
        return;
    }

    if(num<0)
    {
        int max_models=part::max_models_per_part;
        for(int i=0;i<max_models;++i)
        {
            if(p.subparts[i].value==value)
                continue;

            p.subparts[i].model.free();
            p.subparts[i].value.assign(value);

            char key[7]="FILE_";
            key[5]=i+'0';
            const char *model_name=a->get_value(key);
            if(!model_name || strcmp(model_name,"nil")==0)
            {
                p.subparts[i].model.free();
                continue;
            }

            model_ref ref=get_shared_models().access(model_name);
            if(!ref.is_valid())
            {
                nya_log::get_log()<<"Unable to set character attribute::Invalid model ref\n";
                //return;
            }

            p.subparts[i].model=ref;
        }
    }
    else if(p.subparts[num].value!=value)
    {
        char key[7]="FILE_";
        key[5]=num+'0';
        model_ref &ref=p.subparts[num].model;
        ref.free();
        const char *model_name=a->get_value(key);
        if(!model_name||strcmp(model_name,"nil")==0)
            return;

        p.subparts[num].value.assign(value);

        ref=get_shared_models().access(model_name);
        if(!ref.is_valid())
            nya_log::get_log()<<"Unable to set character attribute::Invalid model ref\n";
    }

    if(id==body && p.subparts[0].model.is_valid())
    {
        m_body_blend_group_idx=-1;
        
        set_anim(m_anim_name.c_str());
        m_body_group_count=p.subparts[0].model->get_groups_count();
        for(int i=0;i<m_body_group_count;++i)
        {
            const char *name=p.subparts[0].model->get_group_name(i);
            if(!name)
                continue;
            
            if(strstr(name,"matu"))
            {
                m_body_blend_group_idx=i;
                break;
            }
        }
    }
}

const char *character::get_attrib(const char *key,int num)
{
    if(!key)
        return 0;
    
    part_id id=get_part_id(key);
    if(id==invalid_part)
        return 0;
    
    m_parts[id].subparts[num].value.c_str();
    
    return 0;
}

void character::copy_attrib(const character &from)
{
    set_attrib("BODY",from.m_parts[body].subparts[0].value.c_str());
    set_attrib("EYE",from.m_parts[eye].subparts[0].value.c_str()); 
    set_attrib("UNDER",from.m_parts[under].subparts[0].value.c_str(),0);
    set_attrib("UNDER",from.m_parts[under].subparts[1].value.c_str(),1);
    set_attrib("SOCKS",from.m_parts[socks].subparts[0].value.c_str());
    set_attrib("COSTUME",from.m_parts[costume].subparts[0].value.c_str(),0);
    set_attrib("COSTUME",from.m_parts[costume].subparts[1].value.c_str(),1);
    set_attrib("HEAD",from.m_parts[head].subparts[0].value.c_str());
    set_attrib("FACE",from.m_parts[face].subparts[0].value.c_str());
    set_attrib("NECK",from.m_parts[neck].subparts[0].value.c_str());
    set_attrib("ARM",from.m_parts[arm].subparts[0].value.c_str());
    set_attrib("SHOES",from.m_parts[shoes].subparts[0].value.c_str());
    set_attrib("HAIR",from.m_parts[hair].subparts[0].value.c_str()); 
}

void character::reset_attrib()
{
    for(int i=0;i<max_parts;++i)
        m_parts[i].free_models();
}

void character::set_anim(const char *anim_name)
{
    if(!anim_name)
        return;
    
    m_anim_name.assign(anim_name);

    //nya_log::get_log()<<"Set anim: "<<anim_name<<"\n";

    model_ref m=m_parts[body].subparts[0].model;
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

//#include "render/platform_specific_gl.h"

void character::draw(bool use_materials)
{
    if(!m_parts[body].subparts[0].model.is_valid())
        return;

    for(int i=0;i<m_body_group_count;++i)
    {
        if(i==m_body_blend_group_idx)
            continue;

        m_parts[body].subparts[0].model->draw(use_materials,i);
    }

    m_parts[eye].draw(use_materials);
    m_parts[hair].draw(use_materials);
    m_parts[head].draw(use_materials);

    m_parts[body].subparts[0].model->draw(use_materials,m_body_blend_group_idx);

    //glColor4f(1,1,1,0.7);
    for(int i=face;i<max_parts;++i)
        m_parts[i].draw(use_materials);

    //glColor4f(1,1,1,1);
}

void character::release()
{
    for(int i=0;i<max_parts;++i)
        m_parts[i].free_models();
}

const float *character::get_buffer(unsigned int frame) const
{
    model_ref m=m_parts[body].subparts[0].model;
    if(!m.is_valid())
        return 0;

    return m->get_buffer(frame);
}

unsigned int character::get_frames_count() const
{
    model_ref m=m_parts[body].subparts[0].model;
    if(!m.is_valid())
        return 0;

    return m->get_frames_count();
}

unsigned int character::get_bones_count() const
{
    model_ref m=m_parts[body].subparts[0].model;
    if(!m.is_valid())
        return 0;

    return m->get_bones_count();
}

character::part_id character::get_part_id(const char *name)
{
    if(!name)
        return invalid_part;

    switch(name[0])
    {
        case 'B': return body;
        case 'E': return eye;
        case 'U': return under;
        case 'A': return arm;
        case 'C': return costume;
        case 'N': return neck;
        case 'F': return face;

        case 'H':
        {
            if(name[1]=='A')
                return hair;
            
            return head;
        }

        case 'S':
        {
            if(name[1]=='O')
                return socks;
            
            return shoes;
        }            
    };

    return invalid_part;
}

