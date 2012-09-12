//https://code.google.com/p/nya-engine/

#pragma once

#include "resources/resources.h"

struct config
{
    bool wireframe_outline_enabled;
    bool specular_enabled;
    float specular_level;

public:
    config(): wireframe_outline_enabled(true),specular_enabled(true),
              specular_level(0.06) {}

public:
    bool load(nya_resources::resource_data *data);
};

config &get_config();
