//https://code.google.com/p/nya-engine/

#pragma once

#include "render/vbo.h"
#include "render/texture.h"
#include "resources/resources.h"
#include "resources/shared_resources.h"

class tsb_anim;

class tmb_model
{
public:
    bool load(nya_resources::resource_data *data);
    void draw(bool use_materials,int group=-1);
    void release();

    void apply_anim(tsb_anim *anim);

    const float *get_buffer(unsigned int frame) const
    {
        if(m_bones.empty() || !m_frames_count)
            return 0;

        return (float*)&m_anim_bones[m_bones.size()*(frame % m_frames_count)];
    }

    unsigned int get_bones_count() { return (unsigned int)m_bones.size(); }
    unsigned int get_frames_count() { return m_frames_count; }

    unsigned int get_groups_count() { return (unsigned int)m_group_names.size(); }
    const char *get_group_name(unsigned int idx)
    { 
        if(idx>=m_group_names.size())
            return 0;

        return m_group_names[idx].c_str();
    }

    tmb_model(): m_frames_count(0) {}

private:
    nya_render::vbo m_vbo;

    struct material
    {
        unsigned int group;
        
        unsigned int vert_offset;
        unsigned int vert_count;
        
        bool cull_face;

        unsigned int tex_idx;
    };

    std::vector<nya_render::texture> m_textures;
    std::vector<material> m_materials;

private:
    struct bone
    {
        float mat[4][4];
    };

    std::vector<bone> m_bones;

      //ToDo: convert to frames
    unsigned int m_frames_count;
    std::vector<bone> m_anim_bones;

private:
    std::vector<std::string> m_group_names;

public:
    struct locator
    {      
        float pos[3];
        float ang[3];
        float scale[3];

        //std::string name; //ToDo ?
    };

    const locator *get_locator(int idx) const
    {
        if(idx<0 || idx>=m_locators.size())
            return 0;

        return &m_locators[idx];
    }

private:
    std::vector<locator> m_locators;
};

typedef nya_resources::shared_resources<tmb_model,8> shared_models;
typedef shared_models::shared_resource_ref model_ref;

class shared_models_manager: public shared_models
{
    bool fill_resource(const char *name,tmb_model &res);
    bool release_resource(tmb_model &res);
};

shared_models_manager &get_shared_models();
