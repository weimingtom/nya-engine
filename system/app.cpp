//https://code.google.com/p/nya-engine/

#include "app.h"
#include "system.h"

#include <string>

#ifdef _WIN32
#define _WIN32_WINDOWS 0x501

#include "render/platform_specific_gl.h"

  #ifdef WINDOWS_PHONE8
    #include "render/render.h"

    #include <wrl/client.h>
    #include <ppl.h>
    #include <agile.h>
    #include <d3d11_1.h>

    using namespace Windows::ApplicationModel;
    using namespace Windows::ApplicationModel::Core;
    using namespace Windows::ApplicationModel::Activation;
    using namespace Windows::UI::Core;
    using namespace Windows::System;
    using namespace Windows::Foundation;
    using namespace Windows::Graphics::Display;
    using namespace concurrency;

namespace
{

ref class PhoneDirect3DApp sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
    friend ref class Direct3DApplicationSource;
public:
	//IFrameworkView.
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
    {
	    applicationView->Activated +=
		    ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &PhoneDirect3DApp::OnActivated);

	    CoreApplication::Suspending +=
		    ref new EventHandler<SuspendingEventArgs^>(this, &PhoneDirect3DApp::OnSuspending);

	    CoreApplication::Resuming +=
		    ref new EventHandler<Platform::Object^>(this, &PhoneDirect3DApp::OnResuming);
    }

	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window)
    {
	    window->VisibilityChanged +=
		    ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &PhoneDirect3DApp::OnVisibilityChanged);

	    window->Closed += 
		    ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &PhoneDirect3DApp::OnWindowClosed);

	    window->PointerPressed +=
		    ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PhoneDirect3DApp::OnPointerPressed);

	    window->PointerMoved +=
		    ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PhoneDirect3DApp::OnPointerMoved);

	    window->PointerReleased +=
		    ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PhoneDirect3DApp::OnPointerReleased);

        m_window=window;

        CreateContext();
        CreateTargets();

        m_height=int(m_renderTargetSize.Height);
        m_app.on_resize((unsigned int)m_renderTargetSize.Width,(unsigned int)m_renderTargetSize.Height);
        m_app.on_init_splash();
        m_time=nya_system::get_time();
        //update_splash(app);
        m_app.on_init();

        m_time=nya_system::get_time();
    }

    virtual void Load(Platform::String^ entryPoint)
    {
    }

	virtual void Run()
    {
	    while (!m_windowClosed)
	    {
		    if (m_windowVisible)
		    {
                unsigned long time=nya_system::get_time();
                unsigned int dt=(unsigned)(time-m_time);
                m_time=time;

			    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

                m_app.on_frame(dt);

                if(m_swap_chain->Present(1, 0)==DXGI_ERROR_DEVICE_REMOVED)
                {
                    //ToDo
                }
            }
		    else
			    CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
	    }
    }
	virtual void Uninitialize()
    {
    }

