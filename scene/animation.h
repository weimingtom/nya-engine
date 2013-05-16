//https://code.google.com/p/nya-engine/

#pragma once

#include "shared_resources.h"
#include "render/animation.h"

namespace nya_scene
{

struct shared_animation
{
    nya_render::animation anim;

    bool release()
    {
        anim.release();
        return true;
    }
};

class animation: public scene_shared<shared_animation>
{
    friend class mesh;

public:
    static bool load_vmd(shared_animation &res,resource_data &data,const char* name);

public:
    unsigned int get_duration() const;

public:
    animation() { register_load_function(load_vmd); }
};

}
