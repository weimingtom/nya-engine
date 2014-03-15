//https://code.google.com/p/nya-engine/

#import "PmdView.h"
#import "PmdDocument.h"

#include "load_vmd.h"

#include "scene/camera.h"
#include "system/system.h"

void viewer_camera::add_rot(float dx,float dy)
{
    m_rot_x+=dx;
    m_rot_y+=dy;

    const float max_angle=360.0f;

    if(m_rot_x>max_angle)
        m_rot_x-=max_angle;

    if(m_rot_x< -max_angle)
        m_rot_x+=max_angle;

    if(m_rot_y>max_angle)
        m_rot_y-=max_angle;

    if(m_rot_y< -max_angle)
        m_rot_y+=max_angle;

    update();
}

void viewer_camera::add_pos(float dx,float dy,float dz)
{
    m_pos.x-=dx;
    m_pos.y-=dy;
    m_pos.z-=dz;
    if(m_pos.z<5.0f)
        m_pos.z=5.0f;

    if(m_pos.z>200.0f)
        m_pos.z=200.0f;

    update();
}

void viewer_camera::set_aspect(float aspect)
{
    nya_scene::camera_proxy cam=nya_scene::get_camera();
    if(cam.is_valid())
        cam->set_proj(55.0,aspect,1.0,300.0);

    update();
}

void viewer_camera::update()
{
    nya_scene::camera_proxy cam=nya_scene::get_camera();
    if(!cam.is_valid())
        return;

    cam->set_rot(m_rot_x,m_rot_y,0.0);

    nya_math::quat rot(-m_rot_y*3.14f/180.0f,-m_rot_x*3.14f/180.0f,0.0f);
    nya_math::vec3 pos=rot.rotate(m_pos);

    cam->set_pos(pos.x,pos.y+10.0f,pos.z);
}

void flip_vertical(unsigned char *data,int width,int height,int bpp)
{
    const int line_size=width*bpp;
    const int top=line_size*(height-1);
    const int half=line_size*height/2;

    unsigned char tmp[4];

    for(int offset=0;offset<half;offset+=line_size)
    {
        unsigned char *ha=data+offset;
        unsigned char *hb=data+top-offset;

        for(int w=0;w<line_size;w+=bpp)
        {
            unsigned char *a=ha+w;
            unsigned char *b=hb+w;
            memcpy(tmp,a,bpp);
            memcpy(a,b,bpp);
            memcpy(b,tmp,bpp);
        }
    }
}

bool load_texture(nya_scene::shared_texture &res,nya_scene::resource_data &texture_data,const char* name)
{
    if(!texture_data.get_size())
        return false;

    NSData *data=[NSData dataWithBytesNoCopy:texture_data.get_data() 
                                          length: texture_data.get_size() freeWhenDone:NO];
    if(data==nil)
    {
        nya_log::get_log()<<"unable to load texture: NSData error\n";
        return false;
    }

    NSBitmapImageRep *image=[NSBitmapImageRep imageRepWithData:data];
    if(image==nil)
    {
        nya_log::get_log()<<"unable to load texture: invalid file\n";
        return false;
    }

    unsigned int bpp=(unsigned int)[image bitsPerPixel];

    nya_render::texture::color_format format;

    if(bpp==24)
        format=nya_render::texture::color_rgb;
    else if(bpp==32)
        format=nya_render::texture::color_rgba;
    else
    {
        nya_log::get_log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    if([image bitsPerSample]!=8)
    {
        nya_log::get_log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    unsigned int width=(unsigned int)[image pixelsWide];
    unsigned int height=(unsigned int)[image pixelsHigh];
    unsigned char *image_data=[image bitmapData];

    flip_vertical(image_data,width,height,bpp/8);

    res.tex.build_texture(image_data,width,height,format);

    return true;
}

class shared_context
{
public:
    NSOpenGLContext *allocate()
    {
        if(!m_context)
        {
            NSOpenGLPixelFormatAttribute pixelFormatAttributes[] =
            {
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFADepthSize, 32,
                NSOpenGLPFASampleBuffers,1,NSOpenGLPFASamples,2,
                0
            };

            NSOpenGLPixelFormat *pixelFormat=[[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes] autorelease];

            m_context=[[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
        }

        ++m_ref_count;
        return m_context;
    }

    void free()
    {
        --m_ref_count;
        if(m_ref_count>0)
            return;

        if(m_context)
            [m_context release];

        m_context=0;
        m_ref_count=0;
    }

public:
    static shared_context &get()
    {
        static shared_context holder;
        return holder;
    }

    shared_context(): m_context(0),m_ref_count(0) {}

private:
    NSOpenGLContext *m_context;
    int m_ref_count;
};

@implementation PmdView

-(id)initWithCoder:(NSCoder *)aDecoder
{
    NSOpenGLContext* openGLContext=shared_context::get().allocate();

    self=[super initWithCoder:aDecoder];
    if(self)
    {
        [self setOpenGLContext:openGLContext];
        [openGLContext makeCurrentContext];

        m_animation_timer=[NSTimer timerWithTimeInterval:1.0f/30 target:self
                                            selector:@selector(animate:) userInfo:nil repeats:YES];
        [[NSRunLoop currentRunLoop] addTimer:m_animation_timer forMode:NSDefaultRunLoopMode];
        [[NSRunLoop currentRunLoop] addTimer:m_animation_timer forMode:NSEventTrackingRunLoopMode];
    }

    return self;
}

- (void) mouseDown: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    m_mouse_old=[self convertPoint: pt fromView: nil];
}

- (void) rightMouseDown: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    m_mouse_old=[self convertPoint: pt fromView: nil];
}

- (void) mouseDragged: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    pt=[self convertPoint: pt fromView: nil];

    m_camera.add_rot(pt.x-m_mouse_old.x,-(pt.y-m_mouse_old.y));

    m_mouse_old=pt;

    if(!m_mesh.get_anim().is_valid())
        [self setNeedsDisplay: YES];
}

- (void) rightMouseDragged: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    pt=[self convertPoint: pt fromView: nil];

    m_camera.add_pos((pt.x-m_mouse_old.x)/20.0f,(pt.y-m_mouse_old.y)/20.0f,0.0f);

    m_mouse_old=pt;

    if(!m_mesh.get_anim().is_valid())
        [self setNeedsDisplay: YES];
}

