//https://code.google.com/p/nya-engine/

#include "ui/list.h"

namespace nya_ui
{

void list::draw(layer &layer)
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    layer.draw_rect(r,m_style.list);

    rect scroll_area;
    scroll_area.w=m_style.scroll_width;
    scroll_area.h=r.h-m_style.button_height*2;
    scroll_area.x=r.x+r.w-m_style.scroll_width;
    scroll_area.y=r.y+m_style.button_height;

    layer.draw_rect(scroll_area,m_style.scroll_area);

    rect button;
    button.w=scroll_area.w;
    button.x=scroll_area.x;
    button.h=m_style.button_height;
    button.y=r.y;

    layer.draw_rect(button,m_style.button);

    rect button_up=button;
    button.y=r.y+r.h-m_style.button_height;

    layer.draw_rect(button,m_style.button);
}

}
