
//https://code.google.com/p/nya-engine/

#include "app.h"
#include "system.h"
#include "memory/tmp_buffer.h"

#include <string>

#include "render/render.h"

#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import <QuartzCore/QuartzCore.h>

@interface view_controller : UIViewController 
{
    EAGLContext *context;

    BOOL animating;
    CFTimeInterval m_time;
    float m_scale;

    NSInteger animationFrameInterval;
    __weak CADisplayLink *displayLink;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
- (float)getScale;
- (void)startAnimation;
- (void)stopAnimation;
@end

@interface shared_app_delegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@property (strong, nonatomic) view_controller *viewController;
@end

@class EAGLContext;

@interface EAGLView : UIView {
    GLint framebufferWidth;
    GLint framebufferHeight;
    float m_scale;

    GLuint defaultFramebuffer,colorRenderbuffer,msaaFramebuffer,msaaRenderBuffer,depthRenderbuffer;
}

@property (strong, nonatomic) EAGLContext *context;

- (void)setFramebuffer;
- (float)getScale;
- (BOOL)presentFramebuffer;

@end

@interface view_controller ()
@property (strong, nonatomic) EAGLContext *context;
@property (weak, nonatomic) CADisplayLink *displayLink;
- (void)applicationWillResignActive:(NSNotification *)notification;
- (void)applicationDidBecomeActive:(NSNotification *)notification;
- (void)applicationWillTerminate:(NSNotification *)notification;
- (void)drawFrame;
@end

namespace
{
    class shared_app
    {
    public:
        void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app &app)
        {
            start_fullscreen(w,h,antialiasing,app);
        }

        void start_fullscreen(unsigned int w,unsigned int h,int antialiasing,nya_system::app &app)
        {
            if(m_responder)
                return;

            m_responder=&app;
            m_antialiasing=antialiasing;

            @autoreleasepool 
            {
                UIApplicationMain(0,nil, nil, NSStringFromClass([shared_app_delegate class]));
            }
        }

        void finish(nya_system::app &app)
        {
        }

        void set_title(const char *title)
        {
            if(!title)
                m_title.clear();
            else
                m_title.assign(title);
            /*
            if(!m_window)
                return;

            NSString *title_str=[NSString stringWithCString:m_title.c_str() encoding:NSUTF8StringEncoding];
            [m_window setTitle:title_str];
             */
        }

    public:
        static shared_app &get_app() { static shared_app app; return app; }
        nya_system::app *get_responder() { return m_responder; }
        int get_antialiasing() { return m_antialiasing; }

    public:
        shared_app():m_responder(0),m_antialiasing(0),m_title("Nya engine") {}

    private:
        nya_system::app *m_responder;
        std::string m_title;
        int m_antialiasing;
    };
}

@implementation shared_app_delegate

@synthesize window = _window;
@synthesize viewController = _viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    UIWindow *win=[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window = win;
    [win release];

    view_controller *vc=[[view_controller alloc] init];
    self.viewController = vc;
    [vc release];

    [self.window setRootViewController:self.viewController];

    [self.window makeKeyAndVisible];

    return YES;
}

- (BOOL)application:(UIApplication *)application openURL:(NSURL *)url sourceApplication:(NSString *)sourceApplication annotation:(id)annotation
{
    nya_system::app *responder=shared_app::get_app().get_responder();
    if(responder)
    {
        NSString *s=[url absoluteString];
        if(responder->on_open_url(s.UTF8String))
            return true;
    }

    return false;
}

- (BOOL)application:(UIApplication *)application handleOpenURL:(NSURL *)url
{
    nya_system::app *responder=shared_app::get_app().get_responder();
    if(responder)
    {
        NSString *s=[url absoluteString];
        if(responder->on_open_url(s.UTF8String))
            return true;
    }

    return false;
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    nya_system::app *responder=shared_app::get_app().get_responder();
    if(!responder)
        return;

    const float scale=[self.viewController getScale];

    CGPoint tappedPt = [[touches anyObject] locationInView: self.viewController.view];
    responder->on_mouse_move(tappedPt.x*scale,(self.viewController.view.bounds.size.height-tappedPt.y)*scale);
    responder->on_mouse_button(nya_system::mouse_left,true);
};

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    nya_system::app *responder=shared_app::get_app().get_responder();
    if(!responder)
        return;

    //UITouch *touchSample = [[event allTouches] anyObject];
    //int count=[touchSample tapCount];

    const float scale=[self.viewController getScale];

    CGPoint tappedPt = [[touches anyObject] locationInView: self.viewController.view];
    responder->on_mouse_move(tappedPt.x*scale,(self.viewController.view.bounds.size.height-tappedPt.y)*scale);
};

