//https://code.google.com/p/nya-engine/

#pragma once

#include "scene/mesh.h"

#include "load_pmd.h"
#include "load_pmx.h"

class mmd_mesh
{
public:
    bool load(const char *name);
    void unload();

    void set_anim(const nya_scene::animation &anim,int layer=0) { m_mesh.set_anim(anim,layer); }
    void set_anim(const nya_scene::animation_proxy & anim,int layer=0) { m_mesh.set_anim(anim,layer); }
    const nya_scene::animation_proxy & get_anim(int layer=0) const { return m_mesh.get_anim(layer); }

    //int get_morph_name();
    void set_morph(int idx,float value,bool override=false)
    {
        if(idx<0 || idx>=int(m_morphs.size()))
            return;

        m_morphs[idx].value=value;
        m_morphs[idx].override=override;
    }

    void update(unsigned int dt);
    void draw();

    void set_pos(const nya_math::vec3 &pos) { m_mesh.set_pos(pos); }
    void set_rot(float yaw,float pitch,float roll) { m_mesh.set_rot(yaw,pitch,roll); }
    void set_rot(const nya_math::quat &rot) { m_mesh.set_rot(rot); }
    void set_scale(float scale) { m_mesh.set_scale(scale); }

    mmd_mesh(): m_pmx_data(0) {}

private:
    nya_scene::mesh m_mesh;
    nya_render::vbo m_vbo;
    std::vector<float> m_vertex_data;
    const pmx_loader::additional_data *m_pmx_data;

    struct morph
    {
        bool override;
        float value;
        float last_value;

        morph(): override(false), value(0.0f), last_value(0.0f) {}
    };

    std::vector<morph> m_morphs;
};
