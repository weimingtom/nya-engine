//https://code.google.com/p/nya-engine/

#include "math/vector.h"
#include <vector>
#include <string>

namespace nya_memory { class tmp_buffer_ref; }
namespace nya_scene { class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

class pmd_loader
{
    struct pmd_material_params
    {
        float diffuse[4];
        float shininess;        
        float specular[3];
        float ambient[3];
    };

    struct pmd_vertex
    {
        nya_math::vec3 pos;
        float normal[3];
        float tc[2];
        float bone_idx[2];
        float bone_weight;
        nya_math::vec3 pos2;
    };

    struct pmd_morph_vertex
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

    struct pmd_morph
    {
        morph_type type;
        std::string name;
        std::vector<pmd_morph_vertex> verts;
    };

public:
    static bool load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);
};
