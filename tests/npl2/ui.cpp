//https://code.google.com/p/nya-engine/

#include "ui.h"

#include "ui/list.h"
#include "ui/button.h"

#include "attributes.h"
#include "scene.h"

void ui::init()
{
    m_text_shader.add_program(nya_render::shader::vertex,
                              "void main()"
                              "{"
                              "    gl_TexCoord[0]=gl_MultiTexCoord0;"
                              "    gl_TexCoord[1]=gl_Color;"
                              "    gl_Position=vec4(gl_Vertex.xyz,1.0);"
                              "}");
    
    m_text_shader.add_program(nya_render::shader::pixel,
                              "uniform sampler2D alpha_map;"
                              "void main(void)"											
                              "{"
                              "    float alpha=texture2D(alpha_map,gl_TexCoord[0].xy).r;"
                              "    gl_FragColor=vec4(gl_TexCoord[1].rgb,alpha);"
                              "}");
    
    m_text_shader.set_sampler("alpha_map",0);
    
    m_ui_shader.add_program(nya_render::shader::vertex,
                            "void main()"
                            "{"
                            "    gl_TexCoord[0]=gl_MultiTexCoord0;"
                            "    gl_TexCoord[1]=gl_Color;"
                            "    gl_Position=vec4(gl_Vertex.xyz,1.0);"
                            "}");
    
    m_ui_shader.add_program(nya_render::shader::pixel,
                            "uniform sampler2D base_map;"
                            "void main(void)"											
                            "{"
                            "    vec4 base=texture2D(base_map,gl_TexCoord[0].xy);"
                            "    gl_FragColor=gl_TexCoord[1]*base;"
                            "}");
    
    m_ui_shader.set_sampler("base_map",0);
    
    m_font_tex=nya_resources::get_shared_textures().access("font2.tga");
    m_ui_tex=nya_resources::get_shared_textures().access("ui.tga");
    
    const int btn_width=72;
    const int btn_height=22;
    const int offset=6;
    
    int xpos=offset;
    
    static nya_ui::button customize_btn;
    customize_btn.set_id("customize_btn");
    customize_btn.set_text("Customize");
    customize_btn.set_pos(xpos,offset);
    customize_btn.set_size(btn_width,btn_height);
    add_widget(customize_btn);
    
    xpos+=btn_width+offset;
    
    static nya_ui::button anim_btn;
    anim_btn.set_id("anim_btn");
    anim_btn.set_text("Anims");
    anim_btn.set_pos(xpos,offset);
    anim_btn.set_size(btn_width,btn_height);
    add_widget(anim_btn);
    
    xpos+=btn_width+offset;
    
    static nya_ui::button scenery_btn;
    scenery_btn.set_id("scenery_btn");
    scenery_btn.set_text("Scenery");
    scenery_btn.set_pos(xpos,offset);
    scenery_btn.set_size(btn_width,btn_height);
    add_widget(scenery_btn);
    
    static nya_ui::button options_btn;
    options_btn.set_id("options_btn");
    options_btn.set_text("Options");
    options_btn.set_align(false,true,false,true);
    options_btn.set_pos(get_width()-btn_width-offset,offset);
    options_btn.set_size(btn_width,btn_height);
    add_widget(options_btn);
    
    const int panel_width=228;
    const int panel_pos_y=2*offset+btn_height;
    
    unsigned int panel_height=get_height()-panel_pos_y-offset;
    
    m_customize_pnl.set_align(true,false,true,true);
    m_customize_pnl.set_pos(offset,panel_pos_y);
    m_customize_pnl.set_size(panel_width,panel_height);
    m_customize_pnl.set_visible(false);
    add_widget(m_customize_pnl);
    
    const uint buttons_pnl_h=(btn_height+offset)*5;
    
    m_customise_lst.set_align(true,false,true,true);
    m_customise_lst.set_id("customise_lst");
    m_customise_lst.set_pos(offset,offset+buttons_pnl_h);
    m_customise_lst.set_size(panel_width-offset*2,panel_height-offset*2-buttons_pnl_h);
    m_customize_pnl.add_widget(m_customise_lst);
    
    uint it=0;
    get_attribute_manager().reset_iterator();
    const char *atr=get_attribute_manager().iterate_next();
    if(atr)
    {
        m_customise_lst.add_element("none");
        
        get_attribute_manager().iterate_elements(atr);
        const char *elem=get_attribute_manager().iterate_next_element();
        while(elem)
        {
            m_customise_lst.add_element(elem);
            
            elem=get_attribute_manager().iterate_next_element();
        }
        
        m_customise_group.assign(atr);
        
    }
    
    const char *def_group=atr;
    
    while(atr && it<max_customize_btns)
    {
        if(strcmp(atr,"BG")==0)
        {
            atr=get_attribute_manager().iterate_next();
            continue;
        }
        
        m_customize_btns[it].id.assign(atr);
        nya_ui::button &btn=m_customize_btns[it].btn;
        btn.set_id(atr);
        if(strcmp(atr,"COORDINATE")==0)
            btn.set_text("SET");
        else
            btn.set_text(atr);
        
        uint x=it%3;
        uint y=it/3;
        
        uint cust_btn_width=btn_width-4;
        
        btn.set_pos(offset+(cust_btn_width+offset)*x,buttons_pnl_h+offset-(btn_height+offset)*(y+1));
        btn.set_size(cust_btn_width,btn_height);
        m_customize_pnl.add_widget(btn);
        
        atr=get_attribute_manager().iterate_next();
        ++it;
    }
    
    if(it<max_customize_btns-1)
    {
        m_customize_btns[it].id.assign("COS_UP");
        nya_ui::button &btn=m_customize_btns[it].btn;
        btn.set_id("COS_UP");
        btn.set_text("COS:UP");
        
        uint x=it%3;
        uint y=it/3;
        
        uint cust_btn_width=btn_width-4;
        
        btn.set_pos(offset+(cust_btn_width+offset)*x,buttons_pnl_h+offset-(btn_height+offset)*(y+1));
        btn.set_size(cust_btn_width,btn_height);
        m_customize_pnl.add_widget(btn);
        
        ++it;
    }
    
    if(it<max_customize_btns-1)
    {
        m_customize_btns[it].id.assign("COS_DN");
        nya_ui::button &btn=m_customize_btns[it].btn;
        btn.set_id("COS_DN");
        btn.set_text("COS:DN");
        
        uint x=it%3;
        uint y=it/3;
        
        uint cust_btn_width=btn_width-4;
        
        btn.set_pos(offset+(cust_btn_width+offset)*x,buttons_pnl_h+offset-(btn_height+offset)*(y+1));
        btn.set_size(cust_btn_width,btn_height);
        m_customize_pnl.add_widget(btn);
        
        ++it;
    }
    
    
    //if(m_imouto)
    //    m_customise_lst.select_element(m_imouto->get_attrib(def_group));
    
    m_anim_pnl.set_align(true,false,true,true);
    m_anim_pnl.set_pos(offset,panel_pos_y);
    m_anim_pnl.set_size(panel_width,panel_height);
    m_anim_pnl.set_visible(false);
    add_widget(m_anim_pnl);
    
    m_anim_lst.set_align(true,false,true,true);
    m_anim_lst.set_id("anim_lst");
    m_anim_lst.set_pos(offset,offset);
    m_anim_lst.set_size(panel_width-offset*2,panel_height-offset*2);
    m_anim_pnl.add_widget(m_anim_lst);
    
    m_scenery_pnl.set_align(true,false,true,true);
    m_scenery_pnl.set_pos(offset,panel_pos_y);
    m_scenery_pnl.set_size(panel_width,panel_height);
    m_scenery_pnl.set_visible(false);
    add_widget(m_scenery_pnl);
    
    m_scenery_lst.set_align(true,false,true,true);
    m_scenery_lst.set_id("scenery_lst");
    m_scenery_lst.set_pos(offset,offset);
    m_scenery_lst.set_size(panel_width-offset*2,panel_height-offset*2);
    m_scenery_pnl.add_widget(m_scenery_lst);
    
    m_scenery_lst.add_element("none");
    get_attribute_manager().iterate_elements("BG");
    while(const char *atr=get_attribute_manager().iterate_next_element())
        m_scenery_lst.add_element(atr);
}

