//https://code.google.com/p/nya-engine/

#pragma once

#include "ui/ui.h"

namespace nya_ui
{

class button: public widget
{
public:
    virtual void set_text(const char *text)
    {
        if(!text)
            return;

        m_text.assign(text);
    }

protected:
    virtual void draw(layer &l) {}

    virtual void on_mouse_over()
    {
        if(!has_events())
            return;

        layout::event e;
        e.type="mouse_over";
        send_event(get_id(),e);
    }

    virtual void on_mouse_left()
    {
        if(!has_events())
            return;

        layout::event e;
        e.type="mouse_left";
        send_event(get_id(),e);
    }

    virtual bool on_mouse_button(layout::mbutton button,bool pressed)
    {
        if(!has_events())
            return true;

        layout::event e;

        if(pressed)
            e.type="mouse_left_btn_down";
        else
            e.type="mouse_left_btn_up";

        send_event(get_id(),e);

        return true;
    }

protected:
    std::string m_text;
};

}
