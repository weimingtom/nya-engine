//https://code.google.com/p/nya-engine/

#ifndef list_h
#define list_h

#include "ui/ui.h"

namespace nya_ui
{

struct list_style
{
    uint scroll_area_width;
    uint scroll_width;
    uint button_height;
    uint entry_height;

    layer::rect_style list;
    layer::rect_style entry;
    layer::rect_style scroll;
    layer::rect_style scroll_area;
    layer::rect_style button;

    list_style()
    {
        scroll_area_width=16;
        scroll_width=12;
        button_height=scroll_area_width;
        entry_height=18;

        list.border=true;
        list.border_color.set(0.4,0.3,1.0,1.0);

        entry=scroll=scroll_area=button=list;
    }
};

class list: public widget
{
public:
    virtual void set_style(list_style &s)
    {
        m_style=s;
    }

private:
    virtual void draw(layer &l);

    list_style m_style;
};

}

#endif
