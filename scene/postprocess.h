//https://code.google.com/p/nya-engine/

#pragma once

#include "scene/texture.h"
#include "scene/shader.h"
#include "render/screen_quad.h"
#include "render/fbo.h"

namespace nya_scene
{

struct shared_postprocess
{
    nya_render::screen_quad quad;

    std::map<std::string,nya_scene::texture> textures;

    struct render_target
    {
        nya_render::fbo fbo;
        std::string width,height;
    };

    std::map<std::string,render_target> targets;

    bool release()
    {
        quad.release();
        return true;
    }
};

class postprocess: public scene_shared<shared_postprocess>
{
private:
    virtual void draw_scene(const char *pass,const char *tags) {}

public:
    bool load(const char *name) { return scene_shared::load(name); }
    void unload() { scene_shared::unload(); }
    void resize(unsigned int width,unsigned int height);
    void draw(int dt);

public:
    postprocess(): m_width(0),m_height(0) { default_load_function(load_text); }

public:
    static bool load_text(shared_postprocess &res,resource_data &data,const char* name);

private:
    void update_targets();

private:
    unsigned int m_width;
    unsigned int m_height;
};

}
