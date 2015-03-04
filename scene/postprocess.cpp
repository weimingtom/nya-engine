//https://code.google.com/p/nya-engine/

#include "postprocess.h"
#include "formats/text_parser.h"
#include "formats/math_expr_parser.h"
#include "scene.h"

namespace nya_scene
{

bool postprocess::load(const char *name)
{
    if(!scene_shared::load(name))
        return false;

    for(int i=0;i<(int)m_shared->lines.size();++i)
    {
        const shared_postprocess::line &l=m_shared->lines[i];
        if(l.type=="if" || l.type=="elif" || l.type=="else_if")
            m_conditions[l.name]=false;
    }

    m_quad=nya_memory::shared_ptr<nya_render::screen_quad>(nya_render::screen_quad());
    m_quad->init();

    update();

    return true;
}

void postprocess::resize(unsigned int width,unsigned int height)
{
    m_width=width,m_height=height;
    update();
}

void postprocess::draw(int dt)
{
    nya_render::rect prev_rect=nya_render::get_viewport();

    nya_render::state state;
    std::vector<size_t> textures_set;
    for(size_t i=0;i<m_op.size();++i)
    {
        const size_t idx=m_op[i].idx;
        switch(m_op[i].type)
        {
            case type_set_shader:
                m_op_set_shader[idx].sh.internal().set();
                break;

            case type_set_target:
                nya_render::set_viewport(m_targets[idx].rect);
                m_targets[idx].fbo->bind();
                break;

            case type_set_texture:
                textures_set.push_back(idx);
                m_textures[idx].internal().set(); //ToDo: layer
                break;

            case type_clear:
                nya_render::clear(true,true); //ToDo: clear separately
                break;

            case type_draw_scene:
                draw_scene(m_op_draw_scene[idx].pass.c_str(),m_op_draw_scene[idx].tags.c_str());
                break;

            case type_draw_quad:
                nya_render::set_state(state);
                m_quad->draw();
                break;
        }
    }

    nya_render::set_viewport(prev_rect);
    nya_scene::shader_internal::unset();
    nya_render::fbo::unbind();
    for(size_t i=0;i<textures_set.size();++i)
        m_textures[i].internal().unset();
}

bool postprocess::load_text(shared_postprocess &res,resource_data &data,const char* name)
{
    nya_formats::text_parser parser;
    if(!parser.load_from_data((char *)data.get_data()))
        return false;

    res.lines.resize(parser.get_sections_count());
    for(int i=0;i<parser.get_sections_count();++i)
    {
        shared_postprocess::line &l=res.lines[i];
        l.type.assign(parser.get_section_type(i)+1);
        const char *name=parser.get_section_name(i);
        l.name.assign(name?name:"");

        l.values.resize(parser.get_subsections_count(i));
        for(int j=0;j<parser.get_subsections_count(i);++j)
        {
            l.values[j].first=parser.get_subsection_type(i,j);
            const char *value=parser.get_subsection_value(i,j);
            l.values[j].second=value?value:"";
        }
    }

    return true;
}

void postprocess::set_condition(const char *condition,bool value)
{
    if(!condition)
        return;

    conditions_map::iterator it=m_conditions.find(condition);
    if(it==m_conditions.end())
        return;

    if(it->second==value)
        return;

    it->second=value;
    update();
}

bool postprocess::get_condition(const char *condition) const
{
    if(!condition)
        return false;

    conditions_map::const_iterator it=m_conditions.find(condition);
    if(it==m_conditions.end())
        return false;

    return it->second;
}

template <typename op_t,typename t,typename op_e> t &add_op(op_t &ops,std::vector<t> &spec_ops,op_e type)
{
    ops.resize(ops.size()+1);
    ops.back().type=type;
    ops.back().idx=spec_ops.size();
    spec_ops.resize(spec_ops.size()+1);
    return spec_ops.back();
}

void postprocess::update()
{
    clear_ops();

    if(!m_shared.is_valid())
        return;

    std::vector<std::pair<bool,bool> > ifs;
    m_targets.resize(1);
    m_targets.back().rect.width=m_width,m_targets.back().rect.height=m_height;
    m_targets.back().fbo=nya_memory::shared_ptr<nya_render::fbo>(nya_render::fbo());
    typedef std::map<std::string,size_t> targets_map;
    targets_map targets;

    for(int i=0;i<(int)m_shared->lines.size();++i)
    {
        const shared_postprocess::line &l=m_shared->lines[i];

        if(l.type=="if")
        {
            ifs.resize(ifs.size()+1);
            ifs.back().first=ifs.back().second=m_conditions[l.name];
            continue;
        }
        else if(l.type=="else")
        {
            if(ifs.empty())
            {
                log()<<"postprocess: syntax error - else without if in file "<<m_shared.get_name()<<"\n";
                return;
            }

            ifs.back().second=!ifs.back().first;
            continue;
        }
        else if(l.type=="elif" || l.type=="else_if")
        {
            if(ifs.empty())
            {
                log()<<"postprocess: syntax error - else without if in file "<<m_shared.get_name()<<"\n";
                return;
            }

            if(ifs.back().first)
            {
                ifs.back().second=false;
                continue;
            }

            ifs.back().second=m_conditions[l.name];
            if(ifs.back().second)
                ifs.back().first=true;
            continue;
        }
        else if(l.type=="end")
        {
            if(ifs.empty())
            {
                log()<<"postprocess: syntax error - end without if in file "<<m_shared.get_name()<<"\n";
                return;
            }
            else
                ifs.pop_back();
            continue;
        }

        bool should_contiue=false;
        for(int j=0;j<(int)ifs.size();++j)
        {
            if(!ifs[j].second)
            {
                should_contiue=true;
                break;
            }
        }

        if(should_contiue)
            continue;

        if(l.type=="target")
        {
            const char *color=l.get_value("color"),*depth=l.get_value("depth");
            const char *width=l.get_value("width"),*height=l.get_value("height");

            nya_formats::math_expr_parser p;
            p.set_var("width",m_width);
            p.set_var("height",m_height);
            const unsigned int w=p.parse(width)?(unsigned int)p.calculate():m_width;
            const unsigned int h=p.parse(height)?(unsigned int)p.calculate():m_height;

            if(targets.find(l.name)==targets.end())
            {
                targets[l.name]=m_targets.size();
                m_targets.resize(m_targets.size()+1);
                m_targets.back().rect.width=w,m_targets.back().rect.height=h;
                m_targets.back().fbo=nya_memory::shared_ptr<nya_render::fbo>(nya_render::fbo());
            }
            else
            {
                log()<<"postprocess: target "<<l.name<<" redifinition in file "<<m_shared.get_name()<<"\n";
                continue;
            }

            if(!w || !h)
                continue;

            if(color)
            {
                textures_map::iterator it=m_textures_map.find(color);
                if(it!=m_textures_map.end())
                {
                    const nya_scene::texture &t=m_textures[it->second];
                    if(t.get_width()!=w || t.get_height()!=h)
                        log()<<"postprocess: texture "<<color<<" with different size in file "<<m_shared.get_name()<<"\n";
                }

                m_textures_map[color]=m_textures.size();
                m_textures.resize(m_textures.size()+1);

                if(m_textures.back().build(0,w,h,nya_render::texture::color_rgba))
                    m_targets.back().fbo->set_color_target(m_textures.back().internal().get_shared_data()->tex);
            }

            if(depth)
            {
                textures_map::iterator it=m_textures_map.find(depth);
                if(it!=m_textures_map.end())
                {
                    const nya_scene::texture &t=m_textures[it->second];
                    if(t.get_width()!=w || t.get_height()!=h)
                        log()<<"postprocess: texture "<<depth<<" with different size in file "<<m_shared.get_name()<<"\n";
                }

                m_textures_map[depth]=m_textures.size();
                m_textures.resize(m_textures.size()+1);

                if(m_textures.back().build(0,w,h,nya_render::texture::depth16))
                    m_targets.back().fbo->set_depth_target(m_textures.back().internal().get_shared_data()->tex);
            }
        }
        else if(l.type=="set_shader")
        {
            op_set_shader &o=add_op(m_op,m_op_set_shader,type_set_shader);
            o.sh.load(l.name.c_str());
        }
        else if(l.type=="set_target")
        {
            m_op.resize(m_op.size()+1);
            m_op.back().type=type_set_target;
            targets_map::const_iterator it=targets.find(l.name);
            m_op.back().idx=it==targets.end()?0:it->second;
        }
        else if(l.type=="set_texture")
        {
            if(l.values.empty())
                continue;

            textures_map::const_iterator it=m_textures_map.find(l.values.front().first);
            if(it==m_textures_map.end())
                continue; //ToDo: create texture and function get_texture

            m_op.resize(m_op.size()+1);
            m_op.back().type=type_set_texture;
            m_op.back().idx=it->second;
        }
        else if(l.type=="clear")
        {
            m_op.resize(m_op.size()+1);
            m_op.back().type=type_clear;
        }
        else if(l.type=="draw_scene")
        {
            op_draw_scene &o=add_op(m_op,m_op_draw_scene,type_draw_scene);
            o.pass=l.name;
            if(l.values.empty())
                continue;

            o.tags=l.values.front().first;
        }
        else if(l.type=="draw_quad")
        {
            m_op.resize(m_op.size()+1);
            m_op.back().type=type_draw_quad;
        }
        else
            log()<<"postprocess: unknown operation "<<l.type<<" in file "<<m_shared.get_name()<<"\n";
    }
}

void postprocess::unload()
{
    if(m_quad.get_ref_count()==1)
        m_quad->release();
    m_quad.free();
    scene_shared::unload();
    clear_ops();
}

void postprocess::clear_ops()
{
    m_op.clear();
    m_op_draw_scene.clear();
    m_op_set_shader.clear();
    m_textures_map.clear();
    m_textures.clear();

    for(size_t i=0;i<m_targets.size();++i)
    {
        if(m_targets[i].fbo.get_ref_count()==1)
            m_targets[i].fbo->release();
    }

    m_targets.clear();
}

const char *shared_postprocess::line::get_value(const char *name) const
{
    if(!name)
        return 0;

    for(size_t i=0;i<values.size();++i)
    {
        if(values[i].first==name)
            return values[i].second.c_str();
    }

    return 0;
}

}
