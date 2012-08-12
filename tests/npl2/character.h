//https://code.google.com/p/nya-engine/

#pragma once

#include "tmb_model.h"

#include <string>
#include <map>
#include <list>

class character
{
public:
    void set_attrib(const char *key, const char *value,int num=-1);
    const char *get_attrib(const char *key,int num=0);
    void reset_attrib();
    void set_anim(const char *anim_name);
    const char *get_anim() const { return m_anim_name.c_str(); }

    void draw(bool use_materials);

    void release();
    
    void copy_attrib(const character &from);

    const float *get_buffer(unsigned int frame) const;
    unsigned int get_frames_count() const;
    unsigned int get_bones_count() const;
    
    character(): m_body_group_count(0), m_body_blend_group_idx(0) {}

private:
    enum part_id
    {
        invalid_part=-1,
        body,
        eye,
        hair,
        head,
        face,
        socks,
        under,
        arm,
        shoes,
        costume,
        neck,
        max_parts
    };

    part_id get_part_id(const char *name);  //unstrict

private:
    struct part
    {
        struct subpart
        {
            std::string value;
            model_ref model;
        };

        static const int max_models_per_part=4;
        subpart subparts[max_models_per_part];
        
        void draw(bool use_materials)
        {
            for(int i=2;i>0;--i)
            {
                model_ref &m=subparts[i-1].model;
                if(m.is_valid())
                    m->draw(use_materials);
            }
        }

        void free_models()
        {
            for(unsigned int i=0;i<max_models_per_part;++i)
            {
                subparts[i].model.free();
                subparts[i].value.clear();
            }
        }
    };

    part m_parts[max_parts];
    std::string m_anim_name;
    int m_body_group_count;
    int m_body_blend_group_idx;
};
