//https://code.google.com/p/nya-engine/

#pragma once

#include "button_codes.h"

namespace nya_system
{

enum mouse_button
{
    mouse_left,
    mouse_middle,
    mouse_right
};

class app
{
public:
    virtual void on_init_splash() {}
    virtual void on_init() {}
    virtual void on_splash(unsigned int dt) {}
    virtual void on_frame(unsigned int dt) {}
    virtual void on_free() {}

public:
    virtual void on_suspend() {}
    virtual void on_restore() {}

public:
    virtual void on_mouse_move(int x,int y) {}
    virtual void on_mouse_button(mouse_button button,bool pressed) {}
    virtual void on_mouse_scroll(int dx,int dy) {}
    virtual void on_keyboard(unsigned int key,bool pressed) {}
    virtual void on_resize(unsigned int w,unsigned int h) {}

public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing);
    void start_fullscreen(unsigned int w,unsigned int h);

public:
    void set_title(const char *title);

public:
    void limit_framerate(unsigned int fps);
    void set_mouse_pos(int x,int y);
    void update_splash();
    void finish();
};

}

