//https://code.google.com/p/nya-engine/

#include "app.h"
#include "system.h"

#include <Cocoa/Cocoa.h>

@interface shared_app_delegate : NSObject <NSApplicationDelegate>
{
    nya_system::app_responder *m_app;
}

-(id)init_with_responder:(nya_system::app_responder*)responder;

@end

namespace
{

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        [[NSAutoreleasePool alloc] init];

        [NSApplication sharedApplication];

        NSRect viewRect=NSMakeRect(x,y,w,h);

        m_window=[[NSWindow alloc] initWithContentRect:viewRect styleMask:NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];

        [m_window setTitle:@"Nya engine"];
        [m_window setOpaque:YES];

        [[NSWindowController alloc] initWithWindow:m_window];

        shared_app_delegate *delegate=[[shared_app_delegate alloc] init_with_responder:&app];

        [NSApp setDelegate:delegate];

        setup_menu();

        [m_window orderFrontRegardless];
        [NSApp run];
    }

    void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
    {
        //ToDo
    }

    void finish(nya_system::app_responder &app)
    {
    }


private:
    void setup_menu()
    {
        NSMenu *mainMenuBar;
        NSMenu *appMenu;
        NSMenuItem *menuItem;
        
        mainMenuBar=[[NSMenu alloc] init];
        
        appMenu=[[NSMenu alloc] initWithTitle:@"Nya engine"];
        menuItem=[appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
        
        menuItem=[[NSMenuItem alloc] init];
        [menuItem setSubmenu:appMenu];
        
        [mainMenuBar addItem:menuItem];
        
        [NSApp performSelector:@selector(setAppleMenu:) withObject:appMenu];
        [appMenu release];
        [NSApp setMainMenu:mainMenuBar];
    }

public:
    static shared_app &get_app()
    {
        static shared_app app;
        return app;
    }

    static NSWindow *get_window()
    {
        return get_app().m_window;
    }

public:
    shared_app(): m_window(0) {}

private:
    NSWindow *m_window;
};

}

@interface gl_view : NSOpenGLView 
{
    NSTimer *m_animation_timer;
    nya_system::app_responder *m_app;
    unsigned long m_time;
}
@end

@implementation gl_view

-(void)set_responder:(nya_system::app_responder*)responder;
{
    m_app=responder;
}

-(void)initTimer
{
    m_animation_timer=[NSTimer timerWithTimeInterval:0.01 target:self 
                                       selector:@selector(animate:) userInfo:nil repeats:YES];
    
    [[self window] setAcceptsMouseMovedEvents:YES];
    
    m_time=nya_system::get_time();
    
    [[NSRunLoop currentRunLoop] addTimer:m_animation_timer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:m_animation_timer forMode:NSEventTrackingRunLoopMode];
}

- (void)animate:(id)sender
{
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect 
{
    unsigned long time=nya_system::get_time();
    unsigned int dt=(unsigned int)(time-m_time);
    m_time=time;

    m_app->on_process(dt);
    m_app->on_draw();
    
    [[self openGLContext] flushBuffer];    
}

-(void)reshape 
{
    glViewport(0,0,[self frame].size.width,[self frame].size.height);     
    m_app->on_resize([self frame].size.width,[self frame].size.height);

    [[self openGLContext] update];
    
    [self setNeedsDisplay:YES];
}

- (void)mouseMoved:(NSEvent *)theEvent
{
    NSPoint pt=[theEvent locationInWindow];
    pt=[self convertPoint:pt fromView:nil];
    
    m_app->on_mouse_move(pt.x,pt.y);
}

- (void)mouseDragged:(NSEvent *)theEvent
{
    NSPoint pt=[theEvent locationInWindow];
    pt=[self convertPoint:pt fromView:nil];
    
    m_app->on_mouse_move(pt.x,pt.y);
}

- (void)rightMouseDragged:(NSEvent *)theEvent
{
    NSPoint pt=[theEvent locationInWindow];
    pt=[self convertPoint:pt fromView:nil];
    
    m_app->on_mouse_move(pt.x,pt.y);
}

- (void)mouseDown:(NSEvent *)theEvent
{
    m_app->on_mouse_button(nya_system::mouse_left,true);
}

- (void)mouseUp:(NSEvent *)theEvent
{
    m_app->on_mouse_button(nya_system::mouse_left,false);
}

- (void)rightMouseDown:(NSEvent *)theEvent
{
    m_app->on_mouse_button(nya_system::mouse_right,true);
}

- (void) rightMouseUp: (NSEvent *) theEvent
{
    m_app->on_mouse_button(nya_system::mouse_right,false);
}

- (void)scrollWheel:(NSEvent*)event
{
    m_app->on_mouse_scroll(0,[event deltaY]);
}

-(void)dealloc
{
    [m_animation_timer release];
    
    [super dealloc];
}

@end

@implementation shared_app_delegate

-(id)init_with_responder:(nya_system::app_responder*)responder;
{
    self=[super init];
    if (self)
        m_app=responder;
    
    return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification 
{
    //ToDo: optional multisampling

    NSOpenGLPixelFormatAttribute attrs[] = 
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        NSOpenGLPFASampleBuffers,1,NSOpenGLPFASamples,4,
        0
    };

    NSWindow *window=shared_app::get_window();

    NSOpenGLPixelFormat *format=[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    gl_view *view = [[gl_view alloc] initWithFrame:window.frame pixelFormat:format];
    [format release];

    [view set_responder:m_app];
    
    [window setContentView:view];

    [window makeFirstResponder:view];
    [window setAcceptsMouseMovedEvents:YES];

    if([view openGLContext]==nil) 
    {
        return;
    }

    glEnable(GL_MULTISAMPLE_ARB);

    [view reshape];

    m_app->on_init_splash();
    m_app->on_splash(0);

    [[view openGLContext] flushBuffer];

    m_app->on_init();
    
    [view initTimer];
}

@end

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
    //shared_app::get_app().update_splash(*this);
}

void app::finish()
{
    shared_app::get_app().finish(*this);
}

}
