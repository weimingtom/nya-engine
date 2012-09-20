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

    if(m_button_up_rect.check_point(m_mouse_x,m_mouse_y))
        layer.draw_rect(m_button_up_rect,m_style.button_up_hl);
    else
        layer.draw_rect(m_button_up_rect,m_style.button_up);

    if(m_button_down_rect.check_point(m_mouse_x,m_mouse_y))
        layer.draw_rect(m_button_down_rect,m_style.button_dn_hl);
    else
        layer.draw_rect(m_button_down_rect,m_style.button_dn);

    rect er;
    er.h=m_style.entry_height;
    er.w=r.w-m_scroll_area_rect.w;
    er.x=r.x;

    if(er.h*m_elements.size()>r.h)
    {
        if(m_scroll_rect.check_point(m_mouse_x,m_mouse_y))
            layer.draw_rect(m_scroll_rect,m_style.scroll_hl);
        else
            layer.draw_rect(m_scroll_rect,m_style.scroll);
    }

    layer.set_scissor(r);


    long y=r.y+r.h-er.h;
    if(m_scroll_max)
        y+=(er.h*m_elements.size()-r.h)*m_scroll/m_scroll_max;

    for(int i=0;i<(int)m_elements.size();++i,y-=m_style.entry_height)
    {
        if(y+er.h<r.y||y>(int)(r.y+r.h))
            continue;

        er.y=(uint)y;

        if(i==m_mover && !m_mouse_pressed)
            layer.draw_rect(er,m_style.entry_hl);
        else if(i==m_selected)
            layer.draw_rect(er,m_style.entry_selected);
        else
            layer.draw_rect(er,m_style.entry);

        layer.draw_text(er.x,er.y+er.h/2,m_elements[i].c_str(),layer::left,layer::center);
    }

    layer.remove_scissor();
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

bool list::on_mouse_move(uint x,uint y,bool inside)
{
    m_mouse_x=x;
    m_mouse_y=y;

    bool mleft=false;
    if(inside && x<m_scroll_area_rect.x)
    {
        rect r=get_draw_rect();

        int scrl=0;
        if(m_scroll_max>0)
            scrl=(int)(m_style.entry_height*m_elements.size()-r.h)*m_scroll/m_scroll_max;

        int num=0;
        if(m_style.entry_height>0)
            num=(r.h-(m_mouse_y-r.y)+scrl)/m_style.entry_height;

        if(num<(int)m_elements.size())
        {
            if(num!=m_mover)
            {
                if(has_events())
                {
                    //get_log()<<"Mover: "<<num<<" "<<m_elements[num].c_str()<<"\n";
                    layout::event e;

                    e.type="mover_element";
                    event_data *data=list_event_data::create();

                    data->element=m_elements[num];
                    data->idx=num;

                    e.data=data;

                    send_event(get_id(),e);
                }

                m_mover=num;
            }
        }
        else
            mleft=true;
    }
    else
        mleft=true;

    if(mleft && m_mover>=0)
    {
        if(has_events())
        {
            //get_log()<<"Mleft\n";
            layout::event e;
            e.type="mleft_elements";
            send_event(get_id(),e);
        }

        m_mover=-1;
    }

    if(m_scrolling)
    {
        const int new_scroll=m_scroll_max-(m_mouse_y
                -m_scroll_area_rect.y-m_scroll_rect.h/2);
        m_scroll=clamp(new_scroll,0,m_scroll_max);
        update_rects();

        //m_scroll_abs=(m_style.entry_height*m_elements.size()-r.h)*m_scroll/m_scroll_max;

        return true;
    }

    return false;
}

bool list::on_mouse_button(layout::button button,bool pressed)
{
    m_scrolling=false;

    if(pressed)
    {
        rect r=get_draw_rect();

        if(m_style.entry_height*m_elements.size()>r.h)
        {
            if(m_scroll_rect.check_point(m_mouse_x,m_mouse_y)
               || m_scroll_area_rect.check_point(m_mouse_x,m_mouse_y))
            {
                m_scrolling=true;
                on_mouse_move(m_mouse_x,m_mouse_y,true);
                return true;
            }

            if(m_button_up_rect.check_point(m_mouse_x,m_mouse_y))
            {
                const int delta=(int)ceilf(m_scroll_max*0.1f);
                m_scroll=clamp(m_scroll-delta,0,m_scroll_max);
                update_rects();
                return true;
            }

            if(m_button_down_rect.check_point(m_mouse_x,m_mouse_y))
            {
                const int delta=(int)ceilf(m_scroll_max*0.1f);
                m_scroll=clamp(m_scroll+delta,0,m_scroll_max);
                update_rects();
                return true;
            }
        }

        if(m_mover>=0 && m_mover<(int)m_elements.size())
        {
            if(has_events())
            {
                //get_log()<<"Elem: "<<m_mover<<" "<<m_elements[m_mover].c_str()<<"\n";
                layout::event e;

                e.type="select_element";
                event_data *data=list_event_data::create();

                data->element=m_elements[m_mover];
                data->idx=m_mover;

                e.data=data;

                send_event(get_id(),e);
            }

            m_selected=m_mover;
        }

        m_mouse_hold_y=m_mouse_y;
    }

    return true;
}

bool list::on_mouse_scroll(uint x,uint y)
{
    rect r=get_draw_rect();

    if(m_style.entry_height*m_elements.size()>r.h)
    {
        const int delta=y;//(int)ceilf(m_scroll_max*0.01f)*y;
        m_scroll=clamp(m_scroll-delta,0,m_scroll_max);
        update_rects();

        on_mouse_move(m_mouse_x,m_mouse_y,true);
    }

    return true;
}

}