-(void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    nya_system::app *responder=shared_app::get_app().get_responder();
    if(!responder)
        return;

    const float scale=[self.viewController getScale];

    CGPoint tappedPt = [[touches anyObject] locationInView: self.viewController.view];
    responder->on_mouse_move(tappedPt.x*scale,(self.viewController.view.bounds.size.height-tappedPt.y)*scale);
    responder->on_mouse_button(nya_system::mouse_left,false);
};

@end


@implementation view_controller

@synthesize animating;
@synthesize context;
@synthesize displayLink;

- (void)loadView 
{
    EAGLView *view=[[EAGLView alloc] initWithFrame:[UIScreen mainScreen].bounds];

    self.view = view;
    m_scale = [view getScale];
    [view release];
}

- (float)getScale
{
    return m_scale;
}

static inline NSString *NSStringFromUIInterfaceOrientation(UIInterfaceOrientation orientation) 
{
	switch (orientation) 
    {
		case UIInterfaceOrientationPortrait:           return @"UIInterfaceOrientationPortrait";
		case UIInterfaceOrientationPortraitUpsideDown: return @"UIInterfaceOrientationPortraitUpsideDown";
		case UIInterfaceOrientationLandscapeLeft:      return @"UIInterfaceOrientationLandscapeLeft";
		case UIInterfaceOrientationLandscapeRight:     return @"UIInterfaceOrientationLandscapeRight";
        default: break;
	}
	return @"Unexpected";
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation 
{
    return [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"] indexOfObject:NSStringFromUIInterfaceOrientation(interfaceOrientation)] != NSNotFound;
}

//iOS 6:

- (BOOL)shouldAutorotate 
{
    return YES;
}
/*
- (NSUInteger)supportedInterfaceOrientations 
{
    return UIInterfaceOrientationMaskAll;
}
*/

- (void)viewDidLoad
{
    [super viewDidLoad];

    if(!self.context)
    {
        EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        if(!aContext)
        {
            NSLog(@"Failed to create ES2 context");
            exit(0);
        }

        self.context=aContext;
        [aContext release];

        animating=NO;
        animationFrameInterval=1;
        m_time=0.0;
        self.displayLink = nil;

        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminateActive:) name:UIApplicationWillTerminateNotification object:nil];
    }

    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];

    nya_system::app *responder=shared_app::get_app().get_responder();
    if(responder)
    {
        [(EAGLView *)self.view setFramebuffer];
        responder->on_init_splash();
        responder->on_splash(0);
        responder->on_init();
        [(EAGLView *)self.view presentFramebuffer];
    }

    [self drawFrame];
}

