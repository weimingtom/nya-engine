//https://code.google.com/p/nya-engine/

#import <Cocoa/Cocoa.h>

#include "mmd_mesh.h"

class viewer_camera
{
public:
    void add_rot(float dx,float dy);
    void add_pos(float dx,float dy,float dz);

    void set_aspect(float aspect);

private:
    void update();

public:
    viewer_camera(): m_rot_x(0.0f),m_rot_y(0.0f),m_pos(0.0f,0.0f,22.0f) {}

private:
    float m_rot_x;
    float m_rot_y;

    nya_math::vec3 m_pos;
};

@interface PmdView : NSOpenGLView
{
    NSPoint m_mouse_old;

    mmd_mesh m_mmd_mesh;
    nya_scene::mesh m_mesh;
    viewer_camera m_camera;

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
