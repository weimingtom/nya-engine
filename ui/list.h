//https://code.google.com/p/nya-engine/

#ifndef list_h
#define list_h

#include "ui/ui.h"

namespace nya_ui
{

struct list_style
{
    uint scroll_area_width;
    uint scroll_width;
    uint scroll_height;
    uint button_height;
    uint entry_height;

    layer::rect_style list;
    layer::rect_style entry;
    layer::rect_style scroll;
    layer::rect_style scroll_area;
    layer::rect_style button;

    list_style()
    {
        scroll_area_width=12;
        scroll_width=scroll_area_width;
        scroll_height=20;
        button_height=16;
        entry_height=18;

        list.border=true;
        list.border_color.set(0.4,0.3,1.0,1.0);

        entry=scroll=scroll_area=button=list;
    }
};

class list: public widget
{
public:
    virtual void set_style(list_style &s)
    {
        m_style=s;
    }

protected:
    virtual void draw(layer &l);
    virtual void update_rects();

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
    list(): m_scroll(0), m_scroll_max(0), m_mouse_x(0), m_mouse_y(0),
            m_mouse_hold_y(0), m_scrolling(false) {}

protected:
    list_style m_style;
    uint m_scroll;
    uint m_scroll_max;
    uint m_mouse_x;
    uint m_mouse_y;
    uint m_mouse_hold_y;
    bool m_scrolling;

protected:
    rect m_scroll_rect;
    rect m_scroll_area_rect;
    rect m_button_down_rect;
    rect m_button_up_rect;
};

}

#endif
