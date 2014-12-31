//https://code.google.com/p/nya-engine/

// could be extended externally at runtime via method_exchangeImplementations
// to implement necessary behavior

#pragma once

#if defined __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>

@interface view_controller : UIViewController
{
    CFTimeInterval m_time;

    EAGLContext *context;
    BOOL animating;
    float scale;
    __weak CADisplayLink *displayLink;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (readonly, nonatomic, getter=getScale) float scale;
@end

@interface shared_app_delegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) view_controller *viewController;
@end

#else

#include <Cocoa/Cocoa.h>

@interface shared_app_delegate : NSObject <NSApplicationDelegate>
{
    nya_system::app *m_app;
    int m_antialiasing;
}

-(id)init_with_responder:(nya_system::app*)responder antialiasing:(int)aa;
@end

@interface gl_view : NSOpenGLView<NSWindowDelegate>
{
    NSTimer *m_animation_timer;
    nya_system::app *m_app;
    unsigned long m_time;

    enum state
    {
        state_splash,
        state_init,
        state_draw
    };

    state m_state;

    bool m_shift_pressed;
    bool m_ctrl_pressed;
    bool m_alt_pressed;
}
@end

#endif
#endif
