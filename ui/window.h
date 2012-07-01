//https://code.google.com/p/nya-engine/

#ifndef window_h
#define window_h

#include "ui/ui.h"

namespace nya_ui
{

class window: public widget, public layout
{
public:
    virtual void set_pos(int x,int y);
    virtual void set_size(uint width,uint height);

private:
    virtual void draw(layer &l);
    virtual void parent_resized(uint width,uint height);
    virtual void parent_moved(int x,int y);
    virtual void calc_pos_markers();

private:
    void update_layout_rect();
};

}

#endif
