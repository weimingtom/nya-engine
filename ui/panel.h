//https://code.google.com/p/nya-engine/

#ifndef panel_h
#define panel_h

#include "ui/ui.h"

namespace nya_ui
{

class panel: public widget, public layout
{
public:
    virtual void set_pos(int x,int y);
    virtual void set_size(uint width,uint height);

protected:
    virtual void draw(layer &l);
    virtual void parent_resized(uint width,uint height);
    virtual void parent_moved(int x,int y);
    virtual void calc_pos_markers();

protected:
    //virtual void on_mouse_over();
    virtual void on_mouse_left() { layout::mouse_left(); }
    virtual void on_mouse_move(uint x,uint y) { layout::mouse_move(x,y); }

    virtual void on_mouse_scroll(uint dx,uint dy) 
    { 
        layout::mouse_scroll(dx,dy); 
    }

    virtual void on_mouse_button(layout::button button,bool pressed)
    {
        layout::mouse_button(button,pressed);
    }

protected:
    void update_layout_rect();
};

}

#endif
