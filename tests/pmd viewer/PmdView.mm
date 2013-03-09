//https://code.google.com/p/nya-engine/

#import "PmdView.h"
#import "PmdDocument.h"

#include "scene/camera.h"
#include "memory/memory_reader.h"

void viewer_camera::add_rot(float dx,float dy)
{
    m_rot_x+=dx;
    m_rot_y+=dy;
    
    const float max_angle=360.0f;
    
    if ( m_rot_x > max_angle )
        m_rot_x -= max_angle;
    
    if ( m_rot_x < -max_angle )
        m_rot_x += max_angle;
    
    if ( m_rot_y > max_angle )
        m_rot_y -= max_angle;
    
    if ( m_rot_y < -max_angle )
        m_rot_y += max_angle;
    
    update();
}

void viewer_camera::add_pos(float dx,float dy,float dz)
{
    m_pos.x-=dx;
    m_pos.y-=dy;
    m_pos.z-=dz;
    if(m_pos.z < 5.0f)
        m_pos.z = 5.0f;

    if(m_pos.z > 100.0f)
        m_pos.z = 100.0f;

    update();
}

void viewer_camera::set_aspect(float aspect)
{
    nya_scene::get_camera().set_proj(25.0,aspect,1.0,150.0);
    update();
}

void viewer_camera::update()
{
    nya_scene::get_camera().set_rot(m_rot_x,m_rot_y,0.0);
    
    nya_math::quat rot(nya_math::vec3(-m_rot_y*3.14f/180.0f,-m_rot_x*3.14f/180.0f,0.0f));
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

    NSData * data  = [NSData dataWithBytesNoCopy:texture_data.get_data() 
                                          length: texture_data.get_size() freeWhenDone:NO];
    if (data == nil)
    {
        nya_log::get_log()<<"unable to load texture: NSData error\n";
        return false;
    }

    NSBitmapImageRep *image=[NSBitmapImageRep imageRepWithData:data];
    if (image == nil)
    {
        nya_log::get_log()<<"unable to load texture: invalid file\n";
        return false;
    }

    unsigned int bpp=(unsigned int)[image bitsPerPixel];

    nya_render::texture::color_format format;

    if(bpp==24)
        format=nya_render::texture::color_rgb;
    else if(bpp==32 )
        format=nya_render::texture::color_rgba;
    else
    {
        nya_log::get_log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    unsigned int width =(unsigned int)[image pixelsWide];
    unsigned int height=(unsigned int)[image pixelsHigh];
    unsigned char *image_data=[image bitmapData];

    flip_vertical(image_data,width,height,bpp/8);

    res.tex.build_texture(image_data,width,height,format);

    return true;
}

class pmx_loader
{
    struct pmx_header
    {
        char text_encoding;
        char extended_uv;
        char index_size;
        char texture_idx_size;
        char material_idx_size;
        char bone_idx_size;
        char morph_idx_size;
        char rigidbody_idx_size;
    };

    struct vert
    {
        float pos[3];
        float normal[3];
        float tc[2];
        float bone_idx[4];
        float bone_weight[4];
    };

    struct pmx_material_params
    {
        float diffuse[4];
        float specular[3];
        float shininess;
        float ambient[3];
    };

    static int read_idx(nya_memory::memory_reader &reader,int size)
    {
        switch(size)
        {
            case 1: return reader.read<unsigned char>();
            case 2: return reader.read<unsigned short>();
            case 4: return reader.read<int>();
        }

        return 0;
    }

public:
    static bool load(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
    {
        if(!data.get_size())
            return false;
        
        nya_memory::memory_reader reader(data.get_data(),data.get_size());
        if(!reader.test("PMX ",4))
            return false;
        
        if(reader.read<float>()!=2.0f)
            return false;

        const char header_size=reader.read<char>();
        if(header_size!=sizeof(pmx_header))
            return false;

        const pmx_header header=reader.read<pmx_header>();
        if(header.extended_uv>0)
            return false; //ToDo
        
        for(int i=0;i<4;++i)
        {
            const int size=reader.read<int>();
            reader.skip(size);
        }
        
        const int vert_count=reader.read<int>();

        std::vector<vert> verts(vert_count);

        for(int i=0;i<vert_count;++i)
        {
            vert &v=verts[i];

            v.pos[0]=reader.read<float>();
            v.pos[1]=reader.read<float>();
            v.pos[2]=-reader.read<float>();

            v.normal[0]=reader.read<float>();
            v.normal[1]=reader.read<float>();
            v.normal[2]=-reader.read<float>();

            v.tc[0]=reader.read<float>();
            v.tc[1]=1.0-reader.read<float>();

            switch(reader.read<char>())
            {
                case 0:
                    v.bone_idx[0]=read_idx(reader,header.bone_idx_size);
                    v.bone_weight[0]=1.0f;
                    for(int j=1;j<4;++j)
                        v.bone_weight[j]=v.bone_idx[j]=0.0f;
                break;

                case 1:
                    v.bone_idx[0]=read_idx(reader,header.bone_idx_size);
                    v.bone_idx[1]=read_idx(reader,header.bone_idx_size);
                    
                    v.bone_weight[0]=reader.read<float>();
                    v.bone_weight[1]=1.0f-v.bone_weight[0];
                    
                    for(int j=2;j<4;++j)
                        v.bone_weight[j]=v.bone_idx[j]=0.0f;
                    break;

                case 2:
                    for(int j=0;j<4;++j)
                        v.bone_idx[j]=read_idx(reader,header.bone_idx_size);

                    for(int j=0;j<4;++j)
                        v.bone_weight[j]=reader.read<float>();
                break;

                case 3:
                    v.bone_idx[0]=read_idx(reader,header.bone_idx_size);
                    v.bone_idx[1]=read_idx(reader,header.bone_idx_size);
                    
                    v.bone_weight[0]=reader.read<float>();
                    v.bone_weight[1]=1.0f-v.bone_weight[0];
                    
                    for(int j=2;j<4;++j)
                        v.bone_weight[j]=v.bone_idx[j]=0.0f;

                    reader.skip(sizeof(float)*3*3);
                break;

                default: return false;
            }

            reader.read<float>(); //edge
        }

        res.vbo.set_vertex_data(&verts[0],sizeof(vert),vert_count);
        res.vbo.set_vertices(0,3);
        res.vbo.set_normals(sizeof(float)*3);
        res.vbo.set_tc(0,sizeof(float)*6,2);
        res.vbo.set_tc(1,sizeof(float)*8,4);
        res.vbo.set_tc(2,sizeof(float)*12,4);

        const int indices_count=reader.read<int>();
        if(header.index_size==2)
            res.vbo.set_index_data(reader.get_data(),nya_render::vbo::index2b,indices_count);
        else if(header.index_size==4)
            res.vbo.set_index_data(reader.get_data(),nya_render::vbo::index4b,indices_count);
        else
            return false;

        reader.skip(indices_count*header.index_size);

        const int textures_count=reader.read<int>();
        std::vector<std::string> tex_names(textures_count);
        for(int i=0;i<textures_count;++i)
        {
            const int str_len=reader.read<int>();
            if(header.text_encoding==0)
            {
                /*
                NSData* data = [[[NSData alloc] initWithBytes:reader.get_data()
                                                             length:str_len] autorelease];
                NSString* str = [[NSString alloc] initWithData:data
                                                         encoding:NSUTF16LittleEndianStringEncoding];
                tex_names[i].assign(str.UTF8String);
                */

                const char *data=(const char*)reader.get_data();
                tex_names[i].resize(str_len/2);
                for(int j=0;j<str_len/2;++j)
                    tex_names[i][j]=data[j*2];
            }
            else
                tex_names[i]=std::string((const char*)reader.get_data(),str_len);

            reader.skip(str_len);
        }

        const int mat_count=reader.read<int>();
        res.groups.resize(mat_count);

        nya_scene::shader sh;
        nya_scene::shared_shader sh_;
        sh_.shdr.set_sampler("base",0);
        sh_.samplers_count=1;
        sh_.samplers["diffuse"]=0;
        sh_.vertex="varying vec2 tc; void main() { tc=gl_MultiTexCoord0.xy; gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex; }";
        sh_.pixel="varying vec2 tc; uniform sampler2D base; void main() { vec4 tex=texture2D(base,tc.xy);"
                                                                         //"if(tex.a<0.1) discard;"
                                                                         "gl_FragColor=tex; }";
        sh_.shdr.add_program(nya_render::shader::vertex,sh_.vertex.c_str());
        sh_.shdr.add_program(nya_render::shader::pixel,sh_.pixel.c_str());
        sh.create(sh_);

        std::string path(name);
        size_t p=path.rfind("/");
        if(p==std::string::npos)
            p=path.rfind("\\");
        if(p==std::string::npos)
            path.clear();
        else
            path.resize(p+1);

        for(int i=0,offset=0;i<mat_count;++i)
        {
            nya_scene::shared_mesh::group &g=res.groups[i];
            for(int j=0;j<2;++j)
            {
                const int name_len=reader.read<int>();
                reader.skip(name_len);
            }

            reader.read<pmx_material_params>();
            reader.read<char>();//flag
            reader.skip(sizeof(float)*4);//edge color
            reader.read<float>();//edge size
            const int tex_idx=read_idx(reader,header.texture_idx_size);
            const int sph_tex_idx=read_idx(reader,header.texture_idx_size);
            reader.read<char>();//sphere mode
            const char toon_flag=reader.read<char>();

            int toon_tex_idx=-1;
            if(toon_flag==0)
                toon_tex_idx=read_idx(reader,header.texture_idx_size);
            else if(toon_flag==1)
                toon_tex_idx=reader.read<char>();
            else
                return false;

            const int comment_len=reader.read<int>();
            reader.skip(comment_len);
            g.offset=offset;
            g.count=reader.read<int>();
            offset+=g.count;

            if(tex_idx>=0 && tex_idx<(int)tex_names.size())
            {
                nya_scene::texture tex;
                tex.load((path+tex_names[tex_idx]).c_str());
                g.mat.set_texture("diffuse",tex);
            }

            g.mat.set_shader(sh);
            g.mat.set_blend(true,nya_render::blend::src_alpha,nya_render::blend::inv_src_alpha);
        }

        return true;
    }
};

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
            
            NSOpenGLPixelFormat *pixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes] autorelease];
            
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

    shared_context(): m_context(0), m_ref_count(0) {}

