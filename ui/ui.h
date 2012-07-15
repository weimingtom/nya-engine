//https://code.google.com/p/nya-engine/

/*
    left-bottom alligned
*/

#ifndef ui_h
#define ui_h

#include "log/log.h"

#include <string>
#include <list>
#include <deque>

namespace nya_ui
{

void set_log(nya_log::log *l);
nya_log::log &get_log();

typedef unsigned int uint;

uint clamp(int v,uint from,uint to);

struct rect
{
    uint x;
    uint y;
    uint w;
    uint h;

    bool check_point(uint px, uint py)
    {
        if(px<x) return false;
        if(py<y) return false;
        if(px>x+w) return false;
        if(py>y+h) return false;

        return true;
    }

    rect(): x(0),y(0),w(0),h(0) {}
    rect(uint x,uint y,uint w,uint h):
                x(x),y(y),w(w),h(h) {}
};

class layer;
class widget;
class layout
{
public:
    virtual void add_widget(widget &w);
    virtual void draw_widgets(layer &l);
    virtual void resize(uint width,uint height);
    virtual void move(int x,int y);
    virtual void remove_widget() {}

public:
    enum button
    {
        left_button,
        middle_button,
        right_button
    };

    virtual bool mouse_button(button button,bool pressed);
    virtual bool mouse_move(uint x,uint y);
    virtual void mouse_left();
    virtual bool mouse_scroll(uint dx,uint dy);

public:
    struct event_data {};

    struct event
    {
        std::string sender;
        std::string type;

        event_data *data;

        virtual void free_data() {}

        event(): data(0) {}
    };

public:
    virtual void send_event(event &e) {}

    virtual void process_events(event &e);

public:
    layout(): m_x(0),m_y(0),m_width(0),m_height(0) {}

    //non copiable
private:
    layout(const layout &);
    void operator = (const layout &);

protected:
    typedef std::list<widget*> widgets_list;
    widgets_list m_widgets;
    int m_x;
    int m_y;
    uint m_width;
    uint m_height;

};

class widget
{
    friend class layout;

public:
    virtual void set_id(const char *id) { if(id) m_id.assign(id); }

    virtual void set_pos(int x,int y)
    {
        m_pos_left=x;
        m_pos_bottom=y;

        calc_pos_markers();
    }

    virtual void set_size(uint width,uint height)
    {
        m_width=width;
        m_height=height;

        calc_pos_markers();
    }

    virtual void set_visible(bool visible) { m_visible=visible; }

    virtual void set_align(bool left,bool right,bool top,bool bottom)
    {
        m_align_left=left;
        m_align_right=right;
        m_align_top=top;
        m_align_bottom=bottom;

        calc_pos_markers();
    }

    enum keep_aspect
    {
        none,
        from_width,
        from_height
    };

    virtual void set_keep_aspect(keep_aspect a)
    { m_keep_aspect=a; calc_pos_markers(); }

    virtual bool is_visible() { return m_visible; }

protected:
    virtual const char *get_id() { return m_id.c_str(); }

    virtual void get_pos(int &x,int &y)
    {
        x=m_pos_left;
        y=m_pos_bottom;
    }

    virtual void get_size(uint &width,uint &height)
    {
        width=m_width;
        height=m_height;
    }

    virtual void parent_moved(int x,int y)
    {
        m_parent_pos_x=x;
        m_parent_pos_y=y;

        m_cached_rect=false;
    }

