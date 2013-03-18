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

    if(m_mouse_over && !m_mouse_pressed)
        rs.border_color.set(0.7f,0.6f,1.0f,1.0f);
    else
        rs.border_color.set(0.4f,0.3f,1.0f,1.0f);

    rs.border=true;

    rs.solid_color=rs.border_color;
    rs.solid_color.g+=0.05f;
    rs.solid_color.b-=0.1f;
    rs.solid_color.a=0.8f;
    rs.solid=true;

    layer.draw_rect(r,rs);
 
    if(!m_text.empty())
        layer.draw_text(r.x+r.w/2,r.y+r.h/2,m_text.c_str(),layer::center,layer::center);
}

}
