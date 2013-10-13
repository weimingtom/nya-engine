//https://code.google.com/p/nya-engine/

namespace nya_memory { class tmp_buffer_ref; }
namespace nya_scene { class shared_mesh; typedef nya_memory::tmp_buffer_ref resource_data; }

class pmd_loader
{
    struct mmd_material_params
    {
        float diffuse[4];
        float shininess;        
        float specular[3];
        float ambient[3];
    };

public:
    static bool load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name);
};
