//https://code.google.com/p/nya-engine/

#include "app.h"
#include "system.h"

#include <string>

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
    unsigned long m_time;
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
    
    GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;
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
        void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app_responder &app)
        {
            start_fullscreen(w,h,app);
        }

        void start_fullscreen(unsigned int w,unsigned int h,nya_system::app_responder &app)
        {
            if(m_responder)
                return;

            m_responder=&app;

            @autoreleasepool 
            {
                UIApplicationMain(0,nil, nil, NSStringFromClass([shared_app_delegate class]));
            }
        }

        void finish(nya_system::app_responder &app)
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
        static shared_app &get_app()
        {
            static shared_app app;
            return app;
        }

        nya_system::app_responder *get_responder()
        {
            return m_responder;
        }

    public:
        shared_app():m_responder(0),m_title("Nya engine") {}

    private:        
        nya_system::app_responder *m_responder;
        std::string m_title;
    };
}

@implementation shared_app_delegate

@synthesize window = _window;
@synthesize viewController = _viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];

    self.viewController = [[view_controller alloc] init];
    [self.window setRootViewController:self.viewController];
    [self.viewController release];

    [self.window makeKeyAndVisible];

    return YES;
}

-(void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    nya_system::app_responder *responder=shared_app::get_app().get_responder();
    if(!responder)
        return;

    const float scale=[self.viewController getScale];

    CGPoint tappedPt = [[touches anyObject] locationInView: self.viewController.view];
    responder->on_mouse_move(tappedPt.x*scale,(self.viewController.view.bounds.size.height-tappedPt.y)*scale);
    responder->on_mouse_button(nya_system::mouse_left,true);
};

-(void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    nya_system::app_responder *responder=shared_app::get_app().get_responder();
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
    nya_system::app_responder *responder=shared_app::get_app().get_responder();
    if(!responder)
        return;
    
    const float scale=[self.viewController getScale];

    CGPoint tappedPt = [[touches anyObject] locationInView: self.viewController.view];
    responder->on_mouse_move(tappedPt.x*scale,(self.viewController.view.bounds.size.height-tappedPt.y)*scale);
    responder->on_mouse_button(nya_system::mouse_left,false);
};

- (void)applicationWillResignActive:(UIApplication *)application
{
    /*
     Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
     Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
     */
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    /*
     Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
     If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
     */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    /*
     Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
     */
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    /*
     Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
     */
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    /*
     Called when the application is about to terminate.
     Save data if appropriate.
     See also applicationDidEnterBackground:.
     */
}

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

    if (!self.context) {
        EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        if (!aContext)
            NSLog(@"Failed to create ES2 context");
        else if (![EAGLContext setCurrentContext:aContext])
            NSLog(@"Failed to set ES context current");

        self.context = aContext;

        animating = NO;
        m_time=nya_system::get_time();
        animationFrameInterval = 1;
        self.displayLink = nil;
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive:) name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive:) name:UIApplicationDidBecomeActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminateActive:) name:UIApplicationWillTerminateNotification object:nil];
    }

    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];

    nya_system::app_responder *responder=shared_app::get_app().get_responder();
    if(responder)
    {
        responder->on_init_splash();

        [(EAGLView *)self.view setFramebuffer];
        responder->on_splash(0);
        [(EAGLView *)self.view presentFramebuffer];

        responder->on_init();
    }

    [self drawFrame];
}

