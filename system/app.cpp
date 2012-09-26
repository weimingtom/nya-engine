//https://code.google.com/p/nya-engine/

#include "app.h"
#include "system.h"

#include <string>

#ifdef _WIN32

#include <windows.h>
#include <windowsx.h>
#include "render/platform_specific_gl.h"
#include <gl/wglext.h>

namespace
{

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app_responder &app)
    {
        m_instance=GetModuleHandle(NULL);
        if(!m_instance)
            return;

        WNDCLASS wc;
        wc.cbClsExtra=0;
        wc.cbWndExtra=0;
        wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.hCursor=LoadCursor(NULL,IDC_ARROW);
        wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
        wc.hInstance=m_instance;
        wc.lpfnWndProc=wnd_proc;
        wc.lpszClassName=TEXT("nya_engine");
        wc.lpszMenuName=0;
        wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC;

        if(!RegisterClass(&wc))
            return;

        RECT rect = {x,y,x+w,y+h};
        AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,false);

        m_hwnd = CreateWindow(TEXT("nya_engine"),
                          TEXT("nya_engine"),
                          WS_OVERLAPPEDWINDOW,
                          rect.left,rect.top,
                          rect.right-rect.left,rect.bottom-rect.top,
                          NULL,NULL,m_instance, NULL);

        if(!m_hwnd)
            return;

        ShowWindow(m_hwnd,SW_SHOW);

        m_hdc=GetDC(m_hwnd);

        PIXELFORMATDESCRIPTOR pfd={0};
        pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion=1;
        pfd.dwFlags=PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW;
        pfd.iPixelType=PFD_TYPE_RGBA;
        pfd.cColorBits=24;
        pfd.cAlphaBits=8;
        pfd.cDepthBits=24;

        int pf=ChoosePixelFormat(m_hdc,&pfd);
        if(!pf)
            return;

        if(!SetPixelFormat(m_hdc,pf,&pfd))
            return;

        m_hglrc=wglCreateContext(m_hdc);
        if(!m_hglrc)
            return;

        wglMakeCurrent(m_hdc,m_hglrc);

        if(antialiasing>0)
        {
            if(!nya_render::has_extension("GL_ARB_multisample"))
            {
                //antialiasing=0;
                nya_system::get_log()<<"GL_ARB_multisample not found\n";
            }
        }

        PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB=0;
        if(antialiasing>0)
        {
            wglChoosePixelFormatARB =
            (PFNWGLCHOOSEPIXELFORMATARBPROC)nya_render::get_extension("wglChoosePixelFormatARB");
            if(!wglChoosePixelFormatARB)
            {
                antialiasing=0;
                nya_system::get_log()<<"wglChoosePixelFormatARB not found\n";
            }
        }

        UINT num_aa_formats=0;
        int aa_pf=0;

        if(antialiasing>0)
        {
            int iAttributes[] = { WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
            WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,24,
            WGL_ALPHA_BITS_ARB,8,
            WGL_DEPTH_BITS_ARB,24,
            WGL_STENCIL_BITS_ARB,0,
            WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
            WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
            WGL_SAMPLES_ARB,antialiasing,
            0,0};

            nya_system::get_log()<<"antialiasing init\n";

            if(!wglChoosePixelFormatARB(m_hdc,iAttributes,0,1,&aa_pf,&num_aa_formats))
            {
                nya_system::get_log()<<"wglChoosePixelFormatARB failed\n";
                antialiasing=0;
            }
        }

        if(antialiasing>0)
        {
            wglMakeCurrent (m_hdc, 0);
            wglDeleteContext(m_hglrc);
            ReleaseDC(m_hwnd,m_hdc);
            DestroyWindow (m_hwnd);

            m_hwnd = CreateWindow(TEXT("nya_engine"),
                          TEXT("nya_engine"),
                          WS_OVERLAPPEDWINDOW,
                          rect.left,rect.top,
                          rect.right-rect.left,rect.bottom-rect.top,
                          NULL,NULL,m_instance, NULL);

            ShowWindow(m_hwnd,SW_SHOW);
            m_hdc=GetDC(m_hwnd);

            if(num_aa_formats>=1 && SetPixelFormat(m_hdc,aa_pf,&pfd))
            {
                nya_system::get_log()<<"antialiasiing is set\n";
            }
            else
            {
                antialiasing=0;
                nya_system::get_log()<<"unable to set antialiasiing "<<aa_pf<<" "<<num_aa_formats<<"\n";

                int pf=ChoosePixelFormat(m_hdc,&pfd);
                if(!pf)
                    return;

                if(!SetPixelFormat(m_hdc,pf,&pfd))
                    return;
            }

            m_hglrc=wglCreateContext(m_hdc);
            if(!m_hglrc)
                return;

            wglMakeCurrent(m_hdc,m_hglrc);
        }

        if(antialiasing>0)
            glEnable(GL_MULTISAMPLE_ARB);

        SetWindowLongPtr(m_hwnd,GWL_USERDATA,(LONG)&app);

        app.on_resize(w,h);
        app.on_init_splash();
        m_time=nya_system::get_time();
        update_splash(app);
        app.on_init();

        m_time=nya_system::get_time();

        MSG msg;
        while(m_hwnd)
        {
            if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
            {
                if(msg.message==WM_QUIT)
                    break;

                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                unsigned long time=nya_system::get_time();
                unsigned int dt=(unsigned)(time-m_time);
                m_time=time;

                app.on_process(dt);
                app.on_draw();

                SwapBuffers(m_hdc);
            }
        }

        finish(app);
    }

    void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo
    }

    void finish(nya_system::app_responder &app)
    {
        if(!m_hwnd)
            return;

        app.on_free();

        wglMakeCurrent (m_hdc, 0);
        wglDeleteContext(m_hglrc);
        ReleaseDC(m_hwnd,m_hdc);
        DestroyWindow (m_hwnd);

        m_hwnd=0;
    }

