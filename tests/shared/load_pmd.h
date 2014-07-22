//https://code.google.com/p/nya-engine/

#pragma once

#include "math/vector.h"
#include <vector>
#include <string>

namespace nya_memory { class tmp_buffer_ref; }
namespace nya_scene { class mesh; class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

struct pmd_morph_data
{
    struct morph_vertex
    {
        unsigned short idx;
        nya_math::vec3 pos;
    };

    enum morph_type
    {
        morph_base=0,
        morph_brow,
        morph_eye,
        morph_mouth,
        morph_other
    };

    struct morph
    {
        morph_type type;
        std::string name;
        std::vector<morph_vertex> verts;
    };

    std::vector<morph> morphs;
};

struct pmd_phys_data
{
    struct rigid_body
    {
        std::string name;
        short bone;
        unsigned char collision_group;
        unsigned short collision_mask;
        unsigned char type;
        nya_math::vec3 size;
        nya_math::vec3 location;
        nya_math::vec3 rotation;
        float mass;
        float vel_attenuation;
        float rot_attenuation;
        float bounce;
        float friction;
        unsigned char mode;
    };

    std::vector<rigid_body> rigid_bodies;

    struct joint
    {
        std::string name;
        unsigned int rigid_src;
        unsigned int rigid_dst;
        nya_math::vec3 location;
        nya_math::vec3 rotation;
        nya_math::vec3 location_max;
        nya_math::vec3 location_min;
        nya_math::vec3 rotation_max;
        nya_math::vec3 rotation_min;
        nya_math::vec3 location_spring;
        nya_math::vec3 rotation_spring;
    };

    std::vector<joint> joints;
};

struct pmd_loader
{
public:
    struct pmd_material_params
    {
        float diffuse[4];
        float shininess;        
        float specular[3];
        float ambient[3];
    };

    struct vert
    {
        nya_math::vec3 pos[2];
        nya_math::vec3 normal;
        nya_math::vec2 tc;
        float bone_idx[2];
        float bone_weight;
    };

public:
    static bool load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);

public:
    struct additional_data: public pmd_morph_data, pmd_phys_data
    {
    };

    static const additional_data *get_additional_data(const nya_scene::mesh &m);
};
