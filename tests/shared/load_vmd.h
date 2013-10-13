//https://code.google.com/p/nya-engine/

namespace nya_memory { class tmp_buffer_ref; }
namespace nya_scene { class shared_animation; typedef nya_memory::tmp_buffer_ref resource_data; }

class vmd_loader
{
public:
    static bool load(nya_scene::shared_animation &res,nya_scene::resource_data &data,const char* name);
};
