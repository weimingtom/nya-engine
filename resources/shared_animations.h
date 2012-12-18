//https://code.google.com/p/nya-engine/

#pragma once

#include "resources/shared_resources.h"
#include "render/animation.h"

namespace nya_resources
{
    typedef shared_resources<nya_render::animation,8> shared_animations;
    typedef shared_animations::shared_resource_ref animation_ref;

    class shared_animations_manager: public shared_animations
    {
	private:
        bool fill_resource(const char *name,nya_render::animation &res);
        bool release_resource(nya_render::animation &res);
    };

    shared_animations_manager &get_shared_animations();
}

