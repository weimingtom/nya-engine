//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>

#include "mmd_mesh.h"
#include "render/debug_draw.h"
#include "viewer_camera.h"

@interface PmdView : NSOpenGLView
{
    NSPoint m_mouse_old;
    bool m_backface_cull;
    bool m_show_bones;

    mmd_mesh m_mmd_mesh;
    nya_scene::mesh m_mesh;
    viewer_camera m_camera;
    nya_render::debug_draw m_dd;
    nya_math::vec3 m_light_dir;
    bool m_is_xps;

    NSTimer *m_animation_timer;
    unsigned long m_last_time;

    enum pick_mode
    {
        pick_none,
        pick_showhide,
        pick_assigntexture,
        pick_assignspa,
        pick_assignsph

    } m_pick_mode;

    std::string m_assigntexture_name;
    std::vector<bool> m_show_groups;
}

@end

@interface MorphsWindow : NSObject
{
    IBOutlet NSWindow *m_window;
    IBOutlet NSTableView *m_table;
    mmd_mesh *m_mesh;
    PmdView *m_view;

    struct morph
    {
        float value;
        bool override;

        morph(): value(0.0f),override(false) {}
    };

    std::vector<morph> m_morphs;
}

-(void)displayWindow:(mmd_mesh *)mesh view:(PmdView *)view;

@end