- (void)applicationWillResignActive:(NSNotification *)notification
{
    if ([self isViewLoaded] && self.view.window) {
        [self stopAnimation];
    }
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    if ([self isViewLoaded] && self.view.window) {
        [self startAnimation];
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    if ([self isViewLoaded] && self.view.window) {
        [self stopAnimation];
    }
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    // Tear down context.
    if ([EAGLContext currentContext] == context)
    {
        nya_system::app_responder *responder=shared_app::get_app().get_responder();
        if(responder)
            responder->on_free();
        
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
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
    if ([EAGLContext currentContext] == context)
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
    if (frameInterval >= 1) {
        animationFrameInterval = frameInterval;

        if (animating) {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating) {
        CADisplayLink *aDisplayLink = [[UIScreen mainScreen] displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;

        animating = YES;
    }
}

- (void)stopAnimation
{
    if (animating) {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = NO;
    }
}

- (void)drawFrame
{
    [(EAGLView *)self.view setFramebuffer];

    unsigned long time=nya_system::get_time();
    unsigned int dt=(unsigned int)(time-m_time);
    m_time=time;

    nya_system::app_responder *responder=shared_app::get_app().get_responder();
    if(responder)
    {
        responder->on_process(dt);
        responder->on_draw();
    }

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
    if ((self = [super initWithFrame:frame])) 
    {   
        m_scale=1.0f;
        SEL scaleSelector = NSSelectorFromString(@"scale");
        SEL setContentScaleSelector = NSSelectorFromString(@"setContentScaleFactor:");
        SEL getContentScaleSelector = NSSelectorFromString(@"contentScaleFactor");
        if ([self respondsToSelector: getContentScaleSelector] && [self respondsToSelector: setContentScaleSelector])
        {
            NSMethodSignature *scaleSignature = [UIScreen instanceMethodSignatureForSelector: scaleSelector];
            NSInvocation *scaleInvocation = [NSInvocation invocationWithMethodSignature: scaleSignature];
            [scaleInvocation setTarget: [UIScreen mainScreen]];
            [scaleInvocation setSelector: scaleSelector];
            [scaleInvocation invoke];

            NSInteger returnLength = [[scaleInvocation methodSignature] methodReturnLength];
            if (returnLength == sizeof(float))
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
}

- (void)setContext:(EAGLContext *)newContext
{
    if (context != newContext) {
        [self deleteFramebuffer];

        context = newContext;

        [EAGLContext setCurrentContext:nil];
    }
}

- (float)getScale
{
    return m_scale;
}

- (void)createFramebuffer
{
    if (context && !defaultFramebuffer) 
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

        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);
        //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, framebufferWidth, framebufferHeight);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
        //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
}

- (void)deleteFramebuffer
{
    if (context) 
    {
        [EAGLContext setCurrentContext:context];

        if (defaultFramebuffer) {
            glDeleteFramebuffers(1, &defaultFramebuffer);
            defaultFramebuffer = 0;
        }

        if (colorRenderbuffer) {
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            colorRenderbuffer = 0;
        }
    }
}

- (void)setFramebuffer
{
    if (context) 
    {
        [EAGLContext setCurrentContext:context];

        if (!defaultFramebuffer)
        {
            [self createFramebuffer];
            nya_system::app_responder *responder=shared_app::get_app().get_responder();
            if(responder)
                responder->on_resize(framebufferWidth,framebufferHeight);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        glViewport(0,0,framebufferWidth,framebufferHeight);
    }
}

- (BOOL)presentFramebuffer
{
    BOOL success = FALSE;

    if (context) 
    {
        [EAGLContext setCurrentContext:context];

        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);

        success = [context presentRenderbuffer:GL_RENDERBUFFER];
    }

    return success;
}

- (void)layoutSubviews
{
    // The framebuffer will be re-created at the beginning of the next setFramebuffer method call.
    [self deleteFramebuffer];
}

@end

#else

#include <Cocoa/Cocoa.h>

@interface shared_app_delegate : NSObject <NSApplicationDelegate>
{
    nya_system::app_responder *m_app;
    int m_antialiasing;
}

-(id)init_with_responder:(nya_system::app_responder*)responder antialiasing:(int)aa;

@end

namespace
{

class shared_app
{
public:
    void start_windowed(int x,int y,unsigned int w,unsigned int h,int antialiasing,nya_system::app_responder &app)
    {
        [[NSAutoreleasePool alloc] init];

        [NSApplication sharedApplication];

        NSRect viewRect=NSMakeRect(x,y,w,h);

        m_window=[[NSWindow alloc] initWithContentRect:viewRect styleMask:NSTitledWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];

        NSString *title_str=[NSString stringWithCString:m_title.c_str() encoding:NSUTF8StringEncoding];
        [m_window setTitle:title_str];
        [m_window setOpaque:YES];

        [[NSWindowController alloc] initWithWindow:m_window];

        shared_app_delegate *delegate=[[shared_app_delegate alloc] init_with_responder:&app antialiasing:antialiasing];

        [NSApp setDelegate:delegate];

        setup_menu();

        [m_window orderFrontRegardless];
        [NSApp run];
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
    shared_app(): m_window(0), m_title("Nya engine") {}

private:
    NSWindow *m_window;
    std::string m_title;
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

-(void)set_responder:(nya_system::app_responder*)responder
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

-(void)dealloc
{
    [m_animation_timer release];

    [super dealloc];
}

@end

@implementation shared_app_delegate

-(id)init_with_responder:(nya_system::app_responder*)responder  antialiasing:(int)aa
{
    self=[super init];
    if (self)
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
        NSOpenGLPFASampleBuffers,1,NSOpenGLPFASamples,m_antialiasing,
        0
    };

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
    [window setAcceptsMouseMovedEvents:YES];

    if([view openGLContext]==nil) 
        return;

    if(m_antialiasing)
        glEnable(GL_MULTISAMPLE_ARB);

    [view reshape];

    m_app->on_init_splash();
    m_app->on_splash(0);

    [[view openGLContext] flushBuffer];

    m_app->on_init();

    [view initTimer];
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
    //shared_app::get_app().update_splash(*this);
}

void app::finish()
{
    shared_app::get_app().finish(*this);
}

}
