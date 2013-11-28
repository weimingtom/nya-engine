//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"

namespace nya_ui
{

class panel: public widget, public layout
{
public:
    virtual void set_pos(int x,int y) override;
    virtual void set_size(uint width,uint height) override;

protected:
    virtual void draw(layer &l) override { layout::draw_widgets(l); }
    virtual void process_events(layout::event &e) override;
    virtual void parent_resized(uint width,uint height) override;
    virtual void parent_moved(int x,int y) override;
    virtual void calc_pos_markers() override;

public:
    virtual void add_widget(widget &w) override
    {
        if(&w==this)
            return;

        layout::add_widget(w);
    }

protected:
    virtual void on_mouse_left() override { layout::mouse_left(); widget::on_mouse_left(); }
    virtual bool on_mouse_move(uint x,uint y,bool inside) override
    {
        layout::mouse_move(x,y);
        return inside;
    }

    virtual bool on_mouse_scroll(uint dx,uint dy) override
    {
        layout::mouse_scroll(dx,dy);
        return true;
    }

    virtual bool on_mouse_button(layout::mbutton button,bool pressed) override
    {
        layout::mouse_button(button,pressed);
        return true;
    }

public:
    virtual void send_event(event &e) override { send_to_parent(e); }

protected:
    void update_layout_rect();
};

}
