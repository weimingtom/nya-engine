//https://code.google.com/p/nya-engine/

#pragma once

#include "widgets.h"

typedef nya_ui::uint uint;

class ui: public nya_ui::layer
{
public:
    void init();
    void draw() override;
    void resize(uint width,uint height) override
    {
        widget_renderer::get().resize(width,height);
        nya_ui::layer::resize(width,height);
    }

    void release() { widget_renderer::get().release(); }

public:
    ui(): m_under_top(false),m_under_bottom(false) {}

private:
    virtual void process_events(const event &e) override;

private:
    bool is_props_visible();
    void update_props_panel();
    void modal(bool enabled,int x,int y);

private:
    panel m_anim_pnl;
    panel m_customize_pnl;
    panel m_scenery_pnl;

    panel m_props_pnl;
    slider m_opac_slider;

    panel m_under_pnl;
    bool m_under_top;
    bool m_under_bottom;

    panel m_cos_modal;
    panel m_mod_box;

    struct customize_btn
    {
        std::string id;
        button btn;
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
    list m_customise_lst;
    list m_anim_lst;
    list m_scenery_lst;
};
