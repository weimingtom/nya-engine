//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"
#include "ui/button.h"
#include "ui/list.h"
#include "ui/panel.h"
#include "ui/slider.h"

#include "resources/shared_textures.h"
#include "render/shader.h"

typedef nya_ui::uint uint;

class ui: public nya_ui::layer
{
public:
    void init();
    void draw();

    void draw_text(uint x,uint y,const char *text
                   ,font_align aligh_hor=left,font_align aligh_vert=bottom);

    void draw_rect(nya_ui::rect &r,rect_style &s);

private:
    virtual void process_events(event &e);

private:
    bool is_props_visible();
    void update_props_panel();

private:
    nya_ui::panel m_anim_pnl;
    nya_ui::panel m_customize_pnl;
    nya_ui::panel m_scenery_pnl;

    nya_ui::panel m_props_pnl;
    nya_ui::slider m_opac_slider;

    nya_ui::panel m_cos_modal;
    nya_ui::panel m_under_modal;

    struct customize_btn
    {
        std::string id;
        nya_ui::button btn;
    };

    static const uint max_customize_btns=32;
    customize_btn m_customize_btns[max_customize_btns];

    enum custom_mode
    {
        none,
        cos_up,
        cos_dn
    };

    custom_mode m_custom_mode;

    std::string m_customise_group;

private:
    nya_ui::list m_customise_lst;
    nya_ui::list m_anim_lst;
    nya_ui::list m_scenery_lst;

private:
    nya_resources::texture_ref m_font_tex;
    nya_resources::texture_ref m_ui_tex;
    nya_render::shader m_ui_shader;
    nya_render::shader m_text_shader;
};
