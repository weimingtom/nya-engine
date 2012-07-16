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

    //
    const float font_scale=1.0f;

    //precomputed from font_params
    const int chars_per_row=font_width/char_size;
    const float tc_w=float(char_size)/font_width;
    const float tc_h=float(char_size)/font_height;

    const float offs_w=float(char_offs)/font_width;
    const float offs_h=float(char_offs)/font_height;

    float chs=2.0f*font_scale*(char_size-char_offs);


    const uint char_actual_width=8;//bad magic: should
    //be font-defined array
//=====

    const float w=chs/m_width;
    const float h=chs/m_height;

    float px=-1.0f+2.0f*x/m_width;
    float py=-1.0f+2.0f*y/m_height;

    if(aligh_hor==center)
        px-=0.5f*(w*char_actual_width/char_size*(str_len+1));
    else if(aligh_hor==right)
        px-=(w*char_actual_width/char_size*str_len);

    if(aligh_vert==center)
        py-=0.5f*h;
    else if(aligh_vert==top)
        py-=h;

    const uint elem_per_char=4;
    nya_memory::tmp_buffer_scoped vert_buf(text_str.size()*4*elem_per_char*sizeof(float));
    const size_t tc_buf_offset=str_len*sizeof(float)*2*elem_per_char;

    float dpos=0;
    for(size_t i=0;i<str_len;++i)
    {
        const size_t buf_offset=i*sizeof(float)*2*elem_per_char;
        float *pos=(float*)vert_buf.get_data(buf_offset);
        float *tc=(float*)vert_buf.get_data(tc_buf_offset+buf_offset);

        pos[6]=pos[4]=px+dpos;
        pos[2]=pos[0]=w+pos[6];
        pos[7]=pos[1]=py;
        pos[5]=pos[3]=h+py;

        const uint letter_pos=text_str[i]-32;
        const uint letter_x=(letter_pos)%chars_per_row;
        const uint letter_y=(letter_pos)/chars_per_row;

        const float tcx=tc_w*letter_x-offs_w;
        const float tcy=tc_h*letter_y-offs_h;
        float tcw=tc_w-offs_w;
        float tch=tc_h-offs_h;

        tc[6]=tc[4]=tcx;
        tc[2]=tc[0]=tcx+tcw;
        tc[7]=tc[1]=tcy+tch;
        tc[5]=tc[3]=tcy;

        //for(int i=1;i<8;i+=2)
        //    tc[i]=1.0f-(tc[i]+offs_h);

        dpos+=w*char_actual_width/char_size;
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2,GL_FLOAT,0,vert_buf.get_data());
    glTexCoordPointer(2,GL_FLOAT,0,vert_buf.get_data(tc_buf_offset));

    glDrawArrays(GL_QUADS,0,(GLsizei)str_len*elem_per_char);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
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

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2,GL_FLOAT,0,pos);

    if(s.solid)
    {
        glColor4f(s.solid_color.r,s.solid_color.g,
                  s.solid_color.b,s.solid_color.a);
        glDrawArrays(GL_QUADS,0,4);
    }

    if(s.border)
    {
        glColor4f(s.border_color.r,s.border_color.g,
                  s.border_color.b,s.border_color.a);
        glDrawArrays(GL_LINE_LOOP,0,4);
    }

    glDisableClientState(GL_VERTEX_ARRAY);
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

    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
    {
        widget *w=*it;
        if(!w->is_visible())
            continue;

        if((w->m_mouse_over || (!pressed && w->m_mouse_pressed))
            && w->m_mouse_pressed!=pressed)
        {
            w->on_mouse_button(button,pressed);
            w->m_mouse_pressed=pressed;
            processed=true;
        }
    }
    //get_log()<<"mbutton"<<(int)button<<" "<<(int)pressed<<"\n";

    return processed;
}

bool layout::mouse_move(uint x,uint y)
{
    bool processed=false;

    for(widgets_list::iterator it=m_widgets.begin();
        it!=m_widgets.end();++it)
    {
        widget *w=*it;
        if(!w->is_visible())
            continue;

        bool inside=false;
        if(w->get_draw_rect().check_point(x,y))
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
        if(w->on_mouse_move(x,y,inside))
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
