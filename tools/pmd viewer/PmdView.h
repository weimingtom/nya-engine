//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>

#include "render/vbo.h"
#include "render/texture.h"
#include "resources/shared_resources.h"
//#include "render/shader.h"

#include <vector>

typedef nya_resources::shared_resources<nya_render::texture,8> shared_textures;
typedef shared_textures::shared_resource_ref texture_ref;

class shared_textures_manager: public shared_textures
{
    bool fill_resource(const char *name,nya_render::texture &res);
    bool release_resource(nya_render::texture &res);
};

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
        unsigned int face_offset;
        unsigned int face_count;

        float color[4];

        texture_ref tex;
    };

    std::vector<view_material> m_materials;
    
    shared_textures_manager textures_manager;
}

- (void)draw;

@end