    virtual void parent_resized(uint width,uint height)
    {
        m_parent_width=width;
        m_parent_height=height;

        m_cached_rect=false;
    }

protected:
    virtual void on_mouse_over() {}
    virtual void on_mouse_left() {}
    virtual bool on_mouse_move(uint x,uint y,bool inside) {}
    virtual bool on_mouse_button(layout::button button,bool pressed) {}
    virtual bool on_mouse_scroll(uint x,uint y) {}

protected:
    virtual void draw(layer &l) {}
    virtual void process_events(layout::event &e) {}

protected:
    virtual void send_event(const char *id,layout::event &e)
    {
        if(!m_parent || !id)
            return;

        e.sender.assign(id);
        m_parent->send_event(e);
    }

protected:
    virtual rect get_draw_rect()
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

protected:
    virtual void calc_pos_markers()
    {
        if(m_align_right)
            m_pos_right=m_parent_width-m_width-m_pos_left;
        else if(!m_align_left)
            m_center_x=m_parent_width/2-(m_pos_left+m_width/2);

        if(m_align_top)
            m_pos_top=m_parent_height-m_height-m_pos_bottom;
        else if(!m_align_bottom)
            m_center_y=m_parent_height/2-(m_pos_bottom+m_height/2);

        m_cached_rect=false;
    }

public:
    widget(): m_pos_left(0),m_pos_right(0),m_pos_top(0),m_pos_bottom(0),
              m_center_x(0),m_center_y(0),m_width(0),m_height(0),
              m_parent(0), m_parent_pos_x(0),m_parent_pos_y(0),
              m_parent_width(0),m_parent_height(0),
              m_align_left(true),m_align_right(false),
              m_align_top(false),m_align_bottom(true),
              m_keep_aspect(none),m_visible(true),
              m_mouse_over(false), m_cached_rect(false) {}

    //non copiable
private:
	widget(const widget &);
	void operator = (const widget &);

protected:
    std::string m_id;

    int m_pos_left;
    int m_pos_right;
    int m_pos_top;
    int m_pos_bottom;
    int m_center_x;
    int m_center_y;

    uint m_width;
    uint m_height;

    layout *m_parent;

    int m_parent_pos_x;
    int m_parent_pos_y;
    uint m_parent_width;
    uint m_parent_height;

    bool m_align_left;
    bool m_align_right;
    bool m_align_top;
    bool m_align_bottom;

    keep_aspect m_keep_aspect;

    bool m_visible;
    bool m_mouse_over;
    bool m_mouse_pressed;

    bool m_cached_rect;
    rect m_rect;
};

class layer: public layout
{
public:
    virtual void draw();
    virtual void resize(uint width,uint height);
    virtual void process();

private:
    virtual void process_events(event &e) {}

public:
    virtual void send_event(event &e);

public:
    struct color
    {
        float r;
        float g;
        float b;
        float a;

        void set(float r,float g,float b,float a=1.0f)
        {
            this->r=r;
            this->g=g;
            this->b=b;
            this->a=a;
        }

        color(): r(0),g(0),b(0),a(1.0f) {}
        color(float r,float g,float b,float a=1.0f):
                            r(r), g(g), b(b), a(a) {}
    };

    struct font
    {
        //std::string name;
        uint font_width;
        uint font_height;
        uint char_size;
        uint char_offs;
    };
    
    enum font_align
    {
        left,
        right,
        top,
        bottom,
        center
    };

    virtual void draw_text(uint x,uint y,const char *text
                   ,font_align aligh_hor=left,font_align aligh_vert=bottom);
    virtual void draw_text(uint x,uint y,const char *text,font &f
                   ,font_align aligh_hor=left,font_align aligh_vert=bottom) {}

    struct rect_style
    {
        bool border;
        bool solid;

        color border_color;
        color solid_color;

        //std::string tex_name;
        //rect tc_rect;

        rect_style(): border(false), solid(false) {}
    };

    virtual void draw_rect(rect &r,rect_style &s);
    virtual void set_scissor(rect &r);
    virtual void remove_scissor();

    virtual uint get_width() { return m_width; }
    virtual uint get_height() { return m_height; }

public:
    layer(): m_width(0), m_height(0) {}

private:
    uint m_width;
    uint m_height;

private:
    typedef std::deque<event> events_deque;
    events_deque m_events;

    //font m_default_font;
};

}

#endif
