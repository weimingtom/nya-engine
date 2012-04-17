//https://code.google.com/p/nya-engine/

#import "PmdView.h"
#import "PmdDocument.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

bool shared_textures_manager::fill_resource(const char *name,nya_render::texture &res)
{
    printf(" loading tex %s",name);
    
    NSString *tex_name = [NSString stringWithCString:name encoding:NSUTF8StringEncoding];

    NSData * data  = [NSData dataWithContentsOfFile: tex_name];
    if ( data == nil )
    {
        //[[[[self window] windowController] document] get_log ]
        //<< "unable to load texture: "<<fileName.UTF8String<<"\n";
        
        return false;
    }

    NSBitmapImageRep * image = [NSBitmapImageRep imageRepWithData: data];
    if ( image == nil )
    {
        //[[[[self window] windowController] document] get_log ]
        //<< "unable to load texture: "<<fileName.UTF8String<<"\n";
        
        return false;
    }

    unsigned int    bitsPerPixel = (unsigned int)[image bitsPerPixel];

    nya_render::texture::color_format format;

    if (bitsPerPixel==24)
        format=nya_render::texture::color_rgb;
    else if (bitsPerPixel== 32 )
        format=nya_render::texture::color_rgba;
    else
        return false;

    unsigned int width  = (unsigned int)[image pixelsWide];
    unsigned int height = (unsigned int)[image pixelsHigh];
    unsigned char * imageData = [image bitmapData];

    res.build_texture(imageData,width,height,format);

    return true;
}

bool shared_textures_manager::release_resource(nya_render::texture &res)
{
    printf("\nfreed");
    res.release();

    return true;
}

@implementation PmdView

- (void) mouseDown: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    m_mouse_old = [self convertPoint: pt fromView: nil];
}

- (void) mouseDragged: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    pt = [self convertPoint: pt fromView: nil];
    
    m_rot_y += ((m_mouse_old.y - pt.y) * 180.0f) / 200.0f;
    m_rot_x -= ((m_mouse_old.x - pt.x) * 180.0f) / 200.0f;
    
    if ( m_rot_x > 360 )
        m_rot_x -= 360;
    
    if ( m_rot_x < -360 )
        m_rot_x += 360;
    
    if ( m_rot_y > 360 )
        m_rot_y -= 360;
    
    if ( m_rot_y < -360 )
        m_rot_y += 360;
    
    m_mouse_old = pt;
    
    [self setNeedsDisplay: YES];
}

-(void)reshape
{
    glViewport( 0,0,[self frame].size.width,[self frame].size.height );     
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(25,[self frame].size.width / [self frame].size.height,5,100);
	glTranslatef(0,0,-55);
}

- (void)drawRect:(NSRect)rect 
{
	
	[self draw];

    [ [ self openGLContext ] flushBuffer ];    
}

- (void)draw 
{
    glClearColor(0.2,0.4,0.5,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    PmdDocument *doc=[[[self window] windowController] document];
    if(doc->m_changed)
    {
        if(doc->m_verts.empty())
            return;

        doc->m_changed=false;

        const uint vstride=sizeof(float)*8;
        m_vbo.gen_vertex_data(&doc->m_verts[0],vstride,(unsigned int)doc->m_verts.size()/8);
        m_vbo.set_normals(3*sizeof(float));
        m_vbo.set_tc(0,6*sizeof(float));

        if(doc->m_indices.empty())
            return;

        m_vbo.gen_index_data(&doc->m_indices[0],nya_render::vbo::triangles,nya_render::vbo::index2b,(unsigned int)doc->m_indices.size()/3);

        m_materials.resize(doc->m_materials.size());
        for(unsigned int i=0;i<doc->m_materials.size();++i)
        {
            material & from = doc->m_materials[i];
            view_material & to = m_materials[i];
            to.face_offset = from.face_offset;
            to.face_count = from.face_count;
            
            for(int k=0;k<3;++k)
                to.color[k]=from.diffuse[k]+from.ambient[k];
            to.color[3]=from.diffuse[3];

            if(from.tex_name&&from.tex_name[0])
            {
                for(int i=0;i<20;++i)
                    if(from.tex_name[i]=='*')
                        from.tex_name[i]=0;
                
                printf("\naccessing %s",from.tex_name);
                
                to.tex = textures_manager.access(from.tex_name);
            }
        }
    }

    //glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);

	glEnable     ( GL_DEPTH_TEST );
	glDepthFunc(GL_LESS);
    
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0,0,0,0.3f);        
    glLineWidth(1.0f);    

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(m_rot_y,1.0f,0.0f,0.0f);    
    glRotatef(m_rot_x+180.0f,0.0f,1.0f,0.0f);
    glTranslatef(0,-11,0);
    glEnable(GL_TEXTURE_2D);    

	glColor3f(1,1,1);
    m_vbo.bind();
    
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL,0.5f);
    glDisable(GL_CULL_FACE);

    for(unsigned int i=0;i<m_materials.size();++i)
    {
        view_material & mat = m_materials[i];
        //mat.tex.bind();
        if(mat.tex.is_valid())
            mat.tex->bind();

        glColor4fv(mat.color);
        m_vbo.draw(mat.face_offset,mat.face_count); 
        
        if(mat.tex.is_valid())
            mat.tex->unbind();        
    }
    glEnable(GL_CULL_FACE);

    glDisable(GL_ALPHA_TEST);

    glDisable(GL_TEXTURE_2D);
    //glDisable(GL_LIGHTING);

	glColor4f(0,0,0,0.3f);    

    glPolygonMode(GL_BACK,GL_LINE);
    glCullFace(GL_FRONT);
    glEnable(GL_BLEND);

    m_vbo.draw();

    glDisable(GL_BLEND);   
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

    m_vbo.unbind();

}

-(void) dealloc
{
    m_vbo.release();
    for(unsigned int i=0;i<m_materials.size();++i)
        m_materials[i].tex.free();

    [super dealloc];
}

@end
