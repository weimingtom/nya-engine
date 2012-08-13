//https://code.google.com/p/nya-engine/

#include "log/log.h"
#include "system/app.h"
#include "system/system.h"
#include "render/platform_specific_gl.h"
#include "resources/resources.h"
#include "resources/file_resources_provider.h"
#include "resources/composite_resources_provider.h"
#include "pl2_resources_provider.h"
#include "attributes.h"
#include "scene.h"
#include "ui.h"

void init_resource_system();

class npl2: public nya_system::app
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

	    glClearColor(0,0.3,0.4,1);
        
        //sleep(2);

        init_resource_system();

        get_scene().init();
	    m_ui.init();
	}

	void on_process(unsigned int dt)
	{
	    m_ui.process();
	}

	void on_draw()
	{
        glClear(GL_COLOR_BUFFER_BIT);

        get_scene().draw();
        m_ui.draw();
	}

    void on_mouse_move(int x,int y)
    {
        //nya_log::get_log()<<"mmove "<<x<<" "<<y<<"\n";

        if(m_mouse_drag.left)
        {
            get_scene().get_camera().add_rot(((x-m_mouse_drag.last_x)*180.0f)/200.0f,
                                             -((y-m_mouse_drag.last_y)*180.0f)/200.0f);
        }
        else if(m_mouse_drag.right)
        {
            get_scene().get_camera().add_pos((x-m_mouse_drag.last_x)/20.0f,(y-m_mouse_drag.last_y)/20.0f);
        }
        else
            m_ui.mouse_move(x,y);
        
        m_mouse_drag.last_x=x;
        m_mouse_drag.last_y=y;
    }
    
    void on_mouse_scroll(int dx,int dy)
    {
        //nya_log::get_log()<<"mmove "<<x<<" "<<y<<"\n";
        
        if(!m_ui.mouse_scroll(dx,dy))
            get_scene().get_camera().add_scale(dy*0.1f);
    }

    void on_mouse_button(nya_system::mouse_button button,bool pressed)
    {
        nya_log::get_log()<<"on_mouse_button "<<(int)button<<" "<<(int)pressed<<"\n";
        
        if(button==nya_system::mouse_left)
        {
            if(!m_ui.mouse_button(nya_ui::layout::left_button,pressed) || !pressed)
                m_mouse_drag.left=pressed;
        }
        else if(button==nya_system::mouse_right)
            m_mouse_drag.right=pressed;
    }

    void on_resize(unsigned int w,unsigned int h)
    {
        //nya_log::get_log()<<"on_resize "<<w<<" "<<h<<"\n";

        if(!w || !h)
            return;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(25,float(w)/h,5,1500);
        glTranslatef(0,0,-45);

        m_ui.resize(w,h);
    }

	void on_free() { nya_log::get_log()<<"on_free\n"; }

private:
    ui m_ui;
    
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

void init_resource_system()
{
    static bool init=false;
    if(init)
        return;
    
    init=true;

    const std::string path=std::string(nya_system::get_app_path())+"add-ons/";
    
    static nya_resources::composite_resources_provider cprov;
    cprov.set_ignore_case(true);
    
    static nya_resources::file_resources_provider fprov;
    fprov.set_folder(path.c_str());
    
    nya_resources::resource_info *arch_info=fprov.first_res_info();
    static nya_memory::pool<nya_resources::pl2_resources_provider,16> pl2_providers;
    while(arch_info)
    {
        std::string arch_name(arch_info->get_name());
        size_t pos=arch_name.find_last_of(".");
        if(pos==std::string::npos||arch_name.substr(pos)!=".pl2")
        {
            arch_info=arch_info->get_next();
            continue;
        }
        
        nya_resources::pl2_resources_provider *prov=pl2_providers.allocate();
        prov->open_archieve(arch_info->access());
        cprov.add_provider(prov);
        
        nya_resources::resource_data *attrib=prov->access_attribute();
        if(attrib)
        {
            get_attribute_manager().load(attrib);
            attrib->release();
        }
        
        //nya_resources::get_log()<<arch_name.c_str()<<"\n";
        arch_info=arch_info->get_next();
    }
    
    cprov.add_provider(&fprov);
    
    static nya_resources::file_resources_provider fprov2;
    fprov2.set_folder(nya_system::get_app_path(),false);
    
    cprov.add_provider(&fprov2);
    
    nya_resources::set_resources_provider(&cprov);
}

int main(int argc, char **argv)
{
    nya_log::get_log()<<"npl2 started from path "<<nya_system::get_app_path()<<"\n";

    npl2 app;
    app.start_windowed(100,100,640,480);

    return 0;
}
