//https://code.google.com/p/nya-engine/

#pragma once

#include "load_pmd.h"

namespace nya_memory { class memory_reader; class tmp_buffer_ref; }
namespace nya_scene { class mesh; class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

struct pmx_loader
{
public:
    struct pmx_header
    {
        char text_encoding;
        char extended_uv;
        char index_size;
        char texture_idx_size;
        char material_idx_size;
        char bone_idx_size;
        char morph_idx_size;
        char rigidbody_idx_size;
    };

    struct vert
    {
        nya_math::vec3 pos;
        nya_math::vec3 normal;
        nya_math::vec2 tc;
        float bone_idx[4];
        float bone_weight[4];
    };

    struct pmx_material_params
    {
        float diffuse[4];
        float specular[3];
        float shininess;
        float ambient[3];
    };

    struct pmx_edge_params
    {
        float color[4];
        float width;
    };

    struct pmx_bone
    {
        std::string name;
        int idx;
        int parent;
        std::string parent_name;
        int order;
        nya_math::vec3 pos;

        struct
        {
            bool has_pos;
            bool has_rot;
            int src_idx;
            float k;
        } bound;

        struct ik_link
        {
            int idx;
            bool has_limits;
            nya_math::vec3 from;
            nya_math::vec3 to;
        };

        struct
        {
            bool has;
            int eff_idx;
            int count;
            float k;
            
            std::vector<ik_link> links;
        } ik;
    };

    template<typename t> class flag
    {
    private:
        t m_data;

    public:
        flag(t data) { m_data=data; }
        bool c(t mask) const { return (m_data & mask)!=0; }
    };

public:
    static bool load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);

public:
    struct additional_data: public pmd_morph_data
    {
    };

    static const additional_data *get_additional_data(const nya_scene::mesh &m);
};
