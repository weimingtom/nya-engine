//https://code.google.com/p/nya-engine/

#ifndef ui_h
#define ui_h

#include "log/log.h"

namespace nya_ui
{

typedef unsigned int uint;

class widget
{
public:
    void set_pos(int x,int y) {}
    void set_size(uint width,uint height) {}
    void draw() {}

private:
    void get_pos(int &x, int &y) {}
    void get_size(uint &width,uint &height) {}
    void parent_resized(uint new_width, uint new_height, 
                        uint old_width, uint old_height ) {}
};

void set_log(nya_log::log *l);
nya_log::log &get_log();

}

#endif
