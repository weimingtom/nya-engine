//https://code.google.com/p/nya-engine/

#import "PmdView.h"
#import "PmdDocument.h"

#include "load_vmd.h"
#include "load_xps.h"
#include "load_tdcg.h"

#include "scene/camera.h"
#include "system/system.h"
#include "render/platform_specific_gl.h"
#include "resources/file_resources_provider.h"
#include "resources/composite_resources_provider.h"

void bps16to8(void *data,int width,int height,int channels)
{
    const unsigned char *from=(unsigned char *)data+1;
    unsigned char *to=(unsigned char *)data;

    for(int i=0;i<width*height*channels;++i)
        to[i]=from[i+i];
}

void flip_vertical(unsigned char *data,int width,int height,int channels)
{
    const int line_size=width*channels;
    const int top=line_size*(height-1);
    const int half=line_size*height/2;

    unsigned char tmp[4];

    for(int offset=0;offset<half;offset+=line_size)
    {
        unsigned char *ha=data+offset;
        unsigned char *hb=data+top-offset;

        for(int w=0;w<line_size;w+=channels)
        {
            unsigned char *a=ha+w;
            unsigned char *b=hb+w;
            memcpy(tmp,a,channels);
            memcpy(a,b,channels);
            memcpy(b,tmp,channels);
        }
    }
}

bool load_texture(nya_scene::shared_texture &res,nya_scene::resource_data &texture_data,const char* name)
{
    if(!texture_data.get_size())
        return false;

    //NSImage lose alpha of 32bit bmp textures
    if(texture_data.get_size()>3 && memcmp(texture_data.get_data(),"BM6",3)==0)
    {
        nya_memory::memory_reader reader(texture_data.get_data(),texture_data.get_size());
        reader.seek(28);
        if(reader.read<unsigned int>()==32)
        {
            reader.seek(10);
            const unsigned int data_offset=reader.read<unsigned int>();
            reader.skip(4);
            int width=reader.read<int>();
            int height=reader.read<int>();
            reader.seek(data_offset);
            if(height<0)
            {
                height=-height;
                flip_vertical((unsigned char*)reader.get_data(),width,height,4);
            }

            res.tex.build_texture(reader.get_data(),width,height,nya_render::texture::color_bgra);
            return true;
        }
    }

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

    const unsigned int bpp=(unsigned int)[image bitsPerPixel];
    const unsigned int bps=(unsigned int)[image bitsPerSample];
    if(!bpp || !bps)
    {
        nya_log::log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    const unsigned int channels=bpp/bps;

    nya_render::texture::color_format format;
    switch (channels)
    {
        case 3: format=nya_render::texture::color_rgb; break;
        case 4: format=nya_render::texture::color_rgba; break;

        default: nya_log::log()<<"unable to load texture: unsupported format\n"; return false;
    }

    unsigned int width=(unsigned int)[image pixelsWide];
    unsigned int height=(unsigned int)[image pixelsHigh];

    unsigned char *image_data=[image bitmapData];

    if(bps==16)
        bps16to8(image_data,width,height,channels);

    flip_vertical(image_data,width,height,channels);

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
                NSOpenGLPFAStencilSize, 8,
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

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender { return NSDragOperationGeneric; }

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSArray *draggedFilenames=[[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    NSString *extension=[[[draggedFilenames objectAtIndex:0] pathExtension] lowercaseString];
    NSURL *url=[NSURL fileURLWithPath:[draggedFilenames objectAtIndex:0]];

    if([extension compare:@"vmd"]==NSOrderedSame || [extension compare:@"pose"]==NSOrderedSame)
    {
        [self loadAnim:[url path].UTF8String];
        return YES;
    }

    NSArray *texFileTypes = [NSArray arrayWithObjects:@"spa",@"sph",@"tga",@"bmp",@"png",@"jpg",@"jpeg",@"dds",nil];
    if([texFileTypes indexOfObject:extension]!=NSNotFound)
    {
        NSPoint pt = [self convertPoint:[sender draggingLocation] fromView:nil];
        m_mouse_old=[self convertPoint: pt fromView: nil];

        m_assigntexture_name.assign([url path].UTF8String);

        if([extension compare:@"spa"]==NSOrderedSame)
            m_pick_mode=pick_assignspa;
        else if([extension compare:@"sph"]==NSOrderedSame)
            m_pick_mode=pick_assignsph;
        else
            m_pick_mode=pick_assigntexture;

        [self setNeedsDisplay: YES];

        return YES;
    }

    return NO;
}

- (void) mouseDown: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    m_mouse_old=[self convertPoint: pt fromView: nil];

    const NSUInteger flags = [[NSApp currentEvent] modifierFlags];
    if(flags & NSCommandKeyMask && flags & NSAlternateKeyMask)
    {
        m_pick_mode=pick_showhide;
        [self setNeedsDisplay: YES];
    }
}

- (void) rightMouseDown: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    m_mouse_old=[self convertPoint: pt fromView: nil];

    const NSUInteger flags = [[NSApp currentEvent] modifierFlags];
    if(flags & NSCommandKeyMask && flags & NSAlternateKeyMask)
    {
        m_show_groups.clear();
        [self setNeedsDisplay: YES];
    }
}
/*
- (void) mouseMoved:(NSEvent *)theEvent
{
    NSPoint pt=[theEvent locationInWindow];
    m_mouse_old=[self convertPoint: pt fromView: nil];
}
*/
- (void) mouseDragged: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    pt=[self convertPoint: pt fromView: nil];

    m_camera.add_rot(pt.x-m_mouse_old.x,-(pt.y-m_mouse_old.y));

    if([[NSApp currentEvent] modifierFlags] & NSCommandKeyMask)
        m_light_dir=nya_math::vec3::normalize(nya_scene::get_camera().get_pos());

    m_mouse_old=pt;

    if(!m_mmd_mesh.get_anim().is_valid())
        [self setNeedsDisplay: YES];
}

