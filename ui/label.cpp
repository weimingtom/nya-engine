//https://code.google.com/p/nya-engine/

#include "ui/label.h"

namespace nya_ui
{

void label::draw(layer &layer)
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    if(m_text.empty())
        return;

    layer.draw_text(r.x+r.w/2,r.y+r.h/2,m_text.c_str(),layer::center,layer::center);
}

}
