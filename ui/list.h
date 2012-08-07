//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"
#include <vector>

/*

    ToDo: use button stile instead of rect style and _hl _pressed stuff

*/

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
    layer::rect_style entry_hl;
    layer::rect_style entry_selected;
    layer::rect_style scroll;
    layer::rect_style scroll_hl;
    layer::rect_style scroll_area;
    layer::rect_style button_up;
    layer::rect_style button_dn;
    layer::rect_style button_up_hl;
    layer::rect_style button_dn_hl;

    list_style()
    {
        scroll_area_width=12;
        scroll_width=scroll_area_width;
        scroll_height=20;
        button_height=16;
        entry_height=18;

        list.border=true;
        list.border_color.set(0.4,0.3,1.0,1.0);

        entry=list;

        list.solid=true;
        list.solid_color=list.border_color;
        list.solid_color.a=0.1;

        scroll_area=list;
        scroll_area.solid_color.a=0.2;

        scroll=list;
        scroll.solid_color.a=0.5;

        button_up=button_dn=scroll;

        entry_selected=list;

        entry_selected.solid_color.a=0.3;
        entry_hl=entry;
        entry_hl.solid=true;
        entry_hl.solid_color.set(0.7,0.6,1.0,0.5);
        scroll_hl=button_dn_hl=button_up_hl=entry_hl;
    }
};

class list: public widget
{
public:
    virtual void set_style(list_style &s)
    {
        m_style=s;
    }

    virtual void add_element(const char *name)
    {
        if(!name)
            return;

        m_elements.push_back(name);
    }

    virtual void remove_elements()
    {
        m_elements.clear();
        m_scroll=0;
        m_mover=0;
        m_selected=0;
        update_rects();
    }

    void select_element(uint num)
    {
        m_selected=num;
    }

    void select_element(const char *name)
    {
        if(!name)
            return;

        for(uint i=0;i<m_elements.size();++i)
        {
            if(m_elements[i]==name)
            {
                m_selected=i;
                break;
            }
        }
    }

public:
    struct event_data: public layout::event_data
    {
        std::string element;
        int idx;

        event_data(): idx(-1) {}
    };

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
    list(): m_scroll(0), m_scroll_abs(0), m_scroll_max(0),  m_mouse_x(0), m_mouse_y(0),
            m_mouse_hold_y(0), m_scrolling(false), m_selected(-1), m_mover(-1) {}

protected:
    list_style m_style;
    uint m_scroll;
    uint m_scroll_max;
    uint m_scroll_abs;
    uint m_mouse_x;
    uint m_mouse_y;
    uint m_mouse_hold_y;
    bool m_scrolling;

protected:
    rect m_scroll_rect;
    rect m_scroll_area_rect;
    rect m_button_down_rect;
    rect m_button_up_rect;

protected:
    typedef std::string element;
    std::vector<element> m_elements;
    int m_selected;
    int m_mover;
};

}
