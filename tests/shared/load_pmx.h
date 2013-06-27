//https://code.google.com/p/nya-engine/

namespace nya_memory { class memory_reader; class tmp_buffer_ref; }
namespace nya_scene { class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

class pmx_loader
{
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
        float pos[3];
        float normal[3];
        float tc[2];
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

    static int read_idx(nya_memory::memory_reader &reader,int size);

public:
    static bool load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);
};
