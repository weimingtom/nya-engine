//https://code.google.com/p/nya-engine/

#ifndef button_h
#define button_h

#include "ui/ui.h"

namespace nya_ui
{

class button: public widget
{
private:
    virtual void draw(layer &l);

    virtual void on_mouse_over()
    {
        layout::event e;
        e.type="mouse_over";
        send_event(get_id(),e);
    }

    virtual void on_mouse_left()
    {
        layout::event e;
        e.type="mouse_left";
        send_event(get_id(),e);
    }

    virtual void on_mouse_button(layout::button button,bool pressed)
    {
        layout::event e;

        if(pressed)
            e.type="mouse_left_btn_down";
        else
            e.type="mouse_left_btn_up";

        send_event(get_id(),e);
    }
};

}

#endif