void ui::draw()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    
    //m_ui_shader.bind();
    nya_ui::layer::draw();
    
    //m_ui_shader.unbind();
    /*
     if(!m_font_tex.is_valid())
     return;
     
     glColor4f(0,0,0,1);
     
     m_text_shader.bind();
     m_font_tex->bind();
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
     draw_text(100,100,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");
     
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
     glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
     draw_text(100,150,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890");
     
     m_font_tex->unbind();
     
     m_text_shader.unbind();
     */
    glDisable(GL_BLEND);
}

void ui::draw_text(uint x,uint y,const char *text
                   ,font_align aligh_hor,font_align aligh_vert)
{
    if(!m_font_tex.is_valid())
        return;
    
    glColor4f(0,0,0,1);
    
    m_text_shader.bind();
    m_font_tex->bind();
    
    nya_ui::layer::draw_text(x,y,text,aligh_hor,aligh_vert);
    
    m_font_tex->unbind();
    
    m_text_shader.unbind();
}

void ui::draw_rect(nya_ui::rect &r,rect_style &s)
{
    if(!m_ui_tex.is_valid())
        return;
    
    m_ui_shader.bind();
    m_ui_tex->bind();
    nya_ui::layer::draw_rect(r,s);
    m_ui_tex->unbind();
    m_ui_shader.unbind();
}

