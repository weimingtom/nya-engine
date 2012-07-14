//https://code.google.com/p/nya-engine/

#include "ui/list.h"
#include "math.h"

namespace nya_ui
{

void list::draw(layer &layer)
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    layer.draw_rect(r,m_style.list);
    layer.draw_rect(m_scroll_area_rect,m_style.scroll_area);
    layer.draw_rect(m_button_up_rect,m_style.button);
    layer.draw_rect(m_button_down_rect,m_style.button);
    layer.draw_rect(m_scroll_rect,m_style.scroll);
}

void list::update_rects()
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    uint last_height=m_scroll_area_rect.h
                    -m_style.scroll_height;

    rect &scroll_area=m_scroll_area_rect;
    scroll_area.w=m_style.scroll_width;
    scroll_area.h=r.h-m_style.button_height*2;
    scroll_area.x=r.x+r.w-m_style.scroll_width;
    scroll_area.y=r.y+m_style.button_height;

    if(last_height)
        m_scroll=m_scroll*(scroll_area.h
                    -m_style.scroll_height)/last_height;
    else
        m_scroll=0;

    rect &button=m_button_down_rect;
    button.w=scroll_area.w;
    button.x=scroll_area.x;
    button.h=m_style.button_height;
    button.y=r.y;

    m_button_up_rect=m_button_down_rect;
    rect &button_up=m_button_up_rect;
    button_up.y=r.y+r.h-m_style.button_height;

    rect &scroll=m_scroll_rect;
    scroll.x=scroll_area.x+(m_style.scroll_area_width
                                -m_style.scroll_width)/2;
    scroll.y=r.y+r.h-m_style.button_height
                                -m_style.scroll_height-m_scroll;
    scroll.w=m_style.scroll_width;
    scroll.h=m_style.scroll_height;

    int scroll_max=scroll_area.h-m_style.scroll_height;
    if(scroll_max<0)
        scroll_max=0;
    m_scroll_max=scroll_max;
}

void list::on_mouse_move(uint x,uint y,bool inside)
{
    if(m_scrolling)
    {
        const int new_scroll=m_scroll_max-(m_mouse_y
                -m_scroll_area_rect.y-m_scroll_rect.h/2);
        m_scroll=clamp(new_scroll,0,m_scroll_max);
        update_rects();
    }

    m_mouse_x=x;
    m_mouse_y=y;
}

void list::on_mouse_button(layout::button button,bool pressed)
{
    m_scrolling=false;

    if(pressed)
    {
        if(m_scroll_rect.check_point(m_mouse_x,m_mouse_y)
           || m_scroll_area_rect.check_point(m_mouse_x,m_mouse_y))
        {
            m_scrolling=true;
            on_mouse_move(m_mouse_x,m_mouse_y,true);
        }

        if(m_button_up_rect.check_point(m_mouse_x,m_mouse_y))
        {
            const int delta=(int)ceilf(m_scroll_max*0.1f);
            m_scroll=clamp(m_scroll-delta,0,m_scroll_max);
            update_rects();
        }

        if(m_button_down_rect.check_point(m_mouse_x,m_mouse_y))
        {
            const int delta=(int)ceilf(m_scroll_max*0.1f);
            m_scroll=clamp(m_scroll+delta,0,m_scroll_max);
            update_rects();
        }

        m_mouse_hold_y=m_mouse_y;
    }
}

void list::on_mouse_scroll(uint x,uint y)
{
}

}
