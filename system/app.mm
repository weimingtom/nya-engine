//https://code.google.com/p/nya-engine/

#include "app.h"
namespace
{

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo
    }

    void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo
    }

    void finish(nya_system::app_responder &app)
    {
    }

    void update_splash(nya_system::app_responder &app)
    {
    }

public:
    static shared_app &get_app()
    {
        static shared_app app;
        return app;
    }

public:
    shared_app() {}

private:
};

}

namespace nya_system
{

void app::start_windowed(int x,int y,unsigned int w,unsigned int h)
{
    shared_app::get_app().start_windowed(x,y,w,h,*this);
}

void app::start_fullscreen(unsigned int w,unsigned int h)
{
    shared_app::get_app().start_fullscreen(w,h,*this);
}

void app::set_mouse_pos(int x,int y)
{
}

void app::update_splash()
{
    shared_app::get_app().update_splash(*this);
}

void app::finish()
{
    shared_app::get_app().finish(*this);
}

}