- (void)applicationWillResignActive:(NSNotification *)notification
{
    if([self isViewLoaded] && self.view.window)
    {
        nya_system::app *responder=shared_app::get_app().get_responder();
        if(responder)
            responder->on_suspend();

        [self stopAnimation];
    }
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if([self isViewLoaded] && self.view.window)
    {
        static bool ignore_first=true;
        if(!ignore_first)
        {
            nya_system::app *responder=shared_app::get_app().get_responder();
            if(responder)
                responder->on_restore();
        }
        else
            ignore_first=false;

        [self startAnimation];
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    if([self isViewLoaded] && self.view.window)
        [self stopAnimation];
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    // Tear down context.
    if([EAGLContext currentContext] == context)
    {
        nya_system::app *responder=shared_app::get_app().get_responder();
        if(responder)
            responder->on_free();

        [EAGLContext setCurrentContext:nil];
    }
    
    [super dealloc];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.

    nya_log::log()<<"app recieved memory warning, ";

    size_t tmp_buffers_size=nya_memory::tmp_buffers::get_total_size();
    nya_memory::tmp_buffers::force_free();
    nya_log::log()<<"forced to free "<<tmp_buffers_size-
                    nya_memory::tmp_buffers::get_total_size()<<" bytes\n";
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];

    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];

    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];

    // Tear down context.
    if([EAGLContext currentContext]==context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;	
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if(frameInterval>=1)
    {
        animationFrameInterval = frameInterval;
        if(animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if(!animating)
    {
        CADisplayLink *aDisplayLink = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;

        m_time=self.displayLink.timestamp*1000.0;

        animating = YES;
    }
}

- (void)stopAnimation
{
    if(animating)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = NO;
    }
}

- (void)drawFrame
{
    [(EAGLView *)self.view setFramebuffer];

    CFTimeInterval time=self.displayLink.timestamp;
    unsigned int dt=(unsigned int)((time-m_time)*1000.0);
    m_time=time;

    nya_system::app *responder=shared_app::get_app().get_responder();
    if(responder)
        responder->on_frame(dt);

    [(EAGLView *)self.view presentFramebuffer];
}

@end


@interface EAGLView (PrivateMethods)
- (void)createFramebuffer;
- (void)deleteFramebuffer;
@end

@implementation EAGLView

@synthesize context;

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame 
{
    if((self=[super initWithFrame:frame]))
    {   
        m_scale=1.0f;
        SEL scaleSelector = NSSelectorFromString(@"scale");
        SEL setContentScaleSelector = NSSelectorFromString(@"setContentScaleFactor:");
        SEL getContentScaleSelector = NSSelectorFromString(@"contentScaleFactor");
        if([self respondsToSelector: getContentScaleSelector] && [self respondsToSelector: setContentScaleSelector])
        {
            NSMethodSignature *scaleSignature = [UIScreen instanceMethodSignatureForSelector: scaleSelector];
            NSInvocation *scaleInvocation = [NSInvocation invocationWithMethodSignature: scaleSignature];
            [scaleInvocation setTarget: [UIScreen mainScreen]];
            [scaleInvocation setSelector: scaleSelector];
            [scaleInvocation invoke];

            NSInteger returnLength=[[scaleInvocation methodSignature] methodReturnLength];
            if(returnLength==sizeof(float))
                [scaleInvocation getReturnValue: &m_scale];

            typedef void (*CC_CONTENT_SCALE)(id, SEL, float);
            CC_CONTENT_SCALE method = (CC_CONTENT_SCALE) [self methodForSelector: setContentScaleSelector];
            method(self, setContentScaleSelector, m_scale);
        }

        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
    }
    return self;
}

- (void)dealloc
{
    [self deleteFramebuffer];
    [super dealloc];
}

- (void)setContext:(EAGLContext *)newContext
{
    if(context!=newContext)
    {
        [self deleteFramebuffer];
        context=newContext;
        [EAGLContext setCurrentContext:nil];
    }
}

- (float)getScale { return m_scale; }

- (void)createFramebuffer
{
    if(context && !defaultFramebuffer)
    {
        [EAGLContext setCurrentContext:context];

        glGenFramebuffers(1, &defaultFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

        glGenRenderbuffers(1, &depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);

        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

        [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)self.layer];
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

        const int aa=shared_app::get_app().get_antialiasing();
        if(aa>0)
        {
            glGenFramebuffers(1, &msaaFramebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);

            glGenRenderbuffers(1, &msaaRenderBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, msaaRenderBuffer);

            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, aa, GL_RGB5_A1, framebufferWidth, framebufferHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaRenderBuffer);
            glGenRenderbuffers(1, &depthRenderbuffer);

            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
            glRenderbufferStorageMultisampleAPPLE(GL_RENDERBUFFER, aa, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        }
        else
        {
            glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);
            //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, framebufferWidth, framebufferHeight);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
            //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        }

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
}

- (void)deleteFramebuffer
{
    if(!context)
        return;

    [EAGLContext setCurrentContext:context];

    if(msaaFramebuffer)
    {
        glDeleteFramebuffers(1,&msaaFramebuffer);
        msaaFramebuffer=0;
    }

    if(msaaRenderBuffer)
    {
        glDeleteRenderbuffers(1,&msaaRenderBuffer);
        msaaRenderBuffer=0;
    }

    if(defaultFramebuffer)
    {
        glDeleteFramebuffers(1,&defaultFramebuffer);
        defaultFramebuffer=0;
    }

    if(colorRenderbuffer)
    {
        glDeleteRenderbuffers(1,&colorRenderbuffer);
        colorRenderbuffer=0;
    }

    if(depthRenderbuffer)
    {
        glDeleteRenderbuffers(1,&depthRenderbuffer);
        depthRenderbuffer=0;
    }
}

- (void)setFramebuffer
{
    if(context)
    {
        [EAGLContext setCurrentContext:context];

        if(!defaultFramebuffer)
        {
            [self createFramebuffer];
            nya_system::app *responder=shared_app::get_app().get_responder();
            if(responder)
                responder->on_resize(framebufferWidth,framebufferHeight);
        }

        if(msaaFramebuffer)
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFramebuffer);
        else
            glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

        nya_render::set_viewport(0,0,framebufferWidth,framebufferHeight);
    }
}

- (BOOL)presentFramebuffer
{
    if(!context)
        return false;

    [EAGLContext setCurrentContext:context];

    const GLenum attachments[]={GL_DEPTH_ATTACHMENT};
    glDiscardFramebufferEXT(GL_READ_FRAMEBUFFER_APPLE,1,attachments);

    if(msaaFramebuffer)
    {

        glBindFramebuffer(GL_READ_FRAMEBUFFER_APPLE,msaaFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER_APPLE,defaultFramebuffer);
        glResolveMultisampleFramebufferAPPLE();
    }

    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

    //const GLenum attachments[] = { GL_DEPTH_ATTACHMENT, GL_COLOR_ATTACHMENT0 };
    //glDiscardFramebufferEXT(GL_FRAMEBUFFER , sizeof(attachments)/sizeof(GLenum), attachments);

    return [context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void)layoutSubviews
{
    // The framebuffer will be re-created at the beginning of the next setFramebuffer method call.
    [self deleteFramebuffer];
}

@end

#else

#include <Cocoa/Cocoa.h>
#include <GLKit/GLKit.h>

@interface shared_app_delegate : NSObject <NSApplicationDelegate>
{
    nya_system::app *m_app;
    int m_antialiasing;
}

-(id)init_with_responder:(nya_system::app*)responder antialiasing:(int)aa;

@end

namespace
{

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app &app)
    {
        [NSApplication sharedApplication];

        NSRect viewRect=NSMakeRect(x,y,w,h);

        m_window=[[NSWindow alloc] initWithContentRect:viewRect styleMask:NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];

        NSString *title_str=[NSString stringWithCString:m_title.c_str() encoding:NSUTF8StringEncoding];
        [m_window setTitle:title_str];
        [m_window setOpaque:YES];

        NSWindowController *controller=[[NSWindowController alloc] initWithWindow:m_window];

        shared_app_delegate *delegate=[[shared_app_delegate alloc] init_with_responder:&app antialiasing:antialiasing];

        [[NSApplication sharedApplication] setDelegate:delegate];

        setup_menu();

        [m_window orderFrontRegardless];
        [NSApp run];
        [delegate release];
        [controller release];
    }

    void start_fullscreen(unsigned int w,unsigned int h,int antialiasing,nya_system::app &app)
    {
        //ToDo

        start_windowed(0,0,w,h,0,app);
    }

    void finish(nya_system::app &app)
    {
    }

    void set_title(const char *title)
    {
        if(!title)
            m_title.clear();
        else
            m_title.assign(title);

        if(!m_window)
            return;

        NSString *title_str=[NSString stringWithCString:m_title.c_str() encoding:NSUTF8StringEncoding];
        [m_window setTitle:title_str];
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
        [menuItem release];

        [NSApp performSelector:@selector(setAppleMenu:) withObject:appMenu];
        [appMenu release];
        [NSApp setMainMenu:mainMenuBar];
        [mainMenuBar release];
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
    shared_app(): m_window(0), m_title("Nya engine") {}

private:
    NSWindow *m_window;
    std::string m_title;
};

}

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

@implementation gl_view

-(void)set_responder:(nya_system::app*)responder
{
    m_app=responder;
}

-(void)initView
{
    m_animation_timer=[NSTimer timerWithTimeInterval:0.01 target:self 
                                       selector:@selector(animate:) userInfo:nil repeats:YES];

    [[self window] setAcceptsMouseMovedEvents:YES];
    [[self window] setDelegate: self];

    [[NSRunLoop currentRunLoop] addTimer:m_animation_timer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:m_animation_timer forMode:NSEventTrackingRunLoopMode];

    m_state=state_splash;
}

- (void)animate:(id)sender
{
    [self setNeedsDisplay:YES];
}

- (void)drawRect:(NSRect)rect 
{
    switch(m_state)
    {
        case state_splash:
        {
            m_app->on_init_splash();
            m_app->on_splash(0);
            m_state=state_init;
        }
        break;

        case state_init:
        {
            m_app->on_init();
            m_time=nya_system::get_time();
            m_state=state_draw;
        }
        case state_draw:
        {
            const unsigned long time=nya_system::get_time();
            m_app->on_frame((unsigned int)(time-m_time));
            m_time=time;
        }
        break;
    }

    [[self openGLContext] flushBuffer];
}

-(void)reshape 
{
    nya_render::set_viewport(0,0,[self frame].size.width,[self frame].size.height);
    m_app->on_resize([self frame].size.width,[self frame].size.height);

    [[self openGLContext] update];

    [self setNeedsDisplay:YES];
}

- (void)mouseMoved:(NSEvent *)event
{
    NSPoint pt=[event locationInWindow];
    pt=[self convertPoint:pt fromView:nil];

    m_app->on_mouse_move(pt.x,pt.y);
}

- (void)mouseDragged:(NSEvent *)event
{
    NSPoint pt=[event locationInWindow];
    pt=[self convertPoint:pt fromView:nil];

    m_app->on_mouse_move(pt.x,pt.y);
}

- (void)rightMouseDragged:(NSEvent *)event
{
    NSPoint pt=[event locationInWindow];
    pt=[self convertPoint:pt fromView:nil];

    m_app->on_mouse_move(pt.x,pt.y);
}

- (void)mouseDown:(NSEvent *)event
{
    m_app->on_mouse_button(nya_system::mouse_left,true);
}

- (void)mouseUp:(NSEvent *)event
{
    m_app->on_mouse_button(nya_system::mouse_left,false);
}

- (void)rightMouseDown:(NSEvent *)event
{
    m_app->on_mouse_button(nya_system::mouse_right,true);
}

- (void)rightMouseUp:(NSEvent *)event
{
    m_app->on_mouse_button(nya_system::mouse_right,false);
}

- (void)scrollWheel:(NSEvent*)event
{
    m_app->on_mouse_scroll(0,[event deltaY]);
}

-(unsigned int)cocoaKeyToX11Keycode:(unichar)key_char
{
    if(key_char>='A' && key_char<='Z')
        return nya_system::key_a+key_char-'A';

    if(key_char>='a' && key_char<='z')
        return nya_system::key_a+key_char-'a';

    if(key_char>='1' && key_char<='9')
        return nya_system::key_1+key_char-'1';

    if(key_char>=NSF1FunctionKey && key_char<=NSF35FunctionKey)
        return nya_system::key_f1+key_char-NSF1FunctionKey;

    switch(key_char)
    {
        case NSLeftArrowFunctionKey: return nya_system::key_left;
        case NSRightArrowFunctionKey: return nya_system::key_right;
        case NSUpArrowFunctionKey: return nya_system::key_up;
        case NSDownArrowFunctionKey: return nya_system::key_down;

        case ' ': return nya_system::key_space;
        case '\r': return nya_system::key_return;
        case '\t': return nya_system::key_tab;
        case '0': return nya_system::key_0;

        case 0x1f: return nya_system::key_escape;
        case 0x7f: return nya_system::key_backspace;

        case NSEndFunctionKey: return nya_system::key_end;
        case NSHomeFunctionKey: return nya_system::key_home;
        case NSInsertFunctionKey: return nya_system::key_insert;
        case NSDeleteFunctionKey: return nya_system::key_delete;

        default: break;
    };

    //printf("unknown key: \'%c\' %x\n",key_char,key_char);

    return 0;
}

-(void)keyDown:(NSEvent *)event
{
    NSString *key=[event charactersIgnoringModifiers];
    if([key length]!=1)
        return;

    const unsigned int x11key=[self cocoaKeyToX11Keycode:[key characterAtIndex:0]];
    if(x11key)
        return m_app->on_keyboard(x11key,true);
}

-(void)keyUp:(NSEvent *)event
{
    NSString *key=[event charactersIgnoringModifiers];
    if([key length]!=1)
        return;

    const unsigned int x11key=[self cocoaKeyToX11Keycode:[key characterAtIndex:0]];
    if(x11key)
        return m_app->on_keyboard(x11key,false);
}

- (void)windowWillMiniaturize:(NSNotification *)notification
{
    m_app->on_suspend();
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    m_app->on_restore();
}

-(void)flagsChanged:(NSEvent *)event
{
    //ToDo: caps, left/right alt, ctrl, shift - 4ex: [event modifierFlags] == 131330

    const bool shift_pressed = ([event modifierFlags] & NSShiftKeyMask) == NSShiftKeyMask;
    if(shift_pressed!=m_shift_pressed)
        m_app->on_keyboard(nya_system::key_shift,shift_pressed), m_shift_pressed=shift_pressed;

    const bool ctrl_pressed = ([event modifierFlags] & NSControlKeyMask) == NSControlKeyMask;
    if(ctrl_pressed!=m_ctrl_pressed)
        m_app->on_keyboard(nya_system::key_control,ctrl_pressed), m_ctrl_pressed=ctrl_pressed;

    const bool alt_pressed = ([event modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask;
    if(alt_pressed!=m_alt_pressed)
        m_app->on_keyboard(nya_system::key_alt,alt_pressed), m_alt_pressed=alt_pressed;
}

-(void)dealloc
{
    [m_animation_timer release];

    [super dealloc];
}

@end

@implementation shared_app_delegate

-(id)init_with_responder:(nya_system::app*)responder  antialiasing:(int)aa
{
    self=[super init];
    if(self)
    {
        m_app=responder;
        m_antialiasing=aa;
    }

    return self;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification 
{
    NSOpenGLPixelFormatAttribute attrs[] = 
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        0
    };

    NSOpenGLPixelFormatAttribute attrs_aniso[] = 
    {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 32,
        NSOpenGLPFASampleBuffers,1,NSOpenGLPFASamples,0,
        0
    };  attrs_aniso[6]=m_antialiasing;

    NSWindow *window=shared_app::get_window();

    NSOpenGLPixelFormat *format=0;

    if(m_antialiasing>0)
        format=[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs_aniso];
    else
        format=[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];

    gl_view *view = [[gl_view alloc] initWithFrame:window.frame pixelFormat:format];
    [format release];

    [view set_responder:m_app];

    [window setContentView:view];

    [window makeFirstResponder:view];
    [view release];
    [window setAcceptsMouseMovedEvents:YES];

    if([view openGLContext]==nil) 
        return;

    if(m_antialiasing)
#ifdef GL_MULTISAMPLE
        glEnable(GL_MULTISAMPLE);
#else
        glEnable(GL_MULTISAMPLE_ARB);
#endif

    [view reshape];

    [view initView];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    m_app->on_free();
}

@end

#endif

namespace nya_system
{

void app::start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing)
{
    shared_app::get_app().start_windowed(x,y,w,h,antialiasing,*this);
}

void app::start_fullscreen(unsigned int w,unsigned int h)
{
    shared_app::get_app().start_fullscreen(w,h,0,*this);
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
    //shared_app::get_app().update_splash(*this);
}

void app::finish()
{
    shared_app::get_app().finish(*this);
}

}