- (void) rightMouseDragged: (NSEvent *) theEvent
{
    NSPoint pt=[theEvent locationInWindow];

    pt=[self convertPoint: pt fromView: nil];

    m_camera.add_pos((pt.x-m_mouse_old.x)/20.0f,(pt.y-m_mouse_old.y)/20.0f,0.0f);

    m_mouse_old=pt;

    if(!m_mmd_mesh.get_anim().is_valid())
        [self setNeedsDisplay: YES];
}

- (void) scrollWheel: (NSEvent*) event
{
    m_camera.add_pos(0.0f,0.0f,[event deltaY]);

    if(!m_mmd_mesh.get_anim().is_valid())
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
    nya_scene::animation::register_load_function(xps_loader::load_pose,false);
    anim.load(name.c_str());
    m_mmd_mesh.set_anim(anim);
    m_mesh.set_anim(anim);

    m_last_time=nya_system::get_time();
}

-(void)saveObj:(const std::string &)name
{
    const char *mesh_name=m_mmd_mesh.get_name();
    if(!mesh_name || !mesh_name[0])
        return;

    const nya_scene::shared_mesh *sh=m_mmd_mesh.internal().get_shared_data().operator->();
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

    const nya_render::skeleton &sk=m_mmd_mesh.get_skeleton();

    class obj_mesh
    {
    public:
        void add_vec(const char *id,const nya_math::vec3 &v)
        {
            char buf[512];
            sprintf(buf,"%s %f %f %f\n",id,v.x,v.y,v.z);
            m_data.append(buf);
        }

        void add_vec(const char *id,const nya_math::vec2 &v)
        {
            char buf[512];
            sprintf(buf,"%s %f %f\n",id,v.x,v.y);
            m_data.append(buf);
        }

        void add_face(int idx1,int idx2,int idx3)
        {
            ++idx1,++idx2,++idx3;
            char buf[512];
            sprintf(buf,"f %d/%d/%d %d/%d/%d %d/%d/%d\n",idx1,idx1,idx1,
                                                         idx2,idx2,idx2,
                                                         idx3,idx3,idx3);
            m_data.append(buf);
        }

        void add_group(const char *group_name)
        {
            char buf[512];
            sprintf(buf,"\ng %s\n",group_name);
            m_data.append(buf);
        }

        void add_group(const char *group_name,const char *mat_name)
        {
            char buf[512];
            sprintf(buf,"\ng %s\nusemtl %s\n",group_name,mat_name);
            m_data.append(buf);
        }

        void add_material(const char *mat_name,const char *tex_name)
        {
            m_materials_data.append("\nnewmtl ");
            m_materials_data.append(mat_name);
            m_materials_data.append("\nillum 2\n"
                          "Kd 0.800000 0.800000 0.800000\n"
                          "Ka 0.200000 0.200000 0.200000\n"
                          "Ks 0.000000 0.000000 0.000000\n"
                          "Ke 0.000000 0.000000 0.000000\n"
                          "Ns 0.000000\n");

            if(tex_name)
            {
                m_materials_data.append("map_Kd ");
                m_materials_data.append(tex_name);
                m_materials_data.append("\n");
            }
        }

        bool write_file(const char *filename)
        {
            FILE *o=fopen(filename,"wb");
            if(!o)
                return false;

            if(!m_materials_data.empty())
            {
                std::string name(filename);
                std::string path;
                size_t sep=name.find_last_of("\\/");
                if(sep!=std::string::npos)
                {
                    path=name.substr(0,sep+1);
                    name=name.substr(sep+1,name.size()-sep-1);
                }
                size_t dot=name.find_last_of(".");
                if(dot!=std::string::npos)
                    name=name.substr(0,dot);
                
                name+=".mtl";

                FILE *om=fopen((path+name).c_str(),"wb");
                if(!om)
                    return false;

                fwrite(&m_materials_data[0],1,m_materials_data.length(),om);
                fclose(om);

                char buf[512];
                sprintf(buf,"mtllib %s\n\n",name.c_str());

                fwrite(buf,1,strlen(buf),o);
            }

            if(!m_data.empty())
                fwrite(&m_data[0],1,m_data.length(),o);

            fclose(o);
            return true;
        }

    private:
        std::string m_data;
        std::string m_materials_data;
    } obj;

    std::string obj_file_text;

    const bool is_pmx=mesh_name[strlen(mesh_name)-1]=='x';
    if(is_pmx)
    {
        const pmx_loader::vert *verts=(const pmx_loader::vert *)vert_buf.get_data();
        for(int i=0;i<vcount;++i)
        {
            obj.add_vec("v",verts[i].pos);
            obj.add_vec("vn",verts[i].normal);
            obj.add_vec("vt",verts[i].tc);
        }
    }
    else
    {
        const pmd_loader::vert *verts=(const pmd_loader::vert *)vert_buf.get_data();
        for(int i=0;i<vcount;++i)
        {
            obj.add_vec("v",verts[i].pos);
            obj.add_vec("vn",verts[i].normal);
            obj.add_vec("vt",verts[i].tc);
        }
    }

    vert_buf.free();

    std::string tex_cut_path(mesh_name);
    size_t sep=tex_cut_path.find_last_of("\\/");
    if(sep!=std::string::npos)
        tex_cut_path=tex_cut_path.substr(0,sep+1);

    const unsigned short *inds=(const unsigned short *)inds_buf.get_data();
    for(int i=0;i<(int)sh->groups.size();++i)
    {
        const nya_scene::shared_mesh::group &g=sh->groups[i];
        if(g.name=="edge")
            continue;

        char group_name[255];
        sprintf(group_name,"mesh%0d",i);

        char mat_name[255];
        sprintf(mat_name,"Material%0d",i);

        nya_scene::texture_proxy t=sh->materials[g.material_idx].get_texture("diffuse");
        obj.add_material(mat_name,t.is_valid()?t->get_name()+tex_cut_path.length():0);

        obj.add_group(group_name,mat_name);
        for(int j=g.offset;j<g.offset+g.count;j+=3)
            obj.add_face(inds[j],inds[j+2],inds[j+1]);
    }

    inds_buf.free();

    obj.write_file(name.c_str());
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

    if(m_backface_cull!=doc->m_backface_cull)
    {
        m_backface_cull=doc->m_backface_cull;
        [self setNeedsDisplay:YES];
    }

    if(m_show_bones!=doc->m_show_bones)
    {
        m_show_bones=doc->m_show_bones;
        [self setNeedsDisplay:YES];
    }

    if(m_mmd_mesh.get_anim().is_valid() || m_mesh.get_anim().is_valid())
    {
        unsigned long time=nya_system::get_time();
        m_mmd_mesh.update(int(time-m_last_time));
        m_mesh.update(int(time-m_last_time));
        m_last_time=time;

        [self setNeedsDisplay:YES];
    }
}

