//https://code.google.com/p/nya-engine/

#include "postprocess.h"
#include "formats/text_parser.h"
#include "formats/math_expr_parser.h"
#include "memory/invalid_object.h"
#include "scene.h"

namespace nya_scene
{

bool postprocess::load(const char *name)
{
    if(!scene_shared::load(name))
        return false;

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
                for(int j=0;j<(int)m_op_set_shader[idx].params_map.size();++j)
                {
                    const int param_idx=m_op_set_shader[idx].params_map[j];
                    if(param_idx<0)
                        continue;

                    const nya_math::vec4 &v=m_shader_params[param_idx].second;
                    m_op_set_shader[idx].sh.internal().set_uniform_value(j,v.x,v.y,v.z,v.w);
                }
                break;

            case type_set_target:
                nya_render::set_viewport(m_targets[idx].rect);
                m_targets[idx].fbo->bind();
                break;

            case type_set_texture:
                {
                    const size_t tex_idx=m_op_set_texture[idx].tex_idx;
                    textures_set.push_back(tex_idx);
                    m_textures[tex_idx].internal().set(m_op_set_texture[idx].layer);
                }
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
        m_textures[textures_set[i]].internal().unset();
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

    for(size_t i=0;i<m_conditions.size();++i)
    {
        if(m_conditions[i].first==condition)
        {
            m_conditions[i].second=value;
            update();
            return;
        }
    }

    m_conditions.resize(m_conditions.size()+1);
    m_conditions.back().first=condition;
    m_conditions.back().second=value;
    update();
}

bool postprocess::get_condition(const char *condition) const
{
    if(!condition)
        return false;

    for(size_t i=0;i<m_conditions.size();++i)
    {
        if(m_conditions[i].first==condition)
            return m_conditions[i].second;
    }

    return false;
}

void postprocess::set_variable(const char *name,float value)
{
    if(!name)
        return;

    for(size_t i=0;i<m_variables.size();++i)
    {
        if(m_variables[i].first==name)
        {
            m_variables[i].second=value;
            update();
            return;
        }
    }

    m_variables.resize(m_variables.size()+1);
    m_variables.back().first=name;
    m_variables.back().second=value;
    update();
}

float postprocess::get_variable(const char *name) const
{
    if(!name)
        return 0.0f;

    for(size_t i=0;i<m_conditions.size();++i)
    {
        if(m_conditions[i].first==name)
            return m_conditions[i].second;
    }
    
    return 0.0f;
}

void postprocess::set_shader_param(const char *name,const nya_math::vec4 &value)
{
    if(!name)
        return;

    for(int i=0;i<(int)m_shader_params.size();++i)
    {
        if(m_shader_params[i].first==name)
        {
            m_shader_params[i].second=value;
            update_shader_param(i);
            return;
        }
    }

    const int idx=(int)m_shader_params.size();
    m_shader_params.resize(idx+1);
    m_shader_params.back().first=name;
    m_shader_params.back().second=value;
    update_shader_param(idx);
    update();
}

void postprocess::update_shader_param(int param_idx)
{
    if(param_idx<0 || param_idx>=(int)m_shader_params.size())
        return;

    for(size_t i=0;i<m_op_set_shader.size();++i)
    {
        int idx=-1;
        for(int j=0;j<m_op_set_shader[i].sh.internal().get_uniforms_count();++j)
        {
            if(m_op_set_shader[i].sh.internal().get_uniform(j).name!=m_shader_params[param_idx].first)
                continue;

            idx=j;
            break;
        }

        if(idx<0)
            continue;

        m_op_set_shader[i].params_map.resize(m_op_set_shader[i].sh.internal().get_uniforms_count(),-1);
        m_op_set_shader[i].params_map[idx]=param_idx;
    }
}

const nya_math::vec4 &postprocess::get_shader_param(const char *name) const
{
    if(!name)
        return nya_memory::get_invalid_object<nya_math::vec4>();

    for(size_t i=0;i<m_shader_params.size();++i)
    {
        if(m_shader_params[i].first==name)
            return m_shader_params[i].second;
    }

    return nya_memory::get_invalid_object<nya_math::vec4>();
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
            ifs.back().first=ifs.back().second=get_condition(l.name.c_str());
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

            ifs.back().second=get_condition(l.name.c_str());
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
            p.set_var("screen_width",m_width);
            p.set_var("screen_height",m_height);
            for(size_t i=0;i<m_variables.size();++i)
                p.set_var(m_conditions[i].first.c_str(),m_conditions[i].second);

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

            op_set_texture &o=add_op(m_op,m_op_set_texture,type_set_texture);
            o.tex_idx=it->second;
            o.layer=0;
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

            if(m_op_set_shader.empty())
                continue;

            //ToDo: update texture slots
        }
        else
            log()<<"postprocess: unknown operation "<<l.type<<" in file "<<m_shared.get_name()<<"\n";
    }

    for(int i=0;i<(int)m_shader_params.size();++i)
        update_shader_param(i);
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