protected:
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
    {
        CoreWindow::GetForCurrentThread()->Activate();
    }

	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args)
    {
        SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();
	    //release resources for suspending

        /*
	    create_task([this, deferral]()
	    {
		    deferral->Complete();
	    });
        */
    }

	void OnResuming(Platform::Object^ sender, Platform::Object^ args)
    {
    }

	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args)
    {
    	m_windowClosed=true;
    }

	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
    {
        m_windowVisible=args->Visible;
    }

	float get_resolution_scale()
	{
		 //ToDo: DeviceExtendedProperties.GetValue("PhysicalScreenResolution");

		const int rs = (int)Windows::Graphics::Display::DisplayProperties::ResolutionScale;
		return (rs>0)?rs/100.0f:1.0f;
	}

	void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
    {
		static float rs = get_resolution_scale();
        m_app.on_mouse_move(int(args->CurrentPoint->Position.X*rs),m_height-int(args->CurrentPoint->Position.Y*rs));
        m_app.on_mouse_button(nya_system::mouse_left,true);
    }

	void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
    {
		static float rs = get_resolution_scale();
		m_app.on_mouse_move(int(args->CurrentPoint->Position.X*rs),m_height-int(args->CurrentPoint->Position.Y*rs));
    }

	void OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
    {
        m_app.on_mouse_button(nya_system::mouse_left,false);
    }

    void CreateContext()
    {
	    UINT creationFlags=D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    #if defined(_DEBUG)
	    creationFlags|=D3D11_CREATE_DEVICE_DEBUG;
    #endif
	    D3D_FEATURE_LEVEL featureLevels[] = 
	    {
		    D3D_FEATURE_LEVEL_11_1,
		    D3D_FEATURE_LEVEL_11_0,
		    D3D_FEATURE_LEVEL_10_1,
		    D3D_FEATURE_LEVEL_10_0,
		    D3D_FEATURE_LEVEL_9_3
	    };


	    D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_HARDWARE,nullptr,creationFlags,featureLevels,
                          ARRAYSIZE(featureLevels),D3D11_SDK_VERSION,&m_device,&m_featureLevel,&m_context);

	    nya_render::set_device(m_device);
	    nya_render::set_context(m_context);
        nya_render::cull_face::disable();
    }

    void CreateTargets()
    {
	    m_windowBounds = m_window->Bounds;

	    m_renderTargetSize.Width = int(m_windowBounds.Width * get_resolution_scale());
	    m_renderTargetSize.Height = int(m_windowBounds.Height * get_resolution_scale());

	    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
	    swapChainDesc.Width=static_cast<UINT>(m_renderTargetSize.Width);
	    swapChainDesc.Height=static_cast<UINT>(m_renderTargetSize.Height);
	    swapChainDesc.Format=DXGI_FORMAT_B8G8R8A8_UNORM;
	    swapChainDesc.Stereo=false;
	    swapChainDesc.SampleDesc.Count=1;
	    swapChainDesc.SampleDesc.Quality=0;
	    swapChainDesc.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
	    swapChainDesc.BufferCount=1;
	    swapChainDesc.Scaling=DXGI_SCALING_STRETCH;
	    swapChainDesc.SwapEffect=DXGI_SWAP_EFFECT_DISCARD;
	    swapChainDesc.Flags=0;

        IDXGIDevice1 *device;
        if(m_device->QueryInterface(__uuidof(IDXGIDevice),(void **)&device)<0)
            return;

        IDXGIAdapter *adaptor;
        if(device->GetAdapter(&adaptor)<0)
            return;

        IDXGIFactory2 *factory;
        if(adaptor->GetParent(__uuidof(IDXGIFactory2),(void **)&factory)<0)
            return;

	    Windows::UI::Core::CoreWindow^ window = m_window.Get();
	    if(factory->CreateSwapChainForCoreWindow(m_device,reinterpret_cast<IUnknown*>(window),&swapChainDesc,nullptr,&m_swap_chain)<0)
            return;
		
	    if(device->SetMaximumFrameLatency(1)<0)
            return;

	    ID3D11Texture2D *back_buffer;
	    if(m_swap_chain->GetBuffer(0,__uuidof(ID3D11Texture2D),(void **)&back_buffer)<0)
            return;

	    if(m_device->CreateRenderTargetView(back_buffer,nullptr,&m_renderTargetView)<0)
            return;

	    CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,static_cast<UINT>(m_renderTargetSize.Width),
		                                       static_cast<UINT>(m_renderTargetSize.Height),1,1,D3D11_BIND_DEPTH_STENCIL);
	    ID3D11Texture2D *depthStencil;
	    if(m_device->CreateTexture2D(&depthStencilDesc,nullptr,&depthStencil)<0)
            return;

	    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	    if(m_device->CreateDepthStencilView(depthStencil,&depthStencilViewDesc,&m_depthStencilView)<0)
            return;

        m_context->OMSetRenderTargets(1,&m_renderTargetView,m_depthStencilView);

        nya_render::set_viewport(0,0,(int)m_renderTargetSize.Width,(int)m_renderTargetSize.Height);
    }

private:
    PhoneDirect3DApp(nya_system::app_responder &app): m_app(app),m_windowClosed(false),m_windowVisible(false),m_time(0),m_height(0) {}