- (void)draw
{
    PmdDocument *doc=[[[self window] windowController] document];
    if(!doc->m_model_name.empty())
    {
        static bool once=false;
        if(!once)
        {
            once=true;

            std::string res_path([[NSBundle mainBundle] resourcePath].UTF8String);

            nya_resources::composite_resources_provider *comp_res=new nya_resources::composite_resources_provider();
            comp_res->add_provider(new nya_resources::file_resources_provider());
            nya_resources::file_resources_provider *toon_res=new nya_resources::file_resources_provider();
            toon_res->set_folder((res_path+"/toon/").c_str());
            comp_res->add_provider(toon_res);
            nya_resources::file_resources_provider *sh_res=new nya_resources::file_resources_provider();
            sh_res->set_folder((res_path+"/shaders/").c_str());
            comp_res->add_provider(sh_res);
            nya_resources::file_resources_provider *mat_res=new nya_resources::file_resources_provider();
            mat_res->set_folder((res_path+"/materials/").c_str());
            comp_res->add_provider(mat_res);

            nya_resources::set_resources_provider(comp_res);

            nya_render::set_ignore_platform_restrictions(true);

            nya_scene::texture::register_load_function(nya_scene::texture::load_dds);
            nya_scene::texture::register_load_function(nya_scene::texture::load_tga);
            nya_scene::texture::register_load_function(load_texture);
            nya_scene::texture::set_load_dds_flip(true);
            nya_render::texture::set_default_aniso(4);

            nya_scene::mesh::register_load_function(pmx_loader::load,false);
            nya_scene::mesh::register_load_function(pmd_loader::load,false);
            nya_scene::mesh::register_load_function(xps_loader::load_mesh,false);
            nya_scene::mesh::register_load_function(xps_loader::load_mesh_ascii,false);
            nya_scene::mesh::register_load_function(tdcg_loader::load_hardsave,false);
        }

        nya_render::set_clear_color(1.0f,1.0f,1.0f,0.0f);
        //nya_render::set_clear_color(0.2f,0.4f,0.5f,0.0f);

        m_light_dir=nya_math::vec3(-1.0,1.0,1.0).normalize();

        nya_render::depth_test::enable(nya_render::depth_test::less);

        const bool is_mmd=nya_resources::check_extension(doc->m_model_name.c_str(),"pmd") ||
                          nya_resources::check_extension(doc->m_model_name.c_str(),"pmx");
        if(is_mmd)
            m_mmd_mesh.load(doc->m_model_name.c_str());
        else
        {
            m_is_xps=nya_resources::check_extension(doc->m_model_name.c_str(),"xps") ||
                     nya_resources::check_extension(doc->m_model_name.c_str(),"mesh") ||
                     nya_resources::check_extension(doc->m_model_name.c_str(),"ascii");

            m_mesh.load(doc->m_model_name.c_str());

            if(m_is_xps)
                m_mesh.set_scale(10.0);

            m_mesh.draw();
        }

        nya_render::apply_state(true);

        doc->m_model_name.clear();
        doc->m_mesh=&m_mmd_mesh;
        doc->m_view=self;
    }

    if(m_pick_mode!=pick_none)
    {
        glClearStencil(0);
        nya_render::clear(true,true);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);

        for(int i=0;i<m_mmd_mesh.get_groups_count();++i)
            glStencilFunc(GL_ALWAYS,i+1,-1), m_mmd_mesh.draw_group(i);

        for(int i=0;i<m_mesh.get_groups_count();++i)
            glStencilFunc(GL_ALWAYS,i+1,-1), m_mesh.draw_group(i);

        unsigned int g;
        glReadPixels(m_mouse_old.x,m_mouse_old.y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &g);
        glDisable(GL_STENCIL_TEST);

        //printf("group %d\n",g);

        const char *group_name=m_mmd_mesh.get_group_name(g-1);
        if(g>0 && (!group_name || strcmp(group_name,"edge")!=0))
        {
            --g;
            if(m_pick_mode==pick_showhide)
            {
                m_show_groups.resize(m_mmd_mesh.get_groups_count()+m_mesh.get_groups_count(),true);
                m_show_groups[g]=!m_show_groups[g];

                const nya_scene::shared_mesh *sh=m_mmd_mesh.internal().get_shared_data().operator->();
                if(sh)
                {
                    for(int i=g+1;i<int(sh->groups.size());++i)
                    {
                        if(sh->groups[i].offset==sh->groups[g].offset &&
                           sh->groups[i].count==sh->groups[g].count)
                            m_show_groups[i]=m_show_groups[g];
                    }
                }
            }
            else if(!m_assigntexture_name.empty())
            {
                auto &m=m_mmd_mesh.modify_material(g);
                nya_scene::texture t;
                if(t.load(m_assigntexture_name.c_str()))
                {
                    if(m_pick_mode==pick_assigntexture)
                        m.set_texture("diffuse",t);
                    else if(m_pick_mode==pick_assignspa)
                        m.set_texture("env add",t);
                    else if(m_pick_mode==pick_assignsph)
                        m.set_texture("end mult",t);
                }

                m_assigntexture_name.clear();
            }
        }

        m_pick_mode=pick_none;
    }

    nya_render::state_override so=nya_render::get_state_override();
    so.override_cull_face=m_backface_cull;
    so.cull_face=false;
    nya_render::set_state_override(so);
    xps_loader::set_light_dir(m_light_dir);

    nya_render::clear(true,true);

    if(!m_show_groups.empty())
    {
        for(int i=0;i<int(m_show_groups.size());++i) if(m_show_groups[i]) m_mmd_mesh.draw_group(i);

        if(m_is_xps)
        {
            for(int i=0;i<int(m_show_groups.size());++i) if(m_show_groups[i]) m_mesh.draw_group(i,"opaque");
            for(int i=0;i<int(m_show_groups.size());++i) if(m_show_groups[i]) m_mesh.draw_group(i,"transparent_clip");
            for(int i=0;i<int(m_show_groups.size());++i) if(m_show_groups[i]) m_mesh.draw_group(i,"transparent_blend");
        }
        else
            for(int i=0;i<int(m_show_groups.size());++i) if(m_show_groups[i]) m_mesh.draw_group(i);
    }
    else
    {
        if(m_is_xps)
        {
            m_mesh.draw("opaque");
            m_mesh.draw("transparent_clip");
            m_mesh.draw("transparent_blend");
        }
        else
            m_mesh.draw();

        m_mmd_mesh.draw();
    }

    if(m_show_bones)
    {
        nya_render::clear(false,true);
        m_dd.set_point_size(3.0f);
        m_dd.clear();
        m_dd.add_skeleton(m_mmd_mesh.get_skeleton(),nya_math::vec4(0.0,0.0,1.0,1.0));
        m_dd.add_skeleton(m_mesh.get_skeleton(),nya_math::vec4(0.0,0.0,1.0,1.0));
        m_dd.draw();
    }
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
    m_mmd_mesh.unload();

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

    const NSUInteger flags = [[NSApp currentEvent] modifierFlags];
    const float mult=flags & NSShiftKeyMask?10.0f:1.0f;

    m_mesh->set_morph(int(row),mult*m_morphs[row].value/100.0f,m_morphs[row].override);
    m_mesh->update(0);
    [m_view setNeedsDisplay: YES];
}

-(IBAction)resetAll:(id)sender
{
    const NSUInteger flags = [[NSApp currentEvent] modifierFlags];
    const float value=flags & NSShiftKeyMask?1.0f:0.0f;

    for(int i=0;i<int(m_morphs.size());++i)
    {
        m_morphs[i]=morph();
        m_mesh->set_morph(i,value,false);
    }

    m_mesh->update(0);
    [m_view setNeedsDisplay: YES];
    [m_table reloadData];
}

@end
