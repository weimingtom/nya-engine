//https://code.google.com/p/nya-engine/

#include "ui/button.h"

namespace nya_ui
{

void button::draw(layer &layer)
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    layer::rect_style rs;

    if(m_mouse_over)
        rs.border_color.set(0.7,0.6,1.0,1.0);
    else
        rs.border_color.set(0.4,0.3,1.0,1.0);

    rs.border=true;

    layer.draw_rect(r,rs);
}

}
