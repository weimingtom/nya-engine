//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"

namespace nya_ui
{

class slider: public widget
{
public:
    float get_value() { return m_value; }
    void set_value(float value) { m_value=clamp(value,0.0f,1.0f); update_rects(); }

    virtual void set_size(uint width,uint height)
    {
        m_vertical=width<height;
        widget::set_size(width,height);
    }

    virtual void set_slider_size(uint size) //default is 10
    {
        m_slider_size=size;
        update_rects();
    }

protected:
    void draw(layer &layer) {}
    void update_rects();

protected:
    virtual bool on_mouse_move(uint x,uint y,bool inside);
    virtual bool on_mouse_button(layout::mbutton button,bool pressed);
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
    slider(): m_value(0),m_mouse_pressed(false),m_mouse_last(0),m_slider_size(10),m_vertical(false) {}

protected:
    float m_value;
    bool m_mouse_pressed;
    uint m_mouse_last;

protected:
    uint m_slider_size;
    rect m_slider_rect;
    bool m_vertical;
};

}
