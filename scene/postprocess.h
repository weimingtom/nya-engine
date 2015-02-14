//https://code.google.com/p/nya-engine/

#pragma once

#include "scene/texture.h"
#include "scene/shader.h"
#include "render/screen_quad.h"
#include "render/fbo.h"
#include "memory/shared_ptr.h"

namespace nya_scene
{

struct shared_postprocess
{
    struct line
    {
        std::string type,name;
        std::vector<std::pair<std::string,std::string> > values;
    };

    std::vector<line> lines;

    bool release() { lines.clear(); return true; }
};

class postprocess: public scene_shared<shared_postprocess>
{
private:
    virtual void draw_scene(const char *pass,const char *tags) {}

public:
    bool load(const char *name);
    void unload();

public:
    void resize(unsigned int width,unsigned int height);
    void draw(int dt);

public:
    void set_condition(const char *condition,bool value);
    bool get_condition(const char *condition) const;

public:
    postprocess(): m_width(0),m_height(0) { default_load_function(load_text); }
    ~postprocess() { unload(); }

public:
    static bool load_text(shared_postprocess &res,resource_data &data,const char* name);

private:
    void update();

private:
    unsigned int m_width;
    unsigned int m_height;
    nya_memory::shared_ptr<nya_render::screen_quad> m_quad;
    typedef std::map<std::string,bool> conditions_map;
    conditions_map m_conditions;

    enum op_types
    {
        type_set_target,
        type_set_texture,
        type_set_shader,
        type_draw_scene,
        type_draw_quad
    };

    struct op
    {
        op_types type;
        size_t idx;
    };

    std::vector<op> m_op;

    struct op_draw_scene
    {
        std::string pass;
        std::string tags;
    };

    std::vector<op_draw_scene> m_op_draw_scene;

    struct op_set_shader
    {
        nya_scene::shader sh;
    };

    std::vector<op_set_shader> m_op_set_shader;
};

}
