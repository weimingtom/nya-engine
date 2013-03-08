//https://code.google.com/p/nya-engine/

#include "log/log.h"
#include "log/plain_file_log.h"
#include "system/app.h"
#include "system/system.h"
#include "render/platform_specific_gl.h"
#include "resources/resources.h"
#include "resources/file_resources_provider.h"
#include "resources/shared_animations.h"
#include "scene/mesh.h"
#include "scene/camera.h"

#include "stdio.h"

class viewer_camera
{
public:
    void add_rot(float dx,float dy);
    void add_pos(float dx,float dy,float dz);

    void set_aspect(float aspect);

private:
    void update();

public:
    viewer_camera(): m_rot_x(180.0f), m_rot_y(0), m_pos(0.0f,0.0f,20.0f) {}

private:
    float m_rot_x;
    float m_rot_y;

    nya_math::vec3 m_pos;
};

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
    if(m_pos.z < 0.0f)
        m_pos.z = 0.0f;

    update();
}

void viewer_camera::set_aspect(float aspect)
{
    nya_scene::get_camera().set_proj(70,aspect,0.5,5000.0);
    update();
}

void viewer_camera::update()
{
    nya_scene::get_camera().set_rot(m_rot_x,m_rot_y,0.0);

    nya_math::quat rot(nya_math::vec3(-m_rot_y*3.14f/180.0f,-m_rot_x*3.14f/180.0f,0.0f));
    nya_math::vec3 pos=rot.rotate(m_pos);

    nya_scene::get_camera().set_pos(pos.x,pos.y+10.0f,pos.z);
}

class scene
{
public:
    void init()
    {
        m_char.load("Project DIVA Ex Miku Hatsune/skin00_000.pmd");
        //m_char.load("Appearance Miku/Appearance Miku.pmx");
    }

    void process(unsigned int dt)
    {
    }

    void draw()
    {
        m_char.draw();
    }

private:
    nya_scene::mesh m_char;
};

class character_demo: public nya_system::app
{
private:
	void on_init_splash()
	{
	    nya_log::get_log()<<"on_init_splash\n";

	    glClearColor(0,0.6,0.7,1);
	}

	void on_splash(unsigned int dt)
	{
	    nya_log::get_log()<<"on_splash\n";

	    glClear(GL_COLOR_BUFFER_BIT);
    }

	void on_init()
	{
	    nya_log::get_log()<<"on_init\n";

	    glClearColor(0.2,0.4,0.5,0);
        glEnable(GL_DEPTH_TEST);

	    m_scene.init();
	}

	void on_process(unsigned int dt)
	{
        m_scene.process(dt);

	    static unsigned int fps_counter=0;
	    static unsigned int fps_update_timer=0;

	    ++fps_counter;

	    fps_update_timer+=dt;
	    if(fps_update_timer>1000)
	    {
            char name[255];
            sprintf(name,"character_demo %d fps",fps_counter);
            set_title(name);

            fps_update_timer%=1000;
            fps_counter=0;
	    }
	}

	void on_draw()
	{
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        m_scene.draw();
	}

    void on_mouse_move(int x,int y)
    {
        //nya_log::get_log()<<"mmove "<<x<<" "<<y<<"\n";

        if(m_mouse_drag.left)
        {
            m_camera.add_rot(((x-m_mouse_drag.last_x)*180.0f)/200.0f,
                                             -((y-m_mouse_drag.last_y)*180.0f)/200.0f);
        }
        else if(m_mouse_drag.right)
        {
            m_camera.add_pos((x-m_mouse_drag.last_x)/20.0f,(y-m_mouse_drag.last_y)/20.0f,0.0);
        }

        m_mouse_drag.last_x=x;
        m_mouse_drag.last_y=y;
    }

    void on_mouse_scroll(int dx,int dy)
    {
        m_camera.add_pos(0.0f,0.0f,dy/10.0f);
    }

    void on_mouse_button(nya_system::mouse_button button,bool pressed)
    {
        if(button==nya_system::mouse_left)
            m_mouse_drag.left=pressed;
        else if(button==nya_system::mouse_right)
            m_mouse_drag.right=pressed;
    }

    void on_resize(unsigned int w,unsigned int h)
    {
        nya_log::get_log()<<"on_resize "<<w<<" "<<h<<"\n";

        if(!w || !h)
            return;

        m_camera.set_aspect(float(w)/h);
    }

	void on_free() { nya_log::get_log()<<"on_free\n"; }

private:
    scene m_scene;
    viewer_camera m_camera;

    struct mouse_drag
    {
        bool left;
        bool right;
        int last_x;
        int last_y;

        mouse_drag():left(false),right(false)
                    ,last_x(0),last_y(0) {}
    } m_mouse_drag;
};

int main(int argc, char **argv)
{
    /*
    const std::string log_file_name=std::string(nya_system::get_app_path())+"log.txt";
    nya_log::plain_file_log log;
    log.open(log_file_name.c_str());

    nya_log::set_log(&log);
    */

    nya_log::get_log()<<"character_demo started from path "<<nya_system::get_app_path()<<"\n";

    nya_resources::file_resources_provider fprov;
    fprov.set_folder(nya_system::get_app_path());
    nya_resources::set_resources_provider(&fprov);

    character_demo app;
    app.set_title("Loading, please wait...");
    app.start_windowed(100,100,640,480,0);

//    log.close();

    return 0;
}
