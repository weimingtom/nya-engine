//https://code.google.com/p/nya-engine/

#include "panel.h"

/*
    ToDo: rect_style customisation
*/

namespace nya_ui
{

void panel::draw(layer &layer)
{
    rect r=get_draw_rect();
    if(!r.w || !r.h)
        return;

    layer::rect_style rs;

    rs.border_color.set(0.4,0.3,1.0,1.0);
    rs.border=true;
    //rs.solid_color.set(0.9,0.9,0.9,0.8);
    //rs.solid=true;

    layer.draw_rect(r,rs);

    layout::draw_widgets(layer);
}

void panel::set_pos(int x,int y)
{
    widget::set_pos(x,y);
    update_layout_rect();
}

void panel::set_size(uint width,uint height)
{
    widget::set_size(width,height);
    update_layout_rect();
}

void panel::parent_resized(uint width,uint height)
{
    widget::parent_resized(width,height);
    update_layout_rect();
}

void panel::parent_moved(int x,int y)
{
    widget::parent_moved(x,y);
    update_layout_rect();
}

void panel::calc_pos_markers()
{
    widget::calc_pos_markers();
    update_layout_rect();
}

void panel::update_layout_rect()
{
    rect r=get_draw_rect();
    layout::move(r.x,r.y);
    layout::resize(r.w,r.h);
}

}
