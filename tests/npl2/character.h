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
    const char *get_attrib(const char *key,int num=-1);
    void reset_attrib();
    void set_anim(const char *anim_name);
    const char *get_anim() const { return m_anim_name.c_str(); }
    void set_part_opacity(const char *key,float value,int num=-1);
    float get_part_opacity(const char *key,int num=-1);
    void reset_parts_opacity();

    void draw(bool use_materials);

    void release();

    void copy_attrib(const character &from);

    const float *get_buffer(unsigned int frame) const;
    unsigned int get_frames_count() const;
    unsigned int get_bones_count() const;
    unsigned int get_first_loop_frame() const;

    void set_color(float r,float g,float b)
    {
        m_color[0]=r;
        m_color[1]=g;
        m_color[2]=b;
    }

    character(): m_body_group_count(0), m_body_blend_group_idx(0)
    {
        for(int i=0;i<3;++i)
            m_color[i]=1.0f;
    }

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

    void draw_part(unsigned int idx,bool use_materials);

private:
    struct part
    {
        struct subpart
        {
            std::string value;
            model_ref model;
            float opacity;
            bool outline;

            subpart(): opacity(1.0f), outline(true) {}
        };

        static const int max_models_per_part=4;
        subpart subparts[max_models_per_part];

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
    float m_color[3];
};