- (void) scrollWheel: (NSEvent*) event
{
    m_camera.add_pos(0.0f,0.0f,[event deltaY]);

    if(!m_mesh.get_anim().is_valid())
        [self setNeedsDisplay: YES];
}

-(void)reshape
{
    nya_render::set_viewport(0,0,[self frame].size.width,[self frame].size.height);
    m_camera.set_aspect([self frame].size.width / [self frame].size.height);

    if(!m_mesh.get_anim().is_valid())
        [self setNeedsDisplay: YES];
}

- (void)animate:(id)sender
{
    PmdDocument *doc=[[[self window] windowController] document];
    if(!doc)
    {
        [m_animation_timer invalidate];
        return;
    }

    if(!doc->m_animation_name.empty())
    {
        nya_scene::animation anim;
        nya_scene::animation::register_load_function(vmd_loader::load);
        anim.load(doc->m_animation_name.c_str());
        m_mesh.set_anim(anim);
        doc->m_animation_name.clear();

        m_last_time=nya_system::get_time();
    }

    if(m_mesh.get_anim().is_valid())
    {
        unsigned long time=nya_system::get_time();
        m_mesh.update(int(time-m_last_time));
        m_last_time=time;

        [self setNeedsDisplay:YES];
    }
}

//#include "render/debug_draw.h"

- (void)draw
{
    PmdDocument *doc=[[[self window] windowController] document];
    if(!doc->m_model_name.empty())
    {
        //nya_render::set_clear_color(0.2f,0.4f,0.5f,0.0f);
        nya_render::set_clear_color(1.0f,1.0f,1.0f,0.0f);
        nya_render::depth_test::enable(nya_render::depth_test::less);
        
        nya_scene::texture::register_load_function(load_texture);
        m_mesh.load(doc->m_model_name.c_str());
        nya_render::apply_state(true);
        
        doc->m_model_name.clear();
    }

    nya_render::clear(true,true);

    m_mesh.draw();

    //nya_render::clear(false,true);
    //static nya_render::debug_draw dd; dd.clear(); dd.set_point_size(3.0f);
    //dd.add_skeleton(m_mesh.get_skeleton(),nya_math::vec4(0.0,0.0,1.0,1.0));
    //dd.draw();
}

- (void)drawRect:(NSRect)rect
{
	[self draw];

    [[ self openGLContext ] flushBuffer];
}

-(void) dealloc
{
    m_mesh.unload();

    shared_context::get().free();

    [super dealloc];
}

@end
