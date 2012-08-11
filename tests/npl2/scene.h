//https://code.google.com/p/nya-engine/

#pragma once

#include "character.h"
#include "render/shader.h"

class viewer_camera
{
public:
    void apply();
    void add_rot(float dx,float dy);
    void add_pos(float dx,float dy);
    void add_scale(float ds);

public:
    viewer_camera(): m_rot_x(0), m_rot_y(0), m_scale(1.0f), m_pos_x(0), m_pos_y(0) {}

private:
    float m_rot_x;
    float m_rot_y;
    float m_scale;
    float m_pos_x;
    float m_pos_y;
};

class scene
{
public:
    void init();
    void draw();
    viewer_camera &get_camera() { return m_camera; }
    void set_bkg(const char *name);
    void set_imouto_attr(const char *key,const char *value,int num=-1);
    void set_imouto_preview(const char *key,const char *value,int num=-1);
    void finish_imouto_preview();
    void prev_anim();
    void next_anim();
    void set_anim(unsigned int num);
    const char *get_anim_name(unsigned int num);
    unsigned int get_anims_count();
    void release();
    
private:
    void apply_anim();
    
public:
    scene(): m_sh_mat_uniform(0), m_shbl_mat_uniform(0), m_preview(false), m_anim_time(0), m_has_scenery(false) {}

private:
    nya_render::shader m_shader;
    unsigned int m_sh_mat_uniform;
    nya_render::shader m_shader_black;
    unsigned int m_shbl_mat_uniform;

    nya_render::shader m_shader_scenery;

private:
    character m_imouto;
    character m_imouto_preview;
    bool m_preview;

private:
    viewer_camera m_camera;

private:
    struct anim_info
    {
        std::string name[10];

        anim_info() {}
        anim_info(const char *n) { if(n) name[0].assign(n); }
    };

    std::vector<anim_info> m_anim_list;
    std::vector<anim_info>::iterator m_curr_anim;

    float m_anim_time;

private:
    /*
    float m_bro_dpos_x;
    float m_bro_dpos_y;
    float m_bro_dpos_z;
    */
    tmb_model m_aniki;

private:
    const static int max_bkg_models=3;
    tmb_model m_bkg_models[max_bkg_models]; 
    bool m_has_scenery;
};

scene &get_scene();
