//https://code.google.com/p/nya-engine/

#include "scene.h"

namespace { nya_log::log_base *scene_log=0; }

namespace nya_scene
{

void set_log(nya_log::log_base *l) { scene_log=l; }

nya_log::log_base &log()
{
    static const char *scene_log_tag="scene";
    if(!scene_log)
        return nya_log::log(scene_log_tag);

    scene_log->set_tag(scene_log_tag);
    return *scene_log;
}

}
