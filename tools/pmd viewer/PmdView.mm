//https://code.google.com/p/nya-engine/

#import "PmdView.h"
#import "PmdDocument.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

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
	gluPerspective(25,[self frame].size.width / [self frame].size.height,1,100);
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
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

	glEnable     ( GL_DEPTH_TEST );
	glDepthFunc(GL_LESS);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(m_rot_y,1.0f,0.0f,0.0f);    
    glRotatef(m_rot_x+180.0f,0.0f,1.0f,0.0f);
    glTranslatef(0,-11,0);
    
	glColor3f(1,1,1);
    m_vbo.bind();
    m_vbo.draw();
    m_vbo.unbind();
}

-(void) dealloc
{
    m_vbo.release();
    [super dealloc];
}

@end
