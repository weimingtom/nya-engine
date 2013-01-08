//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"

namespace nya_ui
{

struct slider_style
{
    layer::rect_style area;
    layer::rect_style slider;
    layer::rect_style slider_hl;
    int size;

    slider_style()
    {
        area.border=true;
        area.solid=true;
        area.border_color.set(0.4f,0.3f,1.0f,1.0f);
        area.solid_color=area.border_color;
        area.solid_color.a=0.2f;
        slider=area;
        slider.solid_color.a=0.5f;
        slider_hl=slider;
        slider_hl.solid_color.set(0.7f,0.6f,1.0f,0.5f);
        size=10;
    }
};

class slider: public widget
{
public:
    void get_value();
    void set_value(float value) { m_value=clamp(value,0.0f,1.0f); update_rects(); }

    virtual void set_size(uint width,uint height)
    {
        m_vertical=width<height;
        widget::set_size(width,height);
    }

public:
    struct event_data: public layout::event_data
    {
        float value;

        event_data(): value(0.0f) {}
    };

protected:
    typedef event_data_allocator<event_data> slider_event_data;

protected:
    void draw(layer &layer);
    void update_rects();

protected:
    virtual bool on_mouse_move(uint x,uint y,bool inside);
    virtual bool on_mouse_button(layout::button button,bool pressed);
    virtual bool on_mouse_scroll(uint x,uint y);

protected:
    virtual void parent_moved(int x,int y)
    {
        widget::parent_moved(x,y);
        update_rects();
    }

    virtual void parent_resized(uint width,uint height)
    {
        widget::parent_resized(width,height);
        update_rects();
    }

    virtual void calc_pos_markers()
    {
        widget::calc_pos_markers();
        update_rects();
    }

public:
    slider(): m_value(0),m_mouse_pressed(false),m_mouse_last(0),m_vertical(false){}

protected:
    float m_value;
    bool m_mouse_pressed;
    uint m_mouse_last;

protected:
    slider_style m_style;
    rect m_slider_rect;
    bool m_vertical;
};

}
