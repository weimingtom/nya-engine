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
    nya_scene::get_camera().set_proj(55.0,aspect,1.0,2000.0);
    update();
}

void viewer_camera::update()
{
    nya_scene::get_camera().set_rot(m_rot_x,m_rot_y,0.0);

    nya_math::quat rot(-m_rot_y*3.14f/180.0f,-m_rot_x*3.14f/180.0f,0.0f);
    nya_math::vec3 pos=rot.rotate(m_pos);

    nya_scene::get_camera().set_pos(pos.x,pos.y+10.0f,pos.z);
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
        nya_log::log()<<"unable to load texture: NSData error\n";
        return false;
    }

    NSBitmapImageRep *image=[NSBitmapImageRep imageRepWithData:data];
    if(image==nil)
    {
        nya_log::log()<<"unable to load texture: invalid file\n";
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
        nya_log::log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    if([image bitsPerSample]!=8)
    {
        nya_log::log()<<"unable to load texture: unsupported format\n";
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
                NSOpenGLPFAAccelerated,
                NSOpenGLPFADoubleBuffer,
                NSOpenGLPFADepthSize, 32,
                NSOpenGLPFASampleBuffers,1,NSOpenGLPFASamples,2,
                //NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
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

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    int vsync=true;
    [[self openGLContext] setValues:&vsync forParameter:NSOpenGLCPSwapInterval];
    [[self openGLContext] makeCurrentContext];
}

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

        [self registerForDraggedTypes:[NSArray arrayWithObjects:NSFilenamesPboardType,nil]];
    }

    return self;
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender { return NSDragOperationGeneric; }

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender
{
    NSArray *draggedFilenames = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    if ([[[draggedFilenames objectAtIndex:0] pathExtension] caseInsensitiveCompare:@"vmd"]==NSOrderedSame)
    {
        NSURL *url = [NSURL fileURLWithPath:[draggedFilenames objectAtIndex:0]];
        [self loadAnim: [url path].UTF8String];
        return YES;
    }

    return NO;
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

    [[self openGLContext] update];
    nya_render::apply_state(true);
    [self setNeedsDisplay: YES];
}

-(void)loadAnim:(const std::string &)name
{
    nya_scene::animation anim;
    nya_scene::animation::register_load_function(vmd_loader::load,true);
    anim.load(name.c_str());
    m_mesh.set_anim(anim);

    m_last_time=nya_system::get_time();
}

-(void)saveObj:(const std::string &)name
{
    const char *mesh_name=m_mesh.get_name();
    if(!mesh_name || !mesh_name[0])
        return;

    const nya_scene::shared_mesh *sh=m_mesh.internal().get_shared_data().operator->();
    if(!sh)
        return;

    const nya_render::vbo &vbo=sh->vbo;
    nya_memory::tmp_buffer_ref vert_buf;
    if(!vbo.get_vertex_data(vert_buf))
        return;

    nya_memory::tmp_buffer_ref inds_buf;
    if(!vbo.get_index_data(inds_buf))
        return;

    const int vcount=vbo.get_verts_count();
    const int icount=vbo.get_indices_count();

    const unsigned short *inds=(const unsigned short *)inds_buf.get_data();

    const nya_render::skeleton &sk=m_mesh.get_skeleton();

    const bool is_pmx=mesh_name[strlen(mesh_name)-1]=='x';
    if(is_pmx)
    {
        //ToDo
    }
    else
    {
        /*
         nya_math::vec3 pos;
         nya_math::vec3 pos2;
         float normal[3];
         float tc[2];
         float bone_idx[2];
         float bone_weight;
        */

        const pmd_loader::vert *verts=(const pmd_loader::vert *)vert_buf.get_data();
        for(int i=0;i<vcount;++i)
        {
        }
    }

    vert_buf.free();
    inds_buf.free();


    //ToDo

    printf("saveObj: %s %s\n",name.c_str(),mesh_name);
}

- (void)animate:(id)sender
{
    PmdDocument *doc=[[[self window] windowController] document];
    if(!doc)
    {
        [m_animation_timer invalidate];
        return;
    }

    if(!doc->m_export_obj_name.empty())
    {
        [self saveObj: doc->m_export_obj_name];
        doc->m_export_obj_name.clear();
    }

    if(!doc->m_animation_name.empty())
    {
        [self loadAnim: doc->m_animation_name];
        doc->m_animation_name.clear();
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
        nya_scene::texture::register_load_function(nya_scene::texture::load_dds);
        m_mesh.load(doc->m_model_name.c_str());
        nya_render::apply_state(true);

        doc->m_model_name.clear();
        doc->m_mesh=&m_mesh;
        doc->m_view=self;
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
    [[self openGLContext] makeCurrentContext];

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

@implementation MorphsWindow

-(id)init
{
    if(self = [super init])
    {
        [NSBundle loadNibNamed:@"MorphsConfig" owner:self];
    }

    return self;
}

-(void) dealloc
{
    [m_window close];
    [super dealloc];
}

-(void)displayWindow:(mmd_mesh *)mesh view:(PmdView *)view
{
    m_mesh=mesh;
    m_view=view;
    if(![m_window isVisible])
    {
        [m_window setIsVisible:YES];
        [m_window orderFront:nil];
    }

    if(!m_mesh)
        return;

    m_morphs.resize(m_mesh->get_morphs_count());
    [self resetAll:self];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)pTableViewObj
{
    if(!m_mesh)
        return 0;

    return m_mesh->get_morphs_count();
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(int)row
{
    if(!m_mesh)
        return nil;

    const char *name=m_mesh->get_morph_name(int(row));
    if(!name)
        return nil;

    if([[tableColumn identifier] isEqualToString:@"name"])
        return [NSString stringWithUTF8String:name];

    if([[tableColumn identifier] isEqualToString:@"value"])
        return [NSNumber numberWithFloat:m_morphs[row].value];

    return [NSNumber numberWithBool:m_morphs[row].override];
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)value forTableColumn:(NSTableColumn *)column row:(NSInteger)row
{
    if(!m_mesh)
        return;

    if([[column identifier] isEqualToString:@"value"])
        m_morphs[row].value=[value floatValue];

    if([[column identifier] isEqualToString:@"override"])
        m_morphs[row].override=[value boolValue];

    m_mesh->set_morph(int(row),m_morphs[row].value/100.0f,m_morphs[row].override);
    m_mesh->update(0);
    [m_view setNeedsDisplay: YES];
}

-(IBAction)resetAll:(id)sender
{
    for(int i=0;i<int(m_morphs.size());++i)
    {
        m_morphs[i]=morph();
        m_mesh->set_morph(i,0.0f,false);
    }

    m_mesh->update(0);
    [m_view setNeedsDisplay: YES];
    [m_table reloadData];
}

@end