private:
    NSOpenGLContext *m_context;
    int m_ref_count;
};

@implementation PmdView

-(id)initWithCoder:(NSCoder *)aDecoder
{
    NSOpenGLContext* openGLContext = shared_context::get().allocate();

    self=[super initWithCoder:aDecoder];
    if(self)
    {
        [self setOpenGLContext:openGLContext];
        [openGLContext makeCurrentContext];
    }

    return self;
}

- (void) mouseDown: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    m_mouse_old = [self convertPoint: pt fromView: nil];
}

- (void) rightMouseDown: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    m_mouse_old = [self convertPoint: pt fromView: nil];
}

- (void) mouseDragged: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    pt = [self convertPoint: pt fromView: nil];

    m_camera.add_rot(pt.x-m_mouse_old.x,-(pt.y-m_mouse_old.y));

    m_mouse_old = pt;

    [self setNeedsDisplay: YES];
}

- (void) rightMouseDragged: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];

    pt = [self convertPoint: pt fromView: nil];
    
    m_camera.add_pos((pt.x-m_mouse_old.x)/20.0f,(pt.y-m_mouse_old.y)/20.0f,0.0f);

    m_mouse_old = pt;

    [self setNeedsDisplay: YES];
}

- (void) scrollWheel: (NSEvent*) event
{
    m_camera.add_pos(0.0f,0.0f,[event deltaY]);
    
    [self setNeedsDisplay: YES];
}

-(void)reshape
{
    glViewport( 0,0,[self frame].size.width,[self frame].size.height );     
    
    m_camera.set_aspect([self frame].size.width / [self frame].size.height);

    [self setNeedsDisplay: YES];
}

- (void)draw 
{
    PmdDocument *doc=[[[self window] windowController] document];
    if(!doc->m_model_name.empty())
    {
        glClearColor(0.2,0.4,0.5,0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        nya_scene::texture::register_load_function(load_texture);
        nya_scene::mesh::register_load_function(pmx_loader::load);
        m_mesh.load(doc->m_model_name.c_str());

        doc->m_model_name.clear();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_mesh.draw();
}

- (void)drawRect:(NSRect)rect 
{
	[self draw];

    [ [ self openGLContext ] flushBuffer ];    
}

-(void) dealloc
{
    m_mesh.unload();

    shared_context::get().free();

    [super dealloc];
}

@end