private:
    nya_system::app_responder &m_app;

private:
	bool m_windowClosed;
	bool m_windowVisible;
    unsigned long m_time;
    int m_height;

private:
	ID3D11Device *m_device;
	ID3D11DeviceContext *m_context;
    D3D_FEATURE_LEVEL m_featureLevel;

private:
	Windows::Foundation::Size m_renderTargetSize;
	Windows::Foundation::Rect m_windowBounds;
	Platform::Agile<Windows::UI::Core::CoreWindow> m_window;

private:
    ID3D11RenderTargetView *m_renderTargetView;
    ID3D11DepthStencilView *m_depthStencilView;
    IDXGISwapChain1 *m_swap_chain;
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
    friend class shared_app;
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        return ref new PhoneDirect3DApp(m_app);
    }

private:
    Direct3DApplicationSource(nya_system::app_responder &app): m_app(app) {}

private:
    nya_system::app_responder &m_app;
};

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app_responder &app)
    {
    	auto direct3DApplicationSource = ref new Direct3DApplicationSource(app);
	    CoreApplication::Run(direct3DApplicationSource);
    }

    void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo

        start_windowed(0,0,w,h,0,app);
    }
    
    void finish(nya_system::app_responder &app)
    {
    }

    void set_title(const char *title)
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
};

}

  #else

    #include <windowsx.h>
    #include "render/render.h"

  #ifndef DIRECTX11
	#include <gl/wglext.h>
  #endif

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
                          TEXT(m_title.c_str()),
                          WS_OVERLAPPEDWINDOW,
                          rect.left,rect.top,
                          rect.right-rect.left,rect.bottom-rect.top,
                          NULL,NULL,m_instance, NULL);

        if(!m_hwnd)
            return;

        ShowWindow(m_hwnd,SW_SHOW);
		
  #ifdef DIRECTX11
        UINT create_device_flags=0;
    #ifdef _DEBUG
        //create_device_flags|=D3D11_CREATE_DEVICE_DEBUG;
    #endif

        D3D_DRIVER_TYPE driver_types[]=
        {
            D3D_DRIVER_TYPE_HARDWARE,
            D3D_DRIVER_TYPE_WARP,
            D3D_DRIVER_TYPE_REFERENCE,
        };

        D3D_FEATURE_LEVEL feature_levels[]=
        {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };

	    D3D_FEATURE_LEVEL feature_level=D3D_FEATURE_LEVEL_11_0;

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd,sizeof(sd));
        sd.BufferCount=1;
        sd.BufferDesc.Width=w;
        sd.BufferDesc.Height=h;
        sd.BufferDesc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator=60;
        sd.BufferDesc.RefreshRate.Denominator=1;
        sd.BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow=m_hwnd;
        sd.SampleDesc.Count=1;
        sd.SampleDesc.Quality=0;
        sd.Windowed=TRUE;

        HRESULT hr = S_OK;

        for(int i=0;i<ARRAYSIZE(driver_types);++i)
        {
            D3D_DRIVER_TYPE driver_type=driver_types[i];
            hr=D3D11CreateDeviceAndSwapChain(0,driver_type,0,create_device_flags,feature_levels,
            ARRAYSIZE(feature_levels),D3D11_SDK_VERSION,&sd,&m_swap_chain,&m_device,&feature_level,&m_context);
            if(SUCCEEDED(hr))
                break;
        }
        if(FAILED(hr))
            return;

        recreate_targets(w,h);

        nya_render::set_context(m_context);
        nya_render::set_device(m_device);
        nya_render::cull_face::disable();
  #else
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
                          TEXT(m_title.c_str()),
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
  #endif
        SetWindowTextA(m_hwnd,m_title.c_str());

        SetWindowLongPtr(m_hwnd,GWL_USERDATA,(LONG)&app);

        nya_render::set_viewport(0,0,w,h);

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

                app.on_frame(dt);

  #ifdef DIRECTX11
				m_swap_chain->Present(0,0);
  #else
                SwapBuffers(m_hdc);
  #endif
            }
        }

        finish(app);
    }

    void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo

        start_windowed(0,0,w,h,0,app);
    }

    void finish(nya_system::app_responder &app)
    {
        if(!m_hwnd)
            return;

        app.on_free();

  #ifdef DIRECTX11
		if(m_context)
			m_context->ClearState();

		if(m_color_target)
		{
			m_color_target->Release();
			m_color_target=0;
		}

		if( m_depth_target )
		{
			m_depth_target->Release();
			m_depth_target=0;
		}

		if(m_swap_chain)
		{
			m_swap_chain->Release();
			m_swap_chain=0;
		}

		if(m_context)
		{
			m_context->Release();
			m_context=0;
		}

		if(m_device)
		{
			m_device->Release();
			m_device=0;
		}
  #else
        wglMakeCurrent (m_hdc, 0);
        wglDeleteContext(m_hglrc);
        ReleaseDC(m_hwnd,m_hdc);
        DestroyWindow (m_hwnd);
  #endif
        m_hwnd=0;
    }

  #ifdef DIRECTX11
