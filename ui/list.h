//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"
#include <vector>

namespace nya_ui
{

class list: public widget
{
public:
    virtual void add_element(const char *name)
    {
        if(!name)
            return;

        m_elements.push_back(name);

        update_rects();
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
        unsigned int scrl=m_scroll_max*m_selected/((int)m_elements.size()
                                                   -m_scroll_max/m_entry_height);
        m_scroll=clamp(scrl,0,m_scroll_max);
        update_rects();
    }

    void select_element(const char *name)
    {
        if(!name)
            return;

        for(uint i=0;i<m_elements.size();++i)
        {
            if(m_elements[i]==name)
            {
                select_element(i);
                break;
            }
        }
    }

    const char *get_element(uint idx)
    {
        if(idx>=m_elements.size())
            return 0;

        return m_elements[idx].c_str();
    }

    int get_selected_idx() { return m_selected; }

    const char *get_selected_element() { return get_element((uint)m_selected); }

public:
    virtual void set_scroll_size(uint scroll_area_width,uint scroll_width,
                                 uint scroll_height, uint button_height)
    {
        m_scroll_area_width=scroll_area_width;
        m_scroll_width=scroll_width;
        m_scroll_height=scroll_height;
        m_button_height=button_height;
        update_rects();
    }

    virtual void set_entry_height(uint height )
    {
        m_entry_height=height;
        update_rects();
    }

public:
    struct event_data: public layout::event_data
    {
        std::string element;
        int idx;

        event_data(): idx(-1) {}
    };

protected:
    typedef event_data_allocator<event_data> list_event_data;

protected:
    virtual void draw(layer &l) {}
    virtual void update_rects();

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
    list(): m_scroll(0), m_scroll_max(0),  m_mouse_x(0), m_mouse_y(0),
            m_mouse_hold_y(0), m_scrolling(false), m_selected(-1), m_mover(-1)
    {
        m_scroll_area_width=12;
        m_scroll_width=m_scroll_area_width;
        m_scroll_height=20;
        m_button_height=16;
        m_entry_height=18;
    }

protected:
    uint m_scroll_area_width;
    uint m_scroll_width;
    uint m_scroll_height;
    uint m_button_height;
    uint m_entry_height;

protected:
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

protected:
    typedef std::string element;
    std::vector<element> m_elements;
    int m_selected;
    int m_mover;
};

}
