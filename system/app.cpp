//https://code.google.com/p/nya-engine/

#include "app.h"
#include "system.h"

#ifdef _WIN32

#include <windows.h>

#else

//  fullscreen:
//#include <X11/Xlib.h>
//#include <X11/Xatom.h>
//#include <X11/extensions/xf86vmode.h> //libxxf86vm-dev libXxf86vm.a

#include <GL/glx.h>
#include <GL/gl.h>
#include <X11/X.h>
#include <X11/keysym.h>

namespace
{

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        if(m_dpy)
            return;

        m_dpy=XOpenDisplay(NULL);
        if(!m_dpy)
            return;

        int dummy;
        if(!glXQueryExtension(m_dpy,&dummy,&dummy))
            return;

        static int dbl_buf[]={GLX_RGBA,GLX_DEPTH_SIZE,16,GLX_DOUBLEBUFFER,None};
        XVisualInfo *vi=glXChooseVisual(m_dpy, DefaultScreen(m_dpy),dbl_buf);
        if(!vi)
            return;

        if(vi->c_class!=TrueColor)
            return;

        GLXContext cx=glXCreateContext(m_dpy,vi,None,GL_TRUE);
        if(!cx)
            return;

        Colormap cmap=XCreateColormap(m_dpy,RootWindow(m_dpy,vi->screen),vi->visual,AllocNone);

        XSetWindowAttributes swa;
        swa.colormap=cmap;
        swa.border_pixel=0;
        swa.event_mask=KeyPressMask| ExposureMask|ButtonPressMask|
                       StructureNotifyMask|ButtonReleaseMask | PointerMotionMask;

        m_win=XCreateWindow(m_dpy,RootWindow(m_dpy,vi->screen),x,y,
                  w,h,0,vi->depth,InputOutput,vi->visual,
                  CWBorderPixel|CWColormap|CWEventMask,&swa);

        XSetStandardProperties(m_dpy,m_win,"main","main",None,0,0,NULL);
        glXMakeCurrent(m_dpy,m_win,cx);
        XMapWindow(m_dpy, m_win);

        app.on_resize(w,h);
        app.on_init_splash();
        m_time=nya_system::get_time();
        update_splash(app);
        app.on_init();

        m_time=nya_system::get_time();

        XEvent event;
        while(true)
        {
            while(XPending(m_dpy))
            {
                XNextEvent(m_dpy, &event);
                switch (event.type)
                {
                    case ConfigureNotify:
                    {
                        w=event.xconfigure.width;
                        h=event.xconfigure.height;
                        glViewport(0,0,w,h);
                        app.on_resize(w,h);
                    }
                    break;

                    case MotionNotify:
                        app.on_mouse_move(event.xmotion.x,h-event.xmotion.y);
                    break;

                    case ButtonPress:
                    {
                        const int scroll_modifier=16;

                        switch (event.xbutton.button)
                        {
                            case 1:
                                app.on_mouse_button(nya_system::mouse_left,true);
                            break;

                            case 2:
                                app.on_mouse_button(nya_system::mouse_middle,true);
                            break;

                            case 3:
                                app.on_mouse_button(nya_system::mouse_right,true);
                            break;

                            case 4:
                                app.on_mouse_scroll(0,scroll_modifier);
                            break;

                            case 5:
                                app.on_mouse_scroll(0,-scroll_modifier);
                            break;

                            case 6:
                                app.on_mouse_scroll(scroll_modifier,0);
                            break;

                            case 7:
                                app.on_mouse_scroll(-scroll_modifier,0);
                            break;
                        }
                    }
                    break;

                    case ButtonRelease:
                        switch (event.xbutton.button)
                        {
                            case 1:
                                app.on_mouse_button(nya_system::mouse_left,false);
                            break;

                            case 2:
                                app.on_mouse_button(nya_system::mouse_middle,false);
                            break;

                            case 3:
                                app.on_mouse_button(nya_system::mouse_right,false);
                            break;
                        }
                    break;
                };
            }

            unsigned long time=nya_system::get_time();
            unsigned int dt=(unsigned)(time-m_time);
            m_time=time;

            app.on_process(dt);
            app.on_draw();

            glXSwapBuffers(m_dpy,m_win);
        }
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
        unsigned long time=nya_system::get_time();
        unsigned int dt=(unsigned)(time-m_time);
        m_time=time;

        app.on_splash(dt);
        glXSwapBuffers(m_dpy,m_win);
    }

public:
    static shared_app &get_app()
    {
        static shared_app app;
        return app;
    }

public:
    shared_app():m_dpy(0),m_win(0),m_time(0){}

private:
    Display *m_dpy;
    Window m_win;
    unsigned long m_time;
};

}

#endif

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
