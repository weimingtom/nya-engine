//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"

namespace nya_ui
{

struct panel_style
{
    layer::rect_style panel;

    panel_style()
    {
        panel.border_color.set(0.4,0.3,1.0,1.0);
        panel.border=true;
    }
};

class panel: public widget, public layout
{
public:
    virtual void set_style(panel_style &s)
    {
        m_style=s;
    }

public:
    virtual void set_pos(int x,int y);
    virtual void set_size(uint width,uint height);

protected:
    virtual void draw(layer &l);
    virtual void process_events(layout::event &e);
    virtual void parent_resized(uint width,uint height);
    virtual void parent_moved(int x,int y);
    virtual void calc_pos_markers();

public:
    virtual void add_widget(widget &w)
    {
        if(&w==this)
            return;

        layout::add_widget(w);
    }

protected:
    //virtual void on_mouse_over();
    virtual void on_mouse_left() { layout::mouse_left(); }
    virtual bool on_mouse_move(uint x,uint y,bool inside)
    {
        layout::mouse_move(x,y);
        return inside;
    }

    virtual bool on_mouse_scroll(uint dx,uint dy)
    {
        layout::mouse_scroll(dx,dy);
        return true;
    }

    virtual bool on_mouse_button(layout::button button,bool pressed)
    {
        layout::mouse_button(button,pressed);
        return true;
    }

public:
    virtual void send_event(event &e) {  widget::send_event(e.sender.c_str(),e); }

protected:
    void update_layout_rect();

protected:
    panel_style m_style;
};

}
