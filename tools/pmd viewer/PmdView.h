//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>

#include "render/vbo.h"
#include "render/shader.h"

@interface PmdView : NSOpenGLView
{
    NSPoint m_mouse_old;
    float m_rot_x;
    float m_rot_y;
    
    nya_render::vbo m_vbo;
}

- (void)draw;

@end
