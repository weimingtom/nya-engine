//https://code.google.com/p/nya-engine/

#include "app.h"

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

        NSRect viewRect = NSMakeRect(x,y,w,h);

        NSWindow *window = [[NSWindow alloc] initWithContentRect:viewRect styleMask:NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];

        [window setTitle:@"Nya engine"];
        [window setOpaque:YES];

        //NSWindowController* windowController = 
        [[NSWindowController alloc] initWithWindow:window];

        shared_app_delegate *delegate = [[shared_app_delegate alloc] init_with_responder:&app];

        [NSApp setDelegate:delegate];

        setup_menu();

        [window orderFrontRegardless];
        [NSApp run];
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
    
private:
    void setup_menu()
    {
        NSMenu *mainMenuBar;
        NSMenu *appMenu;
        NSMenuItem *menuItem;
        
        mainMenuBar = [[NSMenu alloc] init];
        
        appMenu = [[NSMenu alloc] initWithTitle:@"Nya engine"];
        menuItem = [appMenu addItemWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@"q"];
        [menuItem setKeyEquivalentModifierMask:NSCommandKeyMask];
        
        menuItem = [[NSMenuItem alloc] init];
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

public:
    shared_app() {}

private:
};

}

@implementation shared_app_delegate

-(id)init_with_responder:(nya_system::app_responder*)responder;
{
    self = [super init];
    if (self)
        m_app = responder;
    
    return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
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
    shared_app::get_app().update_splash(*this);
}

void app::finish()
{
    shared_app::get_app().finish(*this);
}

}
