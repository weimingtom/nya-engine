//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>

#include "render/vbo.h"
#include "render/texture.h"
#include "resources/shared_textures.h"
//#include "render/shader.h"

#include <vector>

@interface PmdView : NSOpenGLView
{
    NSPoint m_mouse_old;
    float m_rot_x;
    float m_rot_y;
    float m_scale;
    float m_pos_x;
    float m_pos_y;

    nya_render::vbo m_vbo;

    struct view_material
    {
        unsigned int ind_offset;
        unsigned int ind_count;

        float color[4];

        nya_resources::texture_ref tex;
    };

    std::vector<view_material> m_materials;
}

- (void)draw;

@end