private:
    static LRESULT CALLBACK wnd_proc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
    {
        nya_system::app_responder *app=(nya_system::app_responder*)GetWindowLongPtr(hwnd,GWL_USERDATA);
        if(!app)
            return DefWindowProc(hwnd,message,wparam,lparam );

        switch(message)
        {
            case WM_SIZE:
            {
                RECT rc;
                GetClientRect(hwnd,&rc);

                app->on_resize(rc.right-rc.left,rc.bottom-rc.top);
            }
            break;

            case WM_CLOSE:
            {
                get_app().finish(*app);
            }
            break;

            case WM_MOUSEWHEEL:
            {
                const int x=GET_X_LPARAM(wparam);
                const int y=GET_Y_LPARAM(wparam);

                app->on_mouse_scroll(x/60,y/60);
            }
            break;

            case WM_MOUSEMOVE:
            {
                const int x=LOWORD(lparam);
                const int y=HIWORD(lparam);

                RECT rc;
                GetClientRect(hwnd,&rc);

                app->on_mouse_move(x,rc.bottom+rc.top-y);
            }
            break;

            case WM_LBUTTONDOWN:
            {
                app->on_mouse_button(nya_system::mouse_left,true);
            }
            break;

            case WM_LBUTTONUP:
            {
                app->on_mouse_button(nya_system::mouse_left,false);
            }
            break;

            case WM_RBUTTONDOWN:
            {
                app->on_mouse_button(nya_system::mouse_right,true);
            }
            break;

            case WM_RBUTTONUP:
            {
                app->on_mouse_button(nya_system::mouse_right,false);
            }
            break;
        };

        return DefWindowProc(hwnd,message,wparam,lparam );
    }

public:
    void set_title(const char *title)
    {
        if(!title)
        {
            m_title.clear();
            return;
        }

        m_title.assign(title);

        //ToDo
    }

    void update_splash(nya_system::app_responder &app)
    {
        unsigned long time=nya_system::get_time();
        unsigned int dt=(unsigned)(time-m_time);
        m_time=time;

        app.on_splash(dt);
        SwapBuffers(m_hdc);
    }

public:
    static shared_app &get_app()
    {
        static shared_app app;
        return app;
    }

public:
    shared_app():m_hdc(0),m_title("Nya engine"),m_time(0) {}