void ui::process_events(event &e)
{
    if(e.sender=="customise_lst")
    {
        if(e.type=="select_element")
        {
            nya_ui::list::event_data *data=dynamic_cast<nya_ui::list::event_data*>(e.data);
            if(!data)
                return;
            
            if(m_custom_mode==cos_up)
                get_scene().set_imouto_attr(m_customise_group.c_str(),data->element.c_str(),0);
            else if(m_custom_mode==cos_dn)
                get_scene().set_imouto_attr(m_customise_group.c_str(),data->element.c_str(),1);
            else
                get_scene().set_imouto_attr(m_customise_group.c_str(),data->element.c_str());
            
            return;
        }
        
        if(e.type=="mover_element")
        {
            //nya_log::get_log()<<"mover_element\n";
            nya_ui::list::event_data *data=dynamic_cast<nya_ui::list::event_data*>(e.data);
            if(!data)
                return;
            
            if(m_custom_mode==cos_up)
                get_scene().set_imouto_preview(m_customise_group.c_str(),data->element.c_str(),0);
            else if(m_custom_mode==cos_dn)
                get_scene().set_imouto_preview(m_customise_group.c_str(),data->element.c_str(),1);
            else
                get_scene().set_imouto_preview(m_customise_group.c_str(),data->element.c_str());
            
            return;
        }
        
        if(e.type=="mleft_elements")
            get_scene().finish_imouto_preview();
        
        return;
    }
    
    if(e.sender=="scenery_lst")
    {
        if(e.type=="select_element")
        {
            nya_ui::list::event_data *data=dynamic_cast<nya_ui::list::event_data*>(e.data);
            if(data)
                get_scene().set_bkg(data->element.c_str());
        }
    }
    
    if(e.type!="mouse_left_btn_down")
        return;
    
    //nya_log::get_log()<<e.type.c_str();
    
    if(e.sender=="customize_btn")
    {
        m_customize_pnl.set_visible(!m_customize_pnl.is_visible());
        m_anim_pnl.set_visible(false);
        m_scenery_pnl.set_visible(false);
        return;
    }
    
    if(e.sender=="anim_btn")
    {
        m_customize_pnl.set_visible(false);
        m_anim_pnl.set_visible(!m_anim_pnl.is_visible());
        m_scenery_pnl.set_visible(false);
        return;
    }
    
    if(e.sender=="scenery_btn")
    {
        m_customize_pnl.set_visible(false);
        m_anim_pnl.set_visible(false);
        m_scenery_pnl.set_visible(!m_scenery_pnl.is_visible());
        return;
    }
    
    for(int i=0;i<max_customize_btns;++i)
    {
        if(e.sender!=m_customize_btns[i].id)
            continue;
        
        m_custom_mode=none;
        
        m_customise_lst.remove_elements();
        if(e.sender!="BODY" && e.sender!="HAIR" && e.sender!="EYE" && e.sender!="COORDINATE")
            m_customise_lst.add_element("none");
        
        if(e.sender=="COS_UP")
            m_custom_mode=cos_up;
        
        if(e.sender=="COS_DN")
            m_custom_mode=cos_dn;
        
        m_customise_group=e.sender;
        
        if(m_custom_mode==cos_up || m_custom_mode==cos_dn)
            m_customise_group.assign("COSTUME");
        
        get_attribute_manager().iterate_elements(m_customise_group.c_str());
        
        const char *elem=get_attribute_manager().iterate_next_element();
        while(elem)
        {
            m_customise_lst.add_element(elem);
            
            elem=get_attribute_manager().iterate_next_element();
        }
        
        
        //ToDo:
        /*
         if(m_imouto)
         {
         if(m_custom_mode==cos_up)
         m_customise_lst.select_element(m_imouto->get_attrib(m_customise_group.c_str(),0));
         else if(m_custom_mode==cos_dn)
         m_customise_lst.select_element(m_imouto->get_attrib(m_customise_group.c_str(),1));
         else
         m_customise_lst.select_element(m_imouto->get_attrib(m_customise_group.c_str()));
         }
         */

        return;
    }
}

