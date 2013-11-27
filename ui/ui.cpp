//https://code.google.com/p/nya-engine/

#include "ui/ui.h"

#include <string>

namespace
{
    nya_log::log *ui_log=0;
}

namespace nya_ui
{

uint clamp(int v,uint from,uint to)
{
    if(v>(int)to) v=to;
    if(v<(int)from) v=from;
    return v;
}

float clamp(float v,float from,float to)
{
    if(v>(int)to) v=to;
    if(v<(int)from) v=from;
    return v;
}

const rect &widget::get_rect()
{
    if(m_cached_rect)
        return m_rect;

    int x,y,w,h;
    if(m_align_left)
    {
        x=m_pos_left+m_parent_pos_x;
        if(m_align_right)
            w=m_parent_width-m_pos_right-m_pos_left;
        else
            w=m_width;
    }
    else if(m_align_right)
    {
        x=m_parent_pos_x+m_parent_width-m_width-m_pos_right;
        w=m_width;
    }
    else
    {
        x=m_parent_pos_x+m_parent_width/2-m_center_x-m_width/2;
        w=m_width;
    }
    
    if(m_align_bottom)
    {
        y=m_pos_bottom+m_parent_pos_y;
        if(m_align_top)
            h=m_parent_height-m_pos_top-m_pos_bottom;
        else
            h=m_height;
    }
    else if(m_align_top)
    {
        y=m_parent_pos_y+m_parent_height-m_height-m_pos_top;
        h=m_height;
    }
    else
    {
        y=m_parent_pos_y+m_parent_height/2-m_center_y-m_height/2;
        h=m_height;
    }
    
    if(m_keep_aspect==from_width)
    {
        h=w*m_height/m_width;
    }
    else if(m_keep_aspect==from_height)
    {
        w=h*m_width/m_height;
    }
    
    if(x<m_parent_pos_x)
        x=m_parent_pos_x;
    if(y<m_parent_pos_y)
        y=m_parent_pos_y;
    
    const int cw=m_parent_pos_x-x+m_parent_width;
    if(w>cw)
        w=cw;
    
    const int ch=m_parent_pos_y-y+m_parent_height;
    if(h>ch)
        h=ch;
    
    m_rect=rect();
    
    if(x>0) m_rect.x=x;
    if(y>0) m_rect.y=y;
    if(w>0) m_rect.w=w;
    if(h>0) m_rect.h=h;
    
    m_cached_rect=true;
    
    return m_rect;
}

void layer::draw()
{
    draw_widgets(*this);
}

void layer::resize(uint width, uint height)
{
    m_width=width;
    m_height=height;

    layout::resize(m_width,m_height);
}

void layer::process()
{
    events_deque events=m_events;
    m_events.clear();

    for(events_deque::iterator it=events.begin();
        it!=events.end();++it)
    {
        //get_log()<<"event: "<<it->sender.c_str()<<" "<<it->type.c_str()<<"\n";
        process_events(*it);

        //layout::process_events(*it);
    }
}

void layout::process_events(layout::event &e)
{
    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
    {
        widget *w=*it;
        w->process_events(e);
    }
}

void layout::add_widget(widget &w)
{
    w.parent_moved(m_x,m_y);
    w.parent_resized(m_width,m_height);
    w.calc_pos_markers();

    w.m_parent=this;

    m_widgets.push_back(&w);
}

void layout::draw_widgets(layer &l)
{
    if(!m_width || !m_height)
        return;

    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
    {
        widget *w=*it;
        if(w->m_visible)
            w->draw(l);
    }
}

void layout::resize(uint width,uint height)
{
    m_width=width;
    m_height=height;

    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
        (*it)->parent_resized(m_width,m_height);
}

void layout::move(int x,int y)
{
    m_x=x;
    m_y=y;

    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
        (*it)->parent_moved(m_x,m_y);
}

bool layout::mouse_button(layout::mbutton button,bool pressed)
{
    bool processed=false;

    for(widgets_list::reverse_iterator it=m_widgets.rbegin();
          it!=m_widgets.rend();++it)
    {
        widget *w=*it;
        if(!w->is_visible())
            continue;

        if(((w->m_mouse_over && !processed) || !pressed)
            && w->m_mouse_pressed!=pressed)
        {
            w->on_mouse_button(button,pressed);
            w->m_mouse_pressed=pressed;
            if(pressed)
                processed=true;
        }
    }
    //get_log()<<"mbutton"<<(int)button<<" "<<(int)pressed<<"\n";

    return processed;
}

bool layout::mouse_move(uint x,uint y)
{
    m_mouse_x=x;
    m_mouse_y=y;

    bool processed=false;

    for(widgets_list::reverse_iterator it=m_widgets.rbegin();
          it!=m_widgets.rend();++it)
    {
        widget *w=*it;
        if(!w->is_visible())
            continue;

        w->update_mouse_over();

        if(!processed && w->on_mouse_move(x,y,w->is_mouse_over()))
            processed=true;
    }
    //get_log()<<"mmove "<<(int)x<<" "<<(int)y<<"\n";

    return processed;
}

void layout::mouse_left()
{
    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
    {
        widget *w=*it;
        if(w->m_mouse_over)
        {
            w->on_mouse_left();
            w->m_mouse_over=false;
        }
    }
}

bool layout::mouse_scroll(uint dx,uint dy)
{
    bool processed=false;
    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
    {
        widget *w=*it;
        if(!w->is_visible())
            continue;

        if(w->m_mouse_over)
        {
            if(w->on_mouse_scroll(dx,dy))
                processed=true;
        }
    }

    return processed;
}

void layer::send_event(event &e)
{
    m_events.push_back(e);

    const uint msg_limit=1024;

    if(m_events.size()>msg_limit)
        m_events.pop_front();
}

void set_log(nya_log::log *l)
{
    ui_log = l;
}

nya_log::log &get_log()
{
    static const char *ui_log_tag="ui";
    if(!ui_log)
    {
        return nya_log::get_log(ui_log_tag);
    }

    ui_log->set_tag(ui_log_tag);
    return *ui_log;
}

}