private:
    HINSTANCE m_instance;
    HWND m_hwnd;
    HDC m_hdc;
    HGLRC m_hglrc;

    std::string m_title;
    unsigned long m_time;
};

}

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
    void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app_responder &app)
    {
        if(m_dpy)
            return;

        m_dpy=XOpenDisplay(NULL);
        if(!m_dpy)
        {
            nya_system::get_log()<<"unable to open x display\n";
            return;
        }

        int dummy;
        if(!glXQueryExtension(m_dpy,&dummy,&dummy))
        {
            nya_system::get_log()<<"unable to querry glx extension\n";
            return;
        }

        static int dbl_buf[]={GLX_RGBA,GLX_DEPTH_SIZE,24,GLX_DOUBLEBUFFER,None};

        static int dbl_buf_aniso[]={GLX_RGBA,GLX_DEPTH_SIZE,24,GLX_DOUBLEBUFFER,
                        GLX_SAMPLE_BUFFERS_ARB,1,GLX_SAMPLES,antialiasing,None};

        XVisualInfo *vi=0;
        if(antialiasing>0)
        {
            vi=glXChooseVisual(m_dpy,DefaultScreen(m_dpy),dbl_buf_aniso);
            if(!vi)
            {
                nya_system::get_log()<<"unable to set antialising\n";
                antialiasing=0;
            }
        }

        if(antialiasing<=0)
            vi=glXChooseVisual(m_dpy,DefaultScreen(m_dpy),dbl_buf);

        if(!vi)
        {
            nya_system::get_log()<<"unable to choose glx visual\n";
            return;
        }

        if(vi->c_class!=TrueColor)
        {
            nya_system::get_log()<<"device does not support TrueColor\n";
            return;
        }

        m_cx=glXCreateContext(m_dpy,vi,None,GL_TRUE);
        if(!m_cx)
        {
            nya_system::get_log()<<"unable to ceate glx context\n";
            return;
        }

        Colormap cmap=XCreateColormap(m_dpy,RootWindow(m_dpy,vi->screen),vi->visual,AllocNone);

        XSetWindowAttributes swa;
        swa.colormap=cmap;
        swa.border_pixel=0;
        swa.event_mask=KeyPressMask| ExposureMask|ButtonPressMask|
                       StructureNotifyMask|ButtonReleaseMask | PointerMotionMask;

        m_win=XCreateWindow(m_dpy,RootWindow(m_dpy,vi->screen),x,y,
                  w,h,0,vi->depth,InputOutput,vi->visual,
                  CWBorderPixel|CWColormap|CWEventMask,&swa);

        XSetStandardProperties(m_dpy,m_win,m_title.c_str(),m_title.c_str(),None,0,0,NULL);
        glXMakeCurrent(m_dpy,m_win,m_cx);
        XMapWindow(m_dpy,m_win);

        if(antialiasing>0)
            glEnable(GL_MULTISAMPLE_ARB);

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

        finish(app);
    }

    void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo
    }

    void finish(nya_system::app_responder &app)
    {
        if(!m_dpy || !m_cx)
            return;

        app.on_free();

        if(!glXMakeCurrent(m_dpy,None,NULL))
        {
            nya_system::get_log()<<"Could not release drawing context.\n";
            return;
        }

        glXDestroyContext(m_dpy,m_cx);
        m_cx=0;
        m_dpy=0;
    }

    void set_title(const char *title)
    {
        if(!title)
        {
            m_title.clear();
            return;
        }

        m_title.assign(title);

        if(!m_dpy || !m_win)
            return;

        XSetStandardProperties(m_dpy,m_win,title,title,None,0,0,NULL);
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
    shared_app():m_dpy(0),m_win(0),m_title("Nya engine"),m_time(0) {}

private:
    Display *m_dpy;
    Window m_win;
    GLXContext m_cx;
    std::string m_title;
    unsigned long m_time;
};

}

#endif

namespace nya_system
{

void app::start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing)
{
    shared_app::get_app().start_windowed(x,y,w,h,antialiasing,*this);
}

void app::start_fullscreen(unsigned int w,unsigned int h)
{
    shared_app::get_app().start_fullscreen(w,h,*this);
}

void app::set_title(const char *title)
{
    shared_app::get_app().set_title(title);
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
