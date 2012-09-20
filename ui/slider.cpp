//https://code.google.com/p/nya-engine/

#include "slider.h"

namespace nya_ui
{

void slider::draw(layer &layer)
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    layer.draw_rect(r,m_style.area);
    layer.draw_rect(m_slider_rect,m_style.slider);
}

void slider::update_rects()
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    m_slider_rect=r;

    if(m_vertical)
    {
        m_slider_rect.h=m_style.size;
        m_slider_rect.y=r.y+(r.h-m_style.size)*m_value;
    }
    else
    {
        m_slider_rect.w=m_style.size;
        m_slider_rect.x=r.x+(r.w-m_style.size)*m_value;
    }
}

bool slider::on_mouse_move(uint x,uint y,bool inside)
{
    float last_value=m_value;

    if(m_vertical)
    {
        if(m_mouse_pressed)
        {
            rect r=get_draw_rect();
            if(r.h<1)
                return false;

            const float new_value=clamp((y-r.y),0,r.h);
            m_value=new_value/r.h;
        }

        m_mouse_last=y;
    }
    else
    {
        if(m_mouse_pressed)
        {
            rect r=get_draw_rect();
            if(r.w<1)
                return false;

            const float new_value=clamp((x-r.x),0,r.w);
            m_value=new_value/r.w;
        }

        m_mouse_last=x;
    }

    if(m_value!=last_value)
    {
        if(has_events())
        {
            layout::event e;

            e.type="value_changed";
            event_data *data=slider_event_data::create();

            data->value=m_value;
            e.data=data;

            send_event(get_id(),e);
        }

        update_rects();
    }

    return inside;
}

bool slider::on_mouse_button(layout::button button,bool pressed)
{
    m_mouse_pressed=pressed;

    if(m_vertical)
        on_mouse_move(0,m_mouse_last,true);
    else
        on_mouse_move(m_mouse_last,0,true);

    return true;
}

bool slider::on_mouse_scroll(uint x,uint y)
{
    /*
    rect r=get_draw_rect();
    if(r.h<1)
        return false;

    m_value=clamp(m_value+float(y)/r.h,0,1.0f);
    update_rects();
*/
    return true;
}

}
