//https://code.google.com/p/nya-engine/

#include "ui/ui.h"
#include "memory/tmp_buffer.h"
#include <string>

/*
    ToDo: abstract render
*/

#include "render/platform_specific_gl.h"

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

void layer::draw_text(uint x,uint y,const char *text
                      ,font_align aligh_hor,font_align aligh_vert)
{
    if(!text)
        return;

    std::string text_str(text);
    if(text_str.empty())
        return;

    const size_t str_len=text_str.size();

    //font_params
    const uint font_width=256;
    const uint font_height=128;
    const uint char_size=16;
    const uint char_offs=1;

    const uint char_actual_width=8;//bad magic: should
    uint char_widths[128-32]; //be font-defined array
    for(int i=0;i<128-32;++i)
    {
        char c=i+32;
        if(c=='i'||c=='j' || c=='l')
            char_widths[i]=5;
        else if(c=='m')
            char_widths[i]=10;
        else
            char_widths[i]=char_actual_width;
    }

    //
    const float font_scale=1.0f;

    //precomputed from font_params
    const int chars_per_row=font_width/char_size;
    const float tc_w=float(char_size)/font_width;
    const float tc_h=float(char_size)/font_height;

    const float offs_w=float(char_offs)/font_width;
    const float offs_h=float(char_offs)/font_height;

    float chs=2.0f*font_scale*(char_size-char_offs);

//=====

    const float w=chs/m_width;
    const float h=chs/m_height;

    if(aligh_hor==center)
        x-=int(0.25f*(chs*char_actual_width/char_size*str_len));
    else if(aligh_hor==right)
        x-=int(0.5f*(chs*char_actual_width/char_size*str_len));

    if(aligh_vert==center)
        y-=int(0.25f*chs);
    else if(aligh_vert==top)
        y-=int(0.5f*chs);

    float px=-1.0f+2.0f*x/m_width;
    float py=-1.0f+2.0f*y/m_height;

    struct vertex
    {
        float x;
        float y;
        float s;
        float t;
    };

    const uint elem_per_char=6;
    nya_memory::tmp_buffer_scoped vert_buf(text_str.size()*elem_per_char*sizeof(vertex));

    vertex *vertices=(vertex*)vert_buf.get_data();

    float dpos=0;
    for(size_t i=0;i<str_len;++i)
    {
        const char c=text_str[i];

        if(c<32 || c>127)
            continue;

        vertex *v=&vertices[elem_per_char*i];

        const uint char_width=char_widths[c-32];

        v[2].x=v[5].x=px+dpos;
        v[1].x=v[0].x=v[2].x+w*char_width/char_size;
        v[0].y=v[5].y=py;
        v[2].y=v[1].y=py+h;

        const uint letter_pos=c-32;
        const uint letter_x=(letter_pos)%chars_per_row;
        const uint letter_y=(letter_pos)/chars_per_row;

        const float tcx=tc_w*letter_x-offs_w;
        const float tcy=tc_h*letter_y;
        float tcw=tc_w-offs_w;
        float tch=tc_h-offs_h;

        const float tc_fix=0.5f*(tc_w-float(char_width)/font_width);

        v[5].s=v[2].s=tcx+tc_fix;
        v[1].s=v[0].s=tcx+tcw-tc_fix;
        v[5].t=v[0].t=tcy+tch;
        v[2].t=v[1].t=tcy;

        v[3]=v[0];
        v[4]=v[2];

        dpos+=w*char_width/char_size;
    }

    m_font_vbo.set_vertex_data(vert_buf.get_data(),sizeof(vertex),str_len*elem_per_char,nya_render::vbo::dynamic_draw);
    m_font_vbo.bind();
    m_font_vbo.draw();
    m_font_vbo.unbind();
}

void layer::draw_rect(rect &r,rect_style &s)
{
    if(!s.border&&!s.solid)
        return;

    float w = 2.0f*r.w/m_width;
    float h = 2.0f*r.h/m_height;

    float px=-1.0f+2.0f*r.x/m_width;
    float py=-1.0f+2.0f*r.y/m_height;

    float pos[8];
    pos[6]=pos[4]=px;
    pos[7]=pos[1]=py;
    pos[5]=pos[3]=h+py;
    pos[2]=pos[0]=w+px;

    m_rect_vbo.set_vertex_data(pos,sizeof(float)*2,4,nya_render::vbo::dynamic_draw);
    m_rect_vbo.set_vertices(0,2);
    m_rect_vbo.bind();

    if(s.solid)
    {
        glColor4f(s.solid_color.r,s.solid_color.g,
                  s.solid_color.b,s.solid_color.a);

        m_rect_vbo.set_element_type(nya_render::vbo::triangle_fan);

        m_rect_vbo.draw();
    }

    if(s.border)
    {
        glColor4f(s.border_color.r,s.border_color.g,
                  s.border_color.b,s.border_color.a);

        m_rect_vbo.set_element_type(nya_render::vbo::line_loop);
        m_rect_vbo.draw();
    }

    m_rect_vbo.unbind();
}

void layer::set_scissor(rect &r)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor(r.x,r.y,r.w,r.h);
}

void layer::remove_scissor()
{
    glDisable(GL_SCISSOR_TEST);
}

void layer::process()
{
    for(events_deque::iterator it=m_events.begin();
        it!=m_events.end();++it)
    {
        //get_log()<<"event: "<<it->sender.c_str()<<" "<<it->type.c_str()<<"\n";
        process_events(*it);

        //layout::process_events(*it);
        it->free_data();
    }

    m_events.clear();
}

void layer::release()
{
    m_rect_vbo.release();
    //m_font_vbo.release();
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

bool layout::mouse_button(layout::button button,bool pressed)
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
    bool processed=false;

    for(widgets_list::reverse_iterator it=m_widgets.rbegin();
          it!=m_widgets.rend();++it)
    {
        widget *w=*it;
        if(!w->is_visible())
            continue;

        bool inside=false;
        if(!processed && w->get_draw_rect().check_point(x,y))
        {
            if(!w->m_mouse_over)
            {
                w->on_mouse_over();
                w->m_mouse_over=true;
            }
            inside=true;
        }
        else if(w->m_mouse_over)
        {
            w->on_mouse_left();
            w->m_mouse_over=false;
        }

        if(!processed && w->on_mouse_move(x,y,inside))
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