private:
    bool recreate_targets(int w,int h)
    {
        HRESULT hr=S_OK;

        m_context->OMSetRenderTargets(0,0,0);

        if(m_color_target)
        {
            m_color_target->Release();
            m_color_target=0;
        }

        if(m_depth_target)
        {
            m_depth_target->Release();
            m_depth_target=0;
        }

        hr=m_swap_chain->ResizeBuffers(0,0,0,DXGI_FORMAT_UNKNOWN,0);
        if(FAILED(hr))
            return false;

        ID3D11Texture2D* pBackBuffer=0;
        hr=m_swap_chain->GetBuffer(0,__uuidof(ID3D11Texture2D ),(LPVOID*)&pBackBuffer);
        if(FAILED(hr))
            return false;

        hr=m_device->CreateRenderTargetView(pBackBuffer,0,&m_color_target);
        pBackBuffer->Release();
        if(FAILED(hr))
            return false;

	    CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,w,h,1,1,D3D11_BIND_DEPTH_STENCIL);

	    ID3D11Texture2D *depthStencil;
	    hr=m_device->CreateTexture2D(&depthStencilDesc,nullptr,&depthStencil);
        if(FAILED(hr))
            return false;

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
        m_device->CreateDepthStencilView(depthStencil,&depthStencilViewDesc,&m_depth_target);
        depthStencil->Release();

        m_context->OMSetRenderTargets(1,&m_color_target,m_depth_target);
        
        return true;
    }
  #endif

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

                const int w=rc.right-rc.left;
                const int h=rc.bottom-rc.top;

  #ifdef DIRECTX11
                get_app().recreate_targets(w,h);
  #endif
                nya_render::set_viewport(0,0,w,h);
                app->on_resize(w,h);
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

        if(m_hwnd)
            SetWindowTextA(m_hwnd,title);
    }

    void update_splash(nya_system::app_responder &app)
    {
        unsigned long time=nya_system::get_time();
        unsigned int dt=(unsigned)(time-m_time);
        m_time=time;

        app.on_splash(dt);

  #ifdef DIRECTX11
  #else
        SwapBuffers(m_hdc);
  #endif
    }

public:
    static shared_app &get_app()
    {
        static shared_app app;
        return app;
    }

public:
    shared_app():
  #ifdef DIRECTX11
        m_device(0),
		m_context(0),
		m_swap_chain(0),
		m_color_target(0),
		m_depth_target(0),
  #else
		m_hdc(0),
  #endif
		m_title("Nya engine"),m_time(0) {}

private:
    HINSTANCE m_instance;
    HWND m_hwnd;
  #ifdef DIRECTX11
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_context;
	IDXGISwapChain* m_swap_chain;
	ID3D11RenderTargetView* m_color_target;
	ID3D11DepthStencilView* m_depth_target;
  #else
    HDC m_hdc;
    HGLRC m_hglrc;
  #endif

    std::string m_title;
    unsigned long m_time;
};

}
  #endif
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

            app.on_frame(dt);

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
