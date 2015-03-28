//https://code.google.com/p/nya-engine/

#pragma once

#include "scene/texture.h"
#include "scene/shader.h"
#include "render/screen_quad.h"
#include "render/fbo.h"
#include "render/render.h"

namespace nya_scene
{

struct shared_postprocess
{
    struct line
    {
        std::string type,name;
        std::vector<std::pair<std::string,std::string> > values;

        const char *get_value(const char *name) const;
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

    void set_variable(const char *name,float value);
    float get_variable(const char *name) const;

    void set_texture(const char *name,const texture_proxy &tex);
    const texture_proxy &get_texture(const char *name) const;

    void set_shader_param(const char *name,const nya_math::vec4 &value);
    const nya_math::vec4 &get_shader_param(const char *name) const;

public:
    postprocess(): m_width(0),m_height(0) { default_load_function(load_text); }
    ~postprocess() { unload(); }

public:
    static bool load_text(shared_postprocess &res,resource_data &data,const char* name);

private:
    void update();
    void update_shader_param(int idx);
    void clear_ops();

private:
    unsigned int m_width,m_height;
    nya_memory::shared_ptr<nya_render::screen_quad> m_quad;

    struct tex_holder
    {
        bool user_set; texture_proxy tex;
        tex_holder() {} tex_holder(bool u,const texture_proxy &t): user_set(u),tex(t) {}
    };

    std::vector<std::pair<std::string,bool> > m_conditions;
    std::vector<std::pair<std::string,float> > m_variables;
    std::vector<std::pair<std::string,tex_holder> > m_textures;
    std::vector<std::pair<std::string,nya_math::vec4> > m_shader_params;

    enum op_types
    {
        type_set_target,
        type_set_texture,
        type_set_shader,
        type_clear,
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
        std::vector<int> params_map;
    };

    std::vector<op_set_shader> m_op_set_shader;

    struct op_set_texture
    {
        size_t tex_idx;
        int layer;
    };

    std::vector<op_set_texture> m_op_set_texture;

    struct op_target
    {
        nya_memory::shared_ptr<nya_render::fbo> fbo;
        nya_render::rect rect;
    };

    std::vector<op_target> m_targets;
};

}
